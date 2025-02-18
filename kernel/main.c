#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "ioqueue.h"
#include "process.h"
#include "syscall.h"
#include "syscall-init.h"
#include "stdio.h"
#include "fs.h"
#include "dir.h"

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);

// 文件相关
// int main(void) {
//    put_str("I am kernel\n");
//    init_all();
//    intr_enable(); // 打开中断
//    // process_execute(u_prog_a, "u_prog_a");
//    // process_execute(u_prog_b, "u_prog_b");
//    // thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
//    // thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

//    uint32_t fd = sys_open("/file1",O_RDWR);
//    printf("open /file1 fd:%d\n",fd);

//    // sys_write(fd, "hello world\n",12);

//    // char buf[64] = { 0 };
//    // int read_bytes = sys_read(fd,buf,6);
//    // printf("1_ read %d byte:\n%s\n",read_bytes,buf);
   
//    // memset(buf,0,64);
//    // sys_lseek(fd,0,SEEK_SET);
//    // read_bytes = sys_read(fd,buf,12);
//    // printf("2_ read %d byte:\n%s\n",read_bytes,buf);

//    sys_close(fd);
//    printf("%d closed now \n",fd);

//    // printf("/file1 delete %s!\n",sys_unlink("/file1") == 0?"done":"fail");
//    while(1);
//    return 0;
// }

// 创建目录
// int main(void) {
//    put_str("I am kernel\n");
//    init_all();
//    // intr_enable(); // 打开中断
//    // process_execute(u_prog_a, "u_prog_a");
//    // process_execute(u_prog_b, "u_prog_b");
//    // thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
//    // thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

//    printf("/dir1/subdir1 create %s!\n",sys_mkdir("/dir1/subdir1") == 0 ? "done" : "fail"); 
//    printf("/dir1 create %s!\n", sys_mkdir("/dir1") == 0? "done" : "fail"); 
//    printf ("now, /dir1/subdir1 create %s ! \n", sys_mkdir("/dir1/subdir1") == 0 ? "done" : "fail"); 
//    int fd = sys_open ("/dir1/subdir1/file2", O_CREAT | O_RDWR); 
//    if (fd != -1) { 
//       printf("/dir1/subdir1/file2 create done!\n"); 
//       sys_write(fd, "Catch me if you can!\n", 21); 
//       sys_lseek (fd, 0, SEEK_SET); 
//       char buf [32] = {0}; 
//       sys_read(fd, buf, 21); 
//       printf ("/dir1'/subdir1/file2 says: \n%s", buf); 
//       sys_close (fd); 
//    }
//    while(1);
//    return 0;
// }

// 打开目录、关闭目录
// int main(void) {
//    put_str("I am kernel\n");
//    init_all();
//    intr_enable(); // 打开中断
//    // process_execute(u_prog_a, "u_prog_a");
//    // process_execute(u_prog_b, "u_prog_b");
//    // thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
//    // thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

//    struct dir* p_dir = sys_opendir("/dir1/subdir1");
//    if(p_dir) {
//       printf("/dir1/subdir1 open done!\n");
//       if(sys_closedir(p_dir) == 0) {
//          printf("/dir1/subdir1 close done!\n");
//       }else {
//          printf("/dir1/subdir1 close fail!\n");
//       }
//    }else {
//       printf("/dir1/subdir1 open fail!\n");
//    }
//    while(1);
//    return 0;
// }

// 读目录
// int main(void) {
//    put_str("I am kernel\n");
//    init_all();
//    intr_enable(); // 打开中断

//    struct dir* p_dir = sys_opendir("/dir1/subdir1"); 
//    if (p_dir) { 
//       printf("/dir1/subdir1 open done!\ncontent:\n"); 
//       char* type = NULL; 
//       struct dir_entry* dir_e = NULL; 
//       while ((dir_e = sys_readdir(p_dir))) { 
//          if (dir_e->f_type == FT_REGULAR) {
//            type = "regular";
//          }else{
//             type = "directory";
//          }
//          printf(" %s %s\n",type,dir_e->filename);
//       }
//       if (sys_closedir (p_dir) == 0) {
//          printf("/dir1/subdir1 close done!\n"); 
//       } else { 
//          printf("/dir1/subdir1 close fail!\n"); 
//       }
//    }else{   
//       printf("/dir1/subdir1 open fail!\n"); 
//    }
//    while(1);
//    return 0;
// }

// 删除目录
// int main(void) {
//    put_str("I am kernel\n");
//    init_all();
//    /**************************测试代码**************************/
//    printf("/dir1 content before delete /dir1/subdir1:\n");
//    struct dir* dir = sys_opendir("/dir1/"); 
//    char* type = NULL;
//    struct dir_entry* dir_e = NULL;
//    while((dir_e = sys_readdir(dir))) {
//       if(dir_e->f_type == FT_REGULAR) {
//          type = "regular";
//       }else {
//          type = "directory";
//       }
//       printf("    %s   %s\n", type, dir_e->filename);
//    }

//    printf("try to delete nonempty directory /dir1/subdir1\n");
//    if(sys_rmdir("/dir1/subdir1") == -1) {
//       printf("sys_rmdir: /dir1/subdir1 delete failed\n");
//    }

   
//    printf("try to delete nonempty directory /dir1/subdir1/file2\n");
//    if(sys_rmdir("/dir1/subdir1/file2") == -1) {
//       printf("sys_rmdir: /dir1/subdir1/file2 delete failed\n");
//    }
//    if(sys_unlink("/dir1/subdir1/file2") == 0) {
//       printf("sys_unlink: /dir1/subdir1/file2 delete done\n");
//    }
   
//    printf("try to delete nonempty directory /dir1/subdir1 again\n");
//    if(sys_rmdir("/dir1/subdir1") == 0) {
//       printf("sys_rmdir: /dir1/subdir1 delete done\n");
//    }

//    printf("/dir1 content after delete /dir1/subdir1:\n");
//    sys_rewinddir(dir);
//    while ((dir_e = sys_readdir(dir))) { 
//       if (dir_e->f_type == FT_REGULAR) {
//          type = "regular";
//       }else{
//          type = "directory";
//       }
//       printf(" %s %s\n",type,dir_e->filename);
//    }
   
//    // if(sys_rmdir("/dir1") == 0) {
//    //    printf("sys_rmdir: /dir1/subdir1 delete done\n");
//    // }
//    /**************************测试代码**************************/
//    while(1);
//    return 0;
// }

// 切换目录
// int main(void) {
//    put_str("I am kernel\n");
//    init_all();
//    intr_enable(); // 打开中断

//    char cwd_buf[32]={0};
//    sys_getcwd(cwd_buf, 32);
//    printf("cwd:%s\n",cwd_buf);
//    sys_chdir("/dir1");
//    printf("change cwd now\n");
//    sys_getcwd(cwd_buf, 32);
//    printf("cwd:%s\n",cwd_buf);
//    while(1);
//    return 0;
// }

int main(void) {
   put_str("I am kernel\n");
   init_all();
   intr_enable(); // 打开中断

   struct stat obj_stat;
   sys_stat("/", &obj_stat);
   printf("/`s info\n   i_no:%d\n   size:%d\n   filetype:%s\n", \
      obj_stat.st_ino, obj_stat.st_size, \
      obj_stat.st_filetype == 2 ? "directory" : "regular");
   sys_stat("/dir1", &obj_stat);
   printf("/dir1`s info\n   i_no:%d\n   size:%d\n   filetype:%s\n", \
      obj_stat.st_ino, obj_stat.st_size, \
      obj_stat.st_filetype == 2 ? "directory" : "regular");

   while(1);
   return 0;
}

/* 在线程中运行的函数 */
void k_thread_a(void* arg) {
   void* addr1 = sys_malloc(256);
   void* addr2 = sys_malloc(255);
   void* addr3 = sys_malloc(254);
   console_put_str(" thread_a malloc addr:0x");
   console_put_int((int)addr1);
   console_put_char(',');
   console_put_int((int)addr2);
   console_put_char(',');
   console_put_int((int)addr3);
   console_put_char('\n');

   int cpu_delay = 100000;
   while(cpu_delay-- > 0);
   sys_free(addr1);
   sys_free(addr2);
   sys_free(addr3);
   while(1);
}

/* 在线程中运行的函数 */
void k_thread_b(void* arg) {
   void* addr1 = sys_malloc(256);
   void* addr2 = sys_malloc(255);
   void* addr3 = sys_malloc(254);
   console_put_str(" thread_b malloc addr:0x");
   console_put_int((int)addr1);
   console_put_char(',');
   console_put_int((int)addr2);
   console_put_char(',');
   console_put_int((int)addr3);
   console_put_char('\n');

   int cpu_delay = 100000;
   while(cpu_delay-- > 0);
   sys_free(addr1);
   sys_free(addr2);
   sys_free(addr3);
   while(1);
}

void u_prog_a(void) {
   void* addr1 = malloc(256);
   void* addr2 = malloc(255);
   void* addr3 = malloc(254);
   printf(" prog_a malloc addr:0x%x, 0x%x, 0x%x\n", (int)addr1, (int)addr2, (int)addr3);

   int cpu_delay = 100000;
   while(cpu_delay-- > 0);
   free(addr1);
   free(addr2);
   free(addr3);
   while(1);
}

void u_prog_b(void) {
   void* addr1 = malloc(256);
   void* addr2 = malloc(255);
   void* addr3 = malloc(254);
   printf(" prog_b malloc addr:0x%x, 0x%x, 0x%x\n", (int)addr1, (int)addr2, (int)addr3);

   int cpu_delay = 100000;
   while(cpu_delay-- > 0);
   free(addr1);
   free(addr2);
   free(addr3);
   while(1);
}