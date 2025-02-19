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
#include "shell.h"
#include "debug.h"

// int main(void){
//    put_str("Welcome,\nI am kernel!\n");
//    init_all();

//    while(1);
//    return 0;
// }

// /* init进程 */
// void init(void) {
//    uint32_t ret_pid = fork();
//    if(ret_pid) {  // 父进程
//       printf("i am father, my pid is %d, child pid is %d\n", getpid(), ret_pid);
//    } else {	  // 子进程
//       printf("i am child, my pid is %d, ret pid is %d\n", getpid(), ret_pid);
//    }
//    while(1);
// }

void init(void);

int main(void){
   put_str("Welcome,\nI am kernel!\n");
   init_all();
   intr_enable();
   cls_screen();
   console_put_str("[jqn@LAPTOP /]$ ");
   while(1);
   return 0;
}

/* init进程 */
void init(void){
   uint32_t ret_pid=fork();
   if (ret_pid){   // 父进程
      printf("parent...\n");
      while (1);
   }else {   // 子进程
      printf("son...\n");
      my_shell();
   }
   PANIC("init: should not be here.");
}