#include "drivers/tty/tty.h"
#include <isr.h>
#include <kernel/printk.h>
#include <stdint.h>

static const char *exceptions[] = {
    "#DE: Divide Error",
    "#DB: Debug Exception",
    " — : NMI Interrupt",
    "#BP: Breakpoint",
    "#OF: Overflow",
    "#BR: BOUND Range Exceeded",
    "#UD: Invalid Opcode (Undefined Opcode)",
    "#NM: Device Not Available (No Math Coprocessor)",
    "#DF: Double Fault",
    "— : Coprocessor Segment Overrun (reserved)",
    "#TS: Invalid TSS",
    "#NP: Segment Not Present",
    "#SS: Stack-Segment Fault",
    "#GP: General Protection",
    "#PF: Page Fault",
    "— : (Intel reserved. Do not use.)",
    "#MF: x87 FPU Floating-Point Error (Math Fault)",
    "#AC: Alignment Check",
    "#MC: Machine Check",
    "#XM: SIMD Floating-Point Exception",
    "#VE: Virtualization Exception",
    "#CP: Control Protection Exception",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use."};

// __attribute__((interrupt))
void interrupt_handler(struct interrupt_frame *frame) {
  if (frame->int_no <= 31) {
    tty_clear();
    printk(LOG_ERR "Exception occurred!\n\n");
    printk(LOG_DEBUG "ISR- No. %d: %s\n", frame->int_no,
           exceptions[frame->int_no]);
    printk(LOG_DEBUG "Error code: 0x%.16llx\n\n", frame->error_code);
    printk(LOG_DEBUG "rax: 0x%.16llx, rbx:    0x%.16llx, rcx: 0x%.16llx, "
                     "rdx: 0x%.16llx\n"
                     "rsi: 0x%.16llx, rdi:    0x%.16llx, rbp: 0x%.16llx, "
                     "r8 : 0x%.16llx\n"
                     "r9 : 0x%.16llx, r10:    0x%.16llx, r11: 0x%.16llx, "
                     "r12: 0x%.16llx\n"
                     "r13: 0x%.16llx, r14:    0x%.16llx, r15: 0x%.16llx, "
                     "ss : 0x%.16llx\n"
                     "rsp: 0x%.16llx, rflags: 0x%.16llx, cs : 0x%.16llx, "
                     "rip: 0x%.16llx\n",
           frame->rax, frame->rbx, frame->rcx, frame->rdx, frame->rsi,
           frame->rdi, frame->rbp, frame->r8, frame->r9, frame->r10, frame->r11,
           frame->r12, frame->r13, frame->r14, frame->r15, frame->ss,
           frame->rsp, frame->rflags, frame->cs, frame->rip);
  }
}
