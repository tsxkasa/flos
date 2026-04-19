#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

struct gdtr {
  uint16_t limit;
  uint64_t ptr;
} __attribute__((packed));

struct gdt_desc {
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_hi;
} __attribute__((packed));

struct tss {
  uint32_t resv0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t resv1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t resv2;
  uint16_t resv3;
  uint16_t iopb_offset;
} __attribute__((packed));

void init_gdt(void);

#endif // _KERNEL_GDT_H
