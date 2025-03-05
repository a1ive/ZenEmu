// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_fw_init(void)
{
	nk.ini->qemu_fw_x86 = get_ini_num(L"Firmware", L"X86", ZEMU_FW_X64_EFI);
	if (nk.ini->qemu_fw_x86 >= ZEMU_FW_X86_MAX)
		nk.ini->qemu_fw_x86 = ZEMU_FW_X64_EFI;
	nk.ini->qemu_fw_arm = get_ini_num(L"Firmware", L"Arm", ZEMU_FW_AA64_EFI);
	if (nk.ini->qemu_fw_arm >= ZEMU_FW_ARM_MAX)
		nk.ini->qemu_fw_arm = ZEMU_FW_AA64_EFI;
}

void
ui_qemu_fw_save(void)
{
	set_ini_num(L"Firmware", L"X86", nk.ini->qemu_fw_x86);
	set_ini_num(L"Firmware", L"Arm", nk.ini->qemu_fw_arm);
}

void
ui_qemu_fw(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_FIRMWARE), ZTXT(ZTXT_FIRMWARE));
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
	{
		nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.2f, 0.2f, 0.2f, 0.2f });
		nk_spacer(ctx);
		UI_OPTION("BIOS", nk.ini->qemu_fw_x86, ZEMU_FW_X86_BIOS);
		UI_OPTION("IA32 EFI", nk.ini->qemu_fw_x86, ZEMU_FW_X86_EFI);
		UI_OPTION("X64 EFI", nk.ini->qemu_fw_x86, ZEMU_FW_X64_EFI);
		break;
	}
	case ZEMU_QEMU_ARCH_AA64:
	{
		nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.2f, 0.2f });
		nk_spacer(ctx);
		UI_OPTION("ARM64 EFI", nk.ini->qemu_fw_arm, ZEMU_FW_AA64_EFI);
		nk_widget_disable_begin(ctx);
		UI_OPTION("ARM32 EFI", nk.ini->qemu_fw_arm, ZEMU_FW_ARM32_EFI);
		nk_widget_disable_end(ctx);
		break;
	}
	default:
		nk_layout_row(ctx, NK_DYNAMIC, 0, 2, (float[2]) { 0.2f, 0.8f });
		nk_spacer(ctx);
		nk_label(ctx, ZTXT(ZTXT_UNSUPPORTED), NK_TEXT_CENTERED);
	}
}
