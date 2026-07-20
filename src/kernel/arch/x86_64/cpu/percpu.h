#ifndef _KERNEL_PERCPU_H
#define _KERNEL_PERCPU_H

#include <macros.h>
#include <stdint.h>

extern char _percpu_start[];
extern char _percpu_end[];

#define DEFINE_PERCPU(type, name)                                              \
  __attribute__((section(".data..percpu"))) type name

#define DECLARE_PERCPU(type, name)                                             \
  extern __attribute__((section(".data..percpu"))) type name

#define PERCPU_OFFSET(var) ((uintptr_t)&(var) - (uintptr_t)_percpu_start)

#define this_cpu_read(var)                                                     \
  ({                                                                           \
    typeof(var) __v;                                                           \
    uintptr_t __off = PERCPU_OFFSET(var);                                      \
    __asm__ __volatile__("mov %%gs:(%1), %0"                                   \
                         : "=r"(__v)                                           \
                         : "r"(__off)                                          \
                         : "memory");                                          \
    __v;                                                                       \
  })

#define this_cpu_write(var, val)                                               \
  do {                                                                         \
    typeof(var) __v = (val);                                                   \
    uintptr_t __off = PERCPU_OFFSET(var);                                      \
    __asm__ __volatile__("mov %0, %%gs:(%1)"                                   \
                         :                                                     \
                         : "r"(__v), "r"(__off)                                \
                         : "memory");                                          \
  } while (0)

#define this_cpu_inc(var)                                                      \
  do {                                                                         \
    uintptr_t __off = PERCPU_OFFSET(var);                                      \
    __asm__ __volatile__("lock incq %%gs:(%0)" : : "r"(__off) : "memory");     \
  } while (0)

#define this_cpu_dec(var)                                                      \
  do {                                                                         \
    uintptr_t __off = PERCPU_OFFSET(var);                                      \
    __asm__ __volatile__("lock decq %%gs:(%0)" : : "r"(__off) : "memory");     \
  } while (0)

void init_percpu();

#endif // _KERNEL_PERCPU_H
