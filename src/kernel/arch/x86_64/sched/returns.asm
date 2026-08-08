%include "percpu.asm"
%include "asm-offsets.inc"

; task_t *current_task
extern current_task

; extern void __kfork_return(void);
global __kfork_return
__kfork_return:
  pop rdi
  pop rax
  sti
  call rax

  PERCPU_OFFSET rbx, current_task
  mov rdi, [gs:rbx]
  mov rsi, 0
  extern exit_task ; void exit_task(task_t *task, int code);
  call exit_task

; TODO:
; extern void __ufork_return(void);
; extern void __attribute__((noreturn)) __execve_return(uint64_t ent, uint64_t sp);
global __execve_return
__execve_return:
  pushfq
  pop rax
  or rax, 0x200

  push qword (GDT_USER_DS | 0x3)
  push qword rsi
  push qword rax
  push qword (GDT_USER_CS | 0x3)
  push qword rdi
  iretq
