#include "mm/mm_types.h"
#include "mm/vm/vm_map.h"
#include <cpu/halt.h>
#include <mm/address.h>
#include <mm/pmap/pmap.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/slab.h>
#include <printk.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define PAGE_PRESENT  (1ull << 0)
#define PAGE_RW       (1ull << 1)
#define PAGE_USER     (1ull << 2)
#define PAGE_PWT      (1ull << 3)
#define PAGE_PCD      (1ull << 4)
#define PAGE_ACCESSED (1ull << 5)
#define PAGE_XD       (1ull << 63)

#define PTE_DIRTY  (1ULL << 6)
#define PTE_PAT    (1ULL << 7)
#define PTE_GLOBAL (1ULL << 8)

#define PDE_PS     (1ULL << 7) /* 2 MiB page */
#define PDE_GLOBAL (1ULL << 8)

#define PDPTE_PS (1ULL << 7) /* 1 GiB page */

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

  if (entry & PAGE_PRESENT) {
    if (entry & PDE_PS || entry & PDPTE_PS)
      return NULL;
    return (uint64_t *)phys_to_higher_half_data(entry & PAGE_PHYS_MASK);
  }

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

static uint64_t parse_common_flags(uint32_t flags) {
  uint64_t arch_flags = PAGE_PRESENT;
  if (flags & MMU_FLAG_WRITE)
    arch_flags |= PAGE_RW;
  if (flags & MMU_FLAG_USER)
    arch_flags |= PAGE_USER;
  if (flags & MMU_FLAG_UC) {
    arch_flags |= PAGE_PCD;
    // arch_flags |= PAGE_PWT;
    arch_flags &= ~PAGE_PWT;
  }
  if (flags & MMU_FLAG_NO_EXEC)
    arch_flags |= PAGE_XD;
  return arch_flags;
}

static uint64_t map(struct page_table_t *table, uintptr_t virt, uintptr_t phys,
                    uint64_t arch_flags) {
  uint16_t pml4_idx = GET_PML4_IDX(virt);
  uint16_t pdpt_idx = GET_PDPT_IDX(virt);
  uint16_t pd_idx = GET_PD_IDX(virt);
  uint16_t pt_idx = GET_PT_IDX(virt);

  uint64_t *pml4 = table->pml4_virt;
  uint64_t *pdpt = get_next_level(pml4, pml4_idx, 1);
  if (!pdpt)
    return 0;
  uint64_t *pd = get_next_level(pdpt, pdpt_idx, 1);
  if (!pd)
    return 0;
  uint64_t *pt = get_next_level(pd, pd_idx, 1);
  if (!pt)
    return 0;

  pt[pt_idx] = phys | arch_flags;

  asm volatile("invlpg (%0)" : : "r"(virt) : "memory");

  return PAGE_SIZE;
}

static uint64_t map_2m(struct page_table_t *table, uintptr_t virt,
                       uintptr_t phys, uint64_t arch_flags) {
  uint16_t pml4_idx = GET_PML4_IDX(virt);
  uint16_t pdpt_idx = GET_PDPT_IDX(virt);
  uint16_t pd_idx = GET_PD_IDX(virt);

  uint64_t *pdpt = get_next_level(table->pml4_virt, pml4_idx, 1);
  if (!pdpt)
    return false;

  uint64_t *pd = get_next_level(pdpt, pdpt_idx, 1);
  if (!pd)
    return 0;

  pd[pd_idx] = phys | arch_flags;

  asm volatile("invlpg (%0)" : : "r"(virt) : "memory");

  return PAGE_SIZE_2M;
}

static uint64_t map_1g(struct page_table_t *table, uintptr_t virt,
                       uintptr_t phys, uint64_t arch_flags) {
  uint16_t pml4_idx = GET_PML4_IDX(virt);
  uint16_t pdpt_idx = GET_PDPT_IDX(virt);

  uint64_t *pdpt = get_next_level(table->pml4_virt, pml4_idx, 1);
  if (!pdpt)
    return 0;

  pdpt[pdpt_idx] = phys | arch_flags;

  asm volatile("invlpg (%0)" : : "r"(virt) : "memory");

  return PAGE_SIZE_1G;
}

static uint64_t map_huge_ok(struct page_table_t *table, uintptr_t virt,
                            uintptr_t phys, uint64_t arch_flags,
                            uint64_t remain) {

  if (phys % PAGE_SIZE_1G == 0 && virt % PAGE_SIZE_1G == 0 &&
      remain >= PAGE_SIZE_1G) {
    arch_flags |= PDPTE_PS;
    return map_1g(table, virt, phys, arch_flags);
  }
  if (phys % PAGE_SIZE_2M == 0 && virt % PAGE_SIZE_2M == 0 &&
      remain >= PAGE_SIZE_2M) {
    arch_flags |= PDE_PS;
    return map_2m(table, virt, phys, arch_flags);
  } else {
    return map(table, virt, phys, arch_flags);
  }
}

uint64_t pmap_map_range(struct page_table_t *table, uintptr_t virt,
                        uintptr_t phys, uint32_t flags, uint64_t remain) {
  uint64_t arch_flags = parse_common_flags(flags);
  uint64_t mapped = 0;

  uint64_t start_virt = ALIGN_DOWN(virt, PAGE_SIZE);
  uint64_t end_virt = ALIGN_UP(virt + remain, PAGE_SIZE);
  remain = end_virt - start_virt;

  virt = start_virt;
  phys = ALIGN_DOWN(phys, PAGE_SIZE);

  while (remain > 0) {
    size_t step;

    if (flags & MMU_FLAG_HUGE_OK) {
      step = map_huge_ok(table, virt, phys, arch_flags, remain);
    } else {
      step = map(table, virt, phys, arch_flags);
    }

    if (!step)
      break;

    virt += step;
    phys += step;
    remain -= step;
    mapped += step;
  }
  return mapped;
}

uint64_t pmap_map_page(struct page_table_t *table, uintptr_t virt,
                       uintptr_t phys, uint32_t flags) {
  virt = ALIGN_DOWN(virt, PAGE_SIZE);
  phys = ALIGN_DOWN(phys, PAGE_SIZE);
  uint64_t arch_flags = parse_common_flags(flags);
  return map(table, virt, phys, arch_flags);
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

void pmap_switch(uintptr_t phy) { asm volatile("mov %0, %%cr3" : : "r"(phy)); }

void pmap_switch_pt(struct page_table_t *table) {
  asm volatile("mov %0, %%cr3" : : "r"(table->pml4_phy));
}
