#include "mm/vm/vm_map.h"
#include <boot/boot.h>
#include <cpu/halt.h>
#include <interrupts/isr.h>
#include <kernel/printk.h>
#include <mm/address.h>
#include <mm/memory_map.h>
#include <mm/pmap/pmap.h>
#include <mm/vm/slab.h>
#include <stddef.h>
#include <stdint.h>

vm_map_t *kernel_vm_map;

extern char _kernel_start[];
extern char _kernel_end[];

extern char _text_start[];
extern char _text_end[];
extern char _rodata_start[];
extern char _rodata_end[];
extern char _data_start[];

static void pf_interrupt_handler(struct interrupt_frame *frame) {
  uint32_t err = frame->error_code;
  unsigned long addr;
  __asm__ __volatile__("mov %%cr2, %0" : "=r"(addr));
  vm_fault_handler(kernel_vm_map, addr, err);

  return;
}

void init_pmap(void) {
  memory_map_t *map = boot_get_memmap();
  kernel_addr_t kernel = boot_get_executable_addr();

  kernel_vm_map = vm_map_create();

  // kernel_vm_map->page_table = pmap_create_table();
  if (!kernel_vm_map || !kernel_vm_map->page_table) {
    printk(LOG_ERR "pmap: could not allocate kernel page table\n");
    hcf();
  }

  for (uint64_t i = 0; i < map->entry_count; i++) {
    memory_map_entry_t *e = &map->entries[i];
    if (e->type == MEMMAP_BAD_MEMORY ||
        e->type == MEMMAP_EXECUTABLE_AND_MODULES)
      continue;

    uintptr_t phys = e->base;

    uint32_t flags = MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC | MMU_FLAG_HUGE_OK;

    pmap_map_range(kernel_vm_map->page_table, phys, phys, flags, e->length);
    pmap_map_range(kernel_vm_map->page_table, phys_to_higher_half_data(phys),
                   phys, flags, e->length);
  }

  uintptr_t kvirt_base = (uintptr_t)_kernel_start;
  uintptr_t kphys_base = kernel.phys;

#define KPHYS(virt) (kphys_base + ((uintptr_t)(virt) - kvirt_base))
#define KSECT(lo, hi, flags)                                                   \
  pmap_map_range(kernel_vm_map->page_table, (uintptr_t)(lo), KPHYS(lo),        \
                 (flags), (uintptr_t)(hi) - (uintptr_t)(lo))

  /* .limine_requests: r */
  KSECT(_kernel_start, _text_start, MMU_FLAG_NO_EXEC);

  /* .text: rx */
  KSECT(_text_start, _text_end, 0);

  /* .rodata: r */
  KSECT(_rodata_start, _rodata_end, MMU_FLAG_NO_EXEC);

  /* .data + .bss: rw */
  KSECT(_data_start, _kernel_end, MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC);

#undef KSECT
#undef KPHYS

  register_interrupt_handler(0xE, pf_interrupt_handler);
  pmap_switch(kernel_vm_map->page_table);

  printk(LOG_INFO "pmap initialized\n");
  printk(LOG_INFO "pmap: switched to kernel page table\n");
}

void pmap_map_mmio(uintptr_t phys, size_t size) {
  uintptr_t virt = phys + boot_get_hhdm_offset();

  size = ALIGN_UP(size, PAGE_SIZE);

  for (size_t off = 0; off < size; off += PAGE_SIZE) {
    pmap_map_page(kernel_vm_map->page_table, virt + off, phys + off,
                  MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC | MMU_FLAG_UC);
  }
}
