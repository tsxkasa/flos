#ifndef _KERNEL_ARCH_MEMORY_H
#define _KERNEL_ARCH_MEMORY_H

#include <boot/boot.h>
#include <mm/mm_types.h>
#define PAGE_SIZE 4096
#define KB_TO_PAGES(kb) (((kb) * 1024) / PAGE_SIZE)
#define BYTES_TO_PAGES(bytes) ((bytes) / PAGE_SIZE)

#define IS_PAGE_ALIGNED(num) (((num) & (PAGE_SIZE - 1)) == 0)

#include <stdint.h>

static inline uintptr_t phys_to_higher_half_data(uintptr_t address) {
  return boot_get_hhdm_offset() + address;
}

static inline uintptr_t higher_half_data_to_phys(uintptr_t address) {
  return address - boot_get_hhdm_offset();
}

#endif // _KERNEL_ARCH_MEMORY_H
