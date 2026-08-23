#include <asm-offsets.h>
#include <gdt.h>
#include <stdio.h>

int main() {
  printf("%%ifndef __ASM_OFFSETS_NASM_INC\n");
  printf("%%define __ASM_OFFSETS_NASM_INC\n");

#define PRINT_DEFINE(x) printf("%%define " #x " %zu\n", (size_t)(x))

  PRINT_DEFINE(OFFSET_PT_PHY);
  PRINT_DEFINE(OFFSET_TASK_STACK);
  PRINT_DEFINE(OFFSET_TASK_KSTACK_TOP);
  PRINT_DEFINE(OFFSET_TASK_VMMAP);
  PRINT_DEFINE(OFFSET_VM_MAP_PT);
  PRINT_DEFINE(OFFSET_TSS_RSP0);

  PRINT_DEFINE(__USER_CS);
  PRINT_DEFINE(__USER_DS);
  PRINT_DEFINE(__KERNEL_CS);
  PRINT_DEFINE(__KERNEL_DS);
  PRINT_DEFINE(__TSS);

#undef PRINT_DEFINE

  printf("%%endif");

  return 0;
}
