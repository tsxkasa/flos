%macro pushall64r 0
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rbp
  push rdi
  push rsi
  push rdx
  push rcx
  push rbx
  push rax
%endmacro

%macro popall64r 0
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rbp
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
%endmacro

extern interrupt_handler

global _isr_handler

_isr_handler:
  pushall64r

  cld

  mov rdi, rsp

  mov rbx, rsp
  and rsp, -16        ; Align stack to a 16-byte boundary

  call interrupt_handler
  mov rsp, rbx

  popall64r

  add rsp, 16

  iretq
