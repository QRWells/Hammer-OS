#ifndef _FS_H_
#define _FS_H_

#include "types.h"
#include "simple_fs.h"

typedef struct {
  u8 filename[28];
  u32 block;
} inode_item;

u64 get_block_addr(int block_num);
inode *lookup(inode *node, char *file_name);
void read_all(inode *node, char *buf);
void ls(inode *node);
char *get_inode_path(inode *inode, char path[256]);

#endif // _FS_H_