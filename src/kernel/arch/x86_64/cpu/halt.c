#include <cpu/halt.h>

void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}
