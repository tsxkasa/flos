#ifndef _KERNEL_MM_TYPES_H
#define _KERNEL_MM_TYPES_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_HUGE (1ull << 7)
#define PAGE_SIZE_2M 0x200000ull
#define KB_TO_PAGES(kb) (((kb) * 1024) / PAGE_SIZE)
#define BYTES_TO_PAGES(bytes) ((bytes) / PAGE_SIZE)

#define IS_PAGE_ALIGNED(num) (((num) & (PAGE_SIZE - 1)) == 0)

typedef struct {
  uint64_t phys;
  uint64_t virt;
} kernel_addr_t;

#endif // _KERNEL_MM_TYPES_H
