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

/*version9*/
char* intr_name[IDT_DESC_CNT];//保存异常的名字
intr_handler idt_table[IDT_DESC_CNT];//中断处理函数，kernel.s中处理函数真正call这个数组里的处理函数

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

/*version9，通用中断处理函数*/
static void general_intr_handler(uint8_t vec_nr) {
   if (vec_nr == 0x27 || vec_nr == 0x2f) {
      // IRQ7和IRQ15会产生伪中断，无需处理
      // 0x2f是从片8259A上的最后一个IRQ引脚，保留项
      return ;
   }
   put_str("occur intterupt,vector number is 0x");
   put_int(vec_nr);
   put_str(",handle this\n");
}

//version9，idt_table intr_name这两个数组进行赋值
// idt_table保存的是中断处理函数数组,供汇编中调用:
/*
intr_entry_table:
%macro VECTOR 2
section .text
intr%1entry:
    %2;中断若有错误码会压在eip后面
    ;保存上下文环境
    push ds
    push es
    push fs
    push gs
    pushad

    ;进行处理时，要给pid应答，即EOI。如果是从片上进入中断；处理往从片上发送EOI外，还要往主片发送EOI
    mov al,0x20;中断结束命令EOI
    out 0xa0,al;向从片发送
    out 0x20,al;向主片发送

    push %1;压入中断向量号

    call [idt_table + %1*4];调用c中的处理函数
    jmp intr_exit;

section .data
    dd intr%1entry ;存中断程序地址，构成intr_entry_table数组，连续分布
%endmacro
*/
// intr_name是中断对应的异常名字，便于排查问题
static void exception_init(void){
   int i;
   for (i = 0; i < IDT_DESC_CNT; i++){
      idt_table[i] = general_intr_handler;//默认处理函数
      intr_name[i] = "unknown";
   }
   intr_name[0] = "#DE Divide Error"; 
   intr_name[1] = "#DB Debug Exception"; 
   intr_name[2] = "NMI Interrupt"; 
   intr_name[3] = "#BP Breakpoint Exception"; 
   intr_name[4] = "#OF Overflow Exception"; 
   intr_name[5] = "#BR BOUND Range Exceeded Exception"; 
   intr_name[6] = "#UD Invalid Opcode Exception"; 
   intr_name[7] = "#NM Device Not Available Exception"; 
   intr_name[8] = "#DF Double Fault Exception"; 
   intr_name[9] = "Coprocessor Segment Overrun"; 
   intr_name[10] = "#TS Invalid TSS Exception"; 
   intr_name[11] = "#NP Segment Not Present"; 
   intr_name[12] = "#SS Stack Fault Exception"; 
   intr_name[13] = "#GP General Protection Exception"; 
   intr_name[14] = "#PF Page-Fault Exception"; 
   // intr_name[15] 第 15 项是 intel 保留项,未使用
   intr_name[16] = "#MF x87 FPU Floating-Point Error"; 
   intr_name[17] = "#AC Alignment Check Exception"; 
   intr_name[18] = "#MC Machine-Check Exception"; 
   intr_name[19] = "#XF SIMD Floating-Point Exception"; 
}



/*完成有关中断的所有初始化工作*/
void idt_init() {
   put_str("idt_init start\n");
   idt_desc_init();	   // 初始化中断描述符表

   exception_init();    // 异常名初始化 注册通用中断处理函数

   pic_init();		   // 初始化8259A

   /* 加载idt */
   uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
   asm volatile("lidt %0" : : "m" (idt_operand));
   put_str("idt_init done\n");
}

