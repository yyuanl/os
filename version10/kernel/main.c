#include "print.h"
#include "init.h"
int main(void) {
	put_char('1');
	put_char('2');
	put_char('\b');
	put_char('3');
	put_str("\nhello word,i am in kernel\n");
	put_str("\nto convert 0x12345678 to string\n");
	put_int(0x12345678);
	put_str("\nto convert 0x00000000 to string\n");
	put_int(0x00000000);
	put_char('\n');
	put_char('e');
	init_all();
	asm volatile("sti");
	while(1);
}