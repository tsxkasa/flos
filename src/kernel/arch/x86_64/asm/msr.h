#ifndef _KERNEL_MSR_H
#define _KERNEL_MSR_H

#include <stdint.h>

#define MSR_EFER           0xC0000080
#define MSR_STAR           0xC0000081
#define MSR_LSTAR          0xC0000082
#define MSR_CSTAR          0xC0000083
#define MSR_SYSCALL_MASK   0xC0000084
#define MSR_FS_BASE        0xC0000100
#define MSR_GS_BASE        0xC0000101
#define MSR_KERNEL_GS_BASE 0xC0000102

inline uint64_t rdmsr(uint32_t msr) {
  uint32_t lo, hi;
  __asm__ __volatile__("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return ((uint64_t)hi << 32) | lo;
}

inline void wrmsr(uint32_t msr, uint64_t val) {
  uint32_t lo = (uint32_t)val;
  uint32_t hi = (uint32_t)(val >> 32);
  __asm__ __volatile__("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
}

#endif // _KERNEL_MSR_H
