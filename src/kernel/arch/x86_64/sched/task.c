#include <cpu/halt.h>
#include <cpu/percpu.h>
#include <macros.h>
#include <mm/address.h>
#include <mm/mm_types.h>
#include <mm/pmap/pmap.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/vm_area.h>
#include <mm/vm/vm_map.h>
#include <printk.h>
#include <sched/returns.h>
#include <sched/scheduler.h>
#include <sched/task.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uacpi/uacpi.h>

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

__attribute__((noreturn)) void ktask_execve(void (*fn)(void *), void *args) {
#define current this_cpu_read(current_task)
  if (current->vmap == kernel_vm_map)
    current->vmap = vm_map_create();

#define UTASK_ENTRY        0x1000ull
#define UTASK_STACK_BOTTOM 0x3000ull
#define UTASK_STACK_TOP    0x5000ull

  uintptr_t entry = UTASK_ENTRY;
  uintptr_t fn_phys = kernel_virt_to_phys((uintptr_t)fn);
  pmap_map_page(current->vmap->page_table, entry,
                ALIGN_DOWN(fn_phys, PAGE_SIZE),
                MMU_FLAG_USER | MMU_FLAG_PRESENT);
  pmap_map_page(current->vmap->page_table, entry + PAGE_SIZE,
                ALIGN_DOWN(fn_phys + PAGE_SIZE, PAGE_SIZE),
                MMU_FLAG_USER | MMU_FLAG_PRESENT);

  uintptr_t fn_addr = (uintptr_t)fn;

  printk(LOG_DEBUG "fn virt  = %p\n", fn_addr);
  printk(LOG_DEBUG "fn phys  = %p\n", fn_phys);
  printk(LOG_DEBUG "fn off   = %p\n", fn_addr & (PAGE_SIZE - 1));

  for (size_t i = 0; i < 2; i++) {
    size_t off = i * PAGE_SIZE;
    uintptr_t page = phys_to_higher_half_data(pmm_alloc_page());
    pmap_map_page(current->vmap->page_table, UTASK_STACK_BOTTOM + off,
                  ALIGN_DOWN(higher_half_data_to_phys(page), PAGE_SIZE),
                  MMU_FLAG_USER | MMU_FLAG_WRITE | MMU_FLAG_PRESENT);
  }

  vm_area_t *code = kmalloc(sizeof(vm_area_t));
  memset(code, 0, sizeof *code);

  code->start = UTASK_ENTRY;
  code->end = UTASK_ENTRY + 2 * PAGE_SIZE;
  code->flags = MMU_FLAG_USER | MMU_FLAG_PRESENT;
  code->object = NULL;
  code->offset = 0;

  code->next = current->vmap->areas;
  current->vmap->areas = code;

  vm_area_t *stack = kmalloc(sizeof(vm_area_t));

  stack->start = UTASK_STACK_BOTTOM;
  stack->end = UTASK_STACK_TOP;
  stack->flags = MMU_FLAG_USER | MMU_FLAG_WRITE | MMU_FLAG_PRESENT;
  stack->object = NULL;
  stack->offset = 0;

  stack->next = current->vmap->areas;
  current->vmap->areas = stack;

  pmap_switch_pt(current->vmap->page_table);
  this_cpu_dec(preempt_count);

  uint8_t *user_code = (uint8_t *)UTASK_ENTRY;
  task_t *cc = current;
  (void)cc;

  printk(LOG_DEBUG "user byte: %02x\n", user_code[0]);
  printk(LOG_DEBUG "fn byte:   %02x\n", ((uint8_t *)fn)[0]);

  printk(LOG_DEBUG "entry = %p\n", entry);
  printk(LOG_DEBUG "stack = %p\n", UTASK_STACK_TOP);

  // asm volatile("cli");
  // hcf();

  entry += fn_addr &
           (PAGE_SIZE - 1);

               __execve_return((uint64_t)entry, (uint64_t)UTASK_STACK_TOP);
#undef current
}
