#include <asm/msr.h>
#include <cpu/cpu.h>
#include <cpu/percpu.h>
#include <pic/apic/apic.h>
#include <printk.h>
#include <stdlib.h>
#include <string.h>

static void *percpu_base[MAX_CPU];

void init_percpu() {
  size_t cpu_count = get_cpu_count();
  size_t percpu_size = _percpu_end - _percpu_start;

  printk(LOG_INFO "percpu_size = %zu;\npercpu_start: %llx; percpu_end: %llx\n",
         percpu_size, _percpu_start, _percpu_end);

  for (size_t i = 0; i < cpu_count; i++) {
    percpu_base[i] = kmalloc(percpu_size);

    memcpy(percpu_base[i], _percpu_start, percpu_size);

    printk(LOG_INFO "percpu %i initialized at %llx.\n", i, percpu_base[i]);
  }

  wrmsr(MSR_GS_BASE, (uint64_t)percpu_base[0]);
}
