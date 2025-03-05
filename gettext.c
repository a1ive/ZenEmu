// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

#define INVALID_ZTXT_ID u8"!!INVALID ID - FIX ME!!"

static const char*
lang_en_us[ZTXT__MAX] =
{
	[ZTXT_QEMU] = u8"QEMU",
	[ZTXT_PATH] = u8"Path",
	[ZTXT_ARCH] = u8"Architecture",
	[ZTXT_CPU] = u8"CPU",
	[ZTXT_SMP] = u8"SMP",
	[ZTXT_NAME] = u8"Name",
	[ZTXT_MEMORY] = u8"Memory",
	[ZTXT_SIZE] = u8"Size",
	[ZTXT_FIRMWARE] = u8"Firmware",
	[ZTXT_BOOT_DEVICE] = u8"Boot Device",
	[ZTXT_DISK_IMAGE] = u8"Disk Image",
	[ZTXT_ISO_IMAGE] = u8"ISO Image",
	[ZTXT_PHYSICAL_DISK] = u8"Physical Disk",
	[ZTXT_CD_ROM] = u8"CD-ROM",
	[ZTXT_FLOPPY_IMAGE] = u8"Floppy Image",
	[ZTXT_PXE] = u8"PXE",
	[ZTXT_TFTP_FOLDER] = u8"TFTP Folder",
	[ZTXT_BOOT_FILE] = u8"Boot File",
	[ZTXT_LINUX_KERNEL] = u8"Linux Kernel",
	[ZTXT_INITRD] = u8"INITRD",
	[ZTXT_CMDLINE] = u8"Command Line",
	[ZTXT_DTB] = u8"Device Tree",
	[ZTXT_NO_DEVICE] = u8"NO DEVICE",
	[ZTXT_COPY] = u8"Copy",
	[ZTXT_SAVE] = u8"Save",
	[ZTXT_START] = u8"Start",
	[ZTXT_LOGS] = u8"Logs",
	[ZTXT_UNSUPPORTED] = u8"Unsupported",
};

static const char*
lang_zh_cn[ZTXT__MAX] =
{
	[ZTXT_QEMU] = u8"QEMU",
	[ZTXT_PATH] = u8"路径",
	[ZTXT_ARCH] = u8"架构",
	[ZTXT_CPU] = u8"CPU",
	[ZTXT_SMP] = u8"SMP",
	[ZTXT_NAME] = u8"名称",
	[ZTXT_MEMORY] = u8"内存",
	[ZTXT_SIZE] = u8"大小",
	[ZTXT_FIRMWARE] = u8"固件",
	[ZTXT_BOOT_DEVICE] = u8"启动设备",
	[ZTXT_DISK_IMAGE] = u8"磁盘镜像",
	[ZTXT_ISO_IMAGE] = u8"ISO 镜像",
	[ZTXT_PHYSICAL_DISK] = u8"物理磁盘",
	[ZTXT_CD_ROM] = u8"CD-ROM",
	[ZTXT_FLOPPY_IMAGE] = u8"软盘镜像",
	[ZTXT_PXE] = u8"PXE",
	[ZTXT_TFTP_FOLDER] = u8"TFTP 文件夹",
	[ZTXT_BOOT_FILE] = u8"启动文件",
	[ZTXT_LINUX_KERNEL] = u8"Linux 内核",
	[ZTXT_INITRD] = u8"INITRD",
	[ZTXT_CMDLINE] = u8"命令行",
	[ZTXT_DTB] = u8"设备树",
	[ZTXT_NO_DEVICE] = u8"无设备",
	[ZTXT_COPY] = u8"复制",
	[ZTXT_SAVE] = u8"保存",
	[ZTXT_START] = u8"启动",
	[ZTXT_LOGS] = u8"日志",
	[ZTXT_UNSUPPORTED] = u8"不支持",
};

// https://learn.microsoft.com/en-us/openspecs/office_standards/ms-oe376/6c085406-a698-4e12-9d4d-c3b0ee3dbc4a
static LANGID static_lang_id;

const char*
ZTXT(GETTEXT_STR_ID id)
{
	if (static_lang_id == 0)
		static_lang_id = GetUserDefaultUILanguage();
	if (id >= ZTXT__MAX)
		return INVALID_ZTXT_ID;

	const char* str = NULL;
	switch (static_lang_id)
	{
	case 2052: // Chinese - People's Republic of China
		str = lang_zh_cn[id];
		break;
	}
	// 1033: English - United States
	if (str == NULL)
		str = lang_en_us[id];
	return str;
}

