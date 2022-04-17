#include "mapping.h"
#include "constants.h"
#include "defs.h"

usize *get_vpn_levels(usize vpn) {
  // use static variable to avoid stack allocation
  static usize res[3];
  res[0] = (vpn >> 18) & 0x1ff;
  res[1] = (vpn >> 9) & 0x1ff;
  res[2] = vpn & 0x1ff;
  return res;
}

usize pa_to_va(usize pa) { return pa + KERNEL_MAP_OFFSET; }

pte *find_entry(mapping self, usize vpn) {
  page_table *root_table = (page_table *)pa_to_va(self.root_ppn << 12);
  usize *levels = get_vpn_levels(vpn);
  pte *entry = &(root_table->entries[levels[0]]);
  int i;
  for (i = 1; i <= 2; i++) {
    if (*entry == 0) {
      // create new page table if not exist
      usize new_ppn = alloc_frame() >> 12;
      *entry = (new_ppn << 10) | PTE_VALID;
    }
    usize next_pa = (*entry & PDE_MASK) << 2;
    entry = &(((page_table *)pa_to_va(next_pa))->entries[levels[i]]);
  }
  return entry;
}

mapping new_mapping() {
  usize root_pa = alloc_frame();
  mapping m = {root_pa >> 12};
  return m;
}

// active page table
void activate_mapping(mapping self) {
  usize satp = self.root_ppn | (8L << 60);
  asm volatile("csrw satp, %0" : : "r"(satp));
  asm volatile("sfence.vma" :::);
}

void map_linear_segment(mapping self, segment segment) {
  usize start_vpn = segment.start_va / PAGE_SIZE;
  usize end_vpn = (segment.end_va - 1) / PAGE_SIZE + 1;
  usize vpn;
  for (vpn = start_vpn; vpn < end_vpn; vpn++) {
    pte *entry = find_entry(self, vpn);
    if (*entry != 0) {
      panic("Virtual address already mapped!\n");
    }
    *entry = ((vpn - KERNEL_PAGE_OFFSET) << 10) | segment.flags | PTE_VALID;
  }
}

mapping new_kernel_mapping() {
  mapping m = new_mapping();

  // .text, r-x
  segment text = {(usize)text_start, (usize)rodata_start,
                  1L | PTE_READABLE | PTE_EXECUTABLE};
  map_linear_segment(m, text);

  // .rodata, r--
  segment rodata = {(usize)rodata_start, (usize)data_start, 1L | PTE_READABLE};
  map_linear_segment(m, rodata);

  // .data, rw-
  segment data = {(usize)data_start, (usize)bss_start,
                  1L | PTE_READABLE | PTE_WRITABLE};
  map_linear_segment(m, data);

  // .bss, rw-
  segment bss = {(usize)bss_start, (usize)kernel_end,
                 1L | PTE_READABLE | PTE_WRITABLE};
  map_linear_segment(m, bss);

  // rest space, rw-
  segment other = {(usize)kernel_end,
                   (usize)(MEMORY_END_PADDR + KERNEL_MAP_OFFSET),
                   1L | PTE_READABLE | PTE_WRITABLE};
  map_linear_segment(m, other);

  return m;
}

void map_kernel() {
  mapping m = new_kernel_mapping();
  activate_mapping(m);
  printf("***** Remap Kernel *****\n");
}