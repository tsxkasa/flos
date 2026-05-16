#ifndef _KERNEL_DEV_PIC_H
#define _KERNEL_DEV_PIC_H

#include <stdint.h>

#define PIC_IRQ_LINE_SYSTEM_TIMER        0
#define PIC_IRQ_LINE_KEYBOARD_CONTROLLER 1
#define PIC_IRQ_LINE_SERIAL_PORT_COM2    3
#define PIC_IRQ_LINE_SERIAL_PORT_COM1    4
#define PIC_IRQ_LINE_LPT2                5
#define PIC_IRQ_LINE_FLOPPY_CONTROLLER   6
#define PIC_IRQ_LINE_LPT1                7
#define PIC_IRQ_LINE_RTC_TIMER           8
#define PIC_IRQ_LINE_ACPI                9
#define PIC_IRQ_LINE_MOUSE_CONTROLLER    12
#define PIC_IRQ_LINE_MATH_COPROCESSOR    13
#define PIC_IRQ_LINE_ATA_CHANNEL_1       14
#define PIC_IRQ_LINE_ATA_CHANNEL_2       15

void pic_remap(int offset1, int offset2);
void pic_mask_all(void);
void pic_irq_set_mask(uint8_t irq_line);
void pic_irq_clear_mask(uint8_t irq_line);
void pic_signal_EOI(uint64_t irq);

#endif // _KERNEL_DEV_PIC_H
