// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_boot_init(void)
{
	nk.ini->qemu_boot_x86 = get_ini_num(L"Boot", L"X86", ZEMU_BOOT_X86_VHD);
	if (nk.ini->qemu_boot_x86 >= ZEMU_BOOT_X86_MAX)
		nk.ini->qemu_boot_x86 = ZEMU_BOOT_X86_VHD;
	nk.ini->qemu_boot_arm = get_ini_num(L"Boot", L"Arm", ZEMU_BOOT_ARM_VHD);
	if (nk.ini->qemu_boot_arm >= ZEMU_BOOT_ARM_MAX)
		nk.ini->qemu_boot_arm = ZEMU_BOOT_ARM_VHD;
}

void
ui_qemu_boot_save(void)
{
	set_ini_num(L"Boot", L"X86", nk.ini->qemu_boot_x86);
	set_ini_num(L"Boot", L"Arm", nk.ini->qemu_boot_arm);
}

void
ui_qemu_boot(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_PC), ZTXT(ZTXT_BOOT_DEVICE));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.4f, 0.4f });
	
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
	{
		nk_spacer(ctx);
		UI_OPTION(ZTXT(ZTXT_DISK_IMAGE), nk.ini->qemu_boot_x86, ZEMU_BOOT_X86_VHD);
		UI_OPTION(ZTXT(ZTXT_ISO_IMAGE), nk.ini->qemu_boot_x86, ZEMU_BOOT_X86_ISO);
		nk_spacer(ctx);
		UI_OPTION(ZTXT(ZTXT_PHYSICAL_DISK), nk.ini->qemu_boot_x86, ZEMU_BOOT_X86_PD);
		UI_OPTION(ZTXT(ZTXT_CD_ROM), nk.ini->qemu_boot_x86, ZEMU_BOOT_X86_CD);
		nk_spacer(ctx);
		UI_OPTION(ZTXT(ZTXT_FLOPPY_IMAGE), nk.ini->qemu_boot_x86, ZEMU_BOOT_X86_VFD);
		UI_OPTION(ZTXT(ZTXT_PXE), nk.ini->qemu_boot_x86, ZEMU_BOOT_X86_PXE);
		break;
	}
	case ZEMU_QEMU_ARCH_AA64:
	{
		nk_spacer(ctx);
		UI_OPTION(ZTXT(ZTXT_DISK_IMAGE), nk.ini->qemu_boot_arm, ZEMU_BOOT_ARM_VHD);
		UI_OPTION(ZTXT(ZTXT_ISO_IMAGE), nk.ini->qemu_boot_arm, ZEMU_BOOT_ARM_ISO);
		nk_spacer(ctx);
		UI_OPTION(ZTXT(ZTXT_PHYSICAL_DISK), nk.ini->qemu_boot_arm, ZEMU_BOOT_ARM_PD);
		UI_OPTION(ZTXT(ZTXT_CD_ROM), nk.ini->qemu_boot_arm, ZEMU_BOOT_ARM_CD);
		nk_spacer(ctx);
		nk_spacer(ctx);
		UI_OPTION(ZTXT(ZTXT_PXE), nk.ini->qemu_boot_arm, ZEMU_BOOT_ARM_PXE);
		break;
	}
	default:
		nk_spacer(ctx);
		nk_spacer(ctx);
		nk_spacer(ctx);
	}
}
