#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#include <interrupts/isr.h>
#include <list.h>
#include <sched/task.h>
#include <stdint.h>

#define DEFAULT_TICKS 10

DECLARE_PERCPU(bool, need_resched);

void schedule();
void init_scheduler(void);

void init_runq(task_t *t);
void add_runq(task_t *t);
void remove_runq(task_t *t);
task_t *next_runq();

void sched_yield(void);
void sched_yield_preempt(void);
void sched_add(task_t *t);

void sched_run_bsp(void (*bsp)(void *));

uint64_t alloc_tid();
uint64_t alloc_pid();

#endif // _KERNEL_SCHEDULER_H
