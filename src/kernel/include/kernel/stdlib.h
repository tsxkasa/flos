#ifndef _KERNEL_STDLIB_H
#define _KERNEL_STDLIB_H

void init_kmalloc(void);
void *kmalloc(int size);
void kfree(void *ptr);

#endif // _KERNEL_STDLIB_H
