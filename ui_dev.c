// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

void
ui_qemu_dev(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_PC), ZTXT(ZTXT_PERIPHERAL));

	nk_layout_row(ctx, NK_DYNAMIC, 0, 6, (float[4]) { nk.sq, 0.2f - nk.sq, 0.3f, 0.5f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_DISPLAY), ZTXT(ZTXT_DISPLAY), &nk.ini->cur->graphics);
	if (!nk.ini->cur->graphics)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->vgadev, OPT_SZ, NULL);
	nk_spacer(ctx);
	if (!nk.ini->cur->graphics)
		nk_widget_disable_end(ctx);

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

	nk_layout_row(ctx, NK_DYNAMIC, 0, 6, (float[6]) { nk.sq, 0.2f - nk.sq, 0.3f, 0.16f, 0.16f, 0.16f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_NETWORK), ZTXT(ZTXT_NETWORK), &nk.ini->cur->net);
	if (!nk.ini->cur->net)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->netdev, OPT_SZ, NULL);
	nk_spacer(ctx);
	nk_spacer(ctx);
	nk_spacer(ctx);
	if (!nk.ini->cur->net)
		nk_widget_disable_end(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 6, (float[6]) { nk.sq, 0.2f - nk.sq, 0.2f, 0.2f, 0.1f, 0.3f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_AUDIO), ZTXT(ZTXT_AUDIO), &nk.ini->cur->audio);
	if (!nk.ini->cur->audio)
		nk_widget_disable_begin(ctx);
	nk_checkbox_label(ctx, ZTXT(ZTXT_INTEL_HDA), &nk.ini->cur->audio_hda);
	if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		nk_spacer(ctx);
	else
		nk_checkbox_label(ctx, ZTXT(ZTXT_PC_SPEAKER), &nk.ini->cur->audio_spk);
	nk_label(ctx, ZTXT(ZTXT_BACKEND), NK_TEXT_RIGHT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->audiodev, OPT_SZ, NULL);
	if (!nk.ini->cur->audio)
		nk_widget_disable_end(ctx);
}
