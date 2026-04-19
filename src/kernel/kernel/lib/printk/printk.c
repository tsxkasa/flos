#include <drivers/tty/tty.h>
#include <kernel/kprintf.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <stdarg.h>

#define PRINK_BUF_SIZE 1024
const char string_buffer[1024];

void printk(const char *format, ...) {
  va_list ptr;
  va_start(ptr, format);
  int len = vsnprintf((char *)&string_buffer, PRINK_BUF_SIZE, format, ptr);

  if (len > 0) {
    tty_write(string_buffer, (size_t)len);
  }
  va_end(ptr);
}
