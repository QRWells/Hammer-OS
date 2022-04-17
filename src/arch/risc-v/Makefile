K=kernel
L=lib
I=include

OBJS =                	\
    $K/entry.o        	\
	$K/sbi.o			\
    $K/kernel.o			\
	$K/interrupt.o	 	\
	$K/timer.o			\
	$K/heap.o			\
	$K/memory.o			\
	$K/mapping.o		\
	$K/thread.o			\
	$K/processor.o 		\
	$L/printf.o			\

TOOLPREFIX := riscv64-linux-gnu-
ifeq ($(shell uname),Darwin)
    TOOLPREFIX=riscv64-unknown-elf-
endif
CC = $(TOOLPREFIX)gcc
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

kernel: $(subst .c,.o,$(wildcard $K/*.c)) $(subst .S,.o,$(wildcard $K/*.S)) $(subst .c,.o,$(wildcard $L/*.c))
	$(LD) $(LDFLAGS) -T $K/linker.ld -o $K/kernel $(OBJS)
	$(OBJCOPY) $K/kernel -O binary image

# compile all .c file to .o file
$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# compile all .c file to .o file
$L/%.o: $L/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# compile all .S file to .o file
$K/%.o: $K/%.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f */*.d */*.o $K/kernel image image.asm
    
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
