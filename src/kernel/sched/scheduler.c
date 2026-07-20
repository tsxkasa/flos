#include <asm/interrupts.h>
#include <cpu/percpu.h>
#include <mm/pmap/pmap.h>
#include <printk.h>
#include <rbtree.h>
#include <sched/scheduler.h>
#include <sched/task.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <timer.h>

DEFINE_PERCPU(task_t *, current_task);
DEFINE_PERCPU(uint64_t, preempt_count);
DEFINE_PERCPU(bool, need_resched);

static volatile uint64_t pid = 0;
static volatile uint64_t tid = 0;

static task_t dummy;

void schedule(void) {
#define current this_cpu_read(current_task)
  this_cpu_inc(preempt_count);

  this_cpu_write(need_resched, false);

  task_t *prev = current;
  prev->sched.vruntime++;

  if (current->tid != 0)
    add_runq(current);

  task_t *next = next_runq();

  if (next->tid == 0) {
    // balance + resched
  }

  current->flags &= ~TASK_RUNNING;
  next->flags |= TASK_RUNNING;
  next->flags &= ~TASK_PREEMPTED;

  // unsigned long cr3_val;
  // __asm__ __volatile__("mov %%cr3, %0"
  //                      : "=r"(cr3_val)
  //                      : /* no input */
  //                      : /* no clobbers */
  // );
  // printk(LOG_DEBUG
  //        "switching to task: vmap=%p pt=%p phys=%llx (current cr3=%llx)\n",
  //        next->vmap, next->vmap->page_table,
  //        *(uint64_t *)((char *)(next->vmap->page_table) + 8), cr3_val);

  __switch_to(current, next);
#undef current
}

void sched_add(task_t *t) { add_runq(t); }

void sched_yield() { schedule(); }

void sched_yield_preempt() {
  this_cpu_inc(preempt_count);
  this_cpu_read(current_task)->flags |= TASK_PREEMPTED;
  this_cpu_dec(preempt_count);

  schedule();
}

void init_scheduler() {
  this_cpu_write(need_resched, false);

  memset(&dummy, 0, sizeof(dummy));

  dummy.vmap = kernel_vm_map;

  dummy.tid = 0;

  this_cpu_write(current_task, &dummy);
}

void sched_run_bsp(void (*bsp)(void *)) {
  init_runq(&dummy);
  task_t *t = ktask_fork(&dummy, bsp, 0);
  ktask_wake(t);

  local_timer_start(1000);
  sched_yield();
}

uint64_t alloc_tid() { return tid++; }
uint64_t alloc_pid() { return pid++; }
