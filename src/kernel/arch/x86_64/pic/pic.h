#ifndef _KERNEL_DEV_PIC_H
#define _KERNEL_DEV_PIC_H

#include <stdint.h>

void pic_remap(int offset1, int offset2);
void pic_mask_all(void);
void pic_irq_set_mask(uint8_t irq_line);
void pic_irq_clear_mask(uint8_t irq_line);
void pic_signal_EOI(uint64_t irq);

#endif // _KERNEL_DEV_PIC_H
