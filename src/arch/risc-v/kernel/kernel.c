#include "defs.h"

void main() {
  init_interrupt();
  init_timer();
  init_memory();
  asm volatile("ebreak" :::);
  printf("return from ebreak\n");
  while (1)
    ;
}