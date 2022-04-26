#include "defs.h"
#include "interrupt.h"
#include "types.h"

const usize SYS_READ = 63;
const usize SYS_WRITE = 64;
const usize SYS_EXIT = 93;
const usize SYS_EXEC = 221;

usize syscall(usize id, usize args[3], interrupt_context *context) {
  switch (id) {
  case SYS_WRITE:
    console_putchar(args[0]);
    return 0;
  case SYS_EXIT:
    exit_from_cpu(args[0]);
    return 0;
  case SYS_READ:
    return sys_read(args[0], (u8 *)args[1], args[2]);
  case SYS_EXEC:
    sys_exec((char *)args[0]);
    return 0;
  default:
    printf("Unknown syscall id %d\n", id);
    panic("");
    return -1;
  }
}

usize sys_read(usize fd, u8 *base, usize len) {
  *base = (u8)pop_char();
  return 1;
}

usize sys_exec(char *path) {
  if (execute_cpu(path, get_current_tid()))
    yield_cpu();

  return 0;
}