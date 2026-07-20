#include <cpu/percpu.h>
#include <mm/pmap/pmap.h>
#include <printk.h>
#include <sched/scheduler.h>
#include <sched/task.h>

void ktask_wake(task_t *task) {
  if (task->state != S_TASK_RUNNABLE) {
    task->state = S_TASK_RUNNABLE;
    sched_add(task);
  }
}

void exit_task(task_t *task, int code) {
  if (task->state == S_TASK_RUNNABLE)
    remove_runq(task);

  task->state = S_TASK_ZOMBIE;

  if (task == this_cpu_read(current_task)) {
    sched_yield();
    printk(LOG_ERR "Current task cannot exit");
  }
}

__attribute__((noreturn)) void ktask_execve(void (*fn)(void *), void *args) {
#define current this_cpu_read(current_task)

#undef current
}
