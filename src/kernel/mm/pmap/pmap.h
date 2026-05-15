#ifndef _KERNEL_PMAP_H
#define _KERNEL_PMAP_H

#include <mm/memory_map.h>
#include <mm/mm_types.h>
#include <stdbool.h>
#include <stdint.h>

#define MMU_FLAG_PRESENT (1 << 0)
#define MMU_FLAG_WRITE   (1 << 1)
#define MMU_FLAG_USER    (1 << 2)
#define MMU_FLAG_NO_EXEC (1 << 3)

struct page_table_t;

struct page_table_t *pmap_create_table(void);

void init_pmap(void);

/**
 * @brief destroys and frees the page table
 *
 * @param table page table to destroy
 */
void pmap_destroy_table(struct page_table_t *table);

/**
 * @brief maps one virtual 4KiB page to the page table. it walks the page table
 * to ensure all intermediate page table exists, and installs mapping from the
 * given virtual address to the physical address
 *
 * @param table       pointer to the page table containing the active address
 * space
 * @param virt        virtual address to map, only page aligned portion will be
 * used for indexing page structures
 * @param phys        physical address to map to the given virtual address,
 * caller must guarantee this address is page aligned
 * @param flags       memory access flags: rwx perms
 * @return true if page was mapped, false if anything fails
 */
bool pmap_map_page(struct page_table_t *table, uintptr_t virt, uintptr_t phys,
                   uint32_t flags);

/**
 * @brief maps one virtual 2MiB page to the page table. it walks the page table
 * to ensure all intermediate page table exists, and installs mapping from the
 * given virtual address to the physical address
 *
 * @param table       pointer to the page table containing the active address
 * space
 * @param virt        virtual address to map, only page aligned portion will be
 * used for indexing page structures
 * @param phys        physical address to map to the given virtual address,
 * caller must guarantee this address is page aligned
 * @param flags       memory access flags: rwx perms
 * @return true if page was mapped, false if anything fails
 */
bool pmap_map_page_2m(struct page_table_t *table, uintptr_t virt,
                      uintptr_t phys, uint32_t flags);

/**
 * @brief unmaps one page at the given virtual address
 *
 * @param table table to unmap page from
 * @param virt  virtual address to unmap page
 * @return the physical address page just unmapped or 0 if fails
 */
uintptr_t pmap_unmap_page(struct page_table_t *table, uintptr_t virt);

/**
 * @brief switches the context register
 *
 * @param table the table to switch the paging context to
 */
void pmap_switch(struct page_table_t *table);

#endif // _KERNEL_PMAP_H
