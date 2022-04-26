#include "../include/elf.h"
#include "defs.h"
#include "mapping.h"

mapping new_user_mapping(char *elf) {
  mapping m = new_kernel_mapping();
  elf_header *e_header = (elf_header *)elf;
  // validate ELF header
  if (e_header->magic != ELF_MAGIC) {
    panic("Unknown file type!");
  }
  prog_header *p_Header = (prog_header *)((usize)elf + e_header->phoff);
  usize offset;
  int i;
  // traverse program header
  for (i = 0, offset = (usize)p_Header; i < e_header->phnum;
       i++, offset += sizeof(prog_header)) {
    p_Header = (prog_header *)offset;
    if (p_Header->type != ELF_PROG_LOAD) {
      continue;
    }
    usize flags = convert_elf_flags(p_Header->flags);
    usize vhStart = p_Header->vaddr, vhEnd = vhStart + p_Header->memsz;
    segment segment = {vhStart, vhEnd, flags};
    char *source = (char *)((usize)elf + p_Header->off);
    map_framed_and_copy(m, segment, source, p_Header->filesz);
  }
  return m;
}

// translate elf flags to pte flags
usize convert_elf_flags(u32 flags) {
  usize ma = 1L;
  ma |= PTE_USER;
  if (flags & ELF_PROG_FLAG_EXEC)
    ma |= PTE_EXECUTABLE;
  if (flags & ELF_PROG_FLAG_WRITE)
    ma |= PTE_WRITABLE;
  if (flags & ELF_PROG_FLAG_READ)
    ma |= PTE_READABLE;
  return ma;
}