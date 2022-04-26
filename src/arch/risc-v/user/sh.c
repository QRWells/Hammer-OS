#include "syscall.h"
#include "types.h"
#include "ulib.h"

#define LF 0x0au
#define CR 0x0du
#define BS 0x08u
#define DL 0x7fu

int isEmpty(char *line, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (line[i] == 0)
      break;
    if (line[i] != ' ' && line[i] != '\t') {
      return 0;
    }
  }
  return 1;
}

void empty(char *line, int length) {
  int i;
  for (i = 0; i < length; i++) {
    line[i] = 0;
  }
}

u64 main() {
  char line[256];
  int lineCount = 0;
  printf("Welcome to Hammer-OS!\n");
  printf("$ ");
  while (1) {
    u8 c = getc();
    switch (c) {
    case LF:
    case CR:
      printf("\n");
      if (!isEmpty(line, 256)) {
        sys_exec(line, 0);
        lineCount = 0;
        empty(line, 256);
      }
      printf("$ ");
      break;
    case DL:
      if (lineCount > 0) {
        putchar(BS);
        putchar(' ');
        putchar(BS);
        line[lineCount - 1] = 0;
        lineCount -= 1;
      }
      break;
    default:
      if (lineCount < 255) {
        line[lineCount] = c;
        lineCount += 1;
        putchar(c);
      }
      break;
    }
  }
}
