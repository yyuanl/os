#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"

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
	// asm volatile("sti");
	void* addr = get_kernel_pages(3);
	put_str("\n get_kernel_page start vaddr is");
	put_int((uint32_t)addr);
	put_str("\n");
	while(1);
	return 0;
}