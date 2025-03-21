// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

static const char* edit_vga_list[] =
{
	"VGA", "vmware-svga", "ramfb",
	"virtio-vga", "virtio-gpu",
	"qxl-vga", "qxl", "bochs-display",
};

static const char* edit_usb_list[] =
{
	"qemu-xhci", "nec-usb-xhci",
	"usb-ehci", "ich9-usb-ehci1",
};

static const char* edit_net_list[] =
{
	"e1000", "rtl8139", "virtio", "vmxnet3",
};

static nk_bool
filter_mac(const struct nk_text_edit* box, nk_rune unicode)
{
	if ((box->cursor + 1) % 3 == 0)
		return unicode == ':' ? nk_true : nk_false;
	return isxdigit(unicode) ? nk_true : nk_false;
}

void
ui_qemu_dev(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_PC), ZTXT(ZTXT_PERIPHERAL));

	nk_layout_row(ctx, NK_DYNAMIC, 0, 7,
		(float[7]) { nk.sq, 0.2f - nk.sq, 0.3f - nk.sq, nk.sq, 0.16f, 0.16f, 0.16f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_DISPLAY), ZTXT(ZTXT_DISPLAY), &nk.ini->cur->graphics);
	if (!nk.ini->cur->graphics)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->vgadev, OPT_SZ, nk_filter_ascii);
	if (nk_menu_begin_image_ex(ctx, "#EDIT_VGA", GET_PNG(IDR_PNG_DOWN), nk_vec2(200, 300)))
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		for (size_t i = 0; i < ARRAYSIZE(edit_vga_list); i++)
		{
			if (nk_menu_item_label(ctx, edit_vga_list[i], NK_TEXT_LEFT))
				strcpy_s(nk.ini->cur->vgadev, OPT_SZ, edit_vga_list[i]);
		}
		nk_menu_end(ctx);
	}
	if (!nk.ini->cur->graphics)
		nk_widget_disable_end(ctx);
	nk_spacer(ctx);
	nk_spacer(ctx);
	nk_checkbox_label(ctx, ZTXT(ZTXT_FULLSCREEN), &nk.ini->qemu_fullscreen);

	if (nk.ini->qemu_fullscreen)
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		nk_image_label(ctx, GET_PNG(IDR_PNG_INFO), ZTXT(ZTXT_WARN_FULLSCREEN));
	}

	nk_layout_row(ctx, NK_DYNAMIC, 0, 7,
		(float[7]) { nk.sq, 0.2f - nk.sq, 0.3f - nk.sq, nk.sq, 0.16f, 0.16f, 0.16f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_USB), ZTXT(ZTXT_USB), &nk.ini->cur->usb);
	if (!nk.ini->cur->usb)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->usbctrl, OPT_SZ, nk_filter_ascii);
	if (nk_menu_begin_image_ex(ctx, "#EDIT_USB", GET_PNG(IDR_PNG_DOWN), nk_vec2(200, 300)))
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		for (size_t i = 0; i < ARRAYSIZE(edit_usb_list); i++)
		{
			if (nk_menu_item_label(ctx, edit_usb_list[i], NK_TEXT_LEFT))
				strcpy_s(nk.ini->cur->usbctrl, OPT_SZ, edit_usb_list[i]);
		}
		nk_menu_end(ctx);
	}
	nk_checkbox_label(ctx, ZTXT(ZTXT_KEYBOARD), &nk.ini->cur->usb_kbd);
	nk_checkbox_label(ctx, ZTXT(ZTXT_MOUSE), &nk.ini->cur->usb_mouse);
	nk_checkbox_label(ctx, ZTXT(ZTXT_TABLET), &nk.ini->cur->usb_tablet);
	if (!nk.ini->cur->usb)
		nk_widget_disable_end(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 7,
		(float[7]) { nk.sq, 0.2f - nk.sq, 0.3f - nk.sq, nk.sq, 0.1f, 0.4f - nk.sq, nk.sq });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_NETWORK), ZTXT(ZTXT_NETWORK), &nk.ini->cur->net);
	if (!nk.ini->cur->net)
		nk_widget_disable_begin(ctx);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->netdev, OPT_SZ, nk_filter_ascii);
	if (nk_menu_begin_image_ex(ctx, "#EDIT_NET", GET_PNG(IDR_PNG_DOWN), nk_vec2(200, 300)))
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		for (size_t i = 0; i < ARRAYSIZE(edit_net_list); i++)
		{
			if (nk_menu_item_label(ctx, edit_net_list[i], NK_TEXT_LEFT))
				strcpy_s(nk.ini->cur->netdev, OPT_SZ, edit_net_list[i]);
		}
		nk_menu_end(ctx);
	}
	nk_label(ctx, ZTXT(ZTXT_MAC_ADDRESS), NK_TEXT_RIGHT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->netmac, MAC_SZ, filter_mac);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_DICE)))
	{
		uint8_t mac[6];
		for (size_t i = 0; i < 6; i++)
			mac[i] = (uint8_t)rand();
		sprintf_s(nk.ini->cur->netmac, MAC_SZ, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		nk.ini->cur->netmac[1] = '2';
	}
	if (!nk.ini->cur->net)
		nk_widget_disable_end(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 6, (float[6]) { nk.sq, 0.2f - nk.sq, 0.2f, 0.2f, 0.2f, 0.2f });
	ui_dev_button(ctx, GET_PNG(IDR_PNG_AUDIO), ZTXT(ZTXT_AUDIO), &nk.ini->cur->audio);
	if (!nk.ini->cur->audio)
		nk_widget_disable_begin(ctx);
	nk_checkbox_label(ctx, ZTXT(ZTXT_INTEL_HDA), &nk.ini->cur->audio_hda);
	if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		nk_spacer(ctx);
	else
		nk_checkbox_label(ctx, ZTXT(ZTXT_PC_SPEAKER), &nk.ini->cur->audio_spk);
	nk_label(ctx, ZTXT(ZTXT_BACKEND), NK_TEXT_RIGHT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->audiodev, OPT_SZ, nk_filter_ascii);
	if (!nk.ini->cur->audio)
		nk_widget_disable_end(ctx);
}
