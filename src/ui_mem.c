// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

static const char* edit_mem_list[] =
{
	"256", "512", "1024", "2048", "4096", "8192",
};

void
ui_qemu_mem(struct nk_context* ctx)
{
	char buf[48];
	nk_bool is_running = ui_is_qemu_running();
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_MEMORY), ZTXT(ZTXT_MEMORY));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { 0.2f, 0.2f - nk.sq, nk.sq, 0.1f, 0.5f });
	nk_space_label(ctx, ZTXT(ZTXT_SIZE));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->mem, OPT_SZ, nk_filter_decimal);
	if (nk_menu_begin_image_ex(ctx, "#EDIT_MEM", GET_PNG(IDR_PNG_DOWN), nk_vec2(100, 300)))
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		for (size_t i = 0; i < ARRAYSIZE(edit_mem_list); i++)
		{
			if (nk_menu_item_label(ctx, edit_mem_list[i], NK_TEXT_LEFT))
				strcpy_s(nk.ini->cur->mem, OPT_SZ, edit_mem_list[i]);
		}
		nk_menu_end(ctx);
	}
	nk_label(ctx, "MB", NK_TEXT_LEFT);
	strcpy_s(buf, 48, get_human_size(nk.statex.ullAvailPhys, human_units, 1024));
	nk_labelf(ctx, NK_TEXT_RIGHT, "%lu%% (%s / %s)", nk.statex.dwMemoryLoad,
		buf, get_human_size(nk.statex.ullTotalPhys, human_units, 1024));
	if (!is_running && strtoul(nk.ini->cur->mem, NULL, 10) + 64 >= nk.statex.ullAvailPhys / 1024 / 1024)
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		nk_image_label(ctx, GET_PNG(IDR_PNG_WARN), ZTXT(ZTXT_WARN_OUT_OF_MEM));
	}
}
