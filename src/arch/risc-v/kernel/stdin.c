#include "cond_var.h"
#include "defs.h"

struct {
  queue buf;
  cond_var pushed;
} STDIN;

void push_char(char ch) {
  push_back(&STDIN.buf, (usize)ch);
  notify_condition(&STDIN.pushed);
}

char pop_char() {
  while (1) {
    if (!is_empty(&STDIN.buf)) {
      char ret = (char)pop_front(&STDIN.buf);
      return ret;
    } else {
      wait_condition(&STDIN.pushed);
    }
  }
}