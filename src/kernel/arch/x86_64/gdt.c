#include <gdt.h>
#include <stdio.h>

extern void _load_gdt(struct gdtr *gdt);

static struct gdt_desc gdt_descs[] = {
    // 0x0: null
    {0},

    // 0x8: kernel code
    {.limit = 0xffff,
     .base_low = 0x0000,
     .base_mid = 0x00,
     .access = 0x9a,
     .granularity = 0xa0,
     .base_hi = 0x00},

    // 0x10: kernel data
    {.limit = 0xffff,
     .base_low = 0x0000,
     .base_mid = 0x00,
     .access = 0x92,
     .granularity = 0xc0,
     .base_hi = 0x00},

    // 0x18: usermode data
    {.limit = 0xffff,
     .base_low = 0x0000,
     .base_mid = 0x00,
     .access = 0xf2,
     .granularity = 0xc0,
     .base_hi = 0x00},

    // 0x20: usermode code
    {.limit = 0xffff,
     .base_low = 0x0000,
     .base_mid = 0x00,
     .access = 0xfa,
     .granularity = 0xa0,
     .base_hi = 0x00},

    // 0x28: TSS descriptor low (base 0, limit 0)
    {.limit = 0x0000,
     .base_low = 0x0000,
     .base_mid = 0x00,
     .access = 0x89,
     .granularity = 0xa0,
     .base_hi = 0x00},

    // 0x30: TSS descriptor high (base upper = 0)
    {0}};

static struct gdtr gdt = {
    sizeof(gdt_descs) - 1,
    (uintptr_t)gdt_descs,
};

static struct tss tss;

void init_gdt(void) {
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

  printf("GDT initialized.\n");
}
