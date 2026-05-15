#include <boot/boot.h>
#include <cpu/halt.h>
#include <drivers/input/ps2/keyboard/keyboard.h>
#include <drivers/tty/tty.h>
#include <gdt.h>
#include <interrupts/idt.h>
#include <interrupts/isr.h>
#include <kernel/printk.h>
#include <mm/mm_types.h>

#include <kernel/kassert.h>
#include <kernel/stdlib.h>
#include <mm/pmap/pmap.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/vm_map.h>

#include <pic/apic/apic.h>
#include <pic/pic.h>
#include <pic/pit.h>

#include <kernel/string.h>
#include <stdbool.h>
#include <uacpi/uacpi.h>

#include <stddef.h>
#include <stdint.h>

#define IA32_APIC_BASE_MSR 0x1B
#define APIC_ENABLE        (1ULL << 11)

static inline uint64_t rdmsr(uint32_t msr) {
  uint32_t lo, hi;
  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t val) {
  uint32_t lo = (uint32_t)val;
  uint32_t hi = (uint32_t)(val >> 32);
  __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
}

void disable_lapic(void) {
  uint64_t apic_base = rdmsr(IA32_APIC_BASE_MSR);

  apic_base &= ~APIC_ENABLE;

  wrmsr(IA32_APIC_BASE_MSR, apic_base);
}

// entry point
void kmain(void) {
  boot_init();
  tty_init();
  init_gdt();
  init_idt();

  disable_lapic();

  init_pit(1000);

  init_bitmap_pmm();
  init_vm();
  init_pmap();

  init_kmalloc();

  void *tmp_bfr = kmalloc(PAGE_SIZE * 2);

  uacpi_setup_early_table_access(tmp_bfr, PAGE_SIZE * 2);

  init_apic();

  init_keyboard();

  printk("Hello kernel!\n");

  // We're done, just hang...
  hcf();
}
