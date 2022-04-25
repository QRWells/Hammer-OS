#include "defs.h"
#include "syscall.h"
#include "types.h"

__attribute__((weak)) u64 main() {
  panic("No main linked!\n");
  return 1;
}

void _start(u8 _args, u8 *_argv) {
  extern void init_heap();
  init_heap();
  sys_exit(main());
}