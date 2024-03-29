MEMO                   := memo.bin
LIBC                   := libc
PROJECTS               := drivers kernel $(LIBC)
PROGRAMS               := programs
HOST                   := i386-memo
HOSTARCH               := i386
SYSROOT                := $(abspath sysroot)
AR                     := $(HOST)-ar
AS                     := $(HOST)-as
LD                     := $(HOST)-ld
CC                     := $(HOST)-gcc --sysroot=$(SYSROOT)
GDB                    := $(HOST)-gdb --sysroot=$(SYSROOT)

PREFIX                 := /usr/
BOOTDIR                := /boot
INCLUDEDIR             := $(PREFIX)/include
LIBDIR                 := $(PREFIX)/lib

# Export all variables to make them available to sub-make
export

QEMU_FLAGS             := -d int,cpu_reset --no-reboot --no-shutdown

.PHONY: qemu monitor debug clean fmt $(PROJECTS) $(PROGRAMS)

$(MEMO): $(PROJECTS) $(PROGRAMS)
	./scripts/build.sh

install-headers:
	make -C $(LIBC) install-headers

qemu: $(MEMO)
	qemu-system-i386 -cdrom $(MEMO) $(QEMU_FLAGS)

monitor: $(MEMO)
	qemu-system-i386 -cdrom $(MEMO) -monitor stdio $(QEMU_FLAGS)

debug: $(MEMO)
	qemu-system-i386 -cdrom $(MEMO) $(QEMU_FLAGS) -s -S &> qemu.log &
	gdb ./build/boot/memo.kernel -x ./scripts/gdb.txt

fmt:
	astyle \
		--break-return-type \
		--align-pointer=name \
		--style=attach \
		--indent-switches \
		--max-code-length=100 \
		--recursive './*.c,*.h' \

$(PROJECTS):
ifeq ($(MAKECMDGOALS), clean)
	@echo "cleaning $@"
	make -C $@ clean
else
	make -C $@ install
endif

$(PROGRAMS):
ifeq ($(MAKECMDGOALS), clean)
	@echo "cleaning $@"
	make -C $@ clean
endif

clean: $(PROJECTS) $(PROGRAMS)
	rm -rf sysroot build $(MEMO) qemu.log
