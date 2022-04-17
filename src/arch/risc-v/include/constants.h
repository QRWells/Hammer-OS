#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Constants for heap management
#define KERNEL_HEAP_SIZE 0x800000 // size of kernel heap in bytes
#define MIN_BLOCK_SIZE 0x40       // minimum block size in bytes
#define HEAP_BLOCK_NUM 0x20000    // total number of blocks in heap
#define BUDDY_NODE_NUM 0x3ffff    // total number of buddy nodes

#endif // _CONSTANTS_H_