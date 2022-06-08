#include "types.h"
#include "ulib.h"
#include "syscall.h"

void test() {
  printf("Hello from new thread!\n");
  sys_exit(0); // TODO: implicitly call it.
}

u64 main() {
  printf("Hello from thread main!\n");
  sys_create_thread(test);
  return 0;
}