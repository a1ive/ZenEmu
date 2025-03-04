// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "dev.h"

#include <commdlg.h>

#define FILTER_VHD \
	L"Virtual Disk Image (vhd,vhdx,img)\0*.VHD;*.VHDX;*.IMG\0All Files\0*.*\0"

#define FILTER_ISO \
	L"ISO\0*.ISO\0All Files\0*.*\0"

static void
open_file(CHAR* path, size_t len, LPCWSTR filter)
{
	WCHAR buf[MAX_PATH] = L"";
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nk.wnd;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	if (GetOpenFileNameW(&ofn) == TRUE)
		strcpy_s(path, len, ucs2_to_utf8(buf));
}

void
ui_qemu_obj_init(void)
{

}

void
ui_qemu_obj_save(void)
{

}

static void
obj_vhd(struct nk_context* ctx)
{
	nk_label(ctx, "Disk Image", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_vhd, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		open_file(nk.ini->boot_vhd, MAX_PATH, FILTER_VHD);
}

static void
obj_iso(struct nk_context* ctx)
{
	nk_label(ctx, "ISO Image", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_iso, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		open_file(nk.ini->boot_iso, MAX_PATH, FILTER_ISO);
}

DWORD
nk_disk_list(struct nk_context* ctx, PHY_DRIVE_INFO* items, DWORD count,
	DWORD selected, int item_height, float width);

static void
obj_hd(struct nk_context* ctx)
{
	if (nk.ini->hd_info == NULL)
		nk.ini->hd_count = get_disk_list(FALSE, &nk.ini->hd_info);

	if (nk.ini->boot_hd >= nk.ini->hd_count)
		nk.ini->boot_hd = 0;

	nk_label(ctx, "Physical Disk", NK_TEXT_LEFT);

	if (nk.ini->hd_count == 0)
		nk_label(ctx, "NO DISK", NK_TEXT_CENTERED);
	else
		nk.ini->boot_hd = nk_disk_list(ctx, nk.ini->hd_info, nk.ini->hd_count, nk.ini->boot_hd,
			(int)nk.title_height, 0.7f * nk.width);
	if (nk_button_label(ctx, "Refresh"))
	{
		free(nk.ini->hd_info);
		nk.ini->hd_info = NULL;
		nk.ini->hd_count = 0;
	}
}

static void
ui_qemu_obj_x86(struct nk_context* ctx)
{	
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_label(ctx, "Boot Device", NK_TEXT_LEFT);
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.7f, 0.1f });
	switch (nk.ini->qemu_boot_x86)
	{
	case ZEMU_BOOT_X86_VHD:
		obj_vhd(ctx);
		break;
	case ZEMU_BOOT_X86_ISO:
		obj_iso(ctx);
		break;
	case ZEMU_BOOT_X86_PD:
		obj_hd(ctx);
		break;
	default:
		nk_spacer(ctx);
		nk_spacer(ctx);
		nk_spacer(ctx);
	}
}

static void
ui_qemu_obj_arm(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_label(ctx, "Boot Device", NK_TEXT_LEFT);
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.7f, 0.1f });
	switch (nk.ini->qemu_boot_arm)
	{
	case ZEMU_BOOT_ARM_VHD:
		obj_vhd(ctx);
		break;
	case ZEMU_BOOT_ARM_ISO:
		obj_iso(ctx);
		break;
	case ZEMU_BOOT_ARM_PD:
		obj_hd(ctx);
		break;
	default:
		nk_spacer(ctx);
		nk_spacer(ctx);
		nk_spacer(ctx);
	}
}

void
ui_qemu_obj(struct nk_context* ctx)
{
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		ui_qemu_obj_x86(ctx);
		return;
	case ZEMU_QEMU_ARCH_AA64:
		ui_qemu_obj_arm(ctx);
		return;
	}
}