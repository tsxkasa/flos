#ifndef _ASM_OFFSETS_H
#define _ASM_OFFSETS_H

#include <gdt.h>
#include <mm/pmap/arch_pmap_types.h>
#include <mm/vm/vm_map.h>
#include <sched/task.h>
#include <stddef.h>

#define OFFSET_PT_PHY          offsetof(struct page_table_t, pml4_phy)
#define OFFSET_TASK_STACK      offsetof(task_t, stack)
#define OFFSET_TASK_KSTACK_TOP offsetof(task_t, kstack_top)
#define OFFSET_TASK_VMMAP      offsetof(task_t, vmap)
#define OFFSET_VM_MAP_PT       offsetof(vm_map_t, page_table)
#define OFFSET_TSS_RSP0        offsetof(struct tss_t, rsp0)

#endif // _ASM_OFFSETS_H
