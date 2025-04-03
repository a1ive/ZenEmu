// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "nkctx.h"
#include "../ews/ews.h"

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
	ZEMU_BOOT_DIR,
	ZEMU_BOOT_MAX,
} ZEMU_BOOT_TARGET;

#define OUTBUF_SZ 4096

#define OPT_SZ 32
#define KCMD_SZ 4096

// X2-XX-XX-XX-XX-XX
#define MAC_SZ 18

struct _PHY_DRIVE_INFO;

typedef struct _ZEMU_INI_PROFILE
{
	char smp[OPT_SZ];
	char model[OPT_SZ];
	char mem[OPT_SZ];
	char machine[OPT_SZ];
	nk_bool irqchip;
	nk_bool virt; // ARM only
	nk_bool whpx; // x86 only
	nk_bool graphics;
	char vgadev[OPT_SZ];
	nk_bool pflash;
	nk_bool net;
	char netdev[OPT_SZ];
	char netmac[MAC_SZ];
	nk_bool usb;
	char usbctrl[OPT_SZ];
	nk_bool usb_kbd;
	nk_bool usb_tablet;
	nk_bool usb_mouse;
	nk_bool audio;
	nk_bool audio_hda;
	nk_bool audio_spk;
	char audiodev[OPT_SZ];
	ZEMU_FW fw;
	nk_bool fw_menu;
	char fw_timeout[OPT_SZ];
	ZEMU_BOOT_TARGET boot;
} ZEMU_INI_PROFILE;

typedef enum _ZEMU_DEV_IF
{
	ZEMU_DEV_IF_DEFAULT = 0,
	ZEMU_DEV_IF_IDE,
	ZEMU_DEV_IF_SCSI,
	ZEMU_DEV_IF_SD,
	ZEMU_DEV_IF_MTD,
	ZEMU_DEV_IF_VIRTIO,
	ZEMU_DEV_IF_HDMAX,
	ZEMU_DEV_IF_FLOPPY
} ZEMU_DEV_IF;

typedef struct _ZEMU_DEV_ATTR
{
	nk_bool snapshot;
	ZEMU_DEV_IF devif;
} ZEMU_DEV_ATTR;

typedef enum _ZEMU_DEV_TYPE
{
	ZEMU_DEV_HD = 0,
	ZEMU_DEV_CD,
	ZEMU_DEV_MAX
} ZEMU_DEV_TYPE;

typedef struct _ZEMU_ADD_DEV
{
	ZEMU_DEV_TYPE type;
	ZEMU_DEV_ATTR attr;
	nk_bool is_device;
	nk_bool is_active;
	DWORD id;
	CHAR path[MAX_PATH];
} ZEMU_ADD_DEV;

#define MAX_ADD_DEV 4

typedef enum _ZEMU_SCREEN_SAVE
{
	ZEMU_SCREEN_TO_FILE = 0,
	ZEMU_SCREEN_TO_CLIP,
	ZEMU_SCREEN_TO_MAX
} ZEMU_SCREEN_SAVE;

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

	ZEMU_SCREEN_SAVE qemu_screenshot;
	nk_bool qemu_fullscreen;

	ZEMU_INI_PROFILE profile[ZEMU_QEMU_ARCH_MAX];
	ZEMU_INI_PROFILE* cur;

	CHAR boot_vhd[MAX_PATH];
	ZEMU_DEV_ATTR boot_vhd_attr;

	CHAR boot_iso[MAX_PATH];
	
	CHAR boot_vfd[MAX_PATH];
	ZEMU_DEV_ATTR boot_vfd_attr;

	struct _PHY_DRIVE_INFO* d_info[ZEMU_DEV_MAX];
	DWORD d_count[ZEMU_DEV_MAX];

	DWORD boot_hd;
	ZEMU_DEV_ATTR boot_hd_attr;

	DWORD boot_cd;

	CHAR boot_linux[MAX_PATH];
	CHAR boot_initrd[MAX_PATH];
	CHAR boot_kcmd[KCMD_SZ];
	CHAR boot_dtb[MAX_PATH];
	CHAR boot_shim[MAX_PATH];

	CHAR boot_wim[MAX_PATH];
	int boot_wim_index;

	CHAR net_tftp[MAX_PATH];
	CHAR net_file[MAX_PATH];
	nk_bool net_http;
	CHAR net_http_port[OPT_SZ];

	CHAR boot_dir[MAX_PATH];
	ZEMU_DEV_ATTR boot_dir_attr;

	size_t add_dev_count;
	ZEMU_ADD_DEV add_dev[MAX_ADD_DEV];
	
	EWS_SERVER* ews;
	CHAR output[OUTBUF_SZ + 1];
	size_t output_offset;
	HANDLE output_handle;
	DWORD output_pid;
} ZEMU_INI_DATA;

#define IS_BIOS \
	(nk.ini->qemu_arch == ZEMU_QEMU_ARCH_X64 && nk.ini->cur->fw == ZEMU_FW_X86_BIOS)

#define IS_X86_EFI \
	(nk.ini->qemu_arch == ZEMU_QEMU_ARCH_X64 && nk.ini->cur->fw != ZEMU_FW_X86_BIOS)

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

static inline nk_bool
get_ini_bool(LPCWSTR section, LPCWSTR key, nk_bool fallback)
{
	int value = get_ini_num(section, key, (int)fallback);
	return value ? nk_true : nk_false;
}

LPCWSTR
rel_to_abs(LPCSTR path);

nk_bool
check_path_invalid(const char* str);

uint64_t
get_file_header(LPCWSTR path, void* header, size_t header_len);
