; 主引导程序
;
;LOADER_BASE_ADDE equ 0xA000
;LOADER_START_SECTOR equ 0x2
; --------------------------------------------------
SECTION MBR vstart=0x7c00 ; 把起始地址编译为 0x7c00
    mov ax, cs     ; cs 代码段寄存器
    mov ds, ax     ; dx 数据段寄存器
    mov es, ax     ; es 附加段寄存器
    mov ss, ax     ; ss 堆栈段寄存器
    mov fs, ax     ; fs 80386 后添加的寄存器，无全称
    mov sp, 0x7c00 ; sp 堆栈指针寄存器
    mov ax, 0xb800 ; 用于文本模式显示适配器 0xb800*16+偏移地址
    mov gs,ax	   ; 存入段基址

; 清屏
; --------------------------------------------------
; INT 0x10    功能号: 0x06    功能描述：上卷窗口
; --------------------------------------------------
; 输入：
; AH 功能号 = 0x06
; AL = 上卷的行数(如果为0，表示全部)
; BH = 上卷行属性
; (CL, CH) = 窗口左上角的 (X, Y) 位置
; (DL, DH) = 窗口右下角的 (X, Y) 位置
; 无返回值:
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0	   ; 左上角: (0, 0)
    mov dx, 0x184f ; 右下角: (80, 25)
                   ; VGA 文本模式种，一行只能容纳 80 个字符，共 25 行
                   ; 下标从 0 开始，所以 0x18=24, 0x4f=79
    int 0x10       ; int 0x10

	;输出背景色绿色，前景色红色，并且跳动的字符串"1 MBR"
	mov byte [gs:0x00],'1'
	mov byte [gs:0x01],0xA4	; A 表示绿色背景闪烁，4 表示前景色为红色
	
	mov byte [gs:0x02],' '
	mov byte [gs:0x03],0xA4
	
	mov byte [gs:0x04],'M'
	mov byte [gs:0x05],0xA4
	
	mov byte [gs:0x06],'B'
	mov byte [gs:0x07],0xA4
	
	mov byte [gs:0x08],'R'
	mov byte [gs:0x09],0xA4

    jmp $          ; 程序悬停在此

    times 510-($-$$) db 0
    db 0x55, 0xaa
