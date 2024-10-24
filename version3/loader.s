%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
;jmp loader_start

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
;=============================

;total_mem_bytes用来保存内存容量，以字节位单位
;当前位置的地址：4个描述符+60个描述符空位，64*8=512=0x200,loader加载的地址是0x900，所以
;该变量的地址是0xb00
total_mem_bytes dd 0
;================


SELECTOR_CODE       equ (0x0001<<3) + TI_GDT + RPL0 ;选择子
SELECTOR_DATA       equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO      equ (0x0003<<3) + TI_GDT + RPL0
;================


gdt_ptr dw GDT_LIMIT ;为指令lgdt做准备 把gdt信息加载到gdtr寄存器
        dd GDT_BASE

ards_buf times 244 db 0;1、存所有ard的结构2、拼凑出loader_start地址为0x300
ards_nr dw 0 ;记录ards结构体个数
;计算一下地址:total_mem_bytes地址是0x200,大小是4字节
;gdt_ptr 6字节
;ards_buf 244字节
;ards_nr 2字节
;4+6+244+2=256=0x100，所以loader_start地址(偏移)就手动拼凑为0x300
;================

loader_start:
;-----int 15h eax=0000E820h edx=534D4150h 获取内存布局-----
xor ebx,ebx    ;第一次调用时，ebx值为0
mov edx,0x534d4150 ; 入参，固定标识
mov di,ards_buf     ;接受返回信息的内存地址
.e820_mem_get_loop:
  mov eax,0x0000e820
  mov ecx,20         ;返回buffer大小
  int 0x15           ;调用
  jc .e820_failed_so_try_e801 ;cf位为1则错误发生，尝试0xe801子功能号
  add di,cx          ;移动buffer指针
  inc word [ards_nr] ;标记ards个数
  cmp ebx,0           ;cf不为1，ebx为0则说明ards全部返回，内存检测完毕
  jnz .e820_mem_get_loop
;在所有ards结构中，找出连续内存地址的最高值(base_add_low + length_low)
;ards结构:
  mov cx,[ards_nr] ;遍历每一个ards结构体，循环次数时ards的数量
  mov ebx,ards_buf
  xor edx,edx ;初始化清零
.find_max_mem_area:
  mov eax,[ebx] ;取开始4字节的数据，即baseaddrlow基地址低32位
  add eax,[ebx+8] ;取当前ards元素第8字节的数据，即lengthlow内存长度低32位
  add ebx,20 ;指针移动
  cmp edx,eax
  jge .nest_ards ; if edx >= eax，继续判定下一个ards元素
  mov edx,eax    ;  else edx < eax,则更新edx变量
.nest_ards:
  loop .find_max_mem_area
  jmp .mem_get_ok
;==============================================
;---int 15h ax=e801h 获取内存大小，最大支持4G-----
.e820_failed_so_try_e801:
  mov ax,0xe801
  int 0x15
  jc .e801_failed_so_try88

;计算出低15M的内存，其中数字的kb，要转成byte
  mov cx,0x400 ;1024
  mul cx       ; ax = ax * cx 乘法结果如果eax能放下就全在eax，放不下edx放超出的16位
  shl edx,16   ;处理乘法结果
  and eax,0x0000FFFF ;保留低16位
  or edx,eax   ;经过and or运算，拼凑出32位的乘法结果
  add edx,0x100000 ;加1MB，????历史上留1MB专用
  mov esi,edx ;备份

;计算16MB以上的内存，继续转为byte,单位是64kb
 xor eax,eax
 mov ax,bx
 mov ecx,0x10000 ;64KB
 mul ecx ;32位乘法，默认乘数是eax,乘积是64位，高32位存入edx,低32位存入eax. 4G低32位够用
 add esi,eax
 mov edx,esi
 jmp .mem_get_ok

;----int 15h ah=0x88 获取内存大小，只能获取64M之内---
.e801_failed_so_try88:
  mov ah,0x88
  int 0x15
  jc .error_hlt
  and eax,0x0000FFFF

  ;ax中保存结果 kb单位 转成byte
  mov cx,0x400
  mul cx
  shl edx,16
  or edx, eax
  add edx,0x100000 ;加1MB

.error_hlt:
  jmp $
.mem_get_ok
  mov [total_mem_bytes],edx






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


[bits 32]
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
