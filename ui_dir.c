// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_dir_init(void)
{
	strcpy_s(nk.ini->qemu_dir, MAX_PATH, get_ini_value(L"Qemu", L"Dir", L"qemu"));
	nk.ini->qemu_arch = get_ini_num(L"Qemu", L"Arch", ZEMU_QEMU_ARCH_X64);
	if (nk.ini->qemu_arch >= ZEMU_QEMU_ARCH_MAX)
		nk.ini->qemu_arch = ZEMU_QEMU_ARCH_X64;
}

void
ui_qemu_dir_save(void)
{
	set_ini_value(L"Qemu", L"Dir", L"%s", utf8_to_ucs2(nk.ini->qemu_dir));
	set_ini_num(L"Qemu", L"Arch", nk.ini->qemu_arch);
}

void
ui_qemu_dir(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_QEMU), ZTXT(ZTXT_QEMU));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.8f - nk.sq, nk.sq });
	nk_space_label(ctx, ZTXT(ZTXT_PATH));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->qemu_dir, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_dir(nk.ini->qemu_dir, MAX_PATH);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.4f, 0.4f });
	nk_space_label(ctx, ZTXT(ZTXT_ARCH));
	UI_OPTION("x86_64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_X64);
	UI_OPTION("arm64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_AA64);
}
