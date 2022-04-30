#ifndef _FS_H_
#define _FS_H_

#include "simple_fs.h"
#include "types.h"

#define FD_NONE 0
#define FD_INODE 1

typedef struct {
  u8 fd_type;
  inode *inode;
  usize offset;
} file;

u64 get_block_addr(int block_num);
inode *lookup(inode *node, char *file_name);
void read_all(inode *node, char *buf);
void ls(inode *node);
char *get_inode_path(inode *inode, char path[256]);

#endif // _FS_H_