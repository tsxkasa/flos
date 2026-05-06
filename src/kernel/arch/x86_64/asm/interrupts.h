#ifndef _KERNEL_ASM_INTR_H
#define _KERNEL_ASM_INTR_H

static inline void enable_interrupts(void) { asm volatile("sti"); }

static inline void disable_interrupts(void) { asm volatile("cli"); }

#endif // _KERNEL_ASM_INTR_H
