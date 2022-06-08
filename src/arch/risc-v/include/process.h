#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "constants.h"
#include "interrupt.h"
#include "types.h"
#include "fs.h"

typedef struct {
  usize pid;
  usize satp;
  file fd[16];
  u8 fd_occupied[16];
} process;

#endif