#include <cpu/halt.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <mm/address.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/vm_map.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static void *_alloc_struct(size_t size) {
  // TODO: slab
  uintptr_t phys = pmm_alloc_page();
  if (!phys)
    return NULL;
  void *virt = (void *)phys_to_higher_half_data(phys);
  memset(virt, 0, size);
  return virt;
}

static void _free_struct(void *ptr) {
  if (!ptr)
    return;
  pmm_free_page(higher_half_data_to_phys((uintptr_t)ptr));
}

static vm_area_t *vma_alloc(uintptr_t start, uintptr_t end, uint32_t flags) {
  vm_area_t *vma = _alloc_struct(sizeof(vm_area_t));
  if (!vma)
    return NULL;
  vma->start = start;
  vma->end = end;
  vma->flags = flags;
  vma->next = NULL;
  return vma;
}

static bool vma_overlaps(vm_map_t *space, uintptr_t start, size_t size) {
  uintptr_t end = start + size;
  for (vm_area_t *vma = space->areas; vma; vma = vma->next) {
    if (start < vma->end && end > vma->start)
      return true;
  }
  return false;
}

static void insert_vma(vm_map_t *space, vm_area_t *new_vma) {
  vm_area_t **pp = &space->areas;
  while (*pp && (*pp)->start < new_vma->start)
    pp = &(*pp)->next;
  new_vma->next = *pp;
  *pp = new_vma;
}

vm_map_t *vm_map_create(void) {
  vm_map_t *space = _alloc_struct(sizeof(vm_map_t));
  if (!space) {
    printk(LOG_ERR "vm_map: failed to allocate address_space_t\n");
    return NULL;
  }

  space->page_table = pmap_create_table();
  if (!space->page_table) {
    printk(LOG_ERR "vm_map: failed to create page table\n");
    _free_struct(space);
    return NULL;
  }

  space->areas = NULL;
  return space;
}

void vm_map_destroy(vm_map_t *map) {
  if (!map)
    return;

  vm_area_t *vma = map->areas;
  while (vma) {
    vm_area_t *next = vma->next;
    for (uintptr_t va = vma->start; va < vma->end; va += 0x1000) {
      uintptr_t pa = pmap_unmap_page(map->page_table, va);
      if (pa)
        pmm_free_page(pa);
    }
    _free_struct(vma);
    vma = next;
  }

  pmap_destroy_table(map->page_table);
  _free_struct(map);
}

bool vm_map_allocate_region(vm_map_t *space, uintptr_t start, size_t size,
                            uint32_t flags) {
  if (!space || !size)
    return false;

  size = (size + 0xFFF) & ~(size_t)0xFFF;

  if (vma_overlaps(space, start, size)) {
    printk(LOG_WARN "vm_map: region [%p, %p) overlaps existing VMA\n",
           (void *)start, (void *)(start + size));
    return false;
  }

  vm_area_t *vma = vma_alloc(start, start + size, flags);
  if (!vma)
    return false;

  uintptr_t va;
  for (va = start; va < start + size; va += 0x1000) {
    uintptr_t pa = pmm_alloc_page();
    if (!pa) {
      printk(LOG_ERR "vm_map: out of memory allocating region\n");
      break;
    }

    if (!pmap_map_page(space->page_table, va, pa, flags)) {
      printk(LOG_ERR "vm_map: pmap_map_page failed at %p\n", (void *)va);
      /* Free the page we just allocated but failed to map. */
      pmm_free_page(pa);
      break;
    }
  }

  if (va < start + size) {
    for (uintptr_t rb = start; rb < va; rb += 0x1000) {
      uintptr_t rpa = pmap_unmap_page(space->page_table, rb);
      if (rpa)
        pmm_free_page(rpa);
    }
    _free_struct(vma);
    return false;
  }

  insert_vma(space, vma);
  return true;
}

vm_area_t *vm_map_lookup(vm_map_t *space, uintptr_t addr) {
  for (vm_area_t *vma = space->areas; vma; vma = vma->next) {
    if (addr >= vma->start && addr < vma->end)
      return vma;
  }
  return NULL;
}

void vm_map_free_region(vm_map_t *space, uintptr_t start, size_t size) {
  if (!space || !size)
    return;

  size = (size + 0xFFF) & ~(size_t)0xFFF;
  uintptr_t end = start + size;

  for (uintptr_t va = start; va < end; va += 0x1000) {
    uintptr_t pa = pmap_unmap_page(space->page_table, va);
    if (pa)
      pmm_free_page(pa);
  }

  vm_area_t **pp = &space->areas;
  while (*pp) {
    vm_area_t *vma = *pp;

    if (vma->end <= start || vma->start >= end) {
      pp = &vma->next;
      continue;
    }

    if (vma->start >= start && vma->end <= end) {
      *pp = vma->next;
      _free_struct(vma);
      continue;
    }

    if (vma->start < start && vma->end > end) {
      vm_area_t *tail = vma_alloc(end, vma->end, vma->flags);
      if (tail) {
        tail->next = vma->next;
        vma->end = start;
        vma->next = tail;
        pp = &tail->next;
      } else {
        /* Allocation failed: truncate to avoid a stale range. */
        printk(LOG_WARN "vm_map: could not split VMA, truncating\n");
        vma->end = start;
        pp = &vma->next;
      }
      continue;
    }

    if (vma->start < start) {
      vma->end = start;
      pp = &vma->next;
      continue;
    }

    vma->start = end;
    pp = &vma->next;
  }
}

void vm_fault_handler(vm_map_t *space, uintptr_t fault_address,
                      uint32_t error_code) {
  bool present = error_code & (1u << 0);
  bool write_fault = error_code & (1u << 1);
  bool user_fault = error_code & (1u << 2);
  bool reserved_bit = error_code & (1u << 3);
  bool instr_fetch = error_code & (1u << 4);

  printk(LOG_ERR "vm_map: page fault @ %p  [%s%s%s%s%s]\n",
         (void *)fault_address, present ? "PROTECT " : "NOT_PRESENT ",
         write_fault ? "WRITE " : "READ ", user_fault ? "USER " : "KERNEL ",
         reserved_bit ? "RSVD_BIT " : "", instr_fetch ? "IFETCH" : "");

  // TODO: demand paging

  hcf();
}
