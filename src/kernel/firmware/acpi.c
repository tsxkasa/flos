#include "mm/address.h"
#include <asm/interrupts.h>
#include <boot/boot.h>
#include <kernel/printk.h>
#include <kernel/stdlib.h>
#include <mm/mm_types.h>
#include <mm/pmap/pmap.h>
#include <mm/vm/vm_map.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <uacpi/kernel_api.h>
#include <uacpi/log.h>

// TODO:
#define UACPI_BAREBONES_MODE

extern vm_map_t *kernel_vm_map;

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
  *out_rsdp_address = higher_half_data_to_phys((uintptr_t)boot_get_rsdp());
  return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
  uintptr_t phys = ALIGN_DOWN(addr, PAGE_SIZE);
  size_t offset = addr - phys;
  size_t size = ALIGN_UP(offset + len, PAGE_SIZE);

  uintptr_t virt =
      vm_map_allocate(kernel_vm_map, size, MMU_FLAG_WRITE | MMU_FLAG_NO_EXEC);
  if (!virt)
    return NULL;

  uintptr_t virt_tmp = virt;

  for (size_t i = 0; i < size; i += PAGE_SIZE) {
    if (!pmap_map_page(kernel_vm_map->page_table, virt_tmp, phys,
                       MMU_FLAG_NO_EXEC | MMU_FLAG_WRITE)) {
      uintptr_t v = virt;
      for (size_t j = 0; j < i; j += PAGE_SIZE) {
        pmap_unmap_page(kernel_vm_map->page_table, v);
        v += PAGE_SIZE;
      }
      return NULL;
    }
    virt_tmp += PAGE_SIZE;
    phys += PAGE_SIZE;
  }

  return (void *)(virt + offset);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
  uintptr_t virt = ALIGN_DOWN((uintptr_t)addr, PAGE_SIZE);
  size_t offset = (uintptr_t)addr - virt;
  size_t size = ALIGN_UP(offset + len, PAGE_SIZE);

  vm_map_free_region(kernel_vm_map, virt, size);

  for (size_t i = 0; i < size; i += PAGE_SIZE) {
    if (!pmap_unmap_page(kernel_vm_map->page_table, virt)) {
    }
    virt += PAGE_SIZE;
  }
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *msg) {
  switch (level) {
  case UACPI_LOG_DEBUG:
    printk(LOG_DEBUG "%s", msg);
    break;
  case UACPI_LOG_ERROR:
    printk(LOG_ERR "%s", msg);
    break;
  case UACPI_LOG_INFO:
    printk(LOG_INFO "%s", msg);
    break;
  case UACPI_LOG_WARN:
    printk(LOG_WARN "%s", msg);
    break;
  case UACPI_LOG_TRACE:
  default:
    break;
  }
}

void *uacpi_kernel_alloc(uacpi_size size) { return kmalloc(size); }

void *uacpi_kernel_alloc_zeroed(uacpi_size size) {
  void *a = kmalloc(size);
  memset(a, 0, size);
  return a;
}

void uacpi_kernel_free(void *mem) { kfree(mem); }

// TODO:

uacpi_status uacpi_kernel_initialize(uacpi_init_level current_init_lvl) {}
void uacpi_kernel_deinitialize(void) {}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address,
                                          uacpi_handle *out_handle) {}
void uacpi_kernel_pci_device_close(uacpi_handle) {}

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset,
                                    uacpi_u8 *value) {}
uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset,
                                     uacpi_u16 *value) {}
uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset,
                                     uacpi_u32 *value) {}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset,
                                     uacpi_u8 value) {}
uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset,
                                      uacpi_u16 value) {}
uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset,
                                      uacpi_u32 value) {}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len,
                                 uacpi_handle *out_handle) {}
void uacpi_kernel_io_unmap(uacpi_handle handle) {}

uacpi_status uacpi_kernel_io_read8(uacpi_handle, uacpi_size offset,
                                   uacpi_u8 *out_value) {}
uacpi_status uacpi_kernel_io_read16(uacpi_handle, uacpi_size offset,
                                    uacpi_u16 *out_value) {}
uacpi_status uacpi_kernel_io_read32(uacpi_handle, uacpi_size offset,
                                    uacpi_u32 *out_value) {}

uacpi_status uacpi_kernel_io_write8(uacpi_handle, uacpi_size offset,
                                    uacpi_u8 in_value) {}
uacpi_status uacpi_kernel_io_write16(uacpi_handle, uacpi_size offset,
                                     uacpi_u16 in_value) {}
uacpi_status uacpi_kernel_io_write32(uacpi_handle, uacpi_size offset,
                                     uacpi_u32 in_value) {}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void) { return 0; }

void uacpi_kernel_stall(uacpi_u8 usec) {}

void uacpi_kernel_sleep(uacpi_u64 msec) {}

uacpi_handle uacpi_kernel_create_mutex(void) { return 0; }
void uacpi_kernel_free_mutex(uacpi_handle) {}

uacpi_handle uacpi_kernel_create_event(void) { return 0; }
void uacpi_kernel_free_event(uacpi_handle) {}

uacpi_thread_id uacpi_kernel_get_thread_id(void) { return 0; }

uacpi_interrupt_state uacpi_kernel_disable_interrupts(void) {
  disable_interrupts();
  return 0;
}

void uacpi_kernel_restore_interrupts(uacpi_interrupt_state state) {
  enable_interrupts();
}

// TODO:

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle, uacpi_u16) {}
void uacpi_kernel_release_mutex(uacpi_handle) {}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16) {}

void uacpi_kernel_signal_event(uacpi_handle) {}

void uacpi_kernel_reset_event(uacpi_handle) {}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *) {}

uacpi_status
uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler,
                                       uacpi_handle ctx,
                                       uacpi_handle *out_irq_handle) {}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler,
                                                      uacpi_handle irq_handle) {
}

uacpi_handle uacpi_kernel_create_spinlock(void) {}
void uacpi_kernel_free_spinlock(uacpi_handle) {}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle) {}
void uacpi_kernel_unlock_spinlock(uacpi_handle, uacpi_cpu_flags) {}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type, uacpi_work_handler,
                                        uacpi_handle ctx) {}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {}
