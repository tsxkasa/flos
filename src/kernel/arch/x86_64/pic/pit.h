#ifndef _KERNEL_PIT_H
#define _KERNEL_PIT_H

#include <stdint.h>

void init_pit(uint64_t target_hz);
void pit_sleep(uint32_t ms);

#endif // _KERNEL_PIT_H
