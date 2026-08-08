#ifndef _KERNEL_SYSCALLS_H
#define _KERNEL_SYSCALLS_H

#include <interrupts/isr.h>
#include <stddef.h>
#include <stdint.h>

enum {
  SYSCALL_READ,
  SYSCALL_WRITE,
  SYSCALL_OPEN,
  SYSCALL_CLOSE,
  SYSCALL_AMOUNT
};

void syscall_interrupt_handler(struct interrupt_frame *frame);

long syscall_dispatch(int num, const uint64_t *args);

void init_syscalls();

#endif // _KERNEL_SYSCALLS_H
