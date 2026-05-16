#include <boot/boot.h>
#include <interrupts/isr.h>
#include <mm/mm_types.h>
#include <mm/pmap/pmap.h>
#include <pic/apic/lapic.h>
#include <pic/pit.h>

#define LAPIC_REG_ID            0x020 // Local APIC ID Register
#define LAPIC_REG_VERSION       0x030 // LAPIC Version Register
#define LAPIC_REG_TPR           0x080 // Task Priority Register
#define LAPIC_REG_APR           0x090 // Arbitration Priority Register
#define LAPIC_REG_PPR           0x0A0 // Processor Priority Register
#define LAPIC_REG_EOI           0x0B0 // End Of Interrupt register
#define LAPIC_REG_RRD           0x0C0 // Remote Read Register
#define LAPIC_REG_LDR           0x0D0 // Logical Destination Register
#define LAPIC_REG_DFR           0x0E0 // Destination Format Register
#define LAPIC_REG_SVR           0x0F0 // Spurious Interrupt Vector Register
#define LAPIC_REG_ISR_BASE      0x100 // In-Service Register base
#define LAPIC_REG_TMR_BASE      0x180 // Trigger Mode Register base
#define LAPIC_REG_IRR_BASE      0x200 // Interrupt Request Register base
#define LAPIC_REG_ESR           0x280 // Error Status Register
#define LAPIC_REG_LVT_CMCI      0x2F0 // Corrected Machine Check Interrupt
#define LAPIC_REG_ICR_LOW       0x300 // Interrupt Command Register (low)
#define LAPIC_REG_ICR_HIGH      0x310 // Interrupt Command Register (high)
#define LAPIC_REG_LVT_TIMER     0x320 // Timer interrupt configuration
#define LAPIC_REG_LVT_THERMAL   0x330 // Thermal sensor interrupt
#define LAPIC_REG_LVT_PMC       0x340 // Performance monitoring interrupt
#define LAPIC_REG_LVT_LINT0     0x350 // Local Interrupt 0
#define LAPIC_REG_LVT_LINT1     0x360 // Local Interrupt 1
#define LAPIC_REG_LVT_ERROR     0x370 // Error interrupt configuration
#define LAPIC_REG_TIMER_INITIAL 0x380 // Timer Initial Count Register
#define LAPIC_REG_TIMER_CURRENT 0x390 // Timer Current Count Register
#define LAPIC_REG_TIMER_DIVIDE  0x3E0 // Timer Divide Configuration Register

#define LAPIC_LVT_VECTOR_MASK 0x000000FF

#define LAPIC_LVT_DELIVERY_FIXED   (0 << 8)
#define LAPIC_LVT_DELIVERY_LOWEST  (1 << 8)
#define LAPIC_LVT_DELIVERY_SMI     (2 << 8)
#define LAPIC_LVT_DELIVERY_NMI     (4 << 8)
#define LAPIC_LVT_DELIVERY_INIT    (5 << 8)
#define LAPIC_LVT_DELIVERY_EXTINT  (7 << 8)
#define LAPIC_LVT_DELIVERY_PENDING (1 << 12)
#define LAPIC_LVT_ACTIVE_LOW       (1 << 13)
#define LAPIC_LVT_REMOTE_IRR       (1 << 14)
#define LAPIC_LVT_LEVEL_TRIGGER    (1 << 15)
#define LAPIC_LVT_MASKED           (1 << 16)

#define LAPIC_TIMER_MODE_PERIODIC     (1 << 17)
#define LAPIC_TIMER_MODE_TSC_DEADLINE (2 << 17)

#define LAPIC_SVR_ENABLE (1 << 8)

#define LAPIC_ICR_VECTOR_MASK       0x000000FF
#define LAPIC_ICR_DELIVERY_FIXED    (0 << 8)
#define LAPIC_ICR_DELIVERY_LOWEST   (1 << 8)
#define LAPIC_ICR_DELIVERY_SMI      (2 << 8)
#define LAPIC_ICR_DELIVERY_NMI      (4 << 8)
#define LAPIC_ICR_DELIVERY_INIT     (5 << 8)
#define LAPIC_ICR_DELIVERY_STARTUP  (6 << 8)
#define LAPIC_ICR_DEST_LOGICAL      (1 << 11)
#define LAPIC_ICR_DELIVERY_PENDING  (1 << 12)
#define LAPIC_ICR_LEVEL_ASSERT      (1 << 14)
#define LAPIC_ICR_TRIGGER_LEVEL     (1 << 15)
#define LAPIC_ICR_DEST_SELF         (1 << 18)
#define LAPIC_ICR_DEST_ALL          (2 << 18)
#define LAPIC_ICR_DEST_ALL_BUT_SELF (3 << 18)

#include <kernel/printk.h>
#include <stdint.h>

static volatile void *lapic_base;
static volatile uint64_t lapic_ticks;

static inline uint32_t lapic_read(uint32_t reg) {
  return *(volatile uint32_t *)((uint8_t *)lapic_base + reg);
}

static inline void lapic_write(uint32_t reg, uint32_t val) {
  *(volatile uint32_t *)((uint8_t *)lapic_base + reg) = val;
}

static inline uint64_t rdmsr(uint32_t msr) {
  uint32_t lo, hi;
  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t val) {
  uint32_t lo = (uint32_t)val;
  uint32_t hi = (uint32_t)(val >> 32);
  __asm__ volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
}

static uint64_t spurious_handler(struct interrupt_frame *frame) {
  lapic_eoi();
  return (uint64_t)frame;
}

static uint64_t lapic_timer_hander(struct interrupt_frame *frame) {
  lapic_ticks++;
  lapic_eoi();
  if (lapic_ticks % 1000 == 0) {
    printk(LOG_DEBUG "LAPIC: %u seconds passed, ticks at: %llu\n",
           lapic_ticks / 1000, lapic_ticks);
  }
  return (uint64_t)frame;
}

void lapic_eoi(void) { lapic_write(LAPIC_REG_EOI, 0); }

static uint32_t lapic_ticks_per_ms = 0;

static void lapic_timer_calibrate(void) {
  lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);
  lapic_write(LAPIC_REG_LVT_TIMER, LAPIC_LVT_MASKED);

  lapic_write(LAPIC_REG_TIMER_INITIAL, 0xFFFFFFFF);

  pit_sleep(10);

  uint32_t ticks = 0xFFFFFFFF - lapic_read(LAPIC_REG_TIMER_CURRENT);

  lapic_ticks_per_ms = ticks / 10;
  lapic_write(LAPIC_REG_TIMER_INITIAL, 0);

  printk(LOG_INFO "LAPIC timer: %u ticks/ms\n", lapic_ticks_per_ms);
}

void init_lapic(void) {
#define IA32_APIC_BASE_MSR 0x1B
#define APIC_ENABLE        (1ULL << 11)

  uint64_t msr = rdmsr(IA32_APIC_BASE_MSR);
  uintptr_t phys_base = msr & 0xFFFFF000ULL;
  pmap_map_mmio(phys_base, PAGE_SIZE);

  wrmsr(IA32_APIC_BASE_MSR, msr | APIC_ENABLE);

  lapic_base = (void *)(phys_base + boot_get_hhdm_offset());

  lapic_write(LAPIC_REG_TPR, 0);

  // configure LINT0 as ExtINT (pass PIC interrupts through)
  lapic_write(LAPIC_REG_LVT_LINT0, LAPIC_LVT_DELIVERY_EXTINT);

  lapic_write(LAPIC_REG_LVT_LINT1, LAPIC_LVT_DELIVERY_NMI);

  // enable LAPIC via spurious interrupt vector register
  lapic_write(LAPIC_REG_SVR, LAPIC_SVR_ENABLE | 0xFF);

  printk(LOG_INFO "LAPIC id=%u initialized\n", lapic_read(LAPIC_REG_ID) >> 24);

  register_interrupt_handler(0xFF, spurious_handler);
  register_interrupt_handler(0x20, lapic_timer_hander);

  lapic_timer_calibrate();

  lapic_timer_start(1000);

#undef IA32_APIC_BASE_MSR
#undef APIC_ENABLE
}

void lapic_timer_start(uint32_t hz) {
  uint32_t period = lapic_ticks_per_ms * 1000 / hz;

  lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3); // divide by 16
  lapic_write(LAPIC_REG_LVT_TIMER,
              LAPIC_TIMER_MODE_PERIODIC | INTERRUPT_TIMER_VECTOR);
  lapic_write(LAPIC_REG_TIMER_INITIAL, period);

  printk(LOG_INFO "LAPIC timer started at %u Hz\n", hz);
}
