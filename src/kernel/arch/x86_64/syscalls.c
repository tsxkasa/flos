#include <interrupts/idt.h>
#include <interrupts/isr.h>
#include <kernel/syscalls.h>
#include <printk.h>
#include <stdint.h>

long syscall_dispatch(int num, const uint64_t *args) {
  switch (num) {
  case SYSCALL_READ:
    // return sys_read(args[0], args[1], args[2]);
  case SYSCALL_WRITE:
    printk("printk called from syscall\n");
    // return sys_write(args[0], args[1]);
  default:
    return -1;
  }
}

void syscall_interrupt_handler(struct interrupt_frame *frame) {
  int call_num = frame->rax;
  uint64_t args[6] = {frame->rdi, frame->rsi, frame->rdx,
                      frame->r10, frame->r8,  frame->r9};
  frame->rax = syscall_dispatch(call_num, args);
}

void init_syscalls() {
  register_interrupt_handler(SYSCALL_INTERRUPT_X86_64,
                             syscall_interrupt_handler);
}
