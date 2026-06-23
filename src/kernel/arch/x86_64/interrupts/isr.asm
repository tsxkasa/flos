%include "stackops.asm"
extern interrupt_handler

global _isr_handler

_isr_handler:
  pusha64

  cld

  mov rdi, rsp
  call interrupt_handler

  popa64
  add rsp, 16

  iretq
