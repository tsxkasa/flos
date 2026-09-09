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
#include <stdint.h>
#include <stdlib.h>

#include <pic/apic/apic.h>
#include <pic/pic.h>
#include <pic/pit.h>

#include <stdbool.h>
#include <uacpi/uacpi.h>

static const char syscall_string[] = "printk called from user with arg: %llx\n";
static const char return_syscall[] = "syscall returned value: %llx\n";
void umain(void *args) {
  for (;;) {
    asm volatile("mov $1, %%rax\n\t"
                 "mov %0, %%rdi\n\t"
                 "mov $0xfda3d500, %%rsi\n\t"
                 "int $0x80\n\t"
                 "mov $1, %%rax\n\t"
                 "mov %0, %%rdi\n\t"
                 "movq 0x7ffffff, %%rsi\n\t"
                 "int $0x80\n\t"

                 :
                 : "r"(syscall_string)
                 : "rax", "rdi", "rsi", "memory");
  }
}

void umain2(void *args) {
  for (;;) {
    asm volatile("mov $1, %%rax\n\t"
                 "mov %0, %%rdi\n\t"
                 "mov $0xafafafaf, %%rsi\n\t"
                 "int $0x80\n\t"
                 "mov %1, %%rdi\n\t"
                 "mov %%rax, %%rsi\n\t"
                 "mov $1, %%rax\n\t"
                 "int $0x80"
                 :
                 : "r"(syscall_string), "r"(return_syscall)
                 : "rax", "rdi", "memory");
  }
}

void bsp(void *_) {
  printk(LOG_DEBUG "Executed bsp, arg %llx\n", _);
  printk("Hello kernel!\n");
  init_syscalls();

  task_t *t = utask_spawn(umain2, NULL);
  task_wake(t);

  ktask_execve(umain, 0);
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

  void *tmp_buf = kmalloc(PAGE_SIZE * 2);

  uacpi_setup_early_table_access(tmp_buf, PAGE_SIZE * 2);

  init_apic();

  init_percpu(); // dependent on kmalloc

  init_keyboard();

  init_scheduler();
  // have to run this percpu later
  init_cpu_gdt();

  sched_run_bsp(bsp);

  // We're done, just hang...
  hcf();
}
