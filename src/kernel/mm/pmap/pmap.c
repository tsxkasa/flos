#include "mm/vm/vm_map.h"
#include <boot/boot.h>
#include <cpu/halt.h>
#include <isr.h>
#include <kernel/printk.h>
#include <mm/address.h>
#include <mm/memory_map.h>
#include <mm/pmap/pmap.h>
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

struct page_table_t *kernel_page_table = NULL;

static bool map_range(struct page_table_t *pt, uintptr_t virt, uintptr_t phys,
                      size_t size, uint32_t flags) {
  for (size_t off = 0; off < size; off += PAGE_SIZE) {
    if (!pmap_map_page(pt, virt + off, phys + off, flags)) {
      printk(LOG_ERR "pmap: failed to map virt %p\n", (void *)(virt + off));
      return false;
    }
  }
  return true;
}

static uint64_t pf_interrupt_handler(struct interrupt_frame *frame) {
  uint32_t err = frame->error_code;
  unsigned long addr;
  __asm__ __volatile__("mov %%cr2, %0" : "=r"(addr));
  vm_fault_handler(kernel_vm_map, addr, err);

  return (uint64_t)frame;
}

void init_pmap(void) {
  memory_map_t *map = boot_get_memmap();
  kernel_addr_t kernel = boot_get_executable_addr();

  kernel_vm_map = vm_map_create();

  register_interrupt_handler(0xE, pf_interrupt_handler);

  kernel_page_table = pmap_create_table();
  if (!kernel_page_table) {
    printk(LOG_ERR "pmap: could not allocate kernel page table\n");
    hcf();
  }

  for (uint64_t i = 0; i < map->entry_count; i++) {
    memory_map_entry_t *e = &map->entries[i];
    if (e->type == MEMMAP_BAD_MEMORY)
      continue;

    uintptr_t phys = e->base;
    uintptr_t end = phys + e->length;

    while (phys < end) {
      uintptr_t virt = phys_to_higher_half_data(phys);
      size_t remain = end - phys;

      if ((phys % PAGE_SIZE_2M == 0) && (virt % PAGE_SIZE_2M == 0) &&
          remain >= PAGE_SIZE_2M) {
        pmap_map_page_2m(kernel_page_table, virt, phys,
                         MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC);
        phys += PAGE_SIZE_2M;
      } else {
        pmap_map_page(kernel_page_table, virt, phys,
                      MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC);
        phys += PAGE_SIZE;
      }
    }
  }

  uintptr_t kvirt_base = (uintptr_t)_kernel_start;
  uintptr_t kphys_base = kernel.phys;

#define KPHYS(virt) (kphys_base + ((uintptr_t)(virt) - kvirt_base))
#define KSECT(lo, hi, flags)                                                   \
  map_range(kernel_page_table, (uintptr_t)(lo), KPHYS(lo),                     \
            (uintptr_t)(hi) - (uintptr_t)(lo), (flags))

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

  for (uintptr_t addr = PAGE_SIZE; addr < 0x200000; addr += PAGE_SIZE)
    pmap_map_page(kernel_page_table, addr, addr,
                  MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC);

  pmap_switch(kernel_page_table);
  printk(LOG_INFO "pmap initialized\n");
  printk(LOG_INFO "pmap: switched to kernel page table\n");
}
