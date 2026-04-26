#ifndef _KERNEL_MM_TYPES_H
#define _KERNEL_MM_TYPES_H

#include <stdint.h>

typedef struct {
  uint64_t phys;
  uint64_t virt;
} kernel_addr_t;

#endif // _KERNEL_MM_TYPES_H
