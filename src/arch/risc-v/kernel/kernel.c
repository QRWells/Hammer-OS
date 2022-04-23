#include "defs.h"

asm(".include \"kernel/entry.asm\"");
asm(".include \"kernel/link_fs.asm\"");

void main() {
  printf("Hello, World!\n");
  init_interrupt();
  init_memory();
  init_fs();
  init_thread();
  init_timer();
  run_cpu();
  while (1)
    ;
}