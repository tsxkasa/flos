#include <drivers/input/ps2/keyboard/keyboard.h>
#include <drivers/tty/tty.h>
#include <interrupts/isr.h>
#include <kernel/printk.h>
#include <kernel/sys/io.h>
#include <pic/pic.h>
#include <stdint.h>

static int extended_key = 0;

static uint64_t keyboard_irq_handler(struct interrupt_frame *frame) {
  uint8_t scancode = inb(0x60);

  if (scancode == 0xE0) {
    extended_key = 1;
    pic_signal_EOI(1);
    return (uint64_t)frame;
  }

  if (extended_key) {
    extended_key = 0;

    if (!(scancode & 0x80)) {
      switch (scancode) {
      case 0x49:
        tty_scroll_up();
        break; /* Page Up   */
      case 0x51:
        tty_scroll_down();
        break; /* Page Down */
      }
    }

    pic_signal_EOI(1);
    return (uint64_t)frame;
  }

  pic_signal_EOI(1);
  return (uint64_t)frame;
}

void init_keyboard() {
  register_interrupt_handler(INTERRUPT_KEYBOARD_CONTROLLER_VECTOR,
                             keyboard_irq_handler);
  pic_irq_clear_mask(1);
  outb(0x64, 0xAA);

  printk(LOG_INFO "PS/2 Keyboard initialized.\n");
}
