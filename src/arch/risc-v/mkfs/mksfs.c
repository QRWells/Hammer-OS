#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "simple_fs.h"

// Declare the file system with 1MB of space
#define BLOCK_NUM 256
// Freemap block
#define FREEMAP_NUM 1

// temp free map
u32 free_num = BLOCK_NUM;
char free_map[BLOCK_NUM];

char *rootdir = "rootfs";

// file system's final image
char image[BLOCK_SIZE * BLOCK_NUM];

void main() {
  free_map[0] = 1;
  int i;
  for (i = 0; i < FREEMAP_NUM; i++)
    free_map[1 + i] = 1;
  free_map[FREEMAP_NUM + 1] = 1;
  free_num -= (FREEMAP_NUM + 2);

  super_block s_block;
  s_block.magic = MAGIC_NUM;
  s_block.blocks = BLOCK_NUM;
  s_block.freemapBlocks = FREEMAP_NUM;
  char *info = "SimpleFS";
  for (i = 0; i < strlen(info); i++) {
    s_block.info[i] = info[i];
  }
  s_block.info[i] = '\0';

  // setup root inode
  inode root_inode;
  root_inode.size = 0;
  root_inode.type = TYPE_DIR;
  root_inode.filename[0] = '/';
  root_inode.filename[1] = '\0';

  walk(rootdir, &root_inode, FREEMAP_NUM + 1);

  s_block.unusedBlocks = free_num;

  char *ptr = (char *)get_block_addr(0), *src = (char *)&s_block;
  for (i = 0; i < sizeof(s_block); i++) {
    ptr[i] = src[i];
  }

  ptr = (char *)get_block_addr(1);
  for (i = 0; i < BLOCK_NUM / 8; i++) {
    char c = 0;
    int j;
    for (j = 0; j < 8; j++) {
      if (free_map[i * 8 + j]) {
        c |= (1 << j);
      }
    }
    *ptr = c;
    ptr++;
  }

  copy_inode_to_block(FREEMAP_NUM + 1, &root_inode);

  FILE *img = fopen("fs.img", "w+b");
  fwrite(image, sizeof(image), 1, img);
  fflush(img);
  fclose(img);
}

void walk(char *dir_name, inode *now_inode, u32 now_inode_num) {
  // open pwd
  DIR *dp = opendir(dir_name);
  struct dirent *dirp;

  // first file is "."
  now_inode->direct[0] = now_inode_num;
  if (!strcmp(dir_name, rootdir)) {
    now_inode->direct[1] = now_inode_num;
  }
  int empty_index = 2;

  while ((dirp = readdir(dp))) {
    if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) {
      continue;
    }
    int block_num;
    if (dirp->d_type == DT_DIR) {
      inode dinode;
      dinode.size = 0;
      dinode.type = TYPE_DIR;
      int i;
      for (i = 0; i < strlen(dirp->d_name); i++) {
        dinode.filename[i] = dirp->d_name[i];
      }
      dinode.filename[i] = '\0';
      block_num = get_free_block();
      dinode.direct[0] = block_num;
      dinode.direct[1] = now_inode_num;
      char *tmp = (char *)malloc(strlen(dir_name) + strlen(dirp->d_name) + 1);
      sprintf(tmp, "%s/%s", dir_name, dirp->d_name);
      walk(tmp, &dinode, block_num);

      copy_inode_to_block(block_num, &dinode);
    } else if (dirp->d_type == DT_REG) {
      inode finode;
      finode.type = TYPE_FILE;
      int i;
      for (i = 0; i < strlen(dirp->d_name); i++) {
        finode.filename[i] = dirp->d_name[i];
      }
      finode.filename[i] = '\0';
      char *tmp = (char *)malloc(strlen(dir_name) + strlen(dirp->d_name) + 1);
      sprintf(tmp, "%s/%s", dir_name, dirp->d_name);
      struct stat buf;
      stat(tmp, &buf);
      finode.size = buf.st_size;
      finode.blocks = (finode.size - 1) / BLOCK_SIZE + 1;

      block_num = get_free_block();

      u32 l = finode.size;
      int block_index = 0;
      FILE *fp = fopen(tmp, "rb");
      while (l) {
        int ffb = get_free_block();
        char *buffer = (char *)get_block_addr(ffb);
        size_t size;
        if (l > BLOCK_SIZE)
          size = BLOCK_SIZE;
        else
          size = l;
        fread(buffer, size, 1, fp);
        l -= size;
        if (block_index < 12) {
          finode.direct[block_index] = ffb;
        } else {
          if (finode.indirect == 0) {
            finode.indirect = get_free_block();
          }
          u32 *inaddr = (u32 *)get_block_addr(finode.indirect);
          inaddr[block_index - 12] = ffb;
        }
        block_index++;
      }
      fclose(fp);
      copy_inode_to_block(block_num, &finode);
    } else {
      continue;
    }

    if (empty_index < 12) {
      now_inode->direct[empty_index] = block_num;
    } else {
      if (now_inode->indirect == 0) {
        now_inode->indirect = get_free_block();
      }
      u32 *inaddr = (u32 *)get_block_addr(now_inode->indirect);
      inaddr[empty_index - 12] = block_num;
    }
    empty_index++;
  }
  closedir(dp);
  now_inode->blocks = empty_index;
}

u64 get_block_addr(int block_num) {
  void *addr = (void *)image;
  addr += (block_num * BLOCK_SIZE);
  return (u64)addr;
}

int get_free_block() {
  int i;
  for (i = 0; i < BLOCK_NUM; i++) {
    if (!free_map[i]) {
      free_map[i] = 1;
      --free_num;
      return i;
    }
  }
  printf("get free block failed!\n");
  exit(1);
}

void copy_inode_to_block(int block_num, inode *in) {
  inode *dst = (inode *)get_block_addr(block_num);
  *dst = *in;
}