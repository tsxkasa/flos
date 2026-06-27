#ifndef _KERNEL_MACROS_H
#define _KERNEL_MACROS_H

#include <rwonce.h>

#define __always_inline inline __attribute__((always_inline))
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#endif // _KERNEL_MACROS_H
