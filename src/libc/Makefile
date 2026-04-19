TOPDIR ?= ../..
include $(TOPDIR)/config.mk

OBJDIR := ../../obj/libc

CFLAGS += -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC
CPPFLAGS += -Iinclude

SRCFILES := $(patsubst ./%,%,$(shell find . -type f -name '*.c' -o -name '*.S' -o -name '*.asm' | LC_ALL=C sort))
CFILES := $(filter %.c,$(SRCFILES))

LIBC_OBJS := $(patsubst %.c,$(OBJDIR)/%.c.o,$(CFILES))
BINARIES := $(OBJDIR)/libc.a

.PHONY: all clean install install-headers install-libs

all: $(BINARIES)

$(OBJDIR)/libc.a: $(LIBC_OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $(LIBC_OBJS)

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -rf $(OBJDIR) $(BINARIES)

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(LIBC_OBJS:.o=.d)
