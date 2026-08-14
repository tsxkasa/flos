#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#define IDT_GATE_INTERRUPT 0x0E
#define IDT_GATE_TRAP      0x0F
#define IDT_DPL0           0x00
#define IDT_DPL3           0x60
#define IDT_PRESENT        0x80

#define IDT_KERNEL_INTERRUPT (IDT_PRESENT | IDT_DPL0 | IDT_GATE_INTERRUPT)
#define IDT_USER_INTERRUPT   (IDT_PRESENT | IDT_DPL3 | IDT_GATE_INTERRUPT)

#define SYSCALL_INTERRUPT_X86_64 0x80

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
