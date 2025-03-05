// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

static char static_buf_mem[32];

void
ui_qemu_mem_init(void)
{
	nk.ini->qemu_mem_mb = get_ini_num(L"Memory", L"Size", 4096);
	snprintf(static_buf_mem, 32, "%d", nk.ini->qemu_mem_mb);
}

void
ui_qemu_mem_save(void)
{
	set_ini_num(L"Memory", L"Size", nk.ini->qemu_mem_mb);
}

void
ui_qemu_mem(struct nk_context* ctx)
{
	char buf[48];
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_MEMORY), ZTXT(ZTXT_MEMORY));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.2f, 0.3f, 0.1f, 0.4f });
	nk_space_label(ctx, ZTXT(ZTXT_SIZE));
	if (nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, static_buf_mem, 32, nk_filter_decimal)
		== NK_EDIT_COMMITED)
		nk.ini->qemu_mem_mb = (int)strtol(static_buf_mem, NULL, 10);
	nk_label(ctx, "MB", NK_TEXT_LEFT);
	strcpy_s(buf, 48, get_human_size(nk.statex.ullAvailPhys, human_units, 1024));
	nk_labelf(ctx, NK_TEXT_LEFT, "%lu%% (%s / %s)", nk.statex.dwMemoryLoad,
		buf, get_human_size(nk.statex.ullTotalPhys, human_units, 1024));
}
