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

static inline uintptr_t kernel_virt_to_phys(uintptr_t address) {
  kernel_addr_t k = boot_get_executable_addr();
  return k.phys + (address - k.virt);
}

static inline uintptr_t kernel_phys_to_virt(uintptr_t address) {
  kernel_addr_t k = boot_get_executable_addr();
  return k.virt + (address - k.phys);
}

#endif // _KERNEL_ARCH_MEMORY_H
