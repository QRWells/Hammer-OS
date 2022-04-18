#ifndef _ULIB_H_
#define _ULIB_H_

#include "types.h"

// io.c
u8 getc();
void printf(char *, ...);
void panic(char *);
void putchar(int c);

// malloc.c
void *malloc(u32 size);
void free(void *ptr);

#endif // _ULIB_H_