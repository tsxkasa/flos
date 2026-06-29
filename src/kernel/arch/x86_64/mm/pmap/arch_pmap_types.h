#ifndef _KERNEL_PMAP_TYPES_H
#define _KERNEL_PMAP_TYPES_H
#include <mm/pmap/pmap.h>

struct page_table_t {
  uint64_t *pml4_virt;
  uintptr_t pml4_phy;
};

#endif // _KERNEL_PMAP_TYPES_H
