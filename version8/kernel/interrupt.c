#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h" // todo
#include "print.h"

#define PIC_M_CTRL 0x20 //8259A 可编程控制器，主片控制端口是0x20
#define PIC_M_DATA 0x21 //主片的数据端口0x21
#define PIC_S_CTRL 0xa0 //从片的控制端口
#define PIC_S_DATA 0xa1 //从片的数据端口

#define IDT_DESC_CNT 0x81 // 目前总共支持的中断数



/*中断门描述符结构体*/
struct gate_desc {
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t dcount; //固定值
    uint8_t attribute;
    uint16_t func_offset_high_word;
};

// 静态函数声明 非必须
static void make_idt_desc(struct gate_desc* p_gdesc,uint8_t attr,intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];

extern intr_handler intr_entry_table[IDT_DESC_CNT]; //引用kernel.s中的中断处理函数入口数组


// 配置8259A pic
static void pic_init(void){
    //初始化主片
    outb(PIC_M_CTRL,0x11);// IWC1 边沿触发，级联8259，需要ICW4
    outb(PIC_M_DATA,0x20);// ICW2 起始中断向量号位0x20,即IR[0-7] 为 0x20-0x27
    outb(PIC_M_DATA,0x04);// ICW3 IR2接从片
    outb(PIC_M_DATA,0x01);// ICW4 8086模式，正常EOI

    //初始化从片
    outb(PIC_S_CTRL,0x11);// ICW1 边沿触发，级联8259，需要ICW4
    outb(PIC_S_DATA,0x28);// ICW2起始中断向量号0x28,即IR[8-15]为0x28-0x2F
    outb(PIC_S_DATA,0x02);// ICW3设置从片链接到主片的IR引脚
    outb(PIC_S_DATA,0x01);// ICW4 8086模式，正常EOI

    /*IRQ2用于级联从片，必须打开，否则无法响应从片上的中断
    主片上打开的中断有IRQ0的时钟，IRQ1的键盘和级联从片的IRQ2，其他全部关闭
    */
   outb(PIC_M_DATA,0xf8);

   /*打开从片上的IRQ14，此引脚接受硬盘控制器的中断*/
   outb(PIC_S_DATA,0xbf);

   put_str("  pic_init done\n");
}

//创建中断门描述符
static void make_idt_desc(struct gate_desc* p_gdesc,uint8_t attr,intr_handler function){
   p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
   p_gdesc->selector = SELECTOR_K_CODE;
   p_gdesc->dcount = 0;
   p_gdesc->attribute = attr;
   p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}
/*初始化中断描述符表*/
static void idt_desc_init(void) {
   int i, lastindex = IDT_DESC_CNT - 1;
   for (i = 0; i < IDT_DESC_CNT; i++) {
      make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]); 
   }
/* 单独处理系统调用,系统调用对应的中断门dpl为3,
 * 中断处理程序为单独的syscall_handler */

   put_str("   idt_desc_init done\n");
}


/*完成有关中断的所有初始化工作*/
void idt_init() {
   put_str("idt_init start\n");
   idt_desc_init();	   // 初始化中断描述符表

   pic_init();		   // 初始化8259A

   /* 加载idt */
   uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
   asm volatile("lidt %0" : : "m" (idt_operand));
   put_str("idt_init done\n");
}

