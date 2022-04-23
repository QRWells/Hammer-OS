#include "fs.h"
#include "defs.h"

inode *ROOT_INODE;
char *FREEMAP;

extern void _fs_img_start();

usize get_block_addr(int block_num) {
  void *addr = (void *)_fs_img_start;
  addr += (block_num * BLOCK_SIZE);
  return (usize)addr;
}

void init_fs() {
  FREEMAP = (char *)get_block_addr(1);
  super_block *spBlock = (super_block *)_fs_img_start;
  ROOT_INODE = (inode *)get_block_addr(spBlock->freemapBlocks + 1);
}

void copy_byte_to_buf(char *src, char *dst, int length) {
  int i;
  for (i = 0; i < length; i++)
    dst[i] = src[i];
}

inode *lookup(inode *node, char *file_name) {
  if (file_name[0] == '/') {
    node = ROOT_INODE;
    file_name++;
  }

  if (node == 0)
    node = ROOT_INODE;

  if (*file_name == '\0')
    return node;

  if (node->type != TYPE_DIR)
    return 0;

  char cTarget[strlen(file_name) + 1];
  int i = 0;

  while (*file_name != '/' && *file_name != '\0') {
    cTarget[i] = *file_name;
    file_name++;
    i++;
  }

  cTarget[i] = '\0';
  if (*file_name == '/')
    file_name++;

  if (!strcmp(".", cTarget))
    return lookup(node, file_name);

  if (!strcmp("..", cTarget)) {
    inode *upLevel = (inode *)get_block_addr(node->direct[1]);
    return lookup(upLevel, file_name);
  }

  int blockNum = node->blocks;
  if (blockNum <= 12) {
    for (i = 2; i < blockNum; i++) {
      inode *candidate = (inode *)get_block_addr(node->direct[i]);
      if (!strcmp((char *)candidate->filename, cTarget))
        return lookup(candidate, file_name);
    }
    return 0;
  } else {
    for (i = 2; i < 12; i++) {
      inode *candidate = (inode *)get_block_addr(node->direct[i]);
      if (!strcmp((char *)candidate->filename, cTarget))
        return lookup(candidate, file_name);
    }
    u32 *indirect = (u32 *)get_block_addr(node->indirect);
    for (i = 12; i < blockNum; i++) {
      inode *candidate = (inode *)get_block_addr(indirect[i - 12]);
      if (!strcmp((char *)candidate->filename, cTarget))
        return lookup(candidate, file_name);
    }
    return 0;
  }
}

void read_all(inode *node, char *buf) {
  if (node->type != TYPE_FILE) {
    panic("Cannot read a directory!\n");
  }
  int l = node->size, b = node->blocks;
  if (b <= 12) {
    int i;
    for (i = 0; i < b; i++) {
      char *src = (char *)get_block_addr(node->direct[i]);
      int copySize = l >= 4096 ? 4096 : l;
      copy_byte_to_buf(src, buf, copySize);
      buf += copySize;
      l -= copySize;
    }
  } else {
    int i;
    for (i = 0; i < 12; i++) {
      char *src = (char *)get_block_addr(node->direct[i]);
      int copySize = l >= 4096 ? 4096 : l;
      copy_byte_to_buf(src, buf, copySize);
      buf += copySize;
      l -= copySize;
    }
    u32 *indirect = (u32 *)get_block_addr(node->indirect);
    for (i = 0; i < b - 12; i++) {
      char *src = (char *)get_block_addr(indirect[i]);
      int copySize = l >= 4096 ? 4096 : l;
      copy_byte_to_buf(src, buf, copySize);
      buf += copySize;
      l -= copySize;
    }
  }
}