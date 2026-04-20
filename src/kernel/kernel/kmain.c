#include <boot/boot.h>
#include <cpu/halt.h>
#include <drivers/tty/tty.h>
#include <gdt.h>
#include <idt.h>
#include <kernel/printk.h>

// entry point
void kmain(void) {
  // asm volatile("int3"); // DEBUG
  boot_init();
  tty_init();
  init_gdt();
  init_idt();

  printk("Hello kernel!\n");

  // We're done, just hang...
  hcf();
}
