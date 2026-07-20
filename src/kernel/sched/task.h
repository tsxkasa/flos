#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <cpu/halt.h>
#include <cpu/percpu.h>
#include <interrupts/isr.h>
#include <list.h>
#include <mm/vm/vm_map.h>
#include <rbtree.h>
#include <sched/thread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TASK_FREE      0
#define TASK_KTHREAD   (1 << 0)
#define TASK_PREEMPTED (1 << 1)
#define TASK_RUNNING   (1 << 2)

typedef struct task task_t;

DECLARE_PERCPU(task_t *, current_task);
DECLARE_PERCPU(uint64_t, preempt_count);

extern void __switch_to(void *prev, void *next);

#define MAX_SLICES 10

struct sched_info {
  struct rb_node node;
  uint64_t vruntime; // total runtime weighed by how important runtime is
  uint64_t time_slice;
};

typedef struct task {
  uint64_t stack;
  uint64_t tid;
  uint64_t tgid;

  uint64_t flags;

  vm_map_t *vmap;

  enum {
    S_TASK_INIT,     // initial state, hasn't started yet
    S_TASK_RUNNABLE, // running if running flag set, if not, its runnable
    S_TASK_WAITING,  // wait queues, such as blocks
    S_TASK_SLEEPING, // will be woken up
    S_TASK_ZOMBIE,   // waiting to be reclaimed
    S_TASK_DEAD,     // need to be removed
  } state;
  int prio;
  struct sched_info sched;

  thread_t thread;

  // void (*entry)(void *);
  // void *arg;

  uintptr_t stack_base;
  size_t stack_size;
} task_t;

struct runq {
  task_t *idle;
  struct rb_root_cached tree;
  uint64_t num_threads;
};

task_t *ktask_fork(task_t *parent, void (*entry)(void *), void *args);
void ktask_execve(void (*fn)(void *), void *args);
void ktask_wake(task_t *task);
void exit_task(task_t *task, int code);

static inline task_t *kthread_create(void (*entry)(void *), void *arg) {
  return ktask_fork(this_cpu_read(current_task), (entry), (arg));
}

static inline void idle_task() { hcf(); }

#endif // _KERNEL_TASK_H
