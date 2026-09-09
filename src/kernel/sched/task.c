#include <cpu/percpu.h>
#include <mm/pmap/pmap.h>
#include <mm/vm/vm_map.h>
#include <printk.h>
#include <sched/scheduler.h>
#include <sched/task.h>
#include <stdlib.h>

struct utask_start {
  void (*entry)(void *);
  void *args;
};
__attribute__((noreturn)) static void utask_trampoline(void *arg) {
  struct utask_start *start = arg;

  void (*entry)(void *) = start->entry;
  void *args = start->args;

  kfree(start);

  ktask_execve(entry, args);

  __builtin_unreachable();
}

task_t *utask_spawn(void (*entry)(void *), void *args) {
  struct utask_start *start = kmalloc(sizeof(struct utask_start));
  if (start == 0)
    return 0;
  start->entry = entry;
  start->args = args;
  task_t *t = ktask_spawn(utask_trampoline, start);

  if (t == 0) {
    kfree(start);
    return 0;
  }
  return t;
}

void task_wake(task_t *task) {
  if (task->state != S_TASK_RUNNABLE) {
    task->state = S_TASK_RUNNABLE;
    sched_add(task);
  }
}

void task_exit(int code) {
#define current this_cpu_read(current_task)
  if (current->state == S_TASK_RUNNABLE)
    sched_remove(current);

  current->state = S_TASK_ZOMBIE;

  sched_yield();
  // printk(LOG_ERR "Current task cannot exit");
#undef current
}
