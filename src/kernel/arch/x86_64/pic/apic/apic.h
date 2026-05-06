#ifndef _KERNEL_APIC_H
#define _KERNEL_APIC_H

#include <stdint.h>
struct apic_info {
  uintptr_t lapic;
  uintptr_t ioapic;

  struct {
    uint8_t irq;
    uint8_t vec;
    uint8_t flags;
  } iso[32];

  struct {
    uint8_t apic_id;
    uint8_t enabled;
  } cpus[256];
};

void init_apic(void);

#endif // _KERNEL_APIC_H
