#include "riscv.h"
#include "defs.h"

static usize TICKS = 0;
static const usize INTERVAL = 100000;

void setTimeout() { set_timer(r_time() + INTERVAL); }

void init_timer() {
  w_sie(r_sie() | SIE_STIE);
  w_sstatus(r_sstatus() | SSTATUS_SIE);
  setTimeout();
}

void tick() {
  setTimeout();
  TICKS += 1;
  if (TICKS % 100 == 0) {
    printf("** %d ticks **\n", TICKS);
  }
}