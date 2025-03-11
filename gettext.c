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
	[ZTXT_MODEL] = u8"Model",
	[ZTXT_MACHINE] = u8"Machine",
	[ZTXT_KERNEL_IRQCHIP] = u8"IRQ Chip",
	[ZTXT_VIRT] = u8"Virtualization",
	[ZTXT_MEMORY] = u8"Memory",
	[ZTXT_SIZE] = u8"Size",
	[ZTXT_FIRMWARE] = u8"Firmware",
	[ZTXT_TYPE] = u8"Type",
	[ZTXT_PFLASH] = u8"pflash",
	[ZTXT_DISPLAY] = u8"Display",
	[ZTXT_NETWORK] = u8"Network",
	[ZTXT_PERIPHERAL] = u8"Peripheral",
	[ZTXT_USB] = u8"USB",
	[ZTXT_KEYBOARD] = u8"Keyboard",
	[ZTXT_MOUSE] = u8"Mouse",
	[ZTXT_TABLET] = u8"Tablet",
	[ZTXT_AUDIO]  = u8"Audio",
	[ZTXT_BACKEND] = u8"Backend",
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
	[ZTXT_KERNEL] = u8"Kernel",
	[ZTXT_INITRD] = u8"INITRD",
	[ZTXT_CMDLINE] = u8"cmdline",
	[ZTXT_DTB] = u8"DTB",
	[ZTXT_SHIM_EFI] = u8"shim.efi",
	[ZTXT_WIM_IMAGE] = u8"WIM Image",
	[ZTXT_DIR_VVFAT] = u8"Folder (VVFAT)",
	[ZTXT_DIR] = u8"Folder",
	[ZTXT_SNAPSHOT] = u8"Snapshot",
	[ZTXT_INTERFACE] = u8"Interface",
	[ZTXT_NO_DEVICE] = u8"NO DEVICE",
	[ZTXT_ADDITIONAL] = u8"Additional Storage",
	[ZTXT_FILE] = u8"File",
	[ZTXT_DEVICE] = u8"Device",
	[ZTXT_COPY] = u8"Copy",
	[ZTXT_SAVE] = u8"Save",
	[ZTXT_START] = u8"Start",
	[ZTXT_LOGS] = u8"Logs",
	[ZTXT_WARN_NOT_ADMIN] = u8"No administrator privileges. Click to obtain.",
	[ZTXT_WARN_NON_ASCII] = u8"Paths with commas or non-ASCII characters are NOT supported",
	[ZTXT_WARN_OUT_OF_MEM] = u8"Available memory may be insufficient",
	[ZTXT_UNSUPPORTED] = u8"Unsupported",
};

static const char*
lang_zh_cn[ZTXT__MAX] =
{
	[ZTXT_QEMU] = u8"QEMU",
	[ZTXT_PATH] = u8"路径",
	[ZTXT_ARCH] = u8"架构",
	[ZTXT_CPU] = u8"CPU",
	[ZTXT_SMP] = u8"核心数",
	[ZTXT_NAME] = u8"名称",
	[ZTXT_MODEL] = u8"型号",
	[ZTXT_MACHINE] = u8"机型",
	[ZTXT_KERNEL_IRQCHIP] = u8"IRQ Chip",
	[ZTXT_VIRT] = u8"虚拟化",
	[ZTXT_MEMORY] = u8"内存",
	[ZTXT_SIZE] = u8"大小",
	[ZTXT_FIRMWARE] = u8"固件",
	[ZTXT_TYPE] = u8"类型",
	[ZTXT_PFLASH] = u8"pflash",
	[ZTXT_DISPLAY] = u8"显示",
	[ZTXT_NETWORK] = u8"网络",
	[ZTXT_PERIPHERAL] = u8"外设",
	[ZTXT_USB] = u8"USB",
	[ZTXT_KEYBOARD] = u8"键盘",
	[ZTXT_MOUSE] = u8"鼠标",
	[ZTXT_TABLET] = u8"触摸板",
	[ZTXT_AUDIO] = u8"音频",
	[ZTXT_BACKEND] = u8"后端",
	[ZTXT_BOOT_DEVICE] = u8"启动设备",
	[ZTXT_DISK_IMAGE] = u8"磁盘镜像",
	[ZTXT_ISO_IMAGE] = u8"ISO 镜像",
	[ZTXT_PHYSICAL_DISK] = u8"物理磁盘",
	[ZTXT_CD_ROM] = u8"光驱",
	[ZTXT_FLOPPY_IMAGE] = u8"软盘镜像",
	[ZTXT_PXE] = u8"PXE",
	[ZTXT_TFTP_FOLDER] = u8"TFTP 文件夹",
	[ZTXT_BOOT_FILE] = u8"启动文件",
	[ZTXT_LINUX_KERNEL] = u8"Linux 内核",
	[ZTXT_KERNEL] = u8"内核",
	[ZTXT_INITRD] = u8"INITRD",
	[ZTXT_CMDLINE] = u8"命令行",
	[ZTXT_DTB] = u8"设备树",
	[ZTXT_SHIM_EFI] = u8"shim.efi",
	[ZTXT_WIM_IMAGE] = u8"WIM 镜像",
	[ZTXT_DIR_VVFAT] = u8"文件夹 (VVFAT)",
	[ZTXT_DIR] = u8"文件夹",
	[ZTXT_SNAPSHOT] = u8"快照",
	[ZTXT_INTERFACE] = u8"接口",
	[ZTXT_NO_DEVICE] = u8"无设备",
	[ZTXT_ADDITIONAL] = u8"附加存储",
	[ZTXT_FILE] = u8"文件",
	[ZTXT_DEVICE] = u8"设备",
	[ZTXT_COPY] = u8"复制",
	[ZTXT_SAVE] = u8"保存",
	[ZTXT_START] = u8"启动",
	[ZTXT_LOGS] = u8"日志",
	[ZTXT_WARN_NOT_ADMIN] = u8"无管理员权限，点击获取",
	[ZTXT_WARN_NON_ASCII] = u8"不支持包含逗号或非ASCII字符的路径",
	[ZTXT_WARN_OUT_OF_MEM] = u8"可用内存可能不足",
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

