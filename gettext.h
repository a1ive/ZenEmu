﻿// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

typedef enum _GETTEXT_STR_ID
{
	ZTXT_QEMU = 0,
	ZTXT_PATH,
	ZTXT_ARCH,
	ZTXT_CPU,
	ZTXT_SMP,
	ZTXT_NAME,
	ZTXT_MODEL,
	ZTXT_MACHINE,
	ZTXT_KERNEL_IRQCHIP,
	ZTXT_VIRT,
	ZTXT_MEMORY,
	ZTXT_SIZE,
	ZTXT_FIRMWARE,
	ZTXT_TYPE,
	ZTXT_PFLASH,
	ZTXT_DISPLAY,
	ZTXT_NETWORK,
	ZTXT_PERIPHERAL,
	ZTXT_USB,
	ZTXT_KEYBOARD,
	ZTXT_MOUSE,
	ZTXT_TABLET,
	ZTXT_AUDIO,
	ZTXT_BACKEND,
	ZTXT_BOOT_DEVICE,
	ZTXT_DISK_IMAGE,
	ZTXT_ISO_IMAGE,
	ZTXT_PHYSICAL_DISK,
	ZTXT_CD_ROM,
	ZTXT_HARD_DISK,
	ZTXT_FLOPPY_IMAGE,
	ZTXT_PXE,
	ZTXT_TFTP_FOLDER,
	ZTXT_BOOT_FILE,
	ZTXT_LINUX_KERNEL,
	ZTXT_KERNEL,
	ZTXT_INITRD,
	ZTXT_CMDLINE,
	ZTXT_DTB,
	ZTXT_SHIM_EFI,
	ZTXT_WIM_IMAGE,
	ZTXT_DIR_VVFAT,
	ZTXT_DIR,
	ZTXT_SNAPSHOT,
	ZTXT_INTERFACE,
	ZTXT_NO_DEVICE,
	ZTXT_ADDITIONAL,
	ZTXT_FILE,
	ZTXT_DEVICE,
	ZTXT_COPY,
	ZTXT_SAVE,
	ZTXT_START,
	ZTXT_LOGS,
	ZTXT_WARN_NOT_ADMIN,
	ZTXT_WARN_NON_ASCII,
	ZTXT_WARN_OUT_OF_MEM,
	ZTXT_UNSUPPORTED,

	ZTXT__MAX
} GETTEXT_STR_ID;

const char*
ZTXT(GETTEXT_STR_ID id);
