#include "types.h"
#include "sbi.h"

void consolePutchar(u64 c);
u64 consoleGetchar();
void shutdown();

void printf(char *fmt, ...);

void panic(char *s);