#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <kernel/bitmap.h>
#include <stddef.h>
#include <stdint.h>

struct bitmap_pmm {
  uint64_t max_addressable;
  bitmap_t bitmap;
  uint64_t total_pages;
};

void init_bitmap_pmm();
uintptr_t pmm_alloc_page();
uintptr_t pmm_alloc_page_count(size_t count);
void pmm_free_page(uintptr_t addr);
void pmm_free_page_count(uintptr_t addr, size_t count);

#endif // _KERNEL_PMM_H
