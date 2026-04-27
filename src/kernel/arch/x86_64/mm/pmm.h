#ifndef _KERNEL_ARCH_PMM_H
#define _KERNEL_ARCH_PMM_H

#include <kernel/bitmap.h>
#include <stdint.h>

struct bitmap_pmm {
  uint64_t max_addressable;
  bitmap_t bitmap;
  uint64_t total_pages;
};

void init_bitmap_pmm();
uintptr_t pmm_alloc_page();
void pmm_free_page(uintptr_t addr);

#endif // _KERNEL_ARCH_PMM_H
