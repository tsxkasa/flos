#ifndef _KERNEL_VMM_H
#define _KERNEL_VMM_H

#include <mm/pmap/pmap.h>
#include <mm/vm/vm_area.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct vm_map {
  struct page_table_t *page_table;
  vm_area_t *areas;
} vm_map_t;

void init_vm(void);
vm_map_t *vm_map_create();
void vm_map_destroy(vm_map_t *map);

uintptr_t vm_map_allocate(vm_map_t *space, size_t size, uint32_t flags);
bool vm_map_allocate_region(vm_map_t *space, uintptr_t start, size_t size,
                            uint32_t flags);
void vm_map_free_region(vm_map_t *space, uintptr_t start, size_t size);

vm_area_t *vm_map_lookup(vm_map_t *space, uintptr_t addr);

void vm_fault_handler(vm_map_t *space, uintptr_t fault_address,
                      uint32_t error_code);

#endif // _KERNEL_VMM_H
