TOPDIR := .
include $(TOPDIR)/config.mk

.PHONY: all clean sysroot headers build iso

# Subprojects
PROJECTS := src/libc src/kernel

all: sysroot headers build

sysroot:
	mkdir -p $(SYSROOT)

headers: sysroot
	@for proj in $(PROJECTS); do \
		$(MAKE) -C $$proj TOPDIR=$(abspath $(TOPDIR)) install-headers || exit $$?; \
	done

build: headers
	@for proj in $(PROJECTS); do \
		$(MAKE) -C $$proj TOPDIR=$(abspath $(TOPDIR)) install || exit $$?; \
	done

clean:
	@for proj in $(PROJECTS); do \
		$(MAKE) -C $$proj TOPDIR=$(abspath $(TOPDIR)) clean || exit $$?; \
	done
	rm -rf $(SYSROOT) bin obj

iso: build
	@mkdir -p iso_root/boot/limine
	
	@cp -v $(SYSROOT)/boot/flos iso_root/boot/
	
	@cp -v limine.conf iso_root/boot/limine/
	
	@cp -v external/limine/limine-bios.sys external/limine/limine-bios-cd.bin external/limine/limine-uefi-cd.bin iso_root/boot/limine/ 2>/dev/null || true

	@mkdir -p bin/

	@xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o bin/flos.iso

	@limine bios-install bin/flos.iso
