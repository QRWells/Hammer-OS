#ifndef _SIMPLE_FS_H_
#define _SIMPLE_FS_H_

#include "types.h"

#define BLOCK_SIZE 4096
#define MAGIC_NUM 0x4D534653U // MSFS

typedef struct {
  u32 magic;         // magic number
  u32 blocks;        // total number of blocks of disk
  u32 unusedBlocks;  // unused blocks
  u32 freemapBlocks; // freemap blocks
  u8 info[32];       // other info
} super_block;

#define TYPE_FILE 0
#define TYPE_DIR 1

typedef struct {
  u32 size;        // size of file, 0 for directory
  u32 type;        // type of file
  u8 filename[32]; // file name
  u32 blocks;      // blocks used by file
  u32 direct[12];  // direct blocks
  u32 indirect;    // indirect blocks
} inode;

#endif // _SIMPLE_FS_H_