#include "defs.h"

void main() {
  printf("Hello, World!\n");
  init_interrupt();
  init_memory();
  init_thread();
  init_timer();
  run_cpu();
  while (1)
    ;
}