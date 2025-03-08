// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_cpu(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_CPU), ZTXT(ZTXT_CPU));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.2f, 0.6f });
	nk_space_label(ctx, ZTXT(ZTXT_SMP));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->smp, OPT_SZ, nk_filter_decimal);
	nk_spacer(ctx);
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.3f, 0.5f });
	nk_space_label(ctx, ZTXT(ZTXT_NAME));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->model, OPT_SZ, NULL);
	nk_spacer(ctx);
}
