// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VC_EXTRALEAN
#include <windows.h>

#define UI_OPTION(label, var, val) \
	var = nk_option_label(ctx, (label), var == val) ? val : var

void
ui_qemu_dir_init(void);
void
ui_qemu_dir(struct nk_context* ctx);
void
ui_qemu_dir_save(void);

void
ui_qemu_cpu_init(void);
void
ui_qemu_cpu(struct nk_context* ctx);
void
ui_qemu_cpu_save(void);

void
ui_qemu_mem_init(void);
void
ui_qemu_mem(struct nk_context* ctx);
void
ui_qemu_mem_save(void);

void
ui_qemu_fw_init(void);
void
ui_qemu_fw(struct nk_context* ctx);
void
ui_qemu_fw_save(void);

void
ui_qemu_boot_init(void);
void
ui_qemu_boot(struct nk_context* ctx);
void
ui_qemu_boot_save(void);

void
ui_qemu_obj_init(void);
void
ui_qemu_obj(struct nk_context* ctx);
void
ui_qemu_obj_save(void);

void
ui_qemu_end(struct nk_context* ctx);
