#ifndef _KERNEL_ASSERT_H
#define _KERNEL_ASSERT_H

#include <cpu/halt.h>
#include <kernel/printk.h>
#include <stdbool.h>

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr))
#endif

[[noreturn]] static inline int __assert_fail(const char *expr) {
  printk(LOG_ERR "Assertion failed: %s\n", expr);
  hcf();
}

#endif // _KERNEL_ASSERT_H
