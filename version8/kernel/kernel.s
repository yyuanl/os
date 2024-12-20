[bits 32]
%define ERROR_CODE nop ;如果cpu在中断处理时把错误码压入栈，则不执行任何动作
%define ZERO push 0    ;如果cpu不往栈中压入错误吗，则手动压入，保持栈结构相同

extern put_str;

section .data
intr_str db "interrupt occur!", 0xa, 0
global intr_entry_table

intr_entry_table:
%macro VECTOR 2
section .text
intr%1entry:
    %2     ;中断错误码ERROR_CODE / ZERO
    push intr_str;
    call put_str;
    add esp,4;

    ;如果是从片上进入的中断，需要往主从pic发送EOI中断处理结束的信号
    mov al,0x20
    out 0xa0,al   ;从
    out 0x20,al   ; 主

    add esp,4 ;跨过错误码。cs eip flag寄存器，供给iret使用
    iret;
section .data
    dd intr%1entry ;存中断程序地址，构成intr_entry_table数组，连续分布
%endmacro

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