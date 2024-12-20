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
.mem_get_ok:
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
    ;mov byte [gs:160], 'P'
    ;jmp $

;--------load kernerl 2024.11.4--------------
    mov eax,KERNEL_START_SECTOR ; kernel.bin所在的扇区号
    mov ebx,KERNEL_BIN_BASE_ADDR ;从磁盘写入ebx的地址处
    mov ecx,200;读入的扇区数
    call rd_disk_m_32;------------------------------
    ;构建PDE PTE
    call setup_page

    sgdt [gdt_ptr] ; 先把gdt 挪个地方

    ;显存段段基址+0xc0000 000凑成虚拟地址，高3G
    mov ebx,[gdt_ptr+2]
    ;显存段是第三个段描述符。高4字节是的最高位是段基址的31-24位
    or dword [ebx+0x18+4],0xc0000000
    add dword [gdt_ptr + 2],0xc0000000;gdt基址变成虚拟地址高地址
    add esp,0xc0000000;同理将栈指针页变成虚拟地址高地址 内核
    ;把页目录地址赋值给cr3
    mov eax,PAGE_DIR_TABLE_POS
    mov cr3,eax

    ;打开cr0的pg位 第31位
    mov eax,cr0
    or eax,0x80000000
    mov cr0,eax

    ;开启分页后，用gdt新的地址重新加载
    lgdt [gdt_ptr]
    jmp SELECTOR_CODE:enter_kernel ;保险起见，刷新流水线

enter_kernel:
  call kernel_init
  mov esp,0xc009f000 ;选择较高地址作为栈底。考虑pcb块是4kb，取证得出0xc009f000
  jmp KERNEL_ENTRY_POINT
;==============================以下是封装的代码片段===========================
;kernel_init将内核段拷贝到编译对应的地址
kernel_init:
  xor eax,eax
  xor ebx,ebx ;程序头表地址
  xor ecx,ecx ;程序头表表项即程序头的个数
  xor edx,edx ;记录程序头的尺寸

  mov dx, [KERNEL_BIN_BASE_ADDR + 42]
  mov ebx,[KERNEL_BIN_BASE_ADDR + 28]
  add ebx,KERNEL_BIN_BASE_ADDR
  mov cx,[KERNEL_BIN_BASE_ADDR + 44]

.each_segment:
  cmp byte [ebx + 0],PT_NULL
  je .PTNULL

  ;调用函数mem_cpy(dst,src,size)
  push dword [ebx + 16] ;p_filesz字段表示本段大小
  mov eax,[ebx + 4] ;p_offset字段表示本段相对于起始位置偏移量
  add eax,KERNEL_BIN_BASE_ADDR ;文件被拷贝到KERNEL_BIN_BASE_ADDR
  push eax
  push dword [ebx + 8] ;p_vaddr字段，表示该段应该被放到这个地址，将来让cpu执行该地址处指令
  call mem_cpy
.PTNULL:
  add ebx,edx ;移动指针，指向下一个程序头处
  loop .each_segment
  ret

; ----mem_cpy 内存拷贝---
mem_cpy:
  cld ;清楚内存拷贝操作方向
  push ebp ;备份ebp
  mov ebp,esp
  push ecx ;rep指令使用了ecx，先备份调用前的值，避免影响外层
  mov edi,[ebp + 8]
  mov esi,[ebp + 12]
  mov ecx,[ebp + 16]
  rep movsb ;一个字一个字拷贝

  ;恢复调用前状态
  pop ecx
  pop ebp
  ret



; 创建页目录及页表
setup_page:
  mov ecx,4096 ;clear pde 4kb = 4096Byte
  mov esi,0
.clear_page_dir: ;清空pde页表目录表
  mov byte [PAGE_DIR_TABLE_POS + esi],0
  inc esi
  loop .clear_page_dir

;创建PDE
.create_pde:
  mov eax,PAGE_DIR_TABLE_POS
  add eax,0x1000 ;PAGE_DIR_TABLE_POS + 0x1000是0号页表的首地址。因为页目录PED占4KB
  mov ebx,eax

  ;构造完整的页目录项的值，是一个地址加属性。用户属性,所有特权级都可以访问
  or eax, PG_US_U | PG_RW_W | PG_P
  mov [PAGE_DIR_TABLE_POS + 0x0],eax
  ; 3G=0xc000 000 0,高10位是1100 0000 00b=2^9 + 2^8=(2 + 1)*2^8=3*256=768
  ; PDE的第768项的值也应该是0号页表的首地址.768*4=0xc00,第768项的地址是PAGE_DIR_TABLE_POS+768*4
  mov [PAGE_DIR_TABLE_POS + 0xc00],eax ;构造虚拟地址0xc0000000-0xc00000000 1G属于内核

  sub eax,0x1000
  mov [PAGE_DIR_TABLE_POS + 4096],eax;最后一个页目录项指向自己pde首地址

  ;创建页表项pte
  mov ecx,256
  mov esi,0
  mov edx,PG_US_U | PG_RW_W | PG_P ;属性7
.create_pte:
  mov [ebx+esi*4],edx
  add edx,4096 ;页大小是4kb 4096
  inc esi
  loop .create_pte

; 创建内核其他页表的PED，虽然对应PTE还未创建
  mov eax,PAGE_DIR_TABLE_POS
  add eax,0x200 ;1号页表
  or eax,PG_US_U | PG_RW_W | PG_P
  mov ebx,PAGE_DIR_TABLE_POS
  mov ecx,254
  mov esi,769
.create_kernel_pde:
  mov [ebx+esi*4],eax
  inc esi
  add eax,0x1000
  loop .create_kernel_pde
  ret

;读取硬盘n个扇区.eax LBA扇区号；ebx 内存地址；ecx 读入的扇区数
rd_disk_m_32:
  mov esi,eax;备份eax
  mov di,cx ;备份扇区数到di

;使用硬盘控制器接口读写硬盘
;1、设置要读取的扇区数
  mov dx,0x1f2
  mov al,cl
  out dx,al  ;读取的扇区数

  mov eax,esi ;恢复ax
;2、设置LBA地址

  ;LBA地址7-0位写入端口0x1f3
  mov dx,0x1f3
  out dx,al

  ;LBA地址15-8位写入端口0x1f4
  mov cl,8
  shr eax,cl
  mov dx,0x1f4
  out dx,al

  ;LAB地址23-16位写入端口0x1f5
  shr eax,cl
  mov dx,0x1f5
  out dx,al

  shr eax,cl
  and al,0x0f
  or al,0xe0
  mov dx,0x1f6
  out dx,al

;3、向0x1f7端口写入读命令，0x20
  mov dx,0x1f7
  mov al,0x20
  out dx,al

;4、检测硬盘状态
  .not_ready:
    nop
    in al,dx
    and al,0x88
    cmp al,0x08
    jnz .not_ready

    ;硬盘不忙，则准备从磁盘读数据到内存
;5、从0x1f0端口读数据
  mov ax,di ;di是扇区数
  mov dx,256 ;一个字两字节，每次读2字节，一个扇区需要读512/2=256次
  mul dx
  mov cx,ax
  mov dx,0x1f0
  .go_on_read:
    in ax,dx
    mov [ebx],ax ;ebx 对应152行 内存地址 因为是32位模式，使用ebx。编译器会对机器码加上0x66 0x67反转
    add ebx,2
  loop .go_on_read
  ret
