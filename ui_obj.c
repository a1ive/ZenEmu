// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "dev.h"

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
		ui_open_file(nk.ini->boot_vhd, MAX_PATH, FILTER_VHD);
}

static void
obj_iso(struct nk_context* ctx)
{
	nk_label(ctx, "ISO Image", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_iso, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		ui_open_file(nk.ini->boot_iso, MAX_PATH, FILTER_ISO);
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
obj_cd(struct nk_context* ctx)
{
	if (nk.ini->cd_info == NULL)
		nk.ini->cd_count = get_disk_list(TRUE, &nk.ini->cd_info);

	if (nk.ini->boot_cd >= nk.ini->cd_count)
		nk.ini->boot_cd = 0;

	nk_label(ctx, "CD-ROM", NK_TEXT_LEFT);

	if (nk.ini->cd_count == 0)
		nk_label(ctx, "NO DISK", NK_TEXT_CENTERED);
	else
		nk.ini->boot_cd = nk_disk_list(ctx, nk.ini->cd_info, nk.ini->cd_count, nk.ini->boot_cd,
			(int)nk.title_height, 0.7f * nk.width);
	if (nk_button_label(ctx, "Refresh"))
	{
		free(nk.ini->cd_info);
		nk.ini->cd_info = NULL;
		nk.ini->cd_count = 0;
	}
}

static void
obj_vfd(struct nk_context* ctx)
{
	nk_label(ctx, "Floppy Image", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_vfd, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		ui_open_file(nk.ini->boot_vfd, MAX_PATH, FILTER_VFD);
}

static void
obj_pxe(struct nk_context* ctx)
{
	nk_label(ctx, "TFTP Folder", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->net_tftp, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		ui_open_dir(nk.ini->net_tftp, MAX_PATH);
	nk_label(ctx, "Boot File", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->net_file, MAX_PATH, NULL);
	if (nk_button_label(ctx, ".."))
		ui_open_file(nk.ini->net_file, MAX_PATH, FILTER_ALL);
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
	case ZEMU_BOOT_X86_CD:
		obj_cd(ctx);
		break;
	case ZEMU_BOOT_X86_VFD:
		obj_vfd(ctx);
		break;
	case ZEMU_BOOT_X86_PXE:
		obj_pxe(ctx);
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
	case ZEMU_BOOT_ARM_CD:
		obj_cd(ctx);
		break;
	case ZEMU_BOOT_ARM_PXE:
		obj_pxe(ctx);
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