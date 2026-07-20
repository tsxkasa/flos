#ifndef _KERNEL_CPU_H
#define _KERNEL_CPU_H

#include <pic/apic/apic.h>
#include <stddef.h>
#include <stdint.h>

inline size_t get_cpu_count() { return apic_get_cpu_count(); }

#endif // _KERNEL_CPU_H
