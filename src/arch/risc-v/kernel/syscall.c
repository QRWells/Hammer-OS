#include "types.h"
#include "interrupt.h"
#include "defs.h"

const usize SYS_WRITE = 64;
const usize SYS_EXIT = 93;

usize syscall(usize id, usize args[3], interrupt_context *context) {
  switch (id) {
  case SYS_WRITE:
    console_putchar(args[0]);
    return 0;
  case SYS_EXIT:
    exit_from_cpu(args[0]);
    return 0;
  default:
    printf("Unknown syscall id %d\n", id);
    panic("");
    return -1;
  }
}