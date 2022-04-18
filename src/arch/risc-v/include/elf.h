#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"

// ELF magic number
#define ELF_MAGIC 0x464C457FU

// ELF header
typedef struct {
  u32 magic;
  u8 elf[12];
  u16 type;
  u16 machine;
  u32 version;
  u64 entry;
  u64 phoff;
  u64 shoff;
  u32 flags;
  u16 ehsize;
  u16 phentsize;
  u16 phnum;
  u16 shentsize;
  u16 shnum;
  u16 shstrndx;
} elf_header;

// program segment header
typedef struct {
  u32 type;
  u32 flags;
  u64 off;
  u64 vaddr;
  u64 paddr;
  u64 filesz;
  u64 memsz;
  u64 align;
} prog_header;

// program segment header type
#define ELF_PROG_LOAD 1

// privilege level
#define ELF_PROG_FLAG_EXEC 1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ 4

#endif