// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "nkctx.h"

#define CONV_BUFSZ 65535
#define CONV_BUFSZW (CONV_BUFSZ / sizeof(WCHAR))
#define CONV_BUFSZB (CONV_BUFSZW * sizeof(WCHAR))

LPCSTR ucs2_to_utf8(LPCWSTR src);

LPCWSTR utf8_to_ucs2(LPCSTR src);

extern const char* human_units[];

const char*
get_human_size(UINT64 size, const char* human_sizes[6], UINT64 base);

typedef enum _ZEMU_QEMU_ARCH
{
	ZEMU_QEMU_ARCH_X64 = 0,
	ZEMU_QEMU_ARCH_AA64,
	ZEMU_QEMU_ARCH_MAX,
} ZEMU_QEMU_ARCH;

typedef enum _ZEMU_FW
{
	ZEMU_FW_X64_EFI = 0,
	ZEMU_FW_X86_BIOS,
	ZEMU_FW_X86_EFI,
#define ZEMU_FW_X86_MIN ZEMU_FW_X64_EFI
#define ZEMU_FW_X86_MAX ZEMU_FW_X86_EFI

	ZEMU_FW_AA64_EFI,
	ZEMU_FW_ARM32_EFI,
#define ZEMU_FW_ARM_MIN ZEMU_FW_AA64_EFI
#define ZEMU_FW_ARM_MAX ZEMU_FW_AA64_EFI

	ZEMU_FW_MAX,
} ZEMU_FW;

typedef enum _ZEMU_BOOT_TARGET
{
	ZEMU_BOOT_VHD = 0,
	ZEMU_BOOT_ISO,
	ZEMU_BOOT_PD,
	ZEMU_BOOT_CD,
	ZEMU_BOOT_VFD,
	ZEMU_BOOT_PXE,
	ZEMU_BOOT_LINUX,
	ZEMU_BOOT_WIM,
	ZEMU_BOOT_MAX,
} ZEMU_BOOT_TARGET;

#define OUTBUF_SZ 4096

#define OPT_SZ 32
#define KCMD_SZ 4096

struct _PHY_DRIVE_INFO;

typedef struct _ZEMU_INI_PROFILE
{
	char smp[OPT_SZ];
	char model[OPT_SZ];
	char mem[OPT_SZ];
	char machine[OPT_SZ];
	nk_bool irqchip;
	nk_bool virt; // ARM only
	char vga[OPT_SZ];
	char usb[OPT_SZ];
	nk_bool usb_kbd;
	nk_bool usb_tablet;
	nk_bool usb_mouse;
	ZEMU_FW fw;
	ZEMU_BOOT_TARGET boot;
} ZEMU_INI_PROFILE;

typedef struct _ZEMU_INI_DATA
{
	WCHAR pwd[MAX_PATH];
	WCHAR ini[MAX_PATH];
	CHAR qemu_dir[MAX_PATH];
	CHAR qemu_name[ZEMU_QEMU_ARCH_MAX][OPT_SZ];
	ZEMU_QEMU_ARCH qemu_arch;

	CHAR qemu_fw[ZEMU_FW_MAX][MAX_PATH];

	CHAR qemu_wimldr[ZEMU_FW_MAX][OPT_SZ];
	CHAR qemu_wimhda[OPT_SZ];

	ZEMU_INI_PROFILE profile[ZEMU_QEMU_ARCH_MAX];
	ZEMU_INI_PROFILE* cur;

	CHAR boot_vhd[MAX_PATH];
	CHAR boot_iso[MAX_PATH];
	CHAR boot_vfd[MAX_PATH];

	struct _PHY_DRIVE_INFO* hd_info;
	DWORD hd_count;
	DWORD boot_hd;
	struct _PHY_DRIVE_INFO* cd_info;
	DWORD cd_count;
	DWORD boot_cd;

	CHAR boot_linux[MAX_PATH];
	CHAR boot_initrd[MAX_PATH];
	CHAR boot_kcmd[KCMD_SZ];
	CHAR boot_dtb[MAX_PATH];
	CHAR boot_shim[MAX_PATH];

	CHAR boot_wim[MAX_PATH];

	CHAR net_tftp[MAX_PATH];
	CHAR net_file[MAX_PATH];

	CHAR output[OUTBUF_SZ + 1];
	size_t output_offset;
} ZEMU_INI_DATA;

VOID load_ini(VOID);

VOID save_ini(VOID);

LPCSTR
get_ini_value(LPCWSTR section, LPCWSTR key, LPCWSTR fallback);

VOID
set_ini_value(LPCWSTR section, LPCWSTR key, LPCSTR value);

int
get_ini_num(LPCWSTR section, LPCWSTR key, int fallback);

VOID
set_ini_num(LPCWSTR section, LPCWSTR key, int value);

LPCWSTR
rel_to_abs(LPCSTR path);
