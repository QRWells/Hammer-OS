#include "defs.h"
#include "riscv.h"

static const usize INTERVAL = 100000;

void setTimeout() { set_timer(r_time() + INTERVAL); }

void init_timer() {
  w_sie(r_sie() | SIE_STIE);
  w_sstatus(r_sstatus() | SSTATUS_SIE);
  setTimeout();
}

void tick() { setTimeout(); }