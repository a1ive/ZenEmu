// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"

static UINT m_dpi = USER_DEFAULT_SCREEN_DPI;

void
nkctx_set_dpi_scaling(HWND wnd)
{
	if (nk.font)
	{
		nk_gdipfont_del(nk.font);
		nk.font = NULL;
	}
	if (nk.dpi_scaling)
	{
		RECT rect = { 0 };
		UINT dpi = GetDpiForWindow(wnd);
		nk.dpi_factor = 1.0 * dpi / m_dpi;
		m_dpi = dpi;
		nk.font_size = (int)(nk.font_size * nk.dpi_factor);
		// resize window
		GetWindowRect(wnd, &rect);
		SetWindowPos(wnd, NULL, 0, 0,
			(int)((rect.right - rect.left) * nk.dpi_factor), (int)((rect.bottom - rect.top) * nk.dpi_factor),
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	nk.font = nk_gdip_load_font(nk.font_name, nk.font_size);
	nk_gdip_set_font(nk.font);
}

struct nk_image
nkctx_load_png(WORD id)
{
	HRSRC res = FindResourceW(NULL, MAKEINTRESOURCEW(id), RT_RCDATA);
	if (!res)
		goto fail;
	HGLOBAL mem = LoadResource(NULL, res);
	if (!mem)
		goto fail;
	DWORD size = SizeofResource(NULL, res);
	if (!size)
		goto fail;
	void* data = LockResource(mem);
	if (!data)
		goto fail;
	return nk_gdip_load_image_from_memory(data, size);
fail:
	return nk_image_id(0);
}

void
nkctx_set_style(struct nk_context* ctx)
{
	nk.color[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
	nk.color[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
	nk.color[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
	nk.color[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
	nk.color[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
	nk.color[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
	nk.color[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
	nk.color[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
	nk.color[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
	nk.color[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
	nk.color[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
	nk.color[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
	nk.color[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
	nk.color[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
	nk.color[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
	nk.color[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
	nk.color[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
	nk.color[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
	nk.color[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
	nk.color[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
	nk.color[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
	nk.color[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
	nk.color[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
	nk.color[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
	nk.color[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
	nk.color[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
	nk.color[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
	nk.color[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
	nk.color[NK_COLOR_KNOB] = nk.color[NK_COLOR_SLIDER];
	nk.color[NK_COLOR_KNOB_CURSOR] = nk.color[NK_COLOR_SLIDER_CURSOR];
	nk.color[NK_COLOR_KNOB_CURSOR_HOVER] = nk.color[NK_COLOR_SLIDER_CURSOR_HOVER];
	nk.color[NK_COLOR_KNOB_CURSOR_ACTIVE] = nk.color[NK_COLOR_SLIDER_CURSOR_ACTIVE];

	nk_style_from_table(ctx, nk.color);

	nk.ctx->style.button.padding = nk_vec2(2.0f, 2.0f);
	nk.ctx->style.button.rounding = 2.0f;
	nk.ctx->style.window.scrollbar_size = nk_vec2(16.0f, 16.0f);

	memcpy(&nk.button_style, &ctx->style.button, sizeof(struct nk_style_button));

	nk.title_height = nk.font_size
		+ ctx->style.window.header.padding.y
		+ ctx->style.window.header.label_padding.y;
}
