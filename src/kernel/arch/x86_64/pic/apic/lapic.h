#ifndef _KERNEL_LAPIC_H
#define _KERNEL_LAPIC_H

#include <cpu/percpu.h>
#include <stdint.h>

DECLARE_PERCPU(uint64_t, counter);

void init_lapic(void);
void lapic_eoi(void);
void lapic_timer_start(uint32_t hz);

#endif // _KERNEL_LAPIC_H
