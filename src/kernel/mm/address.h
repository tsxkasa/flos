#ifndef _KERNEL_ARCH_MEMORY_H
#define _KERNEL_ARCH_MEMORY_H

#include <boot/boot.h>
#include <mm/mm_types.h>

#include <stdint.h>

static inline uintptr_t phys_to_higher_half_data(uintptr_t address) {
  return boot_get_hhdm_offset() + address;
}

static inline uintptr_t higher_half_data_to_phys(uintptr_t address) {
  return address - boot_get_hhdm_offset();
}

#endif // _KERNEL_ARCH_MEMORY_H
