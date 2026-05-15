#ifndef _KERNEL_PRINTK_H
#define _KERNEL_PRINTK_H

#define LOG_INFO  "[INFO] "
#define LOG_WARN  "[WARN] "
#define LOG_ERR   "[ERROR] "
#define LOG_DEBUG "[DEBUG] "
#include <stddef.h>

void printk(const char *format, ...);
size_t printk_read_buf(char *out, size_t max);

#endif // _KERNEL_PRINTK_H
