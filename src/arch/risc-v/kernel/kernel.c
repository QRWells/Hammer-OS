#include "defs.h"

void main() {
  printf("Hello, World!\n");
  init_memory();
  init_interrupt();
  init_timer();
  map_kernel();
  asm volatile("ebreak" :::);
  printf("return from ebreak\n");
  init_thread();
  while (1)
    ;
}