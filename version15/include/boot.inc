;loader
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2

;gdt位的定义 高32中的 23-0位
DESC_G_4K           equ 1_00000000000000000000000b ;G位是4k粒度
DESC_D_32           equ  1_0000000000000000000000b ;表示代码指令中的有效地址及操作数是32位
DESC_L              equ   0_000000000000000000000b ;表示32位代码段；1表示64位代码
DESC_AVL            equ    0_00000000000000000000b ;无用位 暂置0
DESC_LIMIT_CODE2    equ     1111_0000000000000000b ;19-16位段界限
DESC_LIMIT_DATA2    equ     1111_0000000000000000b ;19-16位段界限
DESC_LIMIT_VIDEO2   equ      0000_000000000000000b ;?
DESC_P              equ         1_000000000000000b ;表示段存在
DESC_DPL_0          equ          00_0000000000000b ;特权级是0
DESC_DPL_1          equ          01_0000000000000b 
DESC_DPL_2          equ          10_0000000000000b
DESC_DPL_3          equ          11_0000000000000b
DESC_S_CODE         equ            1_000000000000b ;表示普通内存段
DESC_S_DATA         equ            1_000000000000b ;表示普通内存段
DESC_S_sys          equ            0_000000000000b ;系统段
DESC_TYPE_CODE      equ             1000_00000000b ;代码段是可执行的，非一致性，不可读，已访问位a清0
DESC_TYPE_DATA      equ             0010_00000000b ;数据段是不可执行的，向上扩展的，可写，已访问位a清0
DESC_BASE_VALUE     equ                  00000000b ;7-0段基值  

;拼凑代码段描述符高4字节的值 左移的作用是构造出32个bit位置
DESC_CODE_HIGH4     equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2  + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
;拼凑数据段描述符高4字节的值
DESC_DATA_HIGH4     equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2  + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
;拼凑显存段描述符高4字节的值，注意最后是0x0b，因为显存段是从0xb8000开始的
DESC_VIDEO_HIGH4    equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b 

; 选择子部分定义
RPL0 equ 00b ;四种特权级
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
TI_GDT equ 000b ;从gdt中取段描述信息
TI_LDT equ 100b ;从ldt中取段描述信息

PAGE_DIR_TABLE_POS equ 0x100000 ; 超出1M的第一个字节

; ----页表相关属性----
PG_P equ 1b  ; 物理内存不够，置换页到磁盘，此位0 下次访问出发缺页中断，再加载到内存
PG_RW_R equ 00b ; read
PG_RW_W equ 10b ; write

PG_US_S equ 000b ;不能被特权级3的任务访问，处理器只允许特权级为0、1、2的任务访问该PTE或PDE指向的内存
PG_US_U equ 100b ;允许任何特权级的任务访问PTE或PDE指向的内存




LOADER_STACK_TOP equ LOADER_BASE_ADDR


KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_START_SECTOR equ 0x9
KERNEL_ENTRY_POINT equ 0xc0001500
;-------------  program type 定义   --------------
PT_NULL equ 0
