TOPDIR ?= ../..
include $(TOPDIR)/config.mk

OBJDIR := ../../obj/kernel

KERNEL_CFLAGS := $(CFLAGS) \
    -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC \
    -ffunction-sections -fdata-sections \
    -m64 -march=x86-64 -mabi=sysv \
    -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel \
    -nostdlib -fno-builtin

KERNEL_CPPFLAGS := $(CPPFLAGS) -D__is_kernel -Iinclude -I. -Iarch/$(HOSTARCH)
KERNEL_LDFLAGS := $(LDFLAGS) \
    -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 --gc-sections \
    -T linker.ld

KERNEL_LIBS := -L$(DESTDIR)$(LIBDIR) -lk

SRCFILES := $(patsubst ./%,%,$(shell find . -type f -name '*.c' -o -name '*.S' -o -name '*.asm' | LC_ALL=C sort))
CFILES := $(filter %.c,$(SRCFILES))
ASFILES := $(filter %.S,$(SRCFILES))
NASMFILES := $(filter %.asm,$(SRCFILES))

FONTFILES := $(patsubst ./%,%,$(shell find assets -type f -name '*.psf' 2>/dev/null | LC_ALL=C sort))

FONTOBJS := $(patsubst %.psf,$(OBJDIR)/%.psf.o,$(FONTFILES))

OBJS := $(patsubst %.c,$(OBJDIR)/%.c.o,$(CFILES)) \
        $(patsubst %.S,$(OBJDIR)/%.S.o,$(ASFILES)) \
        $(patsubst %.asm,$(OBJDIR)/%.asm.o,$(NASMFILES)) \
        $(FONTOBJS)

FLOS := $(OBJDIR)/flos

.PHONY: all clean install install-headers install-kernel

all: $(OBJDIR)/flos

$(OBJDIR)/flos: $(OBJS) linker.ld
	$(LD) $(KERNEL_LDFLAGS) $(OBJS) $(KERNEL_LIBS) -o $@

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(KERNEL_CFLAGS) $(KERNEL_CPPFLAGS)

$(OBJDIR)/%.S.o: %.S
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(KERNEL_CFLAGS) $(KERNEL_CPPFLAGS)

$(OBJDIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) $< -o $@

$(OBJDIR)/%.psf.o: %.psf
	@mkdir -p $(@D)
	$(OBJCOPY) -O elf64-x86-64 -B i386 -I binary $< $@

clean:
	rm -rf $(OBJDIR)

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: $(OBJDIR)/flos
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(OBJDIR)/flos $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
