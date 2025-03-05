// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "dev.h"
#include "gettext.h"

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
	nk_space_label(ctx, ZTXT(ZTXT_DISK_IMAGE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_vhd, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_vhd, MAX_PATH, FILTER_VHD);
}

static void
obj_iso(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_ISO_IMAGE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_iso, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_iso, MAX_PATH, FILTER_ISO);
}

static void
obj_hd(struct nk_context* ctx)
{
	if (nk.ini->hd_info == NULL)
		nk.ini->hd_count = get_disk_list(FALSE, &nk.ini->hd_info);

	if (nk.ini->boot_hd >= nk.ini->hd_count)
		nk.ini->boot_hd = 0;

	nk_space_label(ctx, ZTXT(ZTXT_PHYSICAL_DISK));

	if (nk.ini->hd_count == 0)
		nk_label(ctx, ZTXT(ZTXT_NO_DEVICE), NK_TEXT_CENTERED);
	else
		nk.ini->boot_hd = nk_disk_list(ctx, nk.ini->hd_info, nk.ini->hd_count, nk.ini->boot_hd,
			(int)nk.title_height, 0.7f * nk.width);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_REFRESH)))
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

	nk_space_label(ctx, ZTXT(ZTXT_CD_ROM));

	if (nk.ini->cd_count == 0)
		nk_label(ctx, ZTXT(ZTXT_NO_DEVICE), NK_TEXT_CENTERED);
	else
		nk.ini->boot_cd = nk_disk_list(ctx, nk.ini->cd_info, nk.ini->cd_count, nk.ini->boot_cd,
			(int)nk.title_height, 0.7f * nk.width);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_REFRESH)))
	{
		free(nk.ini->cd_info);
		nk.ini->cd_info = NULL;
		nk.ini->cd_count = 0;
	}
}

static void
obj_vfd(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_FLOPPY_IMAGE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_vfd, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_vfd, MAX_PATH, FILTER_VFD);
}

static void
obj_pxe(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_TFTP_FOLDER));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->net_tftp, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_dir(nk.ini->net_tftp, MAX_PATH);
	nk_space_label(ctx, ZTXT(ZTXT_BOOT_FILE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->net_file, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->net_file, MAX_PATH, FILTER_ALL);
}

static void
obj_linux(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_LINUX_KERNEL));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_linux, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_linux, MAX_PATH, FILTER_ALL);
	nk_space_label(ctx, ZTXT(ZTXT_INITRD));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_initrd, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_initrd, MAX_PATH, FILTER_ALL);
	nk_space_label(ctx, ZTXT(ZTXT_CMDLINE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_kcmd, KCMD_SZ, NULL);
	nk_spacer(ctx);
	nk_space_label(ctx, ZTXT(ZTXT_DTB));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_dtb, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_dtb, MAX_PATH, FILTER_DTB);
}

static void
ui_qemu_obj_x86(struct nk_context* ctx)
{	
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.8f - nk.sq, nk.sq });
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
	case ZEMU_BOOT_X86_LINUX:
		obj_linux(ctx);
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
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.8f - nk.sq, nk.sq });
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
	case ZEMU_BOOT_ARM_VFD:
		obj_vfd(ctx);
		break;
	case ZEMU_BOOT_ARM_PXE:
		obj_pxe(ctx);
		break;
	case ZEMU_BOOT_ARM_LINUX:
		obj_linux(ctx);
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