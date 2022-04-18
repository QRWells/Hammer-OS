#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

extern void kernel_start();
extern void text_start();
extern void rodata_start();
extern void data_start();
extern void bss_start();
extern void kernel_end();

// Constants for heap management
#define KERNEL_HEAP_SIZE 0x800000 // size of kernel heap in bytes
#define MIN_BLOCK_SIZE 0x40       // minimum block size in bytes
#define HEAP_BLOCK_NUM 0x20000    // total number of blocks in heap
#define BUDDY_NODE_NUM 0x3ffff    // total number of buddy nodes

#define KERNEL_MAP_OFFSET 0xffffffff00000000
#define KERNEL_PAGE_OFFSET 0xffffffff00000

#define PDE_MASK 0x003ffffffffffC00

#define KERNEL_STACK_SIZE 0x80000
#define USER_STACK_SIZE 0x80000
#define USER_STACK_OFFSET 0xffffffff00000000

#define MAX_THREAD 0x40

#endif // _CONSTANTS_H_