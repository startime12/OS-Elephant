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

int main(void) {
    put_str("I am kernel\n");
    init_all();
    intr_enable(); // 打开中断
    while(1);
    return 0;
}