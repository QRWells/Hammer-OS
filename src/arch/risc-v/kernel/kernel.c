#include "defs.h"

asm(".include \"kernel/entry.asm\"");
asm(".include \"kernel/link_fs.asm\"");

void main() {
  printf("Hello, World!\n");
  init_memory();
  init_interrupt();
  init_fs();
  init_timer();
  init_thread();
  run_cpu();
  while (1)
    ;
}