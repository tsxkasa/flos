HOST ?= x86_64-unknown-none-elf
HOSTARCH := x86_64

# User controllable toolchain and toolchain prefix.
TOOLCHAIN ?= llvm
TOOLCHAIN_PREFIX ?=

ifeq ($(TOOLCHAIN),llvm)
    CC := $(TOOLCHAIN_PREFIX)clang
    LD := $(TOOLCHAIN_PREFIX)ld.lld
    AR := $(TOOLCHAIN_PREFIX)llvm-ar
    OBJCOPY := $(TOOLCHAIN_PREFIX)llvm-objcopy
else
    CC := $(TOOLCHAIN_PREFIX)gcc
    LD := $(TOOLCHAIN_PREFIX)ld
    AR := $(TOOLCHAIN_PREFIX)ar
    OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
endif

# Check if CC is Clang.
CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

# If the C compiler is Clang, set the target as needed.
ifeq ($(CC_IS_CLANG),1)
    CC += -target $(HOST)
endif

# Set up the Sysroot paths
SYSROOT := $(abspath $(TOPDIR)/sysroot)
DESTDIR := $(SYSROOT)
PREFIX := /usr
EXEC_PREFIX := $(PREFIX)
BOOTDIR := /boot
INCLUDEDIR := $(PREFIX)/include
LIBDIR := $(EXEC_PREFIX)/lib


DEBUG ?= 0
# Global C Flags
ifeq ($(DEBUG), 1)
CFLAGS ?= -O0 -g -pipe -DDEBUG
else
CFLAGS ?= -O2 -g -pipe -DNDEBUG
endif
CFLAGS += -Wall -Wextra -std=gnu11 -ffreestanding

# Global Preprocessor flags
CPPFLAGS ?=
CPPFLAGS += -I$(TOPDIR)/deps --sysroot=$(SYSROOT) -MMD -MP

# Global NASM flags
NASMFLAGS ?= -f elf64 -g -F dwarf -Wall
