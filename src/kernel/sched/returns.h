#ifndef _KERNEL_TASK_RETURNS_H
#define _KERNEL_TASK_RETURNS_H

#include <stdint.h>

// these are all architecture implementation defined

extern void __kfork_return(void);
extern void __ufork_return(void);
extern void __attribute__((noreturn)) __execve_return(uint64_t sp);

#endif // _KERNEL_TASK_RETURNS_H
