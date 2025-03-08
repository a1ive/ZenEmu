// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_mem(struct nk_context* ctx)
{
	char buf[48];
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_MEMORY), ZTXT(ZTXT_MEMORY));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.2f, 0.2f, 0.1f, 0.5f });
	nk_space_label(ctx, ZTXT(ZTXT_SIZE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->mem, OPT_SZ, nk_filter_decimal);
	nk_label(ctx, "MB", NK_TEXT_LEFT);
	strcpy_s(buf, 48, get_human_size(nk.statex.ullAvailPhys, human_units, 1024));
	nk_labelf(ctx, NK_TEXT_RIGHT, "%lu%% (%s / %s)", nk.statex.dwMemoryLoad,
		buf, get_human_size(nk.statex.ullTotalPhys, human_units, 1024));
}
