// SPDX-License-Identifier: GPL-3.0-or-later

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#pragma warning(disable:4996)
#pragma warning(disable:4116)
#pragma warning(disable:4244)

#include <assert.h>
#include <string.h>
#include <math.h>
#define NK_ASSERT(expr) assert(expr)

#define NK_MEMSET memset
#define NK_MEMCPY memcpy
#define NK_SIN sinf
#define NK_COS cosf
#define NK_STRTOD strtod

#define NK_IMPLEMENTATION
#include <nuklear.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

#define NK_GDIP_IMPLEMENTATION
#include <nuklear_gdip.h>

#include <VersionHelpers.h>

#include "dev.h"

GdipFont*
nk_gdip_load_font(LPCWSTR name, int size)
{
	GdipFont* font = (GdipFont*)calloc(1, sizeof(GdipFont));
	GpFontFamily* family;

	if (!font)
		goto fail;

	if (GdipCreateFontFamilyFromName(name, NULL, &family))
	{
		UINT len = IsWindowsVistaOrGreater() ? sizeof(NONCLIENTMETRICSW) : sizeof(NONCLIENTMETRICSW) - sizeof(int);
		NONCLIENTMETRICSW metrics = { .cbSize = len };
		if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, len, &metrics, 0))
		{
			if (GdipCreateFontFamilyFromName(metrics.lfMessageFont.lfFaceName, NULL, &family))
				goto fail;
		}
		else
			goto fail;
	}

	GdipCreateFont(family, (REAL)size, FontStyleRegular, UnitPixel, &font->handle);
	GdipDeleteFontFamily(family);

	return font;
fail:
	MessageBoxW(NULL, L"Failed to load font", L"Error", MB_OK);
	exit(1);
}

DWORD
nk_disk_list(struct nk_context* ctx, PHY_DRIVE_INFO* items, DWORD count,
	DWORD selected, int item_height, float width)
{
	DWORD i = 0;
	int max_height;
	struct nk_vec2 item_spacing;
	struct nk_vec2 window_padding;
	struct nk_vec2 size;

	NK_ASSERT(ctx);
	NK_ASSERT(items);
	NK_ASSERT(ctx->current);
	if (!ctx || !items || !count)
		return selected;

	item_spacing = ctx->style.window.spacing;
	window_padding = nk_panel_get_padding(&ctx->style, ctx->current->layout->type);
	max_height = NK_MIN(count, 8) * (item_height + (int)item_spacing.y);
	max_height += (int)item_spacing.y * 2 + (int)window_padding.y * 2;
	size.y = max_height;
	
	float x_padding = 2 * window_padding.y + 2 * item_spacing.y;
	size.x = (width > x_padding) ? width - x_padding : width;

	if (nk_combo_begin_text(ctx, items[selected].text, MAX_PATH, size))
	{
		nk_layout_row_dynamic(ctx, (float)item_height, 1);
		for (i = 0; i < count; ++i) {
			if (nk_combo_item_label(ctx, items[i].text, NK_TEXT_LEFT))
				selected = i;
		}
		nk_combo_end(ctx);
	}
	return selected;
}

void
nk_image_label(struct nk_context* ctx, struct nk_image img, const char* str)
{
	struct nk_window* win;
	const struct nk_style* style;
	struct nk_rect bounds;
	struct nk_rect icon;
	struct nk_text text;
	int len;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout) return;

	win = ctx->current;
	style = &ctx->style;
	len = nk_strlen(str);
	if (!nk_widget(&bounds, ctx))
		return;

	icon.w = icon.h = bounds.h;
	icon.x = bounds.x;
	icon.y = bounds.y;

	nk_draw_image(&win->buffer, icon, &img, nk_white);

	bounds.x = icon.x + icon.w + style->window.padding.x + style->window.border;
	bounds.w -= icon.w + style->window.padding.x + style->window.border;

	text.padding.x = style->text.padding.x;
	text.padding.y = style->text.padding.y;
	text.background = style->window.background;
	text.text = style->text.color;
	nk_widget_text(&win->buffer, bounds, str, len, &text, NK_TEXT_LEFT, style->font);
}
