#include <boot/boot.h>
#include <cpu/halt.h>
#include <cpu/percpu.h>
#include <drivers/input/ps2/keyboard/keyboard.h>
#include <drivers/tty/tty.h>
#include <gdt.h>
#include <interrupts/idt.h>
#include <interrupts/isr.h>
#include <kernel/syscalls.h>
#include <mm/mm_types.h>
#include <printk.h>

#include <kassert.h>
#include <mm/pmap/pmap.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/vm_map.h>
#include <sched/scheduler.h>
#include <sched/task.h>
#include <stdlib.h>

#include <pic/apic/apic.h>
#include <pic/pic.h>
#include <pic/pit.h>

#include <stdbool.h>
#include <uacpi/uacpi.h>

void umain(void *args) {
  for (;;) {
    asm volatile("mov $1, %%rax\n\t"
                 "int $0x80"
                 :
                 :
                 : "rax", "memory");
    __asm__ volatile("pause");
  }
}

void bsp(void *_) {
  while (1) {
    printk(LOG_DEBUG "Executed bsp, arg %llx\n", _);

    init_syscalls();

    ktask_execve(umain, 0);
  }
}

// entry point
void kmain(void) {
  boot_init();
  tty_init();
  init_boot_gdt();
  init_idt();

  init_bitmap_pmm();
  init_vm();
  init_pmap();

  init_kmalloc();

  void *tmp_bfr = kmalloc(PAGE_SIZE * 2);

  uacpi_setup_early_table_access(tmp_bfr, PAGE_SIZE * 2);

  init_apic();

  init_percpu(); // dependent on kmalloc

  init_keyboard();

  init_scheduler();
  init_late_gdt();

  sched_run_bsp(bsp);

  printk("Hello kernel!\n");

  // We're done, just hang...
  hcf();
}
