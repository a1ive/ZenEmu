// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

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
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.7f, 0.1f });
	nk_label(ctx, "QEMU Path", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->qemu_dir, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		ui_open_dir(nk.ini->qemu_dir, MAX_PATH);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.4f, 0.4f });
	nk_label(ctx, "Arch", NK_TEXT_LEFT);
	UI_OPTION("x86_64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_X64);
	UI_OPTION("arm64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_AA64);
}
