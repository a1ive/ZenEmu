// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"



void
ui_qemu_dev(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_USB), ZTXT(ZTXT_PERIPHERAL));

	nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { 0.2f, 0.3f, 0.16f, 0.16f, 0.16f });
	nk_space_label(ctx, ZTXT(ZTXT_USB));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->usb, OPT_SZ, NULL);
	if (!nk.ini->cur->usb[0])
		nk_widget_disable_begin(ctx);
	nk_checkbox_label(ctx, ZTXT(ZTXT_KEYBOARD), &nk.ini->cur->usb_kbd);
	nk_checkbox_label(ctx, ZTXT(ZTXT_MOUSE), &nk.ini->cur->usb_mouse);
	nk_checkbox_label(ctx, ZTXT(ZTXT_TABLET), &nk.ini->cur->usb_tablet);
	if (!nk.ini->cur->usb[0])
		nk_widget_disable_end(ctx);
}