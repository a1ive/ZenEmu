// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

static void
ui_dev_button(struct nk_context* ctx,
	struct nk_image img, const char* label, nk_bool* value)
{
	if (*value)
	{
		nk.button_style.normal = nk_style_item_color(nk.color[NK_COLOR_BUTTON]);
		nk.button_style.hover = nk_style_item_color(nk.color[NK_COLOR_WINDOW]);
		nk.button_style.active = nk_style_item_color(nk.color[NK_COLOR_WINDOW]);
	}
	else
	{
		nk.button_style.normal = nk_style_item_color(nk.color[NK_COLOR_WINDOW]);
		nk.button_style.hover = nk_style_item_color(nk.color[NK_COLOR_BUTTON]);
		nk.button_style.active = nk_style_item_color(nk.color[NK_COLOR_BUTTON]);
	}
	if (nk_button_image_styled(ctx, &nk.button_style, img))
		*value = !*value;
	nk_label(ctx, label, NK_TEXT_LEFT);
}

void
ui_qemu_dev(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_PC), ZTXT(ZTXT_PERIPHERAL));

	nk_layout_row(ctx, NK_DYNAMIC, 0, 6, (float[6]) { nk.sq, 0.2f - nk.sq, 0.3f, 0.16f, 0.16f, 0.16f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_USB), ZTXT(ZTXT_USB), &nk.ini->cur->usb);
	if (!nk.ini->cur->usb)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->usbctrl, OPT_SZ, NULL);
	nk_checkbox_label(ctx, ZTXT(ZTXT_KEYBOARD), &nk.ini->cur->usb_kbd);
	nk_checkbox_label(ctx, ZTXT(ZTXT_MOUSE), &nk.ini->cur->usb_mouse);
	nk_checkbox_label(ctx, ZTXT(ZTXT_TABLET), &nk.ini->cur->usb_tablet);
	if (!nk.ini->cur->usb)
		nk_widget_disable_end(ctx);

	ui_dev_button(ctx, GET_PNG(IDR_PNG_NETWORK), ZTXT(ZTXT_NETWORK), &nk.ini->cur->net);
	if (!nk.ini->cur->net)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->netdev, OPT_SZ, NULL);
	nk_spacer(ctx);
	nk_spacer(ctx);
	nk_spacer(ctx);
	if (!nk.ini->cur->net)
		nk_widget_disable_end(ctx);
}
