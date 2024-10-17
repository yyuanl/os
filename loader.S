%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

;构建gdt及其内部的描述符
GDT_BASE:           dd 0x00000000
                    dd 0x00000000
CODE_DESC:          dd 0x0000FFFF
                    dd DESC_CODE_HIGH4
DATA_STACK_DESC:    dd 0x0000FFFF
                    dd DESC_DATA_HIGH4
VIDEO_DESC:         dd 0x80000007
                    dd DESC_VIDEO_HIGH4
GDT_SIZE            equ $ - GDT_BASE  ;当前地址减去起始地址，计算出这块内存大小
GDT_LIMIT           equ GDT_SIZE - 1
times 60 dq 0       ;预留60个描述符的空位？
SELECTOR_CODE       equ (0x0001<<3) + TI_GDT + RPL0 ;选择子
SELECTOR_DATA       equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO      equ (0x0003<<3) + TI_GDT + RPL0

gdt_ptr dw GDT_LIMIT ;为指令lgdt做准备 把gdt信息加载到gdtr寄存器
        dd GDT_BASE

lodermsg db '2 loader in real.'

loader_start:
;调用已有例程 INT 0x10 功能号13 表示打印字符串
;入参
;AH = 13H 子功能号
;BH = 页码
;BL = 属性(若AL=00H或01H)
;CX = 字符串长度
;(DH DL)=坐标(行 列)
;ES:BP=字符串地址
;AL = 显示输出方式
;  0—字符串中只含显示字符，其显示属性在BL中 ;显示后，光标位置不变 
;  1—字符串中只含显示字符，其显示属性在BL中 ;显示后，光标位置改变 
;  2—字符串中含显示字符和显示属性。显示后，光标位置不变 
;  3—字符串中含显示字符和显示属性。显示后，光标位置改变 
mov sp,LOADER_BASE_ADDR
mov bp,lodermsg         ;ES:BP = 字符串地址
mov cx,17               ;CX = 字符串长度
mov ax,0x1301           ;AH = 13,  AL = 01h 
mov bx,0x001f           ;页号为0(BH = 0) 蓝底粉红字(BL = 1fh)
mov dx,0x1800           ;?
int 0x10
;---准备进入保护模式：1 打开A20 2 加载gdt 将cr0的pe位置1------
;1 打开A20 将端口0x92的第1位 置1就可以了 关闭实模式地址回绕
in al,0x92 
or al,0000_0010B
out 0x92,al
;2 加载gdt
lgdt [gdt_ptr]
;将cr0的pe位置1
mov eax, cr0
or eax, 0x00000001
mov cr0, eax

jmp dword SELECTOR_CODE:p_mode_start ;刷新流水线 避免16 和 32 地址混乱流水线把32位指令按照16位译码

[bit 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    mov byte [gs:160], 'P'
    jmp $
;
;
;

;mov byte [gs:0x00],'2'
;mov byte [gs:0x01],0xA4

;mov byte [gs:0x02],' '
;mov byte [gs:0x03],0xA4

;mov byte [gs:0x04],'l'
;mov byte [gs:0x05],0xA4

;mov byte [gs:0x06],'o'
;mov byte [gs:0x07],0xA4

;mov byte [gs:0x08],'a'
;mov byte [gs:0x09],0xA4

;mov byte [gs:0x0a],'d'
;mov byte [gs:0x0b],0xA4

;mov byte [gs:0x0c],'e'
;mov byte [gs:0x0d],0xA4

;mov byte [gs:0x0e],'r'
;mov byte [gs:0x0f],0xA4

;jmp $

