#ifndef _KERNEL_VMA_H
#define _KERNEL_VMA_H

#include <mm/vm/vm_object.h>
#include <stddef.h>
#include <stdint.h>

typedef struct vm_area {
  uintptr_t start;
  uintptr_t end;
  uint32_t flags;

  struct vm_object_t *object;
  size_t offset;

  struct vm_area *next;
} vm_area_t;

#endif // _KERNEL_VMA_H
