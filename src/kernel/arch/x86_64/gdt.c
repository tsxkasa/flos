#include <cpu/percpu.h>
#include <gdt.h>
#include <mm/mm_types.h>
#include <printk.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void _load_gdt(struct gdtr_t *gdt);
extern void _load_early_gdt(struct gdtr_t *gdt);

static struct gdt_desc gdt_template[GDT_ENTRIES] = {
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

    [GDT_ENTRY_TSS_LO] = {.limit = sizeof(struct tss_t) - 1,
                          .base_low = 0x0000,
                          .base_mid = 0x00,
                          .access = 0x89,
                          .granularity = 0x00,
                          .base_hi = 0x00},

    [GDT_ENTRY_TSS_HI] = {0}};

DEFINE_PERCPU(struct gdt_desc, gdt_descs[GDT_ENTRIES]);
DEFINE_PERCPU(struct gdtr_t, gdtr);
DEFINE_PERCPU(struct tss_t, tss);

void init_boot_gdt(void) {
  static struct gdtr_t boot_gdtr = {
      sizeof(gdt_template) - 1,
      (uintptr_t)gdt_template,
  };

  _load_early_gdt(&boot_gdtr);

  printk(LOG_INFO "GDT initialized.\n");
}

// void init_cpu_gdt(void) {
// #define TSS_RSP0_STACK_PAGES 4
//   void *base = kmalloc(TSS_RSP0_STACK_PAGES * PAGE_SIZE);
//
//   if (!base) {
//     printk(LOG_ERR "failed to allocate TSS rsp0 stack\n");
//     return;
//   }
//
//   early_tss.rsp0 = (uint64_t)base + TSS_RSP0_STACK_PAGES * PAGE_SIZE;
//
//   printk(LOG_INFO "TSS rsp0 = %p\n", early_tss.rsp0);
// }

void init_cpu_gdt(void) {
#define TSS_RSP0_STACK_PAGES 4
  struct gdt_desc *tmp_gdt = (struct gdt_desc *)this_cpu_ptr(gdt_descs);
  struct tss_t *tmp_tss = this_cpu_ptr(tss);
  struct gdtr_t *tmp_gdtr = this_cpu_ptr(gdtr);

  memcpy(tmp_gdt, gdt_template, sizeof(gdt_template));
  memset(tmp_tss, 0, sizeof(*tmp_tss));
  tmp_tss->iopb_offset = sizeof(*tmp_tss);

  uint64_t tss_base = (uint64_t)tmp_tss;
  tmp_gdt[GDT_ENTRY_TSS_LO].base_low = tss_base & 0xffff;
  tmp_gdt[GDT_ENTRY_TSS_LO].base_mid = (tss_base >> 16) & 0xff;
  tmp_gdt[GDT_ENTRY_TSS_LO].base_hi = (tss_base >> 24) & 0xff;
  tmp_gdt[GDT_ENTRY_TSS_HI].limit = (tss_base >> 32) & 0xffff;
  tmp_gdt[GDT_ENTRY_TSS_HI].base_low = (tss_base >> 48) & 0xffff;

  tmp_gdtr->limit = sizeof(gdt_template) - 1;
  tmp_gdtr->ptr = (uint64_t)tmp_gdt;

  _load_gdt(tmp_gdtr);

  tmp_tss->rsp0 = tss_base;

  printk(LOG_INFO "TSS rsp0 = %p\n", tmp_tss->rsp0);
}
