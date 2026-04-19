#ifndef _KERNEL_PRINTK_H
#define _KERNEL_PRINTK_H

#define LOG_INFO "[INFO] "
#define LOG_WARN "[WARN] "
#define LOG_ERR "[ERROR] "
#define LOG_DEBUG "[DEBUG] "

void printk(const char *format, ...);

#endif // _KERNEL_PRINTK_H
