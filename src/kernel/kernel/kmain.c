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
  printk("linked %s!\n", "libc");

  int x = 10;
  int result;

  __asm__ volatile("movl %1, %%eax;" // Move variable x into EAX
                   "movl $0, %%ebx;" // Move 0 into EBX
                   "idivl %%ebx;"    // Divide EAX by EBX -> Exception
                   "movl %%eax, %0;" // Result (never reached)
                   : "=r"(result)    // Output
                   : "r"(x)          // Input
                   : "%eax", "%ebx"  // Clobbered registers
  );

  // We're done, just hang...
  hcf();
}
