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

# gen_offsets tool (built with host compiler)
GEN_OFFSETS_SRC := tools/gen_offsets/main.c
GEN_OFFSETS_BIN := $(OBJDIR)/tools/gen_offsets
ASM_OFFSETS_INC := arch/$(HOSTARCH)/generated/asm-offsets.inc

SRC_FOLDERS := . $(TOPDIR)/external/uacpi
SRC_EXCLUDE_STR := *tests* *tools*
SRCFILES := $(patsubst ./%,%,$(shell \
  find $(SRC_FOLDERS) \
    -type f \
    $(foreach p,$(SRC_EXCLUDE_STR),! -path '$(p)') \
    \( -name '*.c' -o -name '*.S' -o -name '*.asm' \) \
  | LC_ALL=C sort))
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

KERNEL_INCDIRS := $(shell find . -type d | sed 's|^|-I|')

# Build the host-side gen_offsets tool with the native compiler
$(GEN_OFFSETS_BIN): $(GEN_OFFSETS_SRC)
	@mkdir -p $(@D)
	$(HOST_CC) -O2 $(KERNEL_INCDIRS) -D__is_kernel \
	    -D"__attribute__(x)=" \
	    -D"__inline__=inline" \
	    -D"__asm__(x)=" \
	    -D"__asm(x)=" \
	    -o $@ $<

# Run gen_offsets to emit the .inc file consumed by NASM sources
$(ASM_OFFSETS_INC): $(GEN_OFFSETS_BIN)
	@mkdir -p $(@D)
	$< > $@

$(patsubst %.asm,$(OBJDIR)/%.asm.o,$(NASMFILES)): $(ASM_OFFSETS_INC)

$(OBJDIR)/flos: $(OBJS) linker.ld
	@mkdir -p $(@D)
	$(LD) $(KERNEL_LDFLAGS) $(OBJS) -o $@

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(KERNEL_CFLAGS) $(KERNEL_CPPFLAGS)

$(OBJDIR)/%.S.o: %.S
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(KERNEL_CFLAGS) $(KERNEL_CPPFLAGS)

$(OBJDIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) -Iarch/$(HOSTARCH)/asm -Iarch/$(HOSTARCH)/generated $< -o $@

$(OBJDIR)/%.psf.o: %.psf
	@mkdir -p $(@D)
	$(OBJCOPY) -O elf64-x86-64 -B i386 -I binary $< $@

clean:
	rm -rf $(OBJDIR)

install: install-kernel

install-kernel: $(OBJDIR)/flos
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp $(OBJDIR)/flos $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
