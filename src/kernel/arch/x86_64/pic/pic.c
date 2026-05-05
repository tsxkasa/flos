#include <kernel/sys/io.h>
#include <pic/pic.h>

#define PIC1 0x20            // I/O base address for master PIC
#define PIC2 0xA0            // I/O base address for slave PIC
#define PIC1_COMMAND PIC1    // master PIC command port
#define PIC1_DATA (PIC1 + 1) // master PIC data port
#define PIC2_COMMAND PIC2    // slave PIC command port
#define PIC2_DATA (PIC2 + 1) // slave PIC data port
#define PIC_EOI 0x20         // end of interrupt command code

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define CASCADE_IRQ 2

/**
 * @brief signals interrupt for irq line, irq should be pic mapped
 * from irq 0 to irq 15
 *
 * @param irq   interrupt line ranging from 0 to 15
 */
void pic_signal_EOI(uint64_t irq) {
  if (irq >= 8)
    outb(PIC2_COMMAND, PIC_EOI);

  outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * @brief remaps pic master and slave to two offsets
 *
 * @param offset1   master PIC vector offset
 * @param offset2   slave PIC vector offset
 */
void pic_remap(int offset1, int offset2) {
  outb(PIC1_COMMAND,
       ICW1_INIT |
           ICW1_ICW4); // starts the initialization sequence (in cascade mode)
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
  io_wait();
  outb(PIC1_DATA, 1 << CASCADE_IRQ); // ICW3: tell Master PIC that there is a
                                     // slave PIC at IRQ2
  io_wait();
  outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();

  outb(PIC1_DATA,
       ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  // enable as needed
  outb(PIC1_DATA, 0xff);
  outb(PIC2_DATA, 0xff);
}

/**
 * @brief masks an interupt vector
 *
 * @param irq   interrupt line ranging from 0 to 15
 */
void pic_irq_set_mask(uint8_t irq_line) {
  uint16_t port;
  uint8_t value;

  if (irq_line < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    irq_line -= 8;
  }
  value = inb(port) | (1 << irq_line);
  outb(port, value);
}

/**
 * @brief clears interrupt mask
 *
 * @param irq   interrupt line ranging from 0 to 15
 */
void pic_irq_clear_mask(uint8_t irq_line) {
  uint16_t port;
  uint8_t value;

  if (irq_line < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    irq_line -= 8;
  }
  value = inb(port) & ~(1 << irq_line);
  outb(port, value);
}
