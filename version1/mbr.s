;将loader加载到内存中 loader约定好大小 就是512字节
%include "boot.inc" ;导入变量，便于使用
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
;------[start]----新内容：加载loader------------
	mov eax,LOADER_START_SECTOR ;起始扇区lba地址，扇区号
	mov bx,LOADER_BASE_ADDR		;写入的地址
	mov cx,4					;要读4个扇区的内容到内存
	call rd_disk_m_16			;读取loader到内存,1个扇区

	jmp LOADER_BASE_ADDR					;跳转到loader，让loader接管

rd_disk_m_16:
	mov esi,eax	;备份eax eax是loader所在的扇区号
	mov di,cx   ;备份cx cx是要都的扇区个数 约定是1个

;用指令和磁盘控制器交互 不必关心内部如何实现
;1 设置读取的扇区数 约定loader在主盘，所以用0x1f2端口
	mov dx,0x1f2
	mov al,cl ;cl=1
	out dx,al ;设置读取1个扇区
	mov eax,esi ;恢复eax寄存器 

;2 描述磁盘扇区的逻辑地址叫LBA 例如0盘0道0扇区。三个8位寄存器(低中高)和device寄存器低4位共28位来表达LBA
;0x1f3 0x1f4 0x1f5分别是三个8位寄存器
	mov dx,0x1f3
	out dx,al ;将ax(LBA) 0-7位写入0x1f3端口

	mov cl,8

	shr eax,cl ;右移8位就将原中间8-15位移动到低8位
	mov dx,0x1f4
	out dx,al ;将ax(LBA) 8-15位写入0x1f4端口

	shr eax,cl ;再右移8位就将原16-23位移动到低8位
	mov dx,0x1f5
	out dx,al ;将ax(LBA) 16-23位写入0x1f5端口

;device寄存器低4位存LBA最后的24-27位，高4位设置成1110,表示使用lba模式来表达磁盘读取地址
	shr eax,cl ;再再右移8位就将原24-27位移动到低8位
	and al,0x0f ;与运算，保留al低四位，高四位设置成0
	or al,0xe0 ;或运算，保留al低四位，高四位设置成1110
	mov dx,0x1f6
	out dx,al
;3 往0x1f7端口写入读命令，0x20
	mov dx,0x1f7
	mov al,0x20
	out dx,al

;4 检测硬盘状态
  .not_ready:
	nop ;sleep一下
	in al,dx ;从0x1f7端口读磁盘状态到al
	and al,0x88 ;与运算保留3 7位，其余位设0。3位是1表示磁盘控制器已经准备好数据传输了，7位是1表示磁盘忙
	cmp al,0x08 ;当al是00001000是cmp的结果就是0 说明磁盘准备好了
	jnz .not_ready ;没准备好，继续等

;5 从0x1f0端口读数据
	mov ax,di ;di是扇区数1
	mov dx,256
	mul dx
	mov cx,ax ;cx是要读多少次，in命令执行多少次，每次只能读2个字节。要读扇区数x一个扇区字节数=1x512=512个字节

	mov dx,0x1f0

  .go_on_read:
	in ax,dx ;将数据读到ax寄存器
	mov [bx],ax ;写到内存地址处
	add bx,2 ;每次之恶能写2字节,所以地址加2
	loop .go_on_read ;循环的终止条件是判断cx是否是0，每执行一次，cx减一
	ret
;-----[end]-----新内容：加载loader------------

	times 510-($-$$) db 0	;$$是本节地址，该命令是将后面空余位置用0填充
	db 0x55,0xaa			;MBR约定，最后两个字节按照约定编写
