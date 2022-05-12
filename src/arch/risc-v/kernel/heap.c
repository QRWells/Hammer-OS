#include "constants.h"
#include "defs.h"

#define IS_POWER_OF_2(x) (!((x) & ((x)-1)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define LEFT_LEAF(index) ((index)*2 + 1)
#define RIGHT_LEAF(index) ((index)*2 + 2)
#define PARENT(index) (((index) + 1) / 2 - 1)

static u8 HEAP[KERNEL_HEAP_SIZE];

struct {
  u32 size;
  u32 longest[BUDDY_NODE_NUM];
} buddy_tree;

int fix_size(int size) {
  int n = size - 1;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return n + 1;
}

// buddy algorithm
void buddy_init(int size) {
  buddy_tree.size = size;
  u32 node_size = size << 1;
  // initialize all nodes to be free
  for (int i = 0; i < (size << 1) - 1; i++) {
    if (IS_POWER_OF_2(i + 1))
      node_size /= 2;

    buddy_tree.longest[i] = node_size;
  }
}

u32 buddy_alloc(u32 size) {
  u32 index = 0;
  u32 nodeSize;
  u32 offset;

  // align size to power of 2
  if (size <= 0)
    size = 1;
  else if (!IS_POWER_OF_2(size))
    size = fix_size(size);

  // there is no enough space
  if (buddy_tree.longest[0] < size) {
    panic("no enough space");
    return -1;
  }

  // find first fit node
  // for(nodeSize = buddy_tree.size; nodeSize != size; nodeSize /= 2) {
  //     if(buddy_tree.longest[LEFT_LEAF(index)] >= size) {
  //         index = LEFT_LEAF(index);
  //     } else {
  //         index = RIGHT_LEAF(index);
  //     }
  // }

  // find the best fit node
  for (nodeSize = buddy_tree.size; nodeSize != size; nodeSize /= 2) {
    u32 left = buddy_tree.longest[LEFT_LEAF(index)];
    u32 right = buddy_tree.longest[RIGHT_LEAF(index)];
    if (left <= right) {
      if (left >= size)
        index = LEFT_LEAF(index);
      else
        index = RIGHT_LEAF(index);
    } else {
      if (right >= size)
        index = RIGHT_LEAF(index);
      else
        index = LEFT_LEAF(index);
    }
  }

  // mark node as used
  buddy_tree.longest[index] = 0;
  // get the offset of the first node on the heap
  offset = (index + 1) * nodeSize - buddy_tree.size;

  // update parent node
  while (index) {
    index = PARENT(index);
    buddy_tree.longest[index] = MAX(buddy_tree.longest[LEFT_LEAF(index)],
                                    buddy_tree.longest[RIGHT_LEAF(index)]);
  }

  return offset;
}

void buddy_free(u32 offset) {
  u32 nodeSize, index = 0;

  nodeSize = 1;
  index = offset + buddy_tree.size - 1;

  // find the node to be freed
  for (; buddy_tree.longest[index]; index = PARENT(index)) {
    nodeSize *= 2;
    if (index == 0) {
      return;
    }
  }
  buddy_tree.longest[index] = nodeSize;

  // merge nodes
  while (index) {
    index = PARENT(index);
    nodeSize *= 2;

    u32 leftLongest, rightLongest;
    leftLongest = buddy_tree.longest[LEFT_LEAF(index)];
    rightLongest = buddy_tree.longest[RIGHT_LEAF(index)];

    if (leftLongest + rightLongest == nodeSize) {
      buddy_tree.longest[index] = nodeSize;
    } else {
      buddy_tree.longest[index] = MAX(leftLongest, rightLongest);
    }
  }
}

// test malloc
void heap_test() {
  printf("Heap:\t%p\n", HEAP);
  void *a = kalloc(100);
  printf("a:\t%p\n", a);
  void *b = kalloc(60);
  printf("b:\t%p\n", b);
  void *c = kalloc(100);
  printf("c:\t%p\n", c);
  kfree(a);
  void *d = kalloc(30);
  printf("d:\t%p\n", d);
  kfree(b);
  kfree(d);
  kfree(c);
  a = kalloc(60);
  printf("a:\t%p\n", a);
  kfree(a);
}

void init_heap() {
  buddy_init(HEAP_BLOCK_NUM);
  // heap_test();
}

void *kalloc(u32 size) {
  if (size == 0)
    return 0;

  // how many blocks are needed
  u32 n = size / MIN_BLOCK_SIZE + ((size & (MIN_BLOCK_SIZE - 1)) != 0 ? 1 : 0);
  u32 block = buddy_alloc(n);
  if (block == -1)
    panic("Failed to allocate memory!\n");

  // clear part of memory space
  u32 total = fix_size(n) * MIN_BLOCK_SIZE;
  u8 *begin = (u8 *)((usize)HEAP + (usize)(block * MIN_BLOCK_SIZE));
  for (u32 i = 0; i < total; i++)
    begin[i] = 0;

  return (void *)begin;
}

void kfree(void *ptr) {
  // if ptr is valid
  if ((usize)ptr < (usize)HEAP)
    return;
  if ((usize)ptr >= (usize)HEAP + KERNEL_HEAP_SIZE - MIN_BLOCK_SIZE)
    return;
  // offset of the block in the heap
  u32 offset = (usize)((usize)ptr - (usize)HEAP);
  buddy_free(offset / MIN_BLOCK_SIZE);
}