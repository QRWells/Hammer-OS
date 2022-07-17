#include "defs.h"
#include "riscv.h"
#include "device_tree.h"

asm(".include \"kernel/entry.asm\"");
asm(".include \"kernel/link_fs.asm\"");

volatile int OTHER_INIT = 0;
volatile int FIRST_HART = 0;

void start_all_harts() {
  for (int i = 0; i < MAX_CPU; ++i) {
    sbi_hart_start(i, 0x80200000u, i);
  }
}

void other_main(usize hart_id) {
  printf("Hello, World at hart %d!\n", hart_id);
  init_interrupt();
  init_memory_other();
  init_timer();
  init_thread_other();
  run_cpu();
  while (1)
    ;
}

// OpenSBI pass hart_id in a0, and the device_tree_addr in a1.
void main(usize hart_id, u8* device_tree_addr) {
  w_tp(hart_id);

  if (__sync_bool_compare_and_swap(&FIRST_HART, 0, 1)) {
    start_all_harts();
  }

  if (hart_id != 0) {
    while (OTHER_INIT == 0)
      ;
    other_main(hart_id);
  }

  device_tree dt = load_device_tree(device_tree_addr);

  init_memory();
  init_interrupt();
  init_timer();
  init_fs();

  if (dt.magic_number == 0xd00dfeed)
    printf("Hello, World at hart %d!\n", hart_id);

  init_thread();
  __sync_synchronize();
  OTHER_INIT = 1;
  run_cpu();
  while (1)
    ;
}