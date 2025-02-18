#include "inode.h"
#include "ide.h"
#include "super_block.h"
#include "debug.h"
#include "fs.h"
#include "string.h"
#include "thread.h"
#include "interrupt.h"
#include "file.h"

// 用来存储inode位置
struct inode_position {
    bool two_sec;       // inode是否跨扇区
    uint32_t sec_lba;   // inode所在的扇区号
    uint32_t off_size;  // inode所在扇区内的字节偏移量
};

// 获取inode所在扇区和扇区内的偏移量，写入inode_pos中
static void inode_locate(struct partition* part, uint32_t inode_no,  \
struct inode_position* inode_pos) {
    ASSERT(inode_no < 4096);                                // MAX_FILES_PER_PART    
    uint32_t inode_table_lba = part->sb->inode_table_lba;   // inode节点表起始扇区
    uint32_t inode_size = sizeof(struct inode);             // inode结构大小
    uint32_t off_size = inode_size * inode_no;              // inode节点在inode节点表中的偏移字节
    uint32_t off_sec = off_size / 512;                      // inode节点在inode节点表中的偏移扇区
    uint32_t off_size_in_sec = off_size % 512;              // inode节点在当前扇区的偏移字节
    uint32_t left_in_sec = 512 - off_size_in_sec;           // inode节点距离当前扇区尾的字节大小
    if(left_in_sec < inode_size) {
        inode_pos->two_sec = true;                          // 当前扇区不可以容纳完整的inode节点
    }else {
        inode_pos->two_sec = false;                         // 当前扇区可以容纳完整的inode节点
    }
    inode_pos->sec_lba = inode_table_lba + off_sec;         // inode所在的扇区号
    inode_pos->off_size = off_size_in_sec;                  // inode所在扇区内的字节偏移量
}

// 将inode写入到硬盘的分区part
void inode_sync(struct partition* part, struct inode* inode, void* io_buf) {
    uint32_t inode_no = inode->i_no;
    struct inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);                       // 获取inode所在扇区和扇区内的偏移量，写入inode_pos中
    ASSERT(inode_pos.sec_lba <= part->start_lba + part->sec_cnt);   // 粗略判断一下获取的数据inode_pos是否正确
    struct inode pure_inode;                                        // 硬盘中inode不需要i_open_cnts和inode_tag
    memcpy(&pure_inode, inode, sizeof(struct inode));
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = false;                                  // 置为false，以保证在硬盘中读出时为可写
    pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;

    char* inode_buf = (char*)io_buf;                                // io_buf是用于硬盘io的缓冲区
    if(inode_pos.two_sec) {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);   // 跨扇区，故读写两个扇区
        memcpy(inode_buf + inode_pos.off_size, &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    }else {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        memcpy(inode_buf + inode_pos.off_size, &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

// 根据i结点号返回相应的i结点
struct inode* inode_open(struct partition* part, uint32_t inode_no) {
    struct list_elem* elem = part->open_inodes.head.next;           // open_inodes链表是为提速创建的缓冲区，被所有任务共享
    struct inode* inode_found;
    while(elem != &part->open_inodes.tail) {                        // 先在已打开的inode链表中找inode
        inode_found = elem2entry(struct inode, inode_tag, elem);    // 把链表中的inode_tag转换成inode
        if(inode_found->i_no == inode_no) {                         // 在已打开的inode链表中找到inode
            inode_found->i_open_cnts ++;
            return inode_found;
        }
        elem = elem->next;
    }

    struct inode_position inode_pos;                                // 由于open_inodes链表中找不到，下面从硬盘上读入此inode并加入此链表
    inode_locate(part, inode_no, &inode_pos);                       // 获取inode所在扇区和扇区内的偏移量，写入inode_pos中
    struct task_struct* cur = running_thread();                     // 获取当前线程，为接下来的申请为所有任务共享的内存区域做准备
    uint32_t* cur_pagedir_bak = cur->pgdir;                         // 备份当前线程的页表
    cur->pgdir = NULL;                                              // 为使通过sys_malloc创建的新inode被所有任务共享，需要将inode置于内核空间，故需要将其页表置为NULL
    inode_found = (struct inode*)sys_malloc(sizeof(struct inode));  // 在内核中申请空间存放inode
    cur->pgdir = cur_pagedir_bak;                                   // 恢复当前线程的页表
    char* inode_buf;
    if(inode_pos.two_sec) {                                         // 从硬盘读取inode
        inode_buf = (char*)sys_malloc(1024);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);   // 这里先用inode_buf暂存，后续转存到内核中
    }else {
        inode_buf = (char*)sys_malloc(512);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
    memcpy(inode_found, inode_buf + inode_pos.off_size, sizeof(struct inode));// 转存到内核
    list_push(&part->open_inodes, &inode_found->inode_tag);         // 因为一会很可能要用到此inode，将其加入open_inodes链表队首
    inode_found->i_open_cnts = 1;
    sys_free(inode_buf);
    return inode_found;
}

// 关闭inode或减少inode的打开数
void inode_close(struct inode* inode) {
    enum intr_status old_status = intr_disable();
    if(--inode->i_open_cnts == 0) {
        list_remove(&inode->inode_tag);
        struct task_struct* cur = running_thread();
        uint32_t* cur_pagedir_bak = cur->pgdir;
        cur->pgdir = NULL;
        sys_free(inode);
        cur->pgdir = cur_pagedir_bak;
    }
    intr_set_status(old_status);
}

// 将硬盘分区 part 上的 inode 清空
void inode_delete(struct partition* part, uint32_t inode_no, void* io_buf) {
    ASSERT(inode_no < 4096);
    struct inode_position inode_pos;
    // inode 位置信息会存入 inode_pos
    inode_locate(part, inode_no, &inode_pos); 
    ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));

    char* inode_buf = (char*)io_buf;
    if (inode_pos.two_sec) { // inode 跨扇区, 读入 2 个扇区
        // 将原硬盘上的内容先读出来
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
        // 将 inode_buf 清 0
        memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
        // 用清 0 的内存数据覆盖磁盘
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    } else { // 未跨扇区, 只读入 1 个扇区就好
        // 将原硬盘上的内容先读出来
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        // 将 inode_buf 清 0
        memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
        // 用清 0 的内存数据覆盖磁盘
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

// 回收 inode 的数据块和 inode 本身
void inode_release(struct partition* part, uint32_t inode_no) {
    struct inode* inode_to_del = inode_open(part, inode_no);
    ASSERT(inode_to_del->i_no == inode_no);

    // 1 回收 inode 占用的所有块
    uint8_t block_idx = 0, block_cnt = 12;
    uint32_t block_bitmap_idx;
    uint32_t all_blocks[140] = {0}; // 12 个直接块 + 128 个间接块

    // a 先将前 12 个直接块存入 all_blocks
    while (block_idx < 12) {
        all_blocks[block_idx] = inode_to_del->i_sectors[block_idx];
        block_idx++;
    }

    // b 如果一级间接块表存在, 将其 128 个间接块读到 all_blocks[12~], 并释放一级间接块表所占的扇区
    if (inode_to_del->i_sectors[12] != 0) {
        ide_read(part->my_disk, inode_to_del->i_sectors[12], all_blocks+12, 1);
        block_cnt = 140;

        // 回收一级间接块表占用的扇区
        block_bitmap_idx = inode_to_del->i_sectors[12] - part->sb->data_start_lba;
        ASSERT(block_bitmap_idx > 0);
        bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }

    // c inode 所有的块地址已经收集到 all_blocks 中, 下面逐个回收
    block_idx = 0;
    while (block_idx < block_cnt) {
        if (all_blocks[block_idx] != 0) {
            block_bitmap_idx = 0;
            block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
            ASSERT(block_bitmap_idx > 0);
            bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
        }
        block_idx++;
    }

    // 2 回收该 inode 所占用的 inode
    bitmap_set(&part->inode_bitmap, inode_no, 0);
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    /****** 以下inode_delete是调试用的 ******
     * 此函数会在inode_table中将此inode清0，
     * 但实际上的不需要的，inode分配是由inode位图控制的，
     * 硬盘上的数据不需要清0，可以直接覆盖 */
    void* io_buf = sys_malloc(1024);
    inode_delete(part, inode_no, io_buf);
    sys_free(io_buf);
    /***************************************/

    inode_close(inode_to_del);
}

// 初始化new_inode
void inode_init(uint32_t inode_no, struct inode* new_inode) {
    new_inode->i_no = inode_no;
    new_inode->i_size = 0;
    new_inode->i_open_cnts = 0;
    new_inode->write_deny = false;

    uint8_t sec_idx = 0;
    while(sec_idx < 13) {
        new_inode->i_sectors[sec_idx] = 0;
        sec_idx ++;
    }
}