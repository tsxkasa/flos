#include <boot/boot.h>
#include <cpu/halt.h>
#include <cpu/percpu.h>
#include <drivers/input/ps2/keyboard/keyboard.h>
#include <drivers/tty/tty.h>
#include <gdt.h>
#include <interrupts/idt.h>
#include <interrupts/isr.h>
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

void test_proc1(void *_) {
  while (1) {
    printk(LOG_DEBUG "Executed test_proc\n");
  }
}

void bsp(void *_) {
  task_t *test = kthread_create(test_proc1, 0);
  ktask_wake(test);
  while (1) {
    printk(LOG_DEBUG "Executed bsp\n");
  }
}

// entry point
void kmain(void) {
  boot_init();
  tty_init();
  init_gdt();
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
  sched_run_bsp(bsp);

  printk("Hello kernel!\n");

  // We're done, just hang...
  hcf();
}
