#ifndef _DEVICE_TREE_H_
#define _DEVICE_TREE_H_

#include "types.h"

typedef struct {
  u32 magic_number;
  u32 totalsize;
  u32 off_dt_struct;
  u32 off_dt_strings;
  u32 off_mem_rsvmap;
  u32 version;
  u32 last_comp_version;
  u32 boot_cpuid_phys;
  u32 size_dt_strings;
  u32 size_dt_struct;
} device_tree;

static inline u32 read_u32(u8 *addr, int offset) {
  // read in big endian
  return ((u32)addr[offset] << 24) | ((u32)addr[offset + 1] << 16) |
         ((u32)addr[offset + 2] << 8) | ((u32)addr[offset + 3]);
}

static inline device_tree load_device_tree(u8 *raw_data) {
  device_tree dt;

  dt.magic_number = read_u32(raw_data, 0);
  dt.totalsize = read_u32(raw_data, 4);
  dt.off_dt_struct = read_u32(raw_data, 8);
  dt.off_dt_strings = read_u32(raw_data, 12);
  dt.off_mem_rsvmap = read_u32(raw_data, 16);
  dt.version = read_u32(raw_data, 20);
  dt.last_comp_version = read_u32(raw_data, 24);
  dt.boot_cpuid_phys = read_u32(raw_data, 28);
  dt.size_dt_strings = read_u32(raw_data, 32);
  dt.size_dt_struct = read_u32(raw_data, 36);

  return dt;
}

#endif