#ifndef _MAPPING_H__
#define _MAPPING_H__

#include "types.h"
#include "memory.h"

typedef usize pte;

typedef struct {
  pte entries[PAGE_SIZE >> 3];
} page_table;

#define PTE_DIRTY (1 << 7)
#define PTE_ACCESSED (1 << 6)
#define PTE_GLOBAL (1 << 5)
#define PTE_USER (1 << 4)
#define PTE_EXECUTABLE (1 << 3)
#define PTE_WRITABLE (1 << 2)
#define PTE_READABLE (1 << 1)
#define PTE_VALID (1 << 0)

typedef struct {
  usize start_va;
  usize end_va;
  usize flags;
} segment;

// mapping of process memory
typedef struct {
  usize root_ppn;
} mapping;

#endif // _MAPPING_H__