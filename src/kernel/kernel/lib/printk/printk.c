#include <drivers/tty/tty.h>
#include <kernel/kprintf.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <stdarg.h>

#define PRINTK_BUF_SIZE 8192

static char string_buffer[PRINTK_BUF_SIZE];
static size_t log_head;
static size_t log_tail;

void printk(const char *format, ...) {
  va_list ptr;
  va_start(ptr, format);

  char temp[256];
  int len = vsnprintf(temp, sizeof(temp), format, ptr);
  va_end(ptr);

  if (len <= 0)
    return;

  for (int i = 0; i < len; i++) {
    size_t next = (log_head + 1) % PRINTK_BUF_SIZE;

    if (next == log_tail) {
      log_tail = (log_tail + 1) % PRINTK_BUF_SIZE;
    }

    string_buffer[log_head] = temp[i];
    log_head = next;
  }
  tty_flush(); // leap flush here for now until i can think of a better place to
               // put it
}

size_t printk_read_buf(char *out, size_t max) {
  size_t count = 0;

  while (count < max && log_tail != log_head) {
    out[count++] = string_buffer[log_tail];
    log_tail = (log_tail + 1) % PRINTK_BUF_SIZE;
  }

  return count;
}
