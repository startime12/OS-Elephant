%include "boot.inc"
SECTION LOADER vstart=LOADER_BASE_ADDR     ;同书上，设置为0x900
LOADER_STACK_TOP equ LOADER_BASE_ADDR      ;初始化栈顶，0x900向下为栈空间
jmp loader_start

;构建GDT及其内部的描述符
   GDT_BASE:   dd   0x00000000             ;没用的第0个段描述符
               dd   0x00000000
   CODE_DESC:  dd   0x0000FFFF
               dd   DESC_CODE_HIGH4
   DATA_STACK_DESE:   dd   0x0000FFFF
                      dd   DESC_DATA_HIGH4
   VIDEO_DESC: dd   0x80000007                ;limit=(0xbffff-0xb8000)/4k=0x7
               dd   DESC_VIDEO_HIGH4          ;此时DPL为0
   GDT_SIZE    equ  $-GDT_BASE                ;地址差作尺寸：当前行地址-GDT_BASE地址
   GDT_LIMIT   equ  GDT_SIZE-1
   times 60 dq 0                              ;此处预留60个描述符空位；dq 定义4字/8字节
   SELECTOR_CODE   equ   (0x0001<<3)+TI_GDT+RPL0
      ;相当于(CODE_DESC-GDT_BASE)/8+TI_GDT+RPL0
   SELECTOR_DATA   equ   (0x0002<<3)+TI_GDT+RPL0
   SELECTOR_VIDEO  equ   (0x0003<<3)+TI_GDT+RPL0

   ;以下是GDT的指针GDTR,6B/48bit，前2字节是GDT界限，后4字节是GDT起始地址
   gdt_ptr     dw  GDT_LIMIT
               dd  GDT_BASE
   loadermsg   db  '2 Loader in real.'

loader_start:

;---------------------------------------------------------------------
;INT 0x10    功能号：0x13     功能描述：打印字符串
;---------------------------------------------------------------------
;输入：
;AH 子功能号=13H
;BH=页码
;BL=属性
;CX=字符串属性
;(DH、DL)=坐标(行、列)
;ES:BP=字符串地址
;AL=显示输出方式
; 0——字符串中只含显示字符，其显示属性在BL中
     ;显示后，光标位置不变
; 1——字符串中只含显示字符，其显示属性在BL中
     ;显示后，光标位置改变
; 2——字符串中含显示字符和显示属性。显示后，光标位置不变
; 3——字符串中含显示字符和显示属性。显示后，光标位置改变
;无返回值
   mov sp,LOADER_BASE_ADDR
   mov bp,loadermsg            ;ES:BP=字符串地址
   mov cx,16                   ;CX=字符串长度
   mov ax,0x1301               ;AX=13,AL=01H
   mov bx,0x001F               ;页号为0(BH=0)蓝底粉红字(BL=1FH)
   mov dx,0x1800
   int 0x10                    ;10H号中断

;--------------------- 准备进入保护模式 ------------------------
;1 打开A20
;2 加载GDT
;3 将cr0的PE位置1

;--------------------------- 打开A20 ---------------------------
   in al,0x92
   or al,0000_0010B            ;简单说，将端口0x92的第1位置1即可
   out 0x92,al

;--------------------------- 加载GDT ---------------------------
   lgdt [gdt_ptr]              ;load GDT [addr]

;-------------------------- cr0第0位置1 ------------------------
   mov eax,cr0
   or eax,0x00000001
   mov cr0,eax

   jmp dword SELECTOR_CODE:p_mode_start     ;刷新流水线。因为要远转移，cs更新，所以流水线上的其它指令都没用了，就会刷新

[bits 32]     ;开启32位指令
p_mode_start:
   mov ax,SELECTOR_DATA
   mov ds,ax
   mov es,ax
   mov ss,ax
   mov esp,LOADER_STACK_TOP
   mov ax,SELECTOR_VIDEO
   mov gs,ax

   mov byte [gs:160],'P'       ;第2行首字符打印P

   jmp $
