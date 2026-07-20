#include <rbtree.h>
#include <sched/scheduler.h>
#include <sched/task.h>
#include <stdlib.h>
#include <string.h>

struct runq *system_runq;

static inline uint64_t min_vruntime(struct runq *r) {
  if (r->tree.rb_leftmost == NULL)
    return 0;

  return rb_entry(r->tree.rb_leftmost, struct sched_info, node)->vruntime;
}

static inline task_t *node_to_task(struct rb_node *node) {
  struct sched_info *sched = rb_entry(node, struct sched_info, node);
  task_t *t = sched == NULL ? NULL : container_of(sched, task_t, sched);
  return t;
}

void init_runq(task_t *t) {
  struct runq *rq = kmalloc(sizeof(struct runq));
  memset(rq, 0, sizeof(*rq));
  system_runq = rq;

  task_t *idle = ktask_fork(t, idle_task, NULL);

  idle->state = S_TASK_INIT;
  idle->tid = 0;

  rq->idle = idle;
}

static inline void tree_insert(struct runq *rq, task_t *t) {
  struct sched_info *se = &t->sched;

  struct rb_node **link = &rq->tree.rb_root.rb_node;
  struct rb_node *parent = NULL;
  struct sched_info *entry;

  while (*link) {
    parent = *link;
    entry = rb_entry(parent, struct sched_info, node);

    if (se->vruntime < entry->vruntime)
      link = &parent->rb_left;
    else
      link = &parent->rb_right;
  }

  rb_link_node(&se->node, parent, link);
  rb_insert_color(&se->node, &rq->tree.rb_root);

  // update cached leftmost
  if (!rq->tree.rb_leftmost ||
      se->vruntime <
          rb_entry(rq->tree.rb_leftmost, struct sched_info, node)->vruntime) {
    rq->tree.rb_leftmost = &se->node;
  }

  rq->num_threads++;
}

static inline void tree_remove(struct runq *rq, task_t *t) {
  struct sched_info *se = &t->sched;

  if (rq->tree.rb_leftmost == &se->node) {
    struct rb_node *next = rb_next(&se->node);
    rq->tree.rb_leftmost = next;
  }

  rb_erase(&se->node, &rq->tree.rb_root);
  rq->num_threads--;
}

void add_runq(task_t *t) {
  struct runq *rq = system_runq;

  if (t->state != S_TASK_INIT)
    t->sched.vruntime = min_vruntime(rq);

  tree_insert(rq, t);
}

void remove_runq(task_t *t) {
  struct runq *rq = system_runq;
  tree_remove(rq, t);
}

task_t *next_runq(void) {
  struct runq *rq = system_runq;

  struct rb_node *node = rb_first_cached(&rq->tree);
  task_t *next = node_to_task(node);
  if (next != NULL)
    tree_remove(rq, next);

  if (next == NULL)
    next = rq->idle;

  return next;
}
