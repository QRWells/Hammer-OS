#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "constants.h"
#include "types.h"

// page size
#define PAGE_SIZE 4096

#define PGROUNDUP(sz) (((sz) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PAGE_SIZE - 1))

#define MEMORY_START_PADDR 0x80000000 // accessable memory start address
#define MEMORY_END_PADDR 0x88000000   // accessable memory end address
#define KERNEL_BEGIN_PADDR 0x80200000 // start of physical address of kernel
#define KERNEL_BEGIN_VADDR 0x80200000 // start of virtual address of kernel

#define MAX_PHYSICAL_PAGES 0x8000 // 0x80000000 ~ 0x88000000

// allocation interface
typedef struct {
  usize (*alloc)(void);
  void (*dealloc)(usize index);
} allocator;

typedef struct {
  // start of usable memory
  usize start_ppn;
  allocator allocator;
} frame_allocator;

#endif // _MEMORY_H_