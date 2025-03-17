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
#include "ui.h"
#include "resource.h"
#include "nkctx_priv.h"

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

static nk_bool
nk_do_button_ex(nk_flags* state,
	struct nk_command_buffer* out, struct nk_rect bounds,
	struct nk_image img, const char* str, int len,
	enum nk_button_behavior behavior, const struct nk_style_button* style,
	const struct nk_user_font* font, const struct nk_input* in)
{
	int ret;
	struct nk_rect icon;
	struct nk_rect content;

	NK_ASSERT(style);
	NK_ASSERT(state);
	NK_ASSERT(font);
	NK_ASSERT(out);
	if (!out || !font || !style || !str)
		return nk_false;

	ret = nk_do_button(state, out, bounds, style, in, behavior, &content);
	icon.y = bounds.y + style->padding.y;
	icon.w = icon.h = bounds.h - 2 * style->padding.y;
	icon.x = bounds.x + 2 * style->padding.x;

	icon.x += style->image_padding.x;
	icon.y += style->image_padding.y;
	icon.w -= 2 * style->image_padding.x;
	icon.h -= 2 * style->image_padding.y;

	if (content.w > icon.w)
	{
		content.x += icon.w;
		content.w -= icon.w;
	}

	if (style->draw_begin)
		style->draw_begin(out, style->userdata);
	nk_draw_button_text_image(out, &bounds, &content, &icon, *state, style, str, len, font, &img);
	if (style->draw_end)
		style->draw_end(out, style->userdata);
	return ret;
}

nk_bool nk_button_ex(struct nk_context* ctx, struct nk_image img, const char* label)
{
	struct nk_window* win;
	struct nk_panel* layout;
	const struct nk_input* in;

	struct nk_rect bounds;
	enum nk_widget_layout_states state;
	int len = nk_strlen(label);

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return 0;

	win = ctx->current;
	layout = win->layout;

	state = nk_widget(&bounds, ctx);
	if (!state)
		return 0;
	in = (state == NK_WIDGET_ROM || state == NK_WIDGET_DISABLED || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
	return nk_do_button_ex(&ctx->last_widget_state, &win->buffer,
		bounds, img, label, len, ctx->button_behavior,
		&ctx->style.button, ctx->style.font, in);
}

static nk_bool
nk_panel_begin_ex(struct nk_context* ctx, const char* title, enum nk_panel_type panel_type)
{
	struct nk_input* in;
	struct nk_window* win;
	struct nk_panel* layout;
	struct nk_command_buffer* out;
	const struct nk_style* style;
	const struct nk_user_font* font;

	struct nk_vec2 scrollbar_size;
	struct nk_vec2 panel_padding;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout) return 0;
	nk_zero(ctx->current->layout, sizeof(*ctx->current->layout));
	if ((ctx->current->flags & NK_WINDOW_HIDDEN) || (ctx->current->flags & NK_WINDOW_CLOSED)) {
		nk_zero(ctx->current->layout, sizeof(struct nk_panel));
		ctx->current->layout->type = panel_type;
		return 0;
	}
	/* pull state into local stack */
	style = &ctx->style;
	font = style->font;
	win = ctx->current;
	layout = win->layout;
	out = &win->buffer;
	in = (win->flags & NK_WINDOW_NO_INPUT) ? 0 : &ctx->input;
#ifdef NK_INCLUDE_COMMAND_USERDATA
	win->buffer.userdata = ctx->userdata;
#endif
	/* pull style configuration into local stack */
	scrollbar_size = style->window.scrollbar_size;
	panel_padding = nk_panel_get_padding(style, panel_type);

	/* window movement */
	if ((win->flags & NK_WINDOW_MOVABLE) && !(win->flags & NK_WINDOW_ROM)) {
		nk_bool left_mouse_down;
		unsigned int left_mouse_clicked;
		int left_mouse_click_in_cursor;

		/* calculate draggable window space */
		struct nk_rect header;
		header.x = win->bounds.x;
		header.y = win->bounds.y;
		header.w = win->bounds.w;
		if (nk_panel_has_header(win->flags, title)) {
			header.h = font->height + 2.0f * style->window.header.padding.y;
			header.h += 2.0f * style->window.header.label_padding.y;
		}
		else header.h = panel_padding.y;

		/* window movement by dragging */
		left_mouse_down = in->mouse.buttons[NK_BUTTON_LEFT].down;
		left_mouse_clicked = in->mouse.buttons[NK_BUTTON_LEFT].clicked;
		left_mouse_click_in_cursor = nk_input_has_mouse_click_down_in_rect(in,
			NK_BUTTON_LEFT, header, nk_true);
		if (left_mouse_down && left_mouse_click_in_cursor && !left_mouse_clicked) {
			win->bounds.x = win->bounds.x + in->mouse.delta.x;
			win->bounds.y = win->bounds.y + in->mouse.delta.y;
			in->mouse.buttons[NK_BUTTON_LEFT].clicked_pos.x += in->mouse.delta.x;
			in->mouse.buttons[NK_BUTTON_LEFT].clicked_pos.y += in->mouse.delta.y;
			ctx->style.cursor_active = ctx->style.cursors[NK_CURSOR_MOVE];
		}
	}

	/* setup panel */
	layout->type = panel_type;
	layout->flags = win->flags;
	layout->bounds = win->bounds;
	layout->bounds.x += panel_padding.x;
	layout->bounds.w -= 2 * panel_padding.x;
	if (win->flags & NK_WINDOW_BORDER) {
		layout->border = nk_panel_get_border(style, win->flags, panel_type);
		layout->bounds = nk_shrink_rect(layout->bounds, layout->border);
	}
	else layout->border = 0;
	layout->at_y = layout->bounds.y;
	layout->at_x = layout->bounds.x;
	layout->max_x = 0;
	layout->header_height = 0;
	layout->footer_height = 0;
	nk_layout_reset_min_row_height(ctx);
	layout->row.index = 0;
	layout->row.columns = 0;
	layout->row.ratio = 0;
	layout->row.item_width = 0;
	layout->row.tree_depth = 0;
	layout->row.height = panel_padding.y;
	layout->has_scrolling = nk_true;
	if (!(win->flags & NK_WINDOW_NO_SCROLLBAR))
		layout->bounds.w -= scrollbar_size.x;
	if (!nk_panel_is_nonblock(panel_type)) {
		layout->footer_height = 0;
		if (!(win->flags & NK_WINDOW_NO_SCROLLBAR) || win->flags & NK_WINDOW_SCALABLE)
			layout->footer_height = scrollbar_size.y;
		layout->bounds.h -= layout->footer_height;
	}

	/* panel header */
	if (nk_panel_has_header(win->flags, title))
	{
		struct nk_text text;
		struct nk_rect header;
		const struct nk_style_item* background = 0;

		/* calculate header bounds */
		header.x = win->bounds.x;
		header.y = win->bounds.y;
		header.w = win->bounds.w;
		header.h = font->height + 2.0f * style->window.header.padding.y;
		header.h += (2.0f * style->window.header.label_padding.y);

		/* shrink panel by header */
		layout->header_height = header.h;
		layout->bounds.y += header.h;
		layout->bounds.h -= header.h;
		layout->at_y += header.h;

		/* select correct header background and text color */
		if (ctx->active == win) {
			background = &style->window.header.active;
			text.text = style->window.header.label_active;
		}
		else if (nk_input_is_mouse_hovering_rect(&ctx->input, header)) {
			background = &style->window.header.hover;
			text.text = style->window.header.label_hover;
		}
		else {
			background = &style->window.header.normal;
			text.text = style->window.header.label_normal;
		}

		/* draw header background */
		header.h += 1.0f;

		switch (background->type) {
		case NK_STYLE_ITEM_IMAGE:
			text.background = nk_rgba(0, 0, 0, 0);
			nk_draw_image(&win->buffer, header, &background->data.image, nk_white);
			break;
		case NK_STYLE_ITEM_NINE_SLICE:
			text.background = nk_rgba(0, 0, 0, 0);
			nk_draw_nine_slice(&win->buffer, header, &background->data.slice, nk_white);
			break;
		case NK_STYLE_ITEM_COLOR:
			text.background = background->data.color;
			nk_fill_rect(out, header, 0, background->data.color);
			break;
		}

		/* window close button */
		{
			struct nk_rect button;
			button.y = header.y + style->window.header.padding.y;
			button.h = header.h - 2 * style->window.header.padding.y;
			button.w = button.h;
			if (win->flags & NK_WINDOW_CLOSABLE) {
				nk_flags ws = 0;
				if (style->window.header.align == NK_HEADER_RIGHT) {
					button.x = (header.w + header.x) - (button.w + style->window.header.padding.x);
					header.w -= button.w + style->window.header.spacing.x + style->window.header.padding.x;
				}
				else {
					button.x = header.x + style->window.header.padding.x;
					header.x += button.w + style->window.header.spacing.x + style->window.header.padding.x;
				}

				if (nk_do_button_image(&ws, &win->buffer, button,
					GET_PNG(IDR_PNG_CLOSE), NK_BUTTON_DEFAULT,
					&style->window.header.close_button, in) && !(win->flags & NK_WINDOW_ROM))
				{
					layout->flags |= NK_WINDOW_HIDDEN;
					layout->flags &= (nk_flags)~NK_WINDOW_MINIMIZED;
				}
			}

			/* window minimize button */
			if (win->flags & NK_WINDOW_MINIMIZABLE) {
				nk_flags ws = 0;
				if (style->window.header.align == NK_HEADER_RIGHT) {
					button.x = (header.w + header.x) - button.w;
					if (!(win->flags & NK_WINDOW_CLOSABLE)) {
						button.x -= style->window.header.padding.x;
						header.w -= style->window.header.padding.x;
					}
					header.w -= button.w + style->window.header.spacing.x;
				}
				else {
					button.x = header.x;
					header.x += button.w + style->window.header.spacing.x + style->window.header.padding.x;
				}
				if (nk_do_button_image(&ws, &win->buffer, button, (layout->flags & NK_WINDOW_MINIMIZED) ?
					GET_PNG(IDR_PNG_MAXIMIZE) : GET_PNG(IDR_PNG_MINIMIZE),
					NK_BUTTON_DEFAULT, &style->window.header.minimize_button, in) && !(win->flags & NK_WINDOW_ROM))
					layout->flags = (layout->flags & NK_WINDOW_MINIMIZED) ?
					layout->flags & (nk_flags)~NK_WINDOW_MINIMIZED :
					layout->flags | NK_WINDOW_MINIMIZED;
			}
		}

		{
			/* window header icon and title */
			int text_len = nk_strlen(title);
			struct nk_rect label = { 0,0,0,0 };
			struct nk_rect icon;
			struct nk_image img = GET_PNG(IDR_PNG_QEMU);
			float t = font->width(font->userdata, font->height, title, text_len);
			text.padding = nk_vec2(0, 0);

			label.x = header.x + style->window.header.padding.x;
			label.x += style->window.header.label_padding.x;
			label.y = header.y + style->window.header.label_padding.y;
			label.h = font->height + 2 * style->window.header.label_padding.y;
			label.w = t + 2 * style->window.header.spacing.x;
			label.w = NK_CLAMP(0, label.w, header.x + header.w - label.x);

			icon.w = icon.h = label.h;
			icon.x = label.x;
			icon.y = label.y;
			nk_draw_image(out, icon, &img, nk_white);

			label.x += icon.w + style->window.header.padding.x;

			nk_widget_text(out, label, (const char*)title, text_len, &text, NK_TEXT_LEFT, font);
		}
	}

	/* draw window background */
	if (!(layout->flags & NK_WINDOW_MINIMIZED) && !(layout->flags & NK_WINDOW_DYNAMIC)) {
		struct nk_rect body;
		body.x = win->bounds.x;
		body.w = win->bounds.w;
		body.y = (win->bounds.y + layout->header_height);
		body.h = (win->bounds.h - layout->header_height);

		switch (style->window.fixed_background.type) {
		case NK_STYLE_ITEM_IMAGE:
			nk_draw_image(out, body, &style->window.fixed_background.data.image, nk_white);
			break;
		case NK_STYLE_ITEM_NINE_SLICE:
			nk_draw_nine_slice(out, body, &style->window.fixed_background.data.slice, nk_white);
			break;
		case NK_STYLE_ITEM_COLOR:
			nk_fill_rect(out, body, style->window.rounding, style->window.fixed_background.data.color);
			break;
		}
	}

	/* set clipping rectangle */
	{
		struct nk_rect clip;
		layout->clip = layout->bounds;
		nk_unify(&clip, &win->buffer.clip, layout->clip.x, layout->clip.y,
			layout->clip.x + layout->clip.w, layout->clip.y + layout->clip.h);
		nk_push_scissor(out, clip);
		layout->clip = clip;
	}
	return !(layout->flags & NK_WINDOW_HIDDEN) && !(layout->flags & NK_WINDOW_MINIMIZED);
}

nk_bool
nk_begin_ex(struct nk_context* ctx, const char* title,
	struct nk_rect bounds, nk_flags flags)
{
	struct nk_window* win;
	struct nk_style* style;
	nk_hash name_hash;
	int name_len;
	int ret = 0;

	NK_ASSERT(ctx);
	NK_ASSERT(title);
	NK_ASSERT(ctx->style.font && ctx->style.font->width && "if this triggers you forgot to add a font");
	NK_ASSERT(!ctx->current && "if this triggers you missed a `nk_end` call");
	if (!ctx || ctx->current || !title)
		return 0;

	/* find or create window */
	style = &ctx->style;
	name_len = (int)nk_strlen(title);
	name_hash = nk_murmur_hash(title, (int)name_len, NK_WINDOW_TITLE);
	win = nk_find_window(ctx, name_hash, title);
	if (!win) {
		/* create new window */
		nk_size name_length = (nk_size)name_len;
		win = (struct nk_window*)nk_create_window(ctx);
		NK_ASSERT(win);
		if (!win) return 0;

		if (flags & NK_WINDOW_BACKGROUND)
			nk_insert_window(ctx, win, NK_INSERT_FRONT);
		else nk_insert_window(ctx, win, NK_INSERT_BACK);
		nk_command_buffer_init(&win->buffer, &ctx->memory, NK_CLIPPING_ON);

		win->flags = flags;
		win->bounds = bounds;
		win->name = name_hash;
		name_length = NK_MIN(name_length, NK_WINDOW_MAX_NAME - 1);
		NK_MEMCPY(win->name_string, title, name_length);
		win->name_string[name_length] = 0;
		win->popup.win = 0;
		win->widgets_disabled = nk_false;
		if (!ctx->active)
			ctx->active = win;
	}
	else {
		/* update window */
		win->flags &= ~(nk_flags)(NK_WINDOW_PRIVATE - 1);
		win->flags |= flags;
		if (!(win->flags & (NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)))
			win->bounds = bounds;
		/* If this assert triggers you either:
		 *
		 * I.) Have more than one window with the same name or
		 * II.) You forgot to actually draw the window.
		 *      More specific you did not call `nk_clear` (nk_clear will be
		 *      automatically called for you if you are using one of the
		 *      provided demo backends). */
		NK_ASSERT(win->seq != ctx->seq);
		win->seq = ctx->seq;
		if (!ctx->active && !(win->flags & NK_WINDOW_HIDDEN)) {
			ctx->active = win;
			ctx->end = win;
		}
	}
	if (win->flags & NK_WINDOW_HIDDEN) {
		ctx->current = win;
		win->layout = 0;
		return 0;
	}
	else nk_start(ctx, win);

	/* window overlapping */
	if (!(win->flags & NK_WINDOW_HIDDEN) && !(win->flags & NK_WINDOW_NO_INPUT))
	{
		int inpanel, ishovered;
		struct nk_window* iter = win;
		float h = ctx->style.font->height + 2.0f * style->window.header.padding.y +
			(2.0f * style->window.header.label_padding.y);
		struct nk_rect win_bounds = (!(win->flags & NK_WINDOW_MINIMIZED)) ?
			win->bounds : nk_rect(win->bounds.x, win->bounds.y, win->bounds.w, h);

		/* activate window if hovered and no other window is overlapping this window */
		inpanel = nk_input_has_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_LEFT, win_bounds, nk_true);
		inpanel = inpanel && ctx->input.mouse.buttons[NK_BUTTON_LEFT].clicked;
		ishovered = nk_input_is_mouse_hovering_rect(&ctx->input, win_bounds);
		if ((win != ctx->active) && ishovered && !ctx->input.mouse.buttons[NK_BUTTON_LEFT].down) {
			iter = win->next;
			while (iter) {
				struct nk_rect iter_bounds = (!(iter->flags & NK_WINDOW_MINIMIZED)) ?
					iter->bounds : nk_rect(iter->bounds.x, iter->bounds.y, iter->bounds.w, h);
				if (NK_INTERSECT(win_bounds.x, win_bounds.y, win_bounds.w, win_bounds.h,
					iter_bounds.x, iter_bounds.y, iter_bounds.w, iter_bounds.h) &&
					(!(iter->flags & NK_WINDOW_HIDDEN)))
					break;

				if (iter->popup.win && iter->popup.active && !(iter->flags & NK_WINDOW_HIDDEN) &&
					NK_INTERSECT(win->bounds.x, win_bounds.y, win_bounds.w, win_bounds.h,
						iter->popup.win->bounds.x, iter->popup.win->bounds.y,
						iter->popup.win->bounds.w, iter->popup.win->bounds.h))
					break;
				iter = iter->next;
			}
		}

		/* activate window if clicked */
		if (iter && inpanel && (win != ctx->end)) {
			iter = win->next;
			while (iter) {
				/* try to find a panel with higher priority in the same position */
				struct nk_rect iter_bounds = (!(iter->flags & NK_WINDOW_MINIMIZED)) ?
					iter->bounds : nk_rect(iter->bounds.x, iter->bounds.y, iter->bounds.w, h);
				if (NK_INBOX(ctx->input.mouse.pos.x, ctx->input.mouse.pos.y,
					iter_bounds.x, iter_bounds.y, iter_bounds.w, iter_bounds.h) &&
					!(iter->flags & NK_WINDOW_HIDDEN))
					break;
				if (iter->popup.win && iter->popup.active && !(iter->flags & NK_WINDOW_HIDDEN) &&
					NK_INTERSECT(win_bounds.x, win_bounds.y, win_bounds.w, win_bounds.h,
						iter->popup.win->bounds.x, iter->popup.win->bounds.y,
						iter->popup.win->bounds.w, iter->popup.win->bounds.h))
					break;
				iter = iter->next;
			}
		}
		if (iter && !(win->flags & NK_WINDOW_ROM) && (win->flags & NK_WINDOW_BACKGROUND)) {
			win->flags |= (nk_flags)NK_WINDOW_ROM;
			iter->flags &= ~(nk_flags)NK_WINDOW_ROM;
			ctx->active = iter;
			if (!(iter->flags & NK_WINDOW_BACKGROUND)) {
				/* current window is active in that position so transfer to top
				 * at the highest priority in stack */
				nk_remove_window(ctx, iter);
				nk_insert_window(ctx, iter, NK_INSERT_BACK);
			}
		}
		else {
			if (!iter && ctx->end != win) {
				if (!(win->flags & NK_WINDOW_BACKGROUND)) {
					/* current window is active in that position so transfer to top
					 * at the highest priority in stack */
					nk_remove_window(ctx, win);
					nk_insert_window(ctx, win, NK_INSERT_BACK);
				}
				win->flags &= ~(nk_flags)NK_WINDOW_ROM;
				ctx->active = win;
			}
			if (ctx->end != win && !(win->flags & NK_WINDOW_BACKGROUND))
				win->flags |= NK_WINDOW_ROM;
		}
	}
	win->layout = (struct nk_panel*)nk_create_panel(ctx);
	ctx->current = win;
	ret = nk_panel_begin_ex(ctx, title, NK_PANEL_WINDOW);
	win->layout->offset_x = &win->scrollbar.x;
	win->layout->offset_y = &win->scrollbar.y;
	return ret;
}

DWORD
nk_disk_list(struct nk_context* ctx, PHY_DRIVE_INFO* items, DWORD count,
	DWORD selected, int item_height)
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
	
	size.x = nk_widget_width(ctx);

	if (nk_combo_begin_text(ctx, items[selected].text, nk_strlen(items[selected].text), size))
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

void
nk_space_label(struct nk_context* ctx, const char* str)
{
	struct nk_window* win;
	const struct nk_style* style;
	int text_len;
	struct nk_rect bounds;
	struct nk_text text;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout) return;

	win = ctx->current;
	style = &ctx->style;

	if (!nk_widget(&bounds, ctx))
		return;
	bounds.x += bounds.h + style->window.padding.x + style->window.border;
	bounds.w -= bounds.h + style->window.padding.x + style->window.border;


	text_len = nk_strlen(str);
	text.padding.x = style->text.padding.x;
	text.padding.y = style->text.padding.y;
	text.background = style->window.background;
	text.text = style->text.color;
	nk_widget_text(&win->buffer, bounds, str, text_len, &text, NK_TEXT_LEFT, style->font);
}

static int
nk_menu_begin_ex(struct nk_context* ctx, struct nk_window* win,
	const char* id, int is_clicked, struct nk_rect header, struct nk_vec2 size)
{
	int is_open = 0;
	int is_active = 0;
	struct nk_rect body;
	struct nk_window* popup;
	nk_hash hash = nk_murmur_hash(id, (int)nk_strlen(id), NK_PANEL_MENU);

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return 0;

	body.x = header.x;
	body.w = size.x;
	body.y = header.y + header.h;
	body.h = size.y;
	if (body.x + body.w > win->bounds.w)
		body.x -= body.w - nk_widget_width(ctx);
	if (body.x < 0.0f)
		body.x = 0.0f;

	popup = win->popup.win;
	is_open = popup ? nk_true : nk_false;
	is_active = (popup && (win->popup.name == hash) && win->popup.type == NK_PANEL_MENU);
	if ((is_clicked && is_open && !is_active) || (is_open && !is_active) ||
		(!is_open && !is_active && !is_clicked)) return 0;
	if (!nk_nonblock_begin(ctx, NK_WINDOW_NO_SCROLLBAR, body, header, NK_PANEL_MENU))
		return 0;

	win->popup.type = NK_PANEL_MENU;
	win->popup.name = hash;
	return 1;
}

nk_bool
nk_menu_begin_image_ex(struct nk_context* ctx, const char* id, struct nk_image img,
	struct nk_vec2 size)
{
	struct nk_window* win;
	struct nk_rect header;
	const struct nk_input* in;
	int is_clicked = nk_false;
	nk_flags state;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return 0;

	win = ctx->current;
	state = nk_widget(&header, ctx);
	if (!state) return 0;
	in = (state == NK_WIDGET_ROM || state == NK_WIDGET_DISABLED || win->layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
	if (nk_do_button_image(&ctx->last_widget_state, &win->buffer, header,
		img, NK_BUTTON_DEFAULT, &ctx->style.button, in))
		is_clicked = nk_true;
	return nk_menu_begin_ex(ctx, win, id, is_clicked, header, size);
}
