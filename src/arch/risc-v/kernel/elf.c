#include "../include/elf.h"
#include "mapping.h"
#include "defs.h"

mapping new_user_mapping(char *elf) {
  mapping m = new_kernel_mapping();
  elf_header *eHeader = (elf_header *)elf;
  // validate ELF header
  if (eHeader->magic != ELF_MAGIC) {
    panic("Unknown file type!");
  }
  prog_header *pHeader = (prog_header *)((usize)elf + eHeader->phoff);
  usize offset;
  int i;
  // traverse program header
  for (i = 0, offset = (usize)pHeader; i < eHeader->phnum;
       i++, offset += sizeof(prog_header)) {
    pHeader = (prog_header *)offset;
    if (pHeader->type != ELF_PROG_LOAD) {
      continue;
    }
    usize flags = convert_elf_flags(pHeader->flags);
    usize vhStart = pHeader->vaddr, vhEnd = vhStart + pHeader->memsz;
    segment segment = {vhStart, vhEnd, flags};
    char *source = (char *)((usize)elf + pHeader->off);
    map_framed_and_copy(m, segment, source, pHeader->filesz);
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