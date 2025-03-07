// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VC_EXTRALEAN
#include <windows.h>
#include <stdbool.h>

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
	ZEMU_FW_X86_MIN = 0,
	ZEMU_FW_X64_EFI,
	ZEMU_FW_X86_BIOS,
	ZEMU_FW_X86_EFI,
	ZEMU_FW_X86_MAX,

	ZEMU_FW_ARM_MIN = 10,
	ZEMU_FW_AA64_EFI,
	ZEMU_FW_ARM32_EFI,
	ZEMU_FW_ARM_MAX,
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
	ZEMU_BOOT_MAX,
} ZEMU_BOOT_TARGET;

#define OUTBUF_SZ 4096

#define MODEL_SZ 32
#define KCMD_SZ 4096

struct _PHY_DRIVE_INFO;

typedef struct _ZEMU_INI_DATA
{
	WCHAR pwd[MAX_PATH];
	WCHAR ini[MAX_PATH];
	CHAR qemu_dir[MAX_PATH];
	ZEMU_QEMU_ARCH qemu_arch;
	int qemu_cpu_num;
	char qemu_cpu_x86[32];
	char qemu_cpu_arm[32];
	//int qemu_cpu_core;
	int qemu_mem_mb;
	ZEMU_FW qemu_fw_x86;
	ZEMU_FW qemu_fw_arm;
	ZEMU_BOOT_TARGET qemu_boot_x86;
	ZEMU_BOOT_TARGET qemu_boot_arm;
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
set_ini_value(LPCWSTR section, LPCWSTR key, LPCWSTR _Printf_format_string_ format, ...);

int
get_ini_num(LPCWSTR section, LPCWSTR key, int fallback);

VOID
set_ini_num(LPCWSTR section, LPCWSTR key, int value);
