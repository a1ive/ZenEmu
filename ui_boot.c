// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "dev.h"
#include "gettext.h"

static void
obj_vhd(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_DISK_IMAGE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_vhd, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_vhd, MAX_PATH, FILTER_VHD);

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_vhd);
}

static void
obj_iso(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_ISO_IMAGE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_iso, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_iso, MAX_PATH, FILTER_ISO);

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_iso);
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

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_vfd);
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

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->net_tftp);
	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->net_file);
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
	if (!IS_BIOS)
	{
		nk_space_label(ctx, ZTXT(ZTXT_SHIM_EFI));
		nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_shim, MAX_PATH, NULL);
		if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
			ui_open_file(nk.ini->boot_shim, MAX_PATH, FILTER_EFI);
	}

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_linux);
	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_initrd);
	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_dtb);
	if (nk.show_warning == nk_false && !IS_BIOS)
		nk.show_warning = check_path_invalid(nk.ini->boot_shim);
}

static void
obj_wim(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_WIM_IMAGE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_wim, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_file(nk.ini->boot_wim, MAX_PATH, FILTER_WIM);

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_wim);
}

static void
obj_dir(struct nk_context* ctx)
{
	nk_space_label(ctx, ZTXT(ZTXT_DIR));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->boot_dir, MAX_PATH, NULL);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
		ui_open_dir(nk.ini->boot_dir, MAX_PATH);

	if (nk.show_warning == nk_false)
		nk.show_warning = check_path_invalid(nk.ini->boot_dir);
}

void
ui_qemu_boot(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_PC), ZTXT(ZTXT_BOOT_DEVICE));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.1f, 0.3f, 0.3f, 0.3f });
	
	nk_spacer(ctx);
	UI_OPTION(ZTXT(ZTXT_DISK_IMAGE), nk.ini->cur->boot, ZEMU_BOOT_VHD);
	UI_OPTION(ZTXT(ZTXT_ISO_IMAGE), nk.ini->cur->boot, ZEMU_BOOT_ISO);
	if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		nk_widget_disable_begin(ctx);
	UI_OPTION(ZTXT(ZTXT_FLOPPY_IMAGE), nk.ini->cur->boot, ZEMU_BOOT_VFD);
	if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		nk_widget_disable_end(ctx);

	nk_spacer(ctx);
	UI_OPTION(ZTXT(ZTXT_PHYSICAL_DISK), nk.ini->cur->boot, ZEMU_BOOT_PD);
	UI_OPTION(ZTXT(ZTXT_CD_ROM), nk.ini->cur->boot, ZEMU_BOOT_CD);
	nk_widget_disable_begin(ctx);
	UI_OPTION(ZTXT(ZTXT_PXE), nk.ini->cur->boot, ZEMU_BOOT_PXE);
	nk_widget_disable_end(ctx);

	nk_spacer(ctx);
	UI_OPTION(ZTXT(ZTXT_LINUX_KERNEL), nk.ini->cur->boot, ZEMU_BOOT_LINUX);
	if (!IS_BIOS)
		nk_widget_disable_begin(ctx);
	UI_OPTION(ZTXT(ZTXT_WIM_IMAGE), nk.ini->cur->boot, ZEMU_BOOT_WIM);
	if (!IS_BIOS)
		nk_widget_disable_end(ctx);
	if (IS_BIOS)
		nk_widget_disable_begin(ctx);
	UI_OPTION(ZTXT(ZTXT_DIR_VVFAT), nk.ini->cur->boot, ZEMU_BOOT_DIR);
	if (IS_BIOS)
		nk_widget_disable_end(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.8f - nk.sq, nk.sq });
	switch (nk.ini->cur->boot)
	{
	case ZEMU_BOOT_VHD:
		obj_vhd(ctx);
		break;
	case ZEMU_BOOT_ISO:
		obj_iso(ctx);
		break;
	case ZEMU_BOOT_PD:
		obj_hd(ctx);
		break;
	case ZEMU_BOOT_CD:
		obj_cd(ctx);
		break;
	case ZEMU_BOOT_VFD:
		obj_vfd(ctx);
		break;
	case ZEMU_BOOT_PXE:
		obj_pxe(ctx);
		break;
	case ZEMU_BOOT_LINUX:
		obj_linux(ctx);
		break;
	case ZEMU_BOOT_WIM:
		obj_wim(ctx);
		break;
	case ZEMU_BOOT_DIR:
		obj_dir(ctx);
		break;
	default:
		nk_spacer(ctx);
		nk_spacer(ctx);
		nk_spacer(ctx);
	}
}
