# Versioning information
#
VERSION := v2.8.0
SBAT_GENERATION := 1

# Abstract target-independent objects
#
OBJECTS := prefix.o vsprintf.o string.o cookie.o
OBJECTS += vdisk.o cpio.o stdio.o xca.o die.o cmdline.o
OBJECTS += patch.o huffman.o lzx.o wim.o wimfile.o pause.o sha1.o

OBJ_EFI := $(OBJECTS)
OBJ_EFI += efi.o efimain.o efiguid.o
OBJ_EFI += efifile.o efipath.o efiboot.o efiblock.o

OBJ_BIOS := $(OBJECTS)
OBJ_BIOS += callback.o main.o startup.o
OBJ_BIOS += int13.o lznt1.o memmap.o paging.o peloader.o

# Target-dependent objects
#
OBJECTS_i386 := $(patsubst %.o,%.i386.o,$(OBJ_EFI))
OBJECTS_x86_64 := $(patsubst %.o,%.x86_64.o,$(OBJ_EFI))
OBJECTS_arm64 := $(patsubst %.o,%.arm64.o,$(OBJ_EFI))
OBJECTS_bios := $(patsubst %.o,%.bios.o,$(OBJ_BIOS))

# Header files
#
HEADERS := $(wildcard *.h)

# Common build tools
#
ECHO		?= echo
CP		?= cp
RM		?= rm
GREP		?= grep
XARGS		?= xargs
CUT		?= cut
CC		?= cc
AS		?= as
LD		?= ld
AR		?= ar
OBJCOPY		?= objcopy

# Build tools for host binaries
#
HOST_CC		:= $(CC)

# Get list of default compiler definitions
#
CCDEFS		:= $(shell $(CC) -E -x c -c /dev/null -dM | $(CUT) -d" " -f2)

# Detect compiler type
#
ifeq ($(filter __clang__,$(CCDEFS)),__clang__)
CC_i386		?= $(CC) -target i386
CC_x86_64	?= $(CC) -target x86_64
CC_arm64	?= $(CC) -target aarch64
CFLAGS		+= -Wno-unused-command-line-argument
else ifeq ($(filter __GNUC__,$(CCDEFS)),__GNUC__)
CC		:= gcc
endif

# Guess appropriate cross-compilation prefixes
#
ifneq ($(filter __x86_64__,$(CCDEFS)),__x86_64__)
CROSS_x86_64	?= x86_64-linux-gnu-
endif
ifneq ($(filter __aarch64__,$(CCDEFS)),__aarch64__)
CROSS_arm64	?= aarch64-linux-gnu-
endif
CROSS_i386	?= $(CROSS_x86_64)

# Build tools for i386 target
#
CC_i386		?= $(CROSS_i386)$(CC)
AS_i386		?= $(CROSS_i386)$(AS)
LD_i386		?= $(CROSS_i386)$(LD)
AR_i386		?= $(CROSS_i386)$(AR)
OBJCOPY_i386	?= $(CROSS_i386)$(OBJCOPY)

# Build tools for x86_64 target
#
CC_x86_64	?= $(CROSS_x86_64)$(CC)
AS_x86_64	?= $(CROSS_x86_64)$(AS)
LD_x86_64	?= $(CROSS_x86_64)$(LD)
AR_x86_64	?= $(CROSS_x86_64)$(AR)
OBJCOPY_x86_64	?= $(CROSS_x86_64)$(OBJCOPY)

# Build tools for arm64 target
#
CC_arm64	?= $(CROSS_arm64)$(CC)
AS_arm64	?= $(CROSS_arm64)$(AS)
LD_arm64	?= $(CROSS_arm64)$(LD)
AR_arm64	?= $(CROSS_arm64)$(AR)
OBJCOPY_arm64	?= $(CROSS_arm64)$(OBJCOPY)

# Build flags for host binaries
#
HOST_CFLAGS	+= -Wall -W -Werror

# Build flags for all targets
#
CFLAGS		+= -Os -ffreestanding -Wall -W -Werror
CFLAGS		+= -nostdinc -I. -fshort-wchar
CFLAGS		+= -DVERSION="\"$(VERSION)\""
CFLAGS		+= -DSBAT_GENERATION="\"$(SBAT_GENERATION)\""
CFLAGS		+= -include compiler.h
ifneq ($(DEBUG),)
CFLAGS		+= -DDEBUG=$(DEBUG)
endif
CFLAGS		+= $(EXTRA_CFLAGS)
LDFLAGS		+= --no-relax

# Build flags for i386 target
#
CFLAGS_i386	:= -m32 -march=i386 -malign-double -fno-pic
ASFLAGS_i386	:= --32
LDFLAGS_i386	:= -m elf_i386

# Build flags for x86_64 target
#
CFLAGS_x86_64	:= -m64 -mno-red-zone -fpie
ASFLAGS_x86_64	:= --64
LDFLAGS_x86_64	:= -m elf_x86_64

# Build flags for arm64 target
#
CFLAGS_arm64	:= -mlittle-endian -mcmodel=small -fno-pic
ASFLAGS_arm64	:= -mabi=lp64 -EL

# Run a test compilation and discard any output
#
CC_TEST		= $(CC_$(1)) $(CFLAGS) $(CFLAGS_$(1)) \
		  -x c -c - -o /dev/null >/dev/null 2>&1

# Test for supported compiler flags
#
CFLAGS_TEST	= $(shell $(CC_TEST) $(2) </dev/null && $(ECHO) '$(2)')

# Enable stack protection if available
#
CFLAGS_SPG	= $(call CFLAGS_TEST,$(1),-fstack-protector-strong \
					  -mstack-protector-guard=global)

# Inhibit unwanted debugging information
#
CFLAGS_CFI	= $(call CFLAGS_TEST,$(1),-fno-dwarf2-cfi-asm \
					  -fno-exceptions \
					  -fno-unwind-tables \
					  -fno-asynchronous-unwind-tables)

# Inhibit warnings from taking address of packed struct members
#
CFLAGS_WNAPM	= $(call CFLAGS_TEST,$(1),-Wno-address-of-packed-member)

# Inhibit spurious warnings from array bounds checking
#
CFLAGS_WNAB	= $(call CFLAGS_TEST,$(1),-Wno-array-bounds)

# Inhibit LTO
#
CFLAGS_NLTO	= $(call CFLAGS_TEST,$(1),-fno-lto)

# Inhibit address-significance table
#
CFLAGS_NAS	= $(call CFLAGS_TEST,$(1),-fno-addrsig)

# Add -maccumulate-outgoing-args if required
#
MS_ABI_TEST_CODE := extern void __attribute__ (( ms_abi )) ms_abi(); \
		    void sysv_abi ( void ) { ms_abi(); }
define CFLAGS_MS_ABI
$(shell $(CC_TEST) -maccumulate-outgoing-args </dev/null && \
	( $(ECHO) '$(MS_ABI_TEST_CODE)' | \
	  $(CC_TEST) -mno-accumulate-outgoing-args || \
	  $(ECHO) '-maccumulate-outgoing-args' ))
endef

# Conditional build flags
#
CFLAGS_COND	= $(CFLAGS_SPG) $(CFLAGS_CFI) $(CFLAGS_WNAPM) $(CFLAGS_WNAB) \
		  $(CFLAGS_NLTO) $(CFLAGS_NAS) $(CFLAGS_MS_ABI)
CFLAGS_i386	+= $(call CFLAGS_COND,i386)
CFLAGS_x86_64	+= $(call CFLAGS_COND,x86_64)
CFLAGS_arm64	+= $(call CFLAGS_COND,arm64)

###############################################################################
#
# Final targets

all : wimldr.x64 wimldr.ia32 wimldr.aa64 wimldr

wimldr.elf : lib.bios.a Makefile
	$(LD_i386) $(LDFLAGS) $(LDFLAGS_i386) -T bios.lds -o $@ -q \
		-Map wimldr.bios.map prefix.bios.o lib.bios.a

wimldr.%.elf : lib.%.a Makefile
	$(LD_$*) $(LDFLAGS) $(LDFLAGS_$*) -T efi.lds -o $@ -q \
		-Map wimldr.$*.map prefix.$*.o lib.$*.a

wimldr.i386.efi : \
wimldr.%.efi : wimldr.%.elf elf2efi32 Makefile
	./elf2efi32 --hybrid $< $@

wimldr.x86_64.efi wimldr.arm64.efi : \
wimldr.%.efi : wimldr.%.elf elf2efi64 Makefile
	./elf2efi64 --hybrid $< $@

wimldr : wimldr.elf Makefile
	$(OBJCOPY) -Obinary $< $@

wimldr.x64 : wimldr.x86_64.efi Makefile
	$(CP) $< $@

wimldr.ia32 : wimldr.i386.efi Makefile
	$(CP) $< $@

wimldr.aa64 : wimldr.arm64.efi Makefile
	$(CP) $< $@

###############################################################################
#
# bios objects

%.bios.s : %.S $(HEADERS) Makefile
	$(CC_i386) $(CFLAGS) $(CFLAGS_i386) -DASSEMBLY -DBIOS -Ui386 -E $< -o $@

%.bios.s : %.c $(HEADERS) Makefile
	$(CC_i386) $(CFLAGS) $(CFLAGS_i386) -DBIOS -S $< -o $@

%.bios.o : %.bios.s Makefile
	$(AS_i386) $(ASFLAGS) $(ASFLAGS_i386) $< -o $@

lib.bios.a : $(OBJECTS_bios) Makefile
	$(RM) -f $@
	$(AR_i386) -r -s $@ $(OBJECTS_bios)

###############################################################################
#
# i386 objects

%.i386.s : %.S $(HEADERS) Makefile
	$(CC_i386) $(CFLAGS) $(CFLAGS_i386) -DASSEMBLY -E $< -o $@

%.i386.s : %.c $(HEADERS) Makefile
	$(CC_i386) $(CFLAGS) $(CFLAGS_i386) -S $< -o $@

%.i386.o : %.i386.s Makefile
	$(AS_i386) $(ASFLAGS) $(ASFLAGS_i386) $< -o $@

lib.i386.a : $(OBJECTS_i386) Makefile
	$(RM) -f $@
	$(AR_i386) -r -s $@ $(OBJECTS_i386)

###############################################################################
#
# x86_64 objects

%.x86_64.s : %.S $(HEADERS) Makefile
	$(CC_x86_64) $(CFLAGS) $(CFLAGS_x86_64) -DASSEMBLY -E $< -o $@

%.x86_64.s : %.c $(HEADERS) Makefile
	$(CC_x86_64) $(CFLAGS) $(CFLAGS_x86_64) -S $< -o $@

%.x86_64.o : %.x86_64.s Makefile
	$(AS_x86_64) $(ASFLAGS) $(ASFLAGS_x86_64) $< -o $@

lib.x86_64.a : $(OBJECTS_x86_64) Makefile
	$(RM) -f $@
	$(AR_x86_64) -r -s $@ $(OBJECTS_x86_64)

###############################################################################
#
# arm64 objects

%.arm64.s : %.S $(HEADERS) Makefile
	$(CC_arm64) $(CFLAGS) $(CFLAGS_arm64) -DASSEMBLY -E $< -o $@

%.arm64.s : %.c $(HEADERS) Makefile
	$(CC_arm64) $(CFLAGS) $(CFLAGS_arm64) -S $< -o $@

%.arm64.o : %.arm64.s Makefile
	$(AS_arm64) $(ASFLAGS) $(ASFLAGS_arm64) $< -o $@

lib.arm64.a : $(OBJECTS_arm64) Makefile
	$(RM) -f $@
	$(AR_arm64) -r -s $@ $(OBJECTS_arm64)

###############################################################################
#
# EFI relocator

elf2efi32 : elf2efi.c Makefile
	$(HOST_CC) $(HOST_CFLAGS) -idirafter . -DEFI_TARGET32 $< -o $@

elf2efi64 : elf2efi.c Makefile
	$(HOST_CC) $(HOST_CFLAGS) -idirafter . -DEFI_TARGET64 $< -o $@

###############################################################################
#
# Cleanup

.PHONY: clean
clean :
	$(RM) -f *.s *.o *.a *.elf *.map
	$(RM) -f elf2efi32 elf2efi64
	$(RM) -f wimldr wimldr.elf
	$(RM) -f wimldr.ia32 wimldr.i386.*
	$(RM) -f wimldr.x64 wimldr.x86_64.*
	$(RM) -f wimldr.aa64 wimldr.arm64.*

.DELETE_ON_ERROR :
