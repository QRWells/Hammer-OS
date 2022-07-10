#include "defs.h"

asm(".include \"kernel/entry.asm\"");
asm(".include \"kernel/link_fs.asm\"");

volatile int INIT = 0;
volatile int OTHER_INIT = 0;

void other_main(usize hart_id) {
  init_interrupt();
  init_memory_other();
  init_timer();
  run_cpu();
}

void main(usize hart_id) {
  printf("Hello, World!\n");

  if (__cmpxchg(&INIT, 0, 1, 4) == 0) {
    for (int i = 0; i < 8; ++i) {
      sbi_hart_start(i, 0x80200000u, i);
    }
  }

  if (hart_id != 0) {
    while (OTHER_INIT == 0)
      ;
    other_main(hart_id);
  }
  init_interrupt();
  init_memory();
  init_timer();
  init_fs();
  init_thread();
  run_cpu();
  while (1)
    ;
}