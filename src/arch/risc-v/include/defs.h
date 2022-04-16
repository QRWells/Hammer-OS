#ifndef _DEFS_H_
#define _DEFS_H_

#include "types.h"
#include "sbi.h"

void consolePutchar(usize c);
usize consoleGetchar();
void shutdown();

void printf(char *fmt, ...);

void panic(char *s);

#endif // _DEFS_H_