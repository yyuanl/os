SECTION MBR vstart=0x7c00   ;声明段起始地址，告诉编译器是以此地址为基值分配指令地址。bios将MBR加载到内存0x7c00处，
		               		;bios将pc寄存器设置为0x7c00,然后cpu自然而然的就可以正确执行我们自己编写的mbr程序了。
	mov ax,cs				;使用cs寄存器初始化ax寄存器,作为中转来初始化其他寄存器
							;cs值是0，因为bios是通过jmp 0:0x7c00跳转到MBR的
	mov ds,ax				;初始化ds
	mov es,ax				;初始化es
	mov ss,ax				;初始化ss
	mov fs,ax				;初始化fs
	mov sp,0x7c00			;初始化栈
    mov ax,0xb800			;字符串显存地址
	mov gs,ax				;操作显存的段基址
;=====================清屏==========================
;调用bios的清屏例程 INT 0x10，参数就是各个寄存器值
;--------------参数说明：------------------
; AH=0x06表示上卷功能号；BH代表窗口上卷后，新窗口顶部的属性；0x07表示白色前景和黑色背景
; (CL,CH) 窗口左上角(x,y)的位置；(DL,DH)窗口右下角(x,y)的位置；y是垂直方向，代表行数
; ax寄存器是16寄存器，AH是ax的高8位，AL是ax的低8位;16进制一个元素是4位;bx同理

	mov ax,0x600
	mov bx,0x700
	mov cx,0				;左上角(0,0)
	mov dx,0x184f			;16进制18等于10进制24，4f等于79；右下角(80,25)
	int 0x10				;调用bios封装好的中断向量表

    mov byte [gs:0x00],'1'
    mov byte [gs:0x01],0xA4

    mov byte [gs:0x02],' '
    mov byte [gs:0x03],0xA4

    mov byte [gs:0x04],'M'
    mov byte [gs:0x05],0xA4

    mov byte [gs:0x06],'B'
    mov byte [gs:0x07],0xA4

    mov byte [gs:0x08],'R'
    mov byte [gs:0x09],0xA4

	jmp $					;死循环，$表示当前指定的地址

	times 510-($-$$) db 0	;$$是本节地址，该命令是将后面空余位置用0填充
	db 0x55,0xaa			;MBR约定，最后两个字节按照约定编写
