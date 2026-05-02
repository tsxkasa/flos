#ifndef _KERNEL_PMAP_H
#define _KERNEL_PMAP_H

#include <mm/memory_map.h>
#include <mm/mm_types.h>
#include <stdbool.h>
#include <stdint.h>

#define MMU_FLAG_PRESENT (1 << 0)
#define MMU_FLAG_WRITE (1 << 1)
#define MMU_FLAG_USER (1 << 2)
#define MMU_FLAG_NO_EXEC (1 << 3)

struct page_table_t;

struct page_table_t *pmap_create_table(void);

void init_pmap(void);

void pmap_destroy_table(struct page_table_t *table);
bool pmap_map_page(struct page_table_t *table, uintptr_t virt, uintptr_t phys,
                   uint32_t flags);
bool pmap_map_page_2m(struct page_table_t *table, uintptr_t virt,
                      uintptr_t phys, uint32_t flags);
uintptr_t pmap_unmap_page(struct page_table_t *table, uintptr_t virt);
void pmap_switch(struct page_table_t *table);

#endif // _KERNEL_PMAP_H
