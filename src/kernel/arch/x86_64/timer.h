#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <pic/apic/lapic.h>

// TODO: make timer type configurable (pit, hpet, lapic...)
#define local_timer_start(hz) lapic_timer_start(hz)

#endif // _KERNEL_TIMER_H
