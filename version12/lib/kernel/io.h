// 利用端口操作设备，例如硬盘、显卡、pid。封装成c函数。

#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"
// 向端口port写入一个字节
static inline void outb(uint16_t port,uint8_t data){
    /*********************************************************
 a表示用寄存器al或ax或eax,对端口指定N表示0~255, d表示用dx存储端口号, 
 %b0表示对应al,%w1表示对应dx */ 
   asm volatile ( "outb %b0, %w1" : : "a" (data), "Nd" (port));    
}

//将addr处起始的word_cnt个字写入端口port
static inline void outsw(uint16_t port,const void* addr,uint32_t word_cnt){
    /*********************************************************
   +表示此限制即做输入又做输出.
   outsw是把ds:esi处的16位的内容写入port端口, 我们在设置段描述符时, 
   已经将ds,es,ss段的选择子都设置为相同的值了,此时不用担心数据错乱。*/
   asm volatile ("cld; rep outsw" : "+S" (addr), "+c" (word_cnt) : "d" (port));
}

// 将从端口port读入的word_cnt个字写入addr*
static inline void insw(uint16_t port,void*addr,uint32_t word_cnt){
    /*
    insw是将从端口port处读入的16位内容写入es:di指向的内存，
    在设置段描述符是，ds es ss段的选择子都设置位相同值，
    没有数据错乱
    */
   asm volatile ("cld;rep insw" : "+D" (addr),"+C" (word_cnt) : "d" (port):"memory");
}



#endif