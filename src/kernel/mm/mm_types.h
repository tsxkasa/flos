#ifndef _KERNEL_MM_TYPES_H
#define _KERNEL_MM_TYPES_H

#include <stdint.h>

#define PAGE_SIZE             4096
#define PAGE_SIZE_2M          0x200000ull
#define KB_TO_PAGES(kb)       (((kb) * 1024) / PAGE_SIZE)
#define BYTES_TO_PAGES(bytes) ((bytes) / PAGE_SIZE)

#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))

#define IS_PAGE_ALIGNED(num) (((num) & (PAGE_SIZE - 1)) == 0)

#define VM_ALLOC_BASE 0x1000

typedef struct {
  uint64_t phys;
  uint64_t virt;
} kernel_addr_t;

#endif // _KERNEL_MM_TYPES_H
