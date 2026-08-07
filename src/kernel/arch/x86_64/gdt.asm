%include "asm-offsets.inc"
global _load_gdt

; void _load_gdt(struct gdtr_t *gdt);
_load_gdt:
  cli ; clear interrupt before setting up idt
  lgdt  [rdi]

  mov ax, GDT_TSS
  ltr ax
 
  mov ax, GDT_KERNEL_DS
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  pop rdi
  mov rax, GDT_KERNEL_CS
  push rax ; push kernel code seg
  push rdi ; push return address
  retfq ; far return
