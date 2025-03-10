// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

#include <shellapi.h>

static BOOL
is_admin(void)
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
	PSID admin_group;
	b = AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0, &admin_group);
	if (b)
	{
		if (!CheckTokenMembership(NULL, admin_group, &b))
			b = FALSE;
		FreeSid(admin_group);
	}
	return b;
}

static BOOL
relaunch_elevated(void)
{
	WCHAR prog[MAX_PATH];
	GetModuleFileNameW(NULL, prog, MAX_PATH);

	SHELLEXECUTEINFOW sei = { 0 };
	sei.cbSize = sizeof(sei);
	sei.lpVerb = L"runas";
	sei.lpFile = prog;
	sei.lpParameters = nk.cmdline;
	sei.nShow = SW_SHOWDEFAULT;

	if (!ShellExecuteExW(&sei))
		return FALSE;
	nkctx_fini(0);
}

void
ui_qemu_dir(struct nk_context* ctx)
{
	if (!is_admin())
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		if (nk_button_image_label(ctx, GET_PNG(IDR_PNG_ADMIN), ZTXT(ZTXT_WARN_NOT_ADMIN), NK_TEXT_CENTERED))
			relaunch_elevated();
	}

	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_QEMU), ZTXT(ZTXT_QEMU));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.8f - nk.sq, nk.sq });
	nk_space_label(ctx, ZTXT(ZTXT_PATH));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->qemu_dir, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_dir(nk.ini->qemu_dir, MAX_PATH);

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->qemu_dir);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.4f, 0.4f });
	nk_space_label(ctx, ZTXT(ZTXT_ARCH));
	UI_OPTION("x86_64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_X64);
	UI_OPTION("arm64", nk.ini->qemu_arch, ZEMU_QEMU_ARCH_AA64);
	nk.ini->cur = &nk.ini->profile[nk.ini->qemu_arch];
}
