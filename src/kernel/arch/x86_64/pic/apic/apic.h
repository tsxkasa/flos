#ifndef _KERNEL_APIC_H
#define _KERNEL_APIC_H

#include <stddef.h>
#include <stdint.h>

#define MAX_IOAPIC 8
#define MAX_ISO    32
#define MAX_CPU    256

struct apic_cpu {
  uint8_t apic_uid;
  uint8_t apic_id;
  uint8_t enabled;
};
struct apic_iso {
  uint8_t source_irq;
  uint8_t gsi;
  uint8_t flags;
};
struct ioapic_info {
  uint8_t id;
  uintptr_t addr;
  uint32_t gsi_base;
};

struct apic_info {
  volatile uintptr_t lapic;

  struct ioapic_info ioapic[MAX_IOAPIC];
  size_t ioapic_count;

  struct apic_iso iso[MAX_ISO];
  size_t iso_count;

  struct apic_cpu cpus[MAX_CPU];
  size_t cpu_count;
};

void init_apic(void);

#endif // _KERNEL_APIC_H
