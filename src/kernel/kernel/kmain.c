#include <boot/boot.h>
#include <cpu/halt.h>
#include <drivers/tty/tty.h>
#include <gdt.h>
#include <kernel/printk.h>

// entry point
void kmain(void) {
  // asm volatile("int3"); // DEBUG
  boot_init();
  terminal_initialize();
  init_gdt();

  printk("Hello kernel!\n");
  printk("linked %s!\n", "libc");
  printk("test %c.\n", 65);

  // We're done, just hang...
  hcf();
}
