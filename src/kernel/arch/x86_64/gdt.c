#include <gdt.h>
#include <mm/mm_types.h>
#include <printk.h>
#include <stdint.h>
#include <stdlib.h>

extern void _load_gdt(struct gdtr_t *gdt);

static struct gdt_desc gdt_descs[GDT_ENTRIES] = {
    [GDT_ENTRY_NULL] = {0},

    [GDT_ENTRY_KERNEL_CS] = {.limit = 0xffff,
                             .base_low = 0x0000,
                             .base_mid = 0x00,
                             .access = 0x9a,
                             .granularity = 0xa0,
                             .base_hi = 0x00},

    [GDT_ENTRY_KERNEL_DS] = {.limit = 0xffff,
                             .base_low = 0x0000,
                             .base_mid = 0x00,
                             .access = 0x92,
                             .granularity = 0xc0,
                             .base_hi = 0x00},

    [GDT_ENTRY_USER_DS] = {.limit = 0xffff,
                           .base_low = 0x0000,
                           .base_mid = 0x00,
                           .access = 0xf2,
                           .granularity = 0xc0,
                           .base_hi = 0x00},

    [GDT_ENTRY_USER_CS] = {.limit = 0xffff,
                           .base_low = 0x0000,
                           .base_mid = 0x00,
                           .access = 0xfa,
                           .granularity = 0xa0,
                           .base_hi = 0x00},

    [GDT_ENTRY_TSS_LO] = {.limit = 0x0000,
                          .base_low = 0x0000,
                          .base_mid = 0x00,
                          .access = 0x89,
                          .granularity = 0xa0,
                          .base_hi = 0x00},

    [GDT_ENTRY_TSS_HI] = {0}};

static struct gdtr_t gdt = {
    sizeof(gdt_descs) - 1,
    (uintptr_t)gdt_descs,
};

static struct tss_t tss;

void init_boot_gdt(void) {
  // zeroes
  for (uint64_t i = 0; i < sizeof(tss); i++)
    ((uint8_t *)(void *)&tss)[i] = 0;

  uint64_t tss_base = ((uint64_t)&tss);

  gdt_descs[5].base_low = tss_base & 0xffff;
  gdt_descs[5].base_mid = (tss_base >> 16) & 0xff;
  gdt_descs[5].base_hi = (tss_base >> 24) & 0xff;
  gdt_descs[5].limit = sizeof(tss);
  gdt_descs[6].limit = (tss_base >> 32) & 0xffff;
  gdt_descs[6].base_low = (tss_base >> 48) & 0xffff;

  _load_gdt(&gdt);

  printk(LOG_INFO "GDT initialized.\n");
}

void init_late_gdt(void) {
#define TSS_RSP0_STACK_PAGES 4
  void *base = kmalloc(TSS_RSP0_STACK_PAGES * PAGE_SIZE);

  if (!base) {
    printk(LOG_ERR "failed to allocate TSS rsp0 stack\n");
    return;
  }

  tss.rsp0 = (uint64_t)base + TSS_RSP0_STACK_PAGES * PAGE_SIZE;

  printk(LOG_INFO "TSS rsp0 = %p\n", tss.rsp0);
}
