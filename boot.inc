;loader
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2

;gdt位的定义
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




