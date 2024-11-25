[bits 32]
%define ERROR_CODE nop ;如果cpu在中断处理时把错误码压入栈，则不执行任何动作
%define ZERO push 0    ;如果cpu不往栈中压入错误吗，则手动压入，保持栈结构相同

extern put_str;
extern idt_table ;version9

section .data
global intr_entry_table

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

section .text
global intr_exit
intr_exit:
;以下时恢复上下文环境
    add esp,4;跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp,4;跳过error_code
    iretd

VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO	;时钟中断对应的入口