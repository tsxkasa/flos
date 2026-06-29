#include <asm-offsets.h>
#include <stdio.h>

int main() {
  printf("%%ifndef __ASM_OFFSETS_NASM_INC\n");
  printf("%%define __ASM_OFFSETS_NASM_INC\n");
  printf("%%define OFFSET_PT_PHY %zu\n", OFFSET_PT_PHY);
  printf("%%define OFFSET_TASK_STACKPTR %zu\n", OFFSET_TASK_STACKPTR);
  printf("%%define OFFSET_TASK_VMMAP %zu\n", OFFSET_TASK_VMMAP);
  printf("%%endif");

  return 0;
}
