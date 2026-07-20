#ifndef _KERNEL_THREAD_H
#define _KERNEL_THREAD_H

#include <cpu/cpu.h>
#include <stdint.h>

typedef struct thread {
  uint64_t sp;
  uint64_t pc;
} thread_t;

#endif // _KERNEL_THREAD_H
