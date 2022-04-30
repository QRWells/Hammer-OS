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
  for (int i = 0; i < length; i++)
    dst[i] = src[i];
}

inode *lookup(inode *node, char *filename) {
  if (filename[0] == '/') {
    node = ROOT_INODE;
    filename++;
  }
  if (node == 0)
    node = ROOT_INODE;
  if (*filename == '\0')
    return node;
  if (node->type != TYPE_DIR)
    return 0;
  char cTarget[strlen(filename) + 1];
  int i = 0;
  while (*filename != '/' && *filename != '\0') {
    cTarget[i] = *filename;
    filename++;
    i++;
  }
  cTarget[i] = '\0';

  if (*filename == '/')
    filename++;
  int blockNum = node->blocks;
  if (blockNum <= 12) {
    for (i = 0; i < blockNum; i++) {
      inode_item candidateItem = node->direct[i];
      if (!strcmp((char *)candidateItem.filename, cTarget)) {
        inode *candidate = (inode *)get_block_addr(node->direct[i].block);
        return lookup(candidate, filename);
      }
    }
    return 0;
  } else {
    for (i = 0; i < 12; i++) {
      inode_item candidateItem = node->direct[i];
      if (!strcmp((char *)candidateItem.filename, cTarget)) {
        inode *candidate = (inode *)get_block_addr(node->direct[i].block);
        return lookup(candidate, filename);
      }
    }
    inode_item *indirect = (inode_item *)get_block_addr(node->indirect);
    for (i = 12; i < blockNum; i++) {
      inode_item candidateItem = indirect[i];
      if (!strcmp((char *)candidateItem.filename, cTarget)) {
        inode *candidate = (inode *)get_block_addr(node->direct[i].block);
        return lookup(candidate, filename);
      }
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
      char *src = (char *)get_block_addr(node->direct[i].block);
      int copySize = l >= 4096 ? 4096 : l;
      copy_byte_to_buf(src, buf, copySize);
      buf += copySize;
      l -= copySize;
    }
  } else {
    int i;
    for (i = 0; i < 12; i++) {
      char *src = (char *)get_block_addr(node->direct[i].block);
      int copySize = l >= 4096 ? 4096 : l;
      copy_byte_to_buf(src, buf, copySize);
      buf += copySize;
      l -= copySize;
    }
    inode_item *indirect = (inode_item *)get_block_addr(node->indirect);
    for (i = 0; i < b - 12; i++) {
      char *src = (char *)get_block_addr(indirect[i].block);
      int copySize = l >= 4096 ? 4096 : l;
      copy_byte_to_buf(src, buf, copySize);
      buf += copySize;
      l -= copySize;
    }
  }
}

void ls(inode *node) {
  if (node->type == TYPE_FILE) {
    printf("%s: is not a directory\n", node->filename);
    return;
  }
  if (node->blocks <= 12) {
    int i;
    for (i = 0; i < node->blocks; i++) {
      printf("%s ", node->direct[i].filename);
    }
  } else {
    int i;
    for (i = 0; i < 12; i++) {
      printf("%s ", node->direct[i].filename);
    }
    inode_item *indirect = (inode_item *)get_block_addr(node->indirect);
    for (i = 0; i < node->blocks - 12; i++) {
      printf("%s ", indirect[i].filename);
    }
  }
  printf("\n");
}

char *get_inode_path(inode *inode, char buf[256]) {
  char *ptr = buf;
  ptr += 256;
  *ptr = 0;
  ptr--;
  if (!strcmp((char *)inode->filename, "/")) {
    *ptr = '/';
    return ptr;
  }
  while (strcmp((char *)inode->filename, "/")) {
    int len = strlen((char *)inode->filename);
    int i;
    for (i = len - 1; i >= 0; i--) {
      *ptr = inode->filename[i];
      ptr--;
    }
    *ptr = '/';
    ptr--;
    inode = lookup(inode, "..");
  }
  return ++ptr;
}

int alloc_fd(thread *thread) {
  for (int i = 0; i < 16; i++)
    if (!thread->process.fd_occupied[i])
      return i;
  return -1;
}

void dealloc_fd(thread *thread, int fd) { thread->process.fd_occupied[fd] = 0; }