TOPDIR ?= ../..
include $(TOPDIR)/config.mk

OBJDIR := ../../obj/libc

# Flags specific to building libc
CFLAGS += -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC
CPPFLAGS += -D__is_libc -Iinclude

LIBK_CFLAGS := $(CFLAGS)
LIBK_CPPFLAGS := $(CPPFLAGS) -D__is_libk

SRCFILES := $(patsubst ./%,%,$(shell find . -type f -name '*.c' -o -name '*.S' -o -name '*.asm' | LC_ALL=C sort))
CFILES := $(filter %.c,$(SRCFILES))
ASFILES := $(filter %.S,$(SRCFILES))
NASMFILES := $(filter %.asm,$(SRCFILES))

# FREEOBJS := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
LIBK_OBJS := $(patsubst %.c,$(OBJDIR)/%.libk.c.o,$(CFILES)) \
             $(patsubst %.S,$(OBJDIR)/%.libk.S.o,$(ASFILES)) \
             $(patsubst %.asm,$(OBJDIR)/%.libk.asm.o,$(NASMFILES))

BINARIES := $(OBJDIR)/libk.a

.PHONY: all clean install install-headers install-libs

all: $(BINARIES)

$(OBJDIR)/libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

$(OBJDIR)/%.libk.c.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

$(OBJDIR)/%.libk.S.o: %.S
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

$(OBJDIR)/%.libk.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINARIES)

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(LIBK_OBJS:.o=.d)
