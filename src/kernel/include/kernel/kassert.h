#ifndef _KERNEL_ASSERT_H
#define _KERNEL_ASSERT_H

#include <cpu/halt.h>
#include <kernel/printk.h>
#include <stdbool.h>

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define kassert(expr)                                                          \
  ((expr) ? (void)0 : __kassert_fail(#expr, __FILE__, __LINE__, __func__))
#endif

[[noreturn]] static inline int __kassert_fail(const char *expr,
                                              const char *file,
                                              unsigned int line,
                                              const char *fun) {
  printk(LOG_ERR "%s:%u: Assertion '%s' failed.\n\t In file: %s", fun, line,
         expr, file);
  hcf();
}

#endif // _KERNEL_ASSERT_H
