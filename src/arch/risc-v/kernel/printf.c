#include <stdarg.h>
#include "types.h"
#include "defs.h"
#include "memory.h"
#include "riscv.h"

static char digits[] = "0123456789abcdef";

static void printint(int xx, int base, int sign) {
  char buf[16];
  int i;
  u32 x;

  if (sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    console_putchar(buf[i]);
}

static void printptr(u64 x) {
  console_putchar('0');
  console_putchar('x');
  for (int i = 0; i < (sizeof(u64) * 2); i++, x <<= 4)
    console_putchar(digits[x >> (sizeof(u64) * 8 - 4)]);
}

void printf(char *fmt, ...) {
  va_list ap;
  int i, c;
  char *s;

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    if (c != '%') {
      console_putchar(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
      break;
    switch (c) {
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, u64));
      break;
    case 's':
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      for (; *s; s++)
        console_putchar(*s);
      break;
    case '%':
      console_putchar('%');
      break;
    default:
      console_putchar('%');
      console_putchar(c);
      break;
    }
  }
}

void backtrace() {
  u64 fp = r_fp();
  u64 top = PGROUNDUP(fp);
  printf("backtrace:\n");
  while (fp != top) {
    printf("%p\n", *(u64 *)(fp - 8));
    fp = *(u64 *)(fp - 16);
  }
}