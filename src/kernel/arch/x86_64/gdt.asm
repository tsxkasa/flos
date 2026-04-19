global _load_gdt

; void _load_gdt(struct gdtr_t *gdt);
_load_gdt:
  lgdt  [rdi] ; first arg in UNIX

  mov ax, 0x28 ; tss seg low
  ltr ax
  
  mov ax, 0x10 ; kernel data seg
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  pop rdi
  mov rax, 0x08 ; kernel code seg
  push rax ; push kernel code seg
  push rdi ; push return address
  retfq ; far return
