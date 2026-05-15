#include <asm/io.h>
#include <interrupts/isr.h>
#include <kernel/printk.h>
#include <pic/pic.h>
#include <pic/pit.h>
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

#define PIT_FREQUENCY 1193182

uint64_t pit_count = 0;

static uint64_t pit_interrupt_handler(struct interrupt_frame *frame) {
  pit_count++;
  pic_signal_EOI(0);
  return (uint64_t)frame;
}

/**
 * @brief initiate programmable interval timer for x64 machines
 *
 * @param target_hz target frequency to calibrate pit for, defaults to 1000 if 0
 * is provided
 */
void init_pit(uint64_t target_hz) {
  if (target_hz < 19 || target_hz > PIT_FREQUENCY) {
    target_hz = 1000;
  }
  uint16_t divisor = PIT_FREQUENCY / target_hz;

  // Command byte:
  // 00 = channel 0
  // 11 = access mode: lobyte/hibyte
  // 010 = mode 2 (rate generator)
  // 0 = binary mode
  outb(PIT_COMMAND, 0x34);

  // Send divisor low byte first
  outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));

  // Then high byte
  outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

  register_interrupt_handler(0x20 + 0x00, pit_interrupt_handler);
  pic_irq_clear_mask(0);
  printk(LOG_INFO "PIT initialized\n");
}
