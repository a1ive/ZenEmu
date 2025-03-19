// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "dev.h"
#include "gettext.h"
#include "version.h"

#include <windowsx.h>
#include <dbt.h>

NK_GUI_CTX nk;

#define REGION_MASK_LEFT    (1 << 0)
#define REGION_MASK_RIGHT   (1 << 1)
#define REGION_MASK_TOP     (1 << 2)
#define REGION_MASK_BOTTOM  (1 << 3)

static LRESULT CALLBACK
nkctx_window_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		nkctx_update(wparam);
		break;
	case WM_DPICHANGED:
		break;
	case WM_NCHITTEST:
		{
			RECT rect = { 0 };
			LONG result = 0;
			LONG x = GET_X_LPARAM(lparam);
			LONG y = GET_Y_LPARAM(lparam);
			LONG w = GetSystemMetricsForDpi(SM_CXFRAME, USER_DEFAULT_SCREEN_DPI)
				+ GetSystemMetricsForDpi(SM_CXPADDEDBORDER, USER_DEFAULT_SCREEN_DPI);
			LONG h = GetSystemMetricsForDpi(SM_CYFRAME, USER_DEFAULT_SCREEN_DPI)
				+ GetSystemMetricsForDpi(SM_CXPADDEDBORDER, USER_DEFAULT_SCREEN_DPI);
			GetWindowRect(wnd, &rect);
			result = REGION_MASK_LEFT * (x < (rect.left + w)) |
				REGION_MASK_RIGHT * (x >= (rect.right - w)) |
				REGION_MASK_TOP * (y < (rect.top + h)) |
				REGION_MASK_BOTTOM * (y >= (rect.bottom - h));
			switch (result)
			{
			case REGION_MASK_LEFT: return HTLEFT;
			case REGION_MASK_RIGHT: return HTRIGHT;
			case REGION_MASK_TOP: return HTTOP;
			case REGION_MASK_BOTTOM: return HTBOTTOM;
			case REGION_MASK_TOP | REGION_MASK_LEFT: return HTTOPLEFT;
			case REGION_MASK_TOP | REGION_MASK_RIGHT: return HTTOPRIGHT;
			case REGION_MASK_BOTTOM | REGION_MASK_LEFT: return HTBOTTOMLEFT;
			case REGION_MASK_BOTTOM | REGION_MASK_RIGHT: return HTBOTTOMRIGHT;
			}
			if (y <= (LONG)(rect.top + nk.title_height) &&
				x <= (LONG)(rect.right - 3 * nk.title_height))
				return HTCAPTION;
		}
		break;
	case WM_SIZE:
		nk.height = HIWORD(lparam);
		nk.width = LOWORD(lparam);
		break;
	}
	if (nk_gdip_handle_event(wnd, msg, wparam, lparam))
		return 0;
	return DefWindowProcW(wnd, msg, wparam, lparam);
}

static void
set_style(struct nk_context* ctx)
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

	memcpy(&nk.button_style, &ctx->style.button, sizeof(struct nk_style_button));

	nk.title_height = nk.font_size
		+ ctx->style.window.header.padding.y
		+ ctx->style.window.header.label_padding.y;
}

static struct nk_image
load_png(WORD id)
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
nkctx_init(int x, int y, LPCWSTR class_name, LPCWSTR title)
{
	DWORD style = WS_POPUP | WS_VISIBLE;
	DWORD exstyle = WS_EX_LAYERED;

	nk.wc.style = CS_DBLCLKS;
	nk.wc.lpfnWndProc = nkctx_window_proc;
	nk.wc.hInstance = nk.inst;
	nk.wc.hIcon = LoadIconW(nk.inst, MAKEINTRESOURCEW(IDI_MAIN_ICON));
	nk.wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	nk.wc.lpszClassName = class_name;
	RegisterClassW(&nk.wc);

	nk.wnd = CreateWindowExW(exstyle, class_name, title, style,
		x, y, (int)nk.width, (int)nk.height,
		NULL, NULL, nk.inst, NULL);

	SetLayeredWindowAttributes(nk.wnd, 0, 255, LWA_ALPHA);

	nk.ctx = nk_gdip_init(nk.wnd, nk.width, nk.height);

	nk.font = nk_gdip_load_font(nk.font_name, nk.font_size);
	nk_gdip_set_font(nk.font);

	for (WORD i = 0; i < sizeof(nk.image) / sizeof(nk.image[0]); i++)
		nk.image[i] = load_png(i + IDR_PNG_MIN);

	set_style(nk.ctx);

	SetTimer(nk.wnd, IDT_TIMER_1S, 1000, (TIMERPROC)NULL);
	nkctx_update(IDT_TIMER_1S);
}

static void
nkctx_main_window(struct nk_context* ctx, float width, float height)
{
	if (!nk_begin_ex(ctx, "ZenEMU", nk_rect(0.0f, 0.0f, width, height),
		NK_WINDOW_BACKGROUND | NK_WINDOW_CLOSABLE | NK_WINDOW_TITLE))
	{
		nk_end(ctx);
		nkctx_fini(0);
	}
	nk_layout_row_dynamic(ctx, 0, 1);
	struct nk_rect rect = nk_layout_widget_bounds(ctx);
	nk.sq = rect.h / rect.w;
	nk_label(ctx, "v" NKGUI_VERSION_STR " " NKGUI_COPYRIGHT, NK_TEXT_CENTERED);
	if (nk.show_warning == nk_true)
	{
		nk.show_warning = nk_false;
		nk_layout_row_dynamic(ctx, 0, 1);
		nk_image_label(ctx, GET_PNG(IDR_PNG_WARN), ZTXT(ZTXT_WARN_NON_ASCII));
	}
	nk.show_warning = check_path_invalid(ucs2_to_utf8(nk.ini->pwd));

	ui_qemu_dir(ctx);
	ui_qemu_cpu(ctx);
	ui_qemu_mem(ctx);
	ui_qemu_fw(ctx);
	ui_qemu_dev(ctx);
	ui_qemu_boot(ctx);
	ui_qemu_hdb(ctx);

	ui_qemu_end(ctx);

	nk_end(ctx);
}

void
nkctx_loop(void)
{
	int running = 1;
	int needs_refresh = 1;

	while (running)
	{
		/* Input */
		MSG msg;
		nk_input_begin(nk.ctx);
		if (needs_refresh == 0)
		{
			if (GetMessageW(&msg, NULL, 0, 0) <= 0)
				running = 0;
			else
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			needs_refresh = 1;
		}
		else
			needs_refresh = 0;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				running = 0;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			needs_refresh = 1;
		}
		nk_input_end(nk.ctx);

		/* GUI */
		//set_style(nk.ctx);
		nkctx_main_window(nk.ctx, (float)nk.width, (float)nk.height);

		ui_popup_window(nk.ctx, (float)nk.width, (float)nk.height);

		/* Draw */
		nk_gdip_render(NK_ANTI_ALIASING_ON, (struct nk_color)NK_COLOR_BLACK);
	}
}

void
nkctx_update(WPARAM wparam)
{
	switch (wparam)
	{
		case IDT_TIMER_1S:
			nk.tilck++;
			nk.statex.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&nk.statex);
			break;
	}
}

_Noreturn void
nkctx_fini(int code)
{
	KillTimer(nk.wnd, IDT_TIMER_1S);
	nk_gdipfont_del(nk.font);
	for (WORD i = 0; i < sizeof(nk.image) / sizeof(nk.image[0]); i++)
		nk_gdip_image_free(nk.image[i]);
	nk_gdip_shutdown();
	UnregisterClassW(nk.wc.lpszClassName, nk.wc.hInstance);
	OleUninitialize();
	exit(code);
}
