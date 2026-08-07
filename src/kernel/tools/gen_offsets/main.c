#include <asm-offsets.h>
#include <stdio.h>

int main() {
  printf("%%ifndef __ASM_OFFSETS_NASM_INC\n");
  printf("%%define __ASM_OFFSETS_NASM_INC\n");

#define PRINT_DEFINE(x) printf("%%define " #x " %zu\n", (size_t)(x))

  PRINT_DEFINE(OFFSET_PT_PHY);
  PRINT_DEFINE(OFFSET_TASK_STACK);
  PRINT_DEFINE(OFFSET_TASK_VMMAP);
  PRINT_DEFINE(OFFSET_VM_MAP_PT);

  PRINT_DEFINE(GDT_USER_CS);
  PRINT_DEFINE(GDT_USER_DS);
  PRINT_DEFINE(GDT_KERNEL_CS);
  PRINT_DEFINE(GDT_KERNEL_DS);
  PRINT_DEFINE(GDT_TSS);

#undef PRINT_DEFINE

  printf("%%endif");

  return 0;
}
