#ifndef _ASM_OFFSETS_H
#define _ASM_OFFSETS_H

#include <mm/pmap/arch_pmap_types.h>
#include <mm/vm/vm_map.h>
#include <sched/thread.h>
#include <stddef.h>

#define OFFSET_PT_PHY        offsetof(struct page_table_t, pml4_phy)
#define OFFSET_TASK_STACKPTR offsetof(task_t, stack_ptr)
#define OFFSET_TASK_VMMAP    offsetof(task_t, vmap)

#endif // _ASM_OFFSETS_H
