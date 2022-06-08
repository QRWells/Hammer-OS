#include "syscall.h"
#include "types.h"
#include "ulib.h"

#define LF 0x0au
#define CR 0x0du
#define BS 0x08u
#define DL 0x7fu
#define CTRLC 0x03u

int is_empty(char *line, int length) {
  for (int i = 0; i < length; i++) {
    if (line[i] == 0)
      break;
    if (line[i] != ' ' && line[i] != '\t') {
      return 0;
    }
  }
  return 1;
}

void empty(char *line, int length) {
  for (int i = 0; i < length; i++) {
    line[i] = 0;
  }
}

int test_str(char *str, char *target) {
  int target_len = strlen(target);
  int str_len = strlen(str);
  if (str_len >= target_len) {
    for (int i = 0; i < target_len; i++) {
      if (str[i] != target[i]) {
        return 0;
      }
    }
    if (str[target_len] != ' ' && str[target_len] != '\t' &&
        str[target_len] != '\0')
      return 0;
  }
  return 1;
}

int is_builtin(char *line, int fd) {
  if (!strcmp("shutdown", line)) {
    sys_shut();
    return 1;
  }
  // int len = strlen(line);
  if (test_str(line, "ls")) {
    line += 3;
    while (*line == ' ' || *line == '\t')
      line++;
    sys_lsdir(line, fd);
    return 1;
  }
  if (test_str(line, "cd")) {
    line += 3;
    while (*line == ' ' || *line == '\t')
      line++;
    if (*line == 0) {
      printf("cd: need a path\n");
      return 1;
    }
    sys_cddir(line, fd);
    return 1;
  }
  if (test_str(line, "pwd")) {
    sys_pwd(fd);
    printf("\n");
    return 1;
  }
  return 0;
}

u64 main() {
  char line[256];
  int lineCount = 0;
  int fd = sys_open("/");
  printf("Welcome!\n");
  sys_pwd(fd);
  printf(" > ");
  while (1) {
    u8 c = getc();
    switch (c) {
    case LF:
    case CR:
      printf("\n");
      if (!is_empty(line, 256)) {
        char *strip = line;
        while (*strip == ' ' || *strip == '\t')
          strip++;
        if (!is_builtin(strip, fd)) {
          sys_exec(strip, fd);
        }
        lineCount = 0;
        empty(line, 256);
      }
      sys_pwd(fd);
      printf(" > ");
      break;
    case CTRLC:
      printf("\n");
      lineCount = 0;
      empty(line, 256);
      sys_pwd(fd);
      printf(" > ");
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