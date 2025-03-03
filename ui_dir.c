// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

#include <shlobj.h>

void
ui_qemu_dir_init(void)
{
	strcpy_s(nk.ini->qemu_dir, MAX_PATH, get_ini_value(L"Qemu", L"Dir", L"qemu"));
	nk.ini->qemu_arch = get_ini_num(L"Qemu", L"Arch", ZEMU_QEMU_ARCH_X64);
	if (nk.ini->qemu_arch >= ZEMU_QEMU_ARCH_MAX)
		nk.ini->qemu_arch = ZEMU_QEMU_ARCH_X64;
}

static int CALLBACK
browse_callback_fn(HWND wnd, UINT msg, LPARAM lparam, LPARAM data)
{
	int rc = 0;
	switch (msg)
	{
	case BFFM_INITIALIZED:
		SendMessageW(wnd, BFFM_SETSELECTION, TRUE, (LPARAM)nk.ini->pwd);
		break;
	case BFFM_VALIDATEFAILED:
		MessageBoxW(wnd, L"Invalid Path", L"ERROR", MB_OK);
		rc = 1;
		break;
	}
	return rc;
}

void
ui_qemu_dir(struct nk_context* ctx)
{
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.7f, 0.1f });
	nk_label(ctx, "QEMU Path", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->qemu_dir, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
	{
		WCHAR dir[MAX_PATH];
		BROWSEINFOW bi =
		{
			.hwndOwner = nk.wnd,
			.pidlRoot = NULL,
			.pszDisplayName = dir,
			.lpszTitle = L"Select Qemu Folder",
			.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_USENEWUI | BIF_VALIDATE,
			.lpfn = browse_callback_fn,
		};
		LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
		if (pidl != NULL)
		{
			SHGetPathFromIDListW(pidl, dir);
			strcpy_s(nk.ini->qemu_dir, MAX_PATH, ucs2_to_utf8(dir));
			CoTaskMemFree(pidl);
		}
	}

	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.4f, 0.4f });
	nk_label(ctx, "Arch", NK_TEXT_LEFT);
	UI_OPTION("x86_64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_X64);
	UI_OPTION("arm64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_AA64);
}
