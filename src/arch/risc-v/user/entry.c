#include "types.h"
#include "syscall.h"
#include "defs.h"

__attribute__((weak)) u64 main() {
  panic("No main linked!\n");
  return 1;
}

extern void init_heap();

void _start(u8 _args, u8 *_argv) {
  init_heap();
  sys_exit(main());
}