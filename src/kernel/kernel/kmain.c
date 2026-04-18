#include <boot/boot.h>
#include <cpu/halt.h>
#include <kernel/tty.h>
#include <stdio.h>

// entry point
void kmain(void) {
  // asm volatile("int3"); // DEBUG
  boot_init();

  terminal_initialize();

  printf("Hello kernel!\n");
  printf("linked %s!\n", "libc");
  printf("test %c.\n", 65);

  // We're done, just hang...
  hcf();
}
