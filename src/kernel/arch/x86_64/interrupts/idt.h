#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

struct idtr_t {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

struct idt_desc {
  uint16_t isr_low;
  uint16_t kernel_cs;
  uint8_t ist;
  uint8_t attributes;
  uint16_t isr_mid;
  uint32_t isr_high;
  uint32_t resv;
} __attribute__((packed));

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);

void init_idt(void);

#endif // _KERNEL_IDT_H
