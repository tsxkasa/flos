#include <cpu/percpu.h>
#include <macros.h>
#include <mm/address.h>
#include <mm/mm_types.h>
#include <mm/pmap/pmap.h>
#include <mm/pmm/pmm.h>
#include <printk.h>
#include <sched/returns.h>
#include <sched/scheduler.h>
#include <sched/task.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

task_t *ktask_fork(task_t *parent, void (*entry)(void *), void *args) {
  if (!parent)
    parent = this_cpu_read(current_task);

  task_t *t = kmalloc(sizeof(task_t));
  memset(t, 0, sizeof(*t));

  t->flags = parent->flags;
  t->tid = alloc_tid();
  t->tgid = t->tid;

  printk(LOG_INFO "forked task %d, to %d\n", parent->tid, t->tid);

  uintptr_t kstack =
      KSTACK_SIZE + phys_to_higher_half_data(pmm_alloc_pages(KSTACK_PAGES));
  uint64_t *stack = (uint64_t *)kstack;

  // setup stack for new task
  t->vmap = kernel_vm_map;
  *--stack = (uint64_t)entry;
  *--stack = (uint64_t)args;
  *--stack = (uint64_t)__kfork_return; // this is where switch_to will return to

  // setup stack for each register of callee saved register, and since this
  // function is for kernel task, it'll be zero
  int i = 6; // rbx, rbp, r12..15
  while (i-- > 0) {
    *--stack = 0;
  }
  t->stack = (uint64_t)stack;

  t->state = S_TASK_INIT;
  return t;
}
