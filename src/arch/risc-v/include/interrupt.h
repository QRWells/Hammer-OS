#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include "types.h"

// interrupt context, scause and stval as parameters, no need to save
typedef struct
{
    usize x[32];
    usize sstatus;
    // interrupt return address
    usize sepc;
} interrupt_context;

#endif // _INTERRUPT_H_