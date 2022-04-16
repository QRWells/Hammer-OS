#include "defs.h"

void main() {
  // printf("Hello from Hammer OS!\n");
  // panic("Nothing to do!");
  init_interrupt();
  asm volatile("ebreak" :::);
  printf("return from ebreak\n");
  while (1)
    ;
}