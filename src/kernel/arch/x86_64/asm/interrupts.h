#ifndef _KERNEL_ASM_INTR_H
#define _KERNEL_ASM_INTR_H

static inline void enable_interrupts(void) { __asm__ __volatile__("sti"); }

static inline void disable_interrupts(void) { __asm__ __volatile__("cli"); }

#endif // _KERNEL_ASM_INTR_H
