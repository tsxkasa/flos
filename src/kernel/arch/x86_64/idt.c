#include <idt.h>
#include <kernel/printk.h>
#include <stdbool.h>
#include <stdint.h>

#define IDT_MAX_DESCRIPTORS 256

__attribute__((aligned(0x10))) static struct idt_desc idt[IDT_MAX_DESCRIPTORS];

static bool vectors[IDT_MAX_DESCRIPTORS];

extern void _load_idt(struct idtr_t *idt);

extern void *isr_stub_table[];

static struct idtr_t idtr;

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
  struct idt_desc *descriptor = &idt[vector];

  descriptor->isr_low = (uint64_t)isr & 0xFFFF;
  descriptor->kernel_cs = 0x08; // kernel code gdt offset
  descriptor->ist = 0;
  descriptor->attributes = flags;
  descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
  descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
  descriptor->resv = 0;
}

void init_idt() {
  idtr.base = (uintptr_t)&idt[0];
  idtr.limit = (uint16_t)sizeof(struct idt_desc) * IDT_MAX_DESCRIPTORS - 1;

  for (uint8_t vector = 0; vector < 32; vector++) {
    idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    vectors[vector] = true;
  }

  _load_idt(&idtr);

  printk(LOG_INFO "IDT initialized.\n");
}
