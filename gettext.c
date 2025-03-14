﻿// SPDX-License-Identifier: GPL-3.0-or-later

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
	[ZTXT_HYPER_V] = u8"Hyper-V",
	[ZTXT_KERNEL_IRQCHIP] = u8"IRQ Chip",
	[ZTXT_VIRT] = u8"Virtualization",
	[ZTXT_MEMORY] = u8"Memory",
	[ZTXT_SIZE] = u8"Size",
	[ZTXT_FIRMWARE] = u8"Firmware",
	[ZTXT_TYPE] = u8"Type",
	[ZTXT_OPTIONS] = u8"Options",
	[ZTXT_BOOT_MENU] = u8"Boot Menu",
	[ZTXT_TIMEOUT] = u8"Timeout",
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
	[ZTXT_INTEL_HDA] = u8"Intel HDA",
	[ZTXT_PC_SPEAKER] = u8"PC Speaker",
	[ZTXT_BOOT_DEVICE] = u8"Boot Device",
	[ZTXT_DISK_IMAGE] = u8"Disk Image",
	[ZTXT_ISO_IMAGE] = u8"ISO Image",
	[ZTXT_PHYSICAL_DISK] = u8"Physical Disk",
	[ZTXT_CD_ROM] = u8"CD-ROM",
	[ZTXT_HARD_DISK] = u8"Hard Disk",
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
	[ZTXT_HYPER_V] = u8"Hyper-V",
	[ZTXT_KERNEL_IRQCHIP] = u8"IRQ Chip",
	[ZTXT_VIRT] = u8"虚拟化",
	[ZTXT_MEMORY] = u8"内存",
	[ZTXT_SIZE] = u8"大小",
	[ZTXT_FIRMWARE] = u8"固件",
	[ZTXT_TYPE] = u8"类型",
	[ZTXT_OPTIONS] = u8"选项",
	[ZTXT_BOOT_MENU] = u8"启动菜单",
	[ZTXT_TIMEOUT] = u8"超时",
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
	[ZTXT_INTEL_HDA] = u8"Intel HDA",
	[ZTXT_PC_SPEAKER] = u8"PC Speaker",
	[ZTXT_BOOT_DEVICE] = u8"启动设备",
	[ZTXT_DISK_IMAGE] = u8"磁盘镜像",
	[ZTXT_ISO_IMAGE] = u8"ISO 镜像",
	[ZTXT_PHYSICAL_DISK] = u8"物理磁盘",
	[ZTXT_CD_ROM] = u8"光盘",
	[ZTXT_HARD_DISK] = u8"硬盘",
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

static const char*
lang_zh_tw[ZTXT__MAX] =
{
	[ZTXT_QEMU] = u8"QEMU",
	[ZTXT_PATH] = u8"路徑",
	[ZTXT_ARCH] = u8"架構",
	[ZTXT_CPU] = u8"CPU",
	[ZTXT_SMP] = u8"核心數",
	[ZTXT_NAME] = u8"名稱",
	[ZTXT_MODEL] = u8"型號",
	[ZTXT_MACHINE] = u8"機型",
	[ZTXT_HYPER_V] = u8"Hyper-V",
	[ZTXT_KERNEL_IRQCHIP] = u8"IRQ Chip",
	[ZTXT_VIRT] = u8"虛擬化",
	[ZTXT_MEMORY] = u8"記憶體",
	[ZTXT_SIZE] = u8"大小",
	[ZTXT_FIRMWARE] = u8"韌體",
	[ZTXT_TYPE] = u8"型別",
	[ZTXT_OPTIONS] = u8"選項",
	[ZTXT_BOOT_MENU] = u8"啟動選單",
	[ZTXT_TIMEOUT] = u8"超時",
	[ZTXT_PFLASH] = u8"pflash",
	[ZTXT_DISPLAY] = u8"顯示",
	[ZTXT_NETWORK] = u8"網路",
	[ZTXT_PERIPHERAL] = u8"外設",
	[ZTXT_USB] = u8"USB",
	[ZTXT_KEYBOARD] = u8"鍵盤",
	[ZTXT_MOUSE] = u8"滑鼠",
	[ZTXT_TABLET] = u8"觸控板",
	[ZTXT_AUDIO] = u8"音訊",
	[ZTXT_BACKEND] = u8"後端",
	[ZTXT_INTEL_HDA] = u8"Intel HDA",
	[ZTXT_PC_SPEAKER] = u8"PC Speaker",
	[ZTXT_BOOT_DEVICE] = u8"啟動裝置",
	[ZTXT_DISK_IMAGE] = u8"磁碟映象",
	[ZTXT_ISO_IMAGE] = u8"ISO 映象",
	[ZTXT_PHYSICAL_DISK] = u8"物理磁碟",
	[ZTXT_CD_ROM] = u8"光碟",
	[ZTXT_HARD_DISK] = u8"硬碟",
	[ZTXT_FLOPPY_IMAGE] = u8"軟盤映象",
	[ZTXT_PXE] = u8"PXE",
	[ZTXT_TFTP_FOLDER] = u8"TFTP 資料夾",
	[ZTXT_BOOT_FILE] = u8"啟動檔案",
	[ZTXT_LINUX_KERNEL] = u8"Linux 核心",
	[ZTXT_KERNEL] = u8"核心",
	[ZTXT_INITRD] = u8"INITRD",
	[ZTXT_CMDLINE] = u8"命令列",
	[ZTXT_DTB] = u8"裝置樹",
	[ZTXT_SHIM_EFI] = u8"shim.efi",
	[ZTXT_WIM_IMAGE] = u8"WIM 映象",
	[ZTXT_DIR_VVFAT] = u8"資料夾 (VVFAT)",
	[ZTXT_DIR] = u8"資料夾",
	[ZTXT_SNAPSHOT] = u8"快照",
	[ZTXT_INTERFACE] = u8"介面",
	[ZTXT_NO_DEVICE] = u8"無裝置",
	[ZTXT_ADDITIONAL] = u8"附加儲存",
	[ZTXT_FILE] = u8"檔案",
	[ZTXT_DEVICE] = u8"裝置",
	[ZTXT_COPY] = u8"複製",
	[ZTXT_SAVE] = u8"儲存",
	[ZTXT_START] = u8"啟動",
	[ZTXT_LOGS] = u8"日誌",
	[ZTXT_WARN_NOT_ADMIN] = u8"無管理員許可權，點選獲取",
	[ZTXT_WARN_NON_ASCII] = u8"不支援包含逗號或非ASCII字元的路徑",
	[ZTXT_WARN_OUT_OF_MEM] = u8"可用記憶體可能不足",
	[ZTXT_UNSUPPORTED] = u8"不支援",
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
	case 1028: // Chinese - Taiwan
		str = lang_zh_tw[id];
		break;
	}
	// 1033: English - United States
	if (str == NULL)
		str = lang_en_us[id];
	return str;
}

