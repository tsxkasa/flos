#include "mm/vm/vm_map.h"
#include <cpu/halt.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <mm/address.h>
#include <mm/pmap/pmap.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/slab.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_PRESENT (1ull << 0)
#define PAGE_RW      (1ull << 1)
#define PAGE_USER    (1ull << 2)
#define PAGE_XD      (1ull << 63)

#define GET_PML4_IDX(vaddr) (((vaddr) >> 39) & 0x1ff)
#define GET_PDPT_IDX(vaddr) (((vaddr) >> 30) & 0x1ff)
#define GET_PD_IDX(vaddr)   (((vaddr) >> 21) & 0x1ff)
#define GET_PT_IDX(vaddr)   (((vaddr) >> 12) & 0x1ff)

#define PAGE_PHYS_MASK     0x000FFFFFFFFFF000ull
#define PAGE_TABLE_ENTRIES 512u

extern vm_map_t *kernel_vm_map;

struct page_table_t {
  uint64_t *pml4_virt;
  uintptr_t pml4_phy;
};

static struct kmem_cache *page_table_cache;

struct page_table_t *pmap_create_table(void) {
  if (!page_table_cache) {
    page_table_cache = kmem_cache_create(sizeof(struct page_table_t));
  }

  uintptr_t phys = pmm_alloc_page();
  if (!phys) {
    printk(LOG_ERR "paging: failed to allocate PML4\n");
    return NULL;
  }

  uint64_t *virt = (uint64_t *)phys_to_higher_half_data(phys);
  memset(virt, 0, PAGE_SIZE);

  if (kernel_vm_map && kernel_vm_map->page_table) {
    for (size_t i = 256; i < PAGE_TABLE_ENTRIES; i++) {
      virt[i] = kernel_vm_map->page_table->pml4_virt[i];
    }
  }

  // uintptr_t table_phys = pmm_alloc_page();
  // if (!table_phys) {
  //   printk(LOG_ERR "paging: failed to allocate page_table_t struct\n");
  //   pmm_free_page(phys);
  //   return NULL;
  // }
  //
  // struct page_table_t *table =
  //     (struct page_table_t *)phys_to_higher_half_data(table_phys);
  // memset(table, 0, sizeof(*table));

  struct page_table_t *table = kmem_cache_alloc(page_table_cache);
  if (!table) {
    pmm_free_page(phys);
    return NULL;
  }

  memset(table, 0, sizeof(*table));

  table->pml4_virt = virt;
  table->pml4_phy = phys;
  return table;
}

void pmap_destroy_table(struct page_table_t *table) {
  if (!table)
    return;

  uint64_t *pml4 = table->pml4_virt;

  // only clean userspace map (0-255)
  for (size_t i = 0; i < 256; i++) {
    if (!(pml4[i] & PAGE_PRESENT))
      continue;
    uint64_t *pdpt =
        (uint64_t *)phys_to_higher_half_data(pml4[i] & PAGE_PHYS_MASK);

    for (size_t j = 0; j < PAGE_TABLE_ENTRIES; j++) {
      if (!(pdpt[j] & PAGE_PRESENT))
        continue;
      uint64_t *pd =
          (uint64_t *)phys_to_higher_half_data(pdpt[j] & PAGE_PHYS_MASK);

      for (size_t k = 0; k < PAGE_TABLE_ENTRIES; k++) {
        if (!(pd[k] & PAGE_PRESENT))
          continue;
        pmm_free_page(pd[k] & PAGE_PHYS_MASK);
      }

      pmm_free_page(pdpt[j] & PAGE_PHYS_MASK);
    }

    pmm_free_page(pml4[i] & PAGE_PHYS_MASK);
  }

  pmm_free_page(table->pml4_phy);

  kmem_cache_free(page_table_cache, table);
}

static uint64_t *get_next_level(uint64_t *current_table, uint16_t index,
                                int allocate) {
  uint64_t entry = current_table[index];

  if (entry & PAGE_PRESENT)
    return (uint64_t *)phys_to_higher_half_data(entry & PAGE_PHYS_MASK);

  if (!allocate)
    return NULL;

  uintptr_t next_phys = pmm_alloc_page();
  if (!next_phys) {
    printk(LOG_ERR "paging: out of memory for page tables\n");
    return NULL;
  }

  uint64_t *next_virt = (uint64_t *)phys_to_higher_half_data(next_phys);
  memset(next_virt, 0, PAGE_SIZE);

  /* Intermediate entries always carry RW|USER so that page-level
   * permissions in the PTE are the real access control. */
  current_table[index] = next_phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
  return next_virt;
}

bool pmap_map_page(struct page_table_t *table, uintptr_t virt, uintptr_t phys,
                   uint32_t flags) {
  uint64_t arch_flags = PAGE_PRESENT;
  if (flags & MMU_FLAG_WRITE)
    arch_flags |= PAGE_RW;
  if (flags & MMU_FLAG_USER)
    arch_flags |= PAGE_USER;
  if (flags & MMU_FLAG_NO_EXEC)
    arch_flags |= PAGE_XD;

  uint16_t pml4_idx = GET_PML4_IDX(virt);
  uint16_t pdpt_idx = GET_PDPT_IDX(virt);
  uint16_t pd_idx = GET_PD_IDX(virt);
  uint16_t pt_idx = GET_PT_IDX(virt);

  uint64_t *pml4 = table->pml4_virt;
  uint64_t *pdpt = get_next_level(pml4, pml4_idx, 1);
  if (!pdpt)
    return false;
  uint64_t *pd = get_next_level(pdpt, pdpt_idx, 1);
  if (!pd)
    return false;
  uint64_t *pt = get_next_level(pd, pd_idx, 1);
  if (!pt)
    return false;

  pt[pt_idx] = phys | arch_flags;

  asm volatile("invlpg (%0)" : : "r"(virt) : "memory");

  return true;
}

bool pmap_map_page_2m(struct page_table_t *table, uintptr_t virt,
                      uintptr_t phys, uint32_t flags) {
  uint64_t arch_flags = PAGE_PRESENT | PAGE_HUGE; // Notice PAGE_HUGE
  if (flags & MMU_FLAG_WRITE)
    arch_flags |= PAGE_RW;
  if (flags & MMU_FLAG_USER)
    arch_flags |= PAGE_USER;
  if (flags & MMU_FLAG_NO_EXEC)
    arch_flags |= PAGE_XD;

  uint16_t pml4_idx = GET_PML4_IDX(virt);
  uint16_t pdpt_idx = GET_PDPT_IDX(virt);
  uint16_t pd_idx = GET_PD_IDX(virt);

  uint64_t *pdpt = get_next_level(table->pml4_virt, pml4_idx, 1);
  if (!pdpt)
    return false;

  uint64_t *pd = get_next_level(pdpt, pdpt_idx, 1);
  if (!pd)
    return false;

  // We write the physical address directly into the Page Directory
  pd[pd_idx] = phys | arch_flags;

  // Optional: only invalidate if this table is active
  asm volatile("invlpg (%0)" : : "r"(virt) : "memory");

  return true;
}

uintptr_t pmap_unmap_page(struct page_table_t *table, uintptr_t virt) {
  uint16_t pml4_idx = GET_PML4_IDX(virt);
  uint16_t pdpt_idx = GET_PDPT_IDX(virt);
  uint16_t pd_idx = GET_PD_IDX(virt);
  uint16_t pt_idx = GET_PT_IDX(virt);

  uint64_t *pml4 = table->pml4_virt;
  uint64_t *pdpt = get_next_level(pml4, pml4_idx, 0);
  if (!pdpt)
    return 0;
  uint64_t *pd = get_next_level(pdpt, pdpt_idx, 0);
  if (!pd)
    return 0;
  uint64_t *pt = get_next_level(pd, pd_idx, 0);
  if (!pt)
    return 0;

  uintptr_t pa = pt[pt_idx] & PAGE_PHYS_MASK;
  pt[pt_idx] = 0;
  asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
  return pa;
}

void pmap_switch(struct page_table_t *table) {
  asm volatile("mov %0, %%cr3" : : "r"(table->pml4_phy));
}
