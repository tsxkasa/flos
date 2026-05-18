#ifndef _KERNEL_STDLIB_H
#define _KERNEL_STDLIB_H

#include <stddef.h>
void init_kmalloc(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif // _KERNEL_STDLIB_H
