%include "asm-offsets.inc"
global _load_early_gdt
global _load_gdt

; void _load_early_gdt(struct gdtr_t *gdt);
_load_early_gdt:
  cli
  lgdt [rdi]

  mov ax, __KERNEL_DS
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  ; mov gs, ax

  pop rdi
  mov rax, __KERNEL_CS
  push rax ; push kernel code seg
  push rdi ; push return address
  retfq ; far return

; void _load_gdt(struct gdtr_t *gdt);
_load_gdt:
  cli ; clear interrupt before setting up idt
  lgdt  [rdi]

  mov ax, __TSS
  ltr ax
 
  mov ax, __KERNEL_DS
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  ; mov gs, ax

  pop rdi
  mov rax, __KERNEL_CS
  push rax ; push kernel code seg
  push rdi ; push return address
  retfq ; far return

