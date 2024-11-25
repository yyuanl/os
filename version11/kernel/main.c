#include "print.h"
#include "init.h"
#include "debug.h"
int main(void){
	put_str("hell world,this is kernel\n");
	init_all();
	ASSERT(1==2);
	while(1);
	return 0;
}
int main1(void) {
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