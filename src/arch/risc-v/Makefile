K = kernel
U = user
I = include

S = scheduler

OBJS =                		\
	$K/sbi.o				\
    $K/kernel.o				\
	$K/interrupt.o	 		\
	$K/timer.o				\
	$K/heap.o				\
	$K/memory.o				\
	$K/mapping.o			\
	$K/thread.o				\
	$K/threadpool.o			\
	$K/processor.o 			\
	$K/printf.o				\
	$K/syscall.o			\
	$K/elf.o				\
	$K/fs.o					\
	$K/string.o				\
	$K/cond_var.o			\
	$K/stdin.o				\
	$K/$S/rr_scheduler.o 	\

USYSPROGS =                 \
    $U/entry.o       		\
    $U/malloc.o      		\
    $U/io.o 				\
	$U/string.o       		\

UPROGS =				  	\
    test 					\
	echo					\
	sh						\

TOOLPREFIX := riscv64-linux-gnu-
ifeq ($(shell uname),Darwin)
    TOOLPREFIX=riscv64-unknown-elf-
endif
CC = $(TOOLPREFIX)gcc
CXX = $(TOOLPREFIX)g++
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

QEMU = qemu-system-riscv64

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I$I
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

LDFLAGS = -z max-page-size=4096

QEMUOPTS = -machine virt -bios default -device loader,file=image,addr=0x80200000 --nographic

all: image

image: kernel

kernel: User $(subst .c,.o,$(wildcard $K/*.c)) $(subst .c,.o,$(wildcard $K/$S/*.c)) $(subst .S,.o,$(wildcard $K/*.S))
	$(LD) $(LDFLAGS) -T $K/linker.ld -o $K/kernel $(OBJS)
	$(OBJCOPY) $K/kernel -O binary image

User: mksfs $(subst .c,.o,$(wildcard $U/*.c))
	mkdir -p rootfs/bin
	for file in $(UPROGS); do											\
		$(LD) $(LDFLAGS) -o rootfs/bin/$$file $(USYSPROGS) $U/$$file.o;	\
	done
	./mksfs

mksfs:
	gcc mkfs/mksfs.c -o mksfs -I$I
	

# compile all .c file to .o file
$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$K/$S/%.o: $K/$S/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$U/%.o: $U/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f */*.d */*.o */*/*.d */*/*.o $K/kernel $U/User image image.asm User mksfs fs.img
	rm -rf rootfs
    
asm: kernel
	$(OBJDUMP) -S $K/kernel > image.asm

qemu: image
	$(QEMU) $(QEMUOPTS)


GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

qemu-gdb: image asm
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)
