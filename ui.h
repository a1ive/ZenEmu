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

#define FILTER_ALL \
	L"All Files\0*.*\0"

void
ui_open_file(CHAR* path, size_t len, LPCWSTR filter);

void
ui_open_dir(CHAR* path, size_t len);

#define GET_PNG(x) nk.image[x - IDR_PNG_MIN]

#define UI_OPTION(label, var, val) \
	var = nk_option_label(ctx, (label), var == val) ? val : var

#define IS_BIOS \
	(nk.ini->qemu_arch == ZEMU_QEMU_ARCH_X64 && nk.ini->cur->fw == ZEMU_FW_X86_BIOS)

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
ui_qemu_boot(struct nk_context* ctx);

void
ui_qemu_obj(struct nk_context* ctx);

void
ui_qemu_end(struct nk_context* ctx);
