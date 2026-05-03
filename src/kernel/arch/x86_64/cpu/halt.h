#ifndef _KERNEL_HALT_H
#define _KERNEL_HALT_H
// Halt and catch fire function.
[[noreturn]] void hcf(void) {
  for (;;) {
    asm("hlt");
  }
}

#endif // _KERNEL_HALT_H
