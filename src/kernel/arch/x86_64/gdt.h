#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

struct tss_t;

#include <cpu/percpu.h>
#include <stdint.h>

struct gdtr_t {
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

struct tss_t {
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

#define GDT_SEL(entry) ((entry) << 3)

enum {
  GDT_ENTRY_NULL,
  GDT_ENTRY_KERNEL_CS,
  GDT_ENTRY_KERNEL_DS,
  GDT_ENTRY_USER_DS,
  GDT_ENTRY_USER_CS,
  GDT_ENTRY_TSS_LO,
  GDT_ENTRY_TSS_HI,

  GDT_ENTRIES
};

#define __KERNEL_CS GDT_ENTRY_SEL(GDT_ENTRY_KERNEL_CS)
#define __KERNEL_DS GDT_ENTRY_SEL(GDT_ENTRY_KERNEL_DS)
#define __USER_DS   GDT_ENTRY_SEL(GDT_ENTRY_USER_DS)
#define __USER_CS   GDT_ENTRY_SEL(GDT_ENTRY_USER_CS)
#define __TSS       GDT_ENTRY_SEL(GDT_ENTRY_TSS_LO)

void init_boot_gdt(void);

#endif // _KERNEL_GDT_H
