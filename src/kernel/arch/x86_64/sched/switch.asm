%include "asm-offsets.inc"
%include "percpu.asm"

global __switch_to

; void __switch_to(* prev, * next)
; prev = rdi, next = rsi
__switch_to:
  push rbx
  push rbp
  push r12
  push r13
  push r14
  push r15

  mov [rdi + OFFSET_TASK_STACK], rsp
  mov rsp, [rsi + OFFSET_TASK_STACK]

  push rdi
  push rax

  ; has to happen here since control doesn't return to schedule()
  extern current_task
  PERCPU_OFFSET r8, current_task
  mov [gs:r8], rsi

  ; TODO: add cpuset save in pmap
  mov rax, [rsi + OFFSET_TASK_VMMAP]
  mov rdi, [rax + OFFSET_VM_MAP_PT]
  extern pmap_switch_pt
  call pmap_switch_pt
 
  pop rax
  pop rdi

  ; has to happen here since control doesn't return to schedule()
  extern preempt_count
  PERCPU_OFFSET r8, preempt_count
  lock dec qword [gs:r8]

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbp
  pop rbx

  ret
