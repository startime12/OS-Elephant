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

void k_thread_a(void*);

int main(){
	put_str("\nI am kernel\n");
	init_all();
    thread_start("k_thread_a",31,k_thread_a,"A_ ");
    intr_enable(); // 打开中断, 使时钟中断起作用
	while(1);
	return 0;
}

void k_thread_a(void* arg){
	char* para = arg;
	void* addr1;
    void* addr2;
    void* addr3;
    void* addr4;
    console_put_str("thread_a_start\n");
    int max = 100;
    while(max-- > 0){
        int size = 128;
        addr1 = sys_malloc(size);
        size *= 2;
        addr2 = sys_malloc(size);
        size *= 2;
        addr3 = sys_malloc(size);
        size *= 8;
        addr4 = sys_malloc(size);
        sys_free(addr1);
        sys_free(addr2);
        sys_free(addr4);
        sys_free(addr3);
    }
    console_put_str("thread_a_end\n");
    while(1);
}