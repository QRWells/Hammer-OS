#include "../include/memory.h"
#include "defs.h"
#include "riscv.h"
#include "types.h"

frame_allocator frameAllocator;

allocator new_allocator(usize start_ppn, usize end_ppn);
usize alloc();
void dealloc(usize ppn);

// init global page frame allocator
void init_frame_allocator(usize start_ppn, usize end_ppn) {
  frameAllocator.start_ppn = start_ppn;
  frameAllocator.allocator = new_allocator(start_ppn, end_ppn);
}

// get start address of physical page
usize alloc_frame() {
  char *start = (char *)(alloc() << 12);
  char *v_start = (char *)(start + KERNEL_MAP_OFFSET);
  for (int i = 0; i < PAGE_SIZE; i++) {
    v_start[i] = 0;
  }
  return (usize)start;
}

// startAddr
void dealloc_frame(usize start_addr) { dealloc(start_addr >> 12); }

volatile usize SATP = 0;

void init_memory() {
  w_sstatus(r_sstatus() | SSTATUS_SUM);
  init_frame_allocator((((usize)(kernel_end)-KERNEL_MAP_OFFSET) >> 12) + 1,
                       MEMORY_END_PADDR >> 12);
  init_heap();
  map_kernel();
  __sync_synchronize();
  SATP = r_satp();
  printf("***** Init Memory *****\n");
}

void init_memory_other() {
  w_sstatus(r_sstatus() | SSTATUS_SUM);
  w_satp(SATP);
  asm volatile("sfence.vma" :::);
}

// implementations of allocator using segment tree

struct {
  u8 node[MAX_PHYSICAL_PAGES << 1];
  // index of the first node of a single block
  usize first_single;
  // length of the interval
  usize length;
  usize start_ppn;
} seg_tree;

allocator new_allocator(usize startPpn, usize endPpn) {
  seg_tree.start_ppn = startPpn - 1;
  seg_tree.length = endPpn - startPpn;
  seg_tree.first_single = 1;
  while (seg_tree.first_single < seg_tree.length + 2) {
    seg_tree.first_single <<= 1;
  }
  usize i = 1;
  for (i = 1; i < (seg_tree.first_single << 1); i++) {
    seg_tree.node[i] = 1;
  }
  for (i = 1; i < seg_tree.length; i++) {
    seg_tree.node[seg_tree.first_single + i] = 0;
  }
  for (i = seg_tree.first_single - 1; i >= 1; i--) {
    seg_tree.node[i] = seg_tree.node[i << 1] & seg_tree.node[(i << 1) | 1];
  }
  allocator ac = {alloc, dealloc};
  return ac;
}

usize alloc() {
  if (seg_tree.node[1] == 1) {
    panic("Physical memory depleted!\n");
  }
  usize p = 1;
  while (p < seg_tree.first_single) {
    if (seg_tree.node[p << 1] == 0) {
      p = p << 1;
    } else {
      p = (p << 1) | 1;
    }
  }
  usize result = p - seg_tree.first_single + seg_tree.start_ppn;
  seg_tree.node[p] = 1;
  p >>= 1;
  while (p >> 0) {
    seg_tree.node[p] = seg_tree.node[p << 1] & seg_tree.node[(p << 1) | 1];
    p >>= 1;
  }
  return result;
}

void dealloc(usize ppn) {
  usize p = ppn - seg_tree.start_ppn + seg_tree.first_single;
  if (seg_tree.node[p] != 1) {
    printf("The page is free, no need to dealloc!\n");
    return;
  }
  seg_tree.node[p] = 0;
  p >>= 1;
  while (p > 0) {
    seg_tree.node[p] = seg_tree.node[p << 1] & seg_tree.node[(p << 1) | 1];
    p >>= 1;
  }
}