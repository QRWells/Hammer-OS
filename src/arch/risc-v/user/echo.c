#include "types.h"
#include "ulib.h"

#define LF 0x0au
#define CR 0x0du
#define BS 0x08u
#define DL 0x7fu

u64 main() {
  printf("Welcome to echo!\n");
  int line_count = 0;
  while (1) {
    u8 c = getc();
    switch (c) {
    case LF:
    case CR:
      line_count = 0;
      putchar(LF);
      putchar(CR);
      break;
    case DL:
      if (line_count > 0) {
        putchar(BS);
        putchar(' ');
        putchar(BS);
        line_count -= 1;
      }
      break;
    default:
      line_count += 1;
      putchar(c);
      break;
    }
  }
}