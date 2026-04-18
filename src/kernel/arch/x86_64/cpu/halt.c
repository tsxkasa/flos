#include <halt.h>

void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}
