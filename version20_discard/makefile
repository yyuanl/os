BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I lib/kernel -I /lib/user -I kernel/ -I device/ -I thread/ -I userprog/
ASFLAGS = -f elf
CFLAGS = -Wall -m32 -fno-stack-protector $(LIB) -c -fno-builtin -W -Wstrict-prototypes \
		-Wmissing-prototypes
LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e mian -Map ${BUILD_DIR}/kernel.map
#LDFLAGS = -m elf_i386 -Ttext $(ENTRY_POINT) -e mian
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/init.o $(BUILD_DIR)/print.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/list.o \
	   $(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o \
	    $(BUILD_DIR)/string.o $(BUILD_DIR)/bitmap.o \
	   $(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/switch.o \
	   $(BUILD_DIR)/sync.o $(BUILD_DIR)/console.o $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/keyboard.o \
	   $(BUILD_DIR)/tss.o $(BUILD_DIR)/process.o


##汇编代码编译
$(BUILD_DIR)/kernel.o: kernel/kernel.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o: lib/kernel/print.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/switch.o: thread/switch.s
	$(AS) $(ASFLAGS) $< -o $@

## c代码编译
$(BUILD_DIR)/string.o: lib/string.c lib/string.h lib/stdint.h kernel/global.h \
	lib/stdint.h kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c lib/kernel/bitmap.h \
	kernel/global.h lib/stdint.h lib/string.h lib/stdint.h \
	lib/kernel/print.h kernel/interrupt.h kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c kernel/memory.h lib/stdint.h lib/kernel/bitmap.h \
	kernel/global.h kernel/global.h kernel/debug.h lib/kernel/print.h \
	lib/kernel/io.h kernel/interrupt.h lib/string.h lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@



$(BUILD_DIR)/init.o: kernel/init.c kernel/init.h lib/kernel/print.h \
	lib/stdint.h kernel/interrupt.h device/timer.h userprog/tss.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h  \
	lib/stdint.h kernel/global.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h \
	lib/stdint.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h \
	lib/kernel/print.h lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c thread/thread.h lib/stdint.h  \
    	kernel/global.h lib/string.h lib/stdint.h kernel/debug.h \
     	 lib/kernel/print.h kernel/memory.h \
      	lib/kernel/bitmap.h thread/thread.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c lib/kernel/list.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: thread/sync.c thread/sync.h kernel/interrupt.h lib/kernel/list.h \
		kernel/global.h kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c device/console.h lib/kernel/print.h lib/stdint.h \
	thread/sync.h thread/thread.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c device/ioqueue.h \
	kernel/interrupt.h kernel/global.h kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c device/keyboard.h device/ioqueue.h \
	lib/stdint.h lib/kernel/io.h lib/kernel/print.h kernel/interrupt.h kernel/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprog/tss.c userprog/tss.h  lib/stdint.h kernel/global.h lib/string.h \
	lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o: userprog/process.c userprog/process.h lib/stdint.h lib/kernel/io.h lib/kernel/print.h userprog/tss.h \
	kernel/global.h kernel/memory.h thread/thread.h lib/kernel/list.h kernel/interrupt.h device/console.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h \
	lib/stdint.h kernel/init.h kernel/memory.h thread/thread.h userprog/process.h \
	device/console.h
	$(CC) $(CFLAGS) $< -o $@





## 链接所有目标文件

$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

mk_dir:
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR);fi

hd:
	dd if=$(BUILD_DIR)/kernel.bin \
	   of=/opt/bochs/hd60M.img \
	   bs=512 count=200 seek=9 conv=notrunc

clean:
	cd $(BUILD_DIR) && rm -rf ./*

build: $(BUILD_DIR)/kernel.bin

all: mk_dir clean build hd

