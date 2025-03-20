// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VC_EXTRALEAN
#include <windows.h>

#define FILTER_VHD \
	L"Disk Image (vhd,vhdx,vdi,vmdk,img,raw)\0" \
	 "*.VHD;*.VHDX;*.VDI;*.VMDK;*.IMG;*.RAW\0" \
	 "QEMU Image (qcow2,qcow)\0" \
	 "*.QCOW2;*.QCOW\0" \
	 "All Files\0*.*\0"

#define FILTER_ISO \
	L"ISO\0*.ISO\0All Files\0*.*\0"

#define FILTER_VFD \
	L"Floppy Image (img)\0*.IMG\0" \
	 "All Files\0*.*\0"

#define FILTER_DTB \
	L"Device Tree Binary (dtb)\0*.DTB\0" \
	 "All Files\0*.*\0"

#define FILTER_EFI \
	L"EFI Application (efi)\0*.EFI\0" \
	 "All Files\0*.*\0"

#define FILTER_WIM \
	L"WIM Image (wim)\0*.WIM\0" \
	 "All Files\0*.*\0"

#define FILTER_PNG \
	L"Portable Network Graphics (png)\0*.PNG\0"

#define FILTER_ALL \
	L"All Files\0*.*\0"

void
ui_open_file(CHAR* path, size_t len, LPCWSTR filter);

void
ui_open_file_by_dir(CHAR* path, size_t len, LPCSTR dir, LPCWSTR filter);

BOOL
ui_save_file(LPWSTR path, size_t len, LPCWSTR filter, LPCWSTR ext);

void
ui_open_dir(CHAR* path, size_t len);

#define GET_PNG(x) nk.image[x - IDR_PNG_MIN]

#define UI_OPTION(label, var, val) \
	var = nk_option_label(ctx, (label), var == val) ? val : var

void
ui_dev_button(struct nk_context* ctx,
	struct nk_image img, const char* label, nk_bool* value);

void
ui_popup_window(struct nk_context* ctx, float width, float height);

void
ui_popup_msg(const char* msg, DWORD icon);

nk_bool ui_is_qemu_running(void);

void ui_reset_log(void);

void
ui_ini_init(void);

void
ui_ini_save(void);

void
ui_qemu_dir(struct nk_context* ctx);

void
ui_qemu_cpu(struct nk_context* ctx);

void
ui_qemu_mem(struct nk_context* ctx);

void
ui_qemu_fw(struct nk_context* ctx);

void
ui_qemu_dev(struct nk_context* ctx);

void
ui_qemu_boot(struct nk_context* ctx);

void
ui_qemu_hdb(struct nk_context* ctx);

void
ui_qemu_start(struct nk_context* ctx);

void
ui_qemu_end(struct nk_context* ctx);
