// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_fw(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_FIRMWARE), ZTXT(ZTXT_FIRMWARE));

	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.2f, 0.2f, 0.2f, 0.2f });
	nk_space_label(ctx, ZTXT(ZTXT_TYPE));
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
	{
		UI_OPTION("BIOS", nk.ini->cur->fw, ZEMU_FW_X86_BIOS);
		UI_OPTION("IA32 EFI", nk.ini->cur->fw, ZEMU_FW_X86_EFI);
		UI_OPTION("X64 EFI", nk.ini->cur->fw, ZEMU_FW_X64_EFI);
		break;
	}
	case ZEMU_QEMU_ARCH_AA64:
	{
		UI_OPTION("ARM64 EFI", nk.ini->cur->fw, ZEMU_FW_AA64_EFI);
		nk_widget_disable_begin(ctx);
		UI_OPTION("ARM32 EFI", nk.ini->cur->fw, ZEMU_FW_ARM32_EFI);
		nk_widget_disable_end(ctx);
		nk_spacer(ctx);
		break;
	}
	default:
		nk_label(ctx, ZTXT(ZTXT_UNSUPPORTED), NK_TEXT_CENTERED);
		nk_spacer(ctx);
		nk_spacer(ctx);
	}
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.3f, 0.5f });
	nk_space_label(ctx, ZTXT(ZTXT_DISPLAY));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->vga, OPT_SZ, NULL);
	nk_checkbox_label(ctx, ZTXT(ZTXT_PFLASH), &nk.ini->cur->pflash);
}
