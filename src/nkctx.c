﻿// SPDX-License-Identifier: GPL-3.0-or-later

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
		nkctx_set_dpi_scaling(wnd);
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
				x <= (LONG)(rect.right - nk.title_height))
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

	nkctx_set_dpi_scaling(nk.wnd);

	for (WORD i = 0; i < sizeof(nk.image) / sizeof(nk.image[0]); i++)
		nk.image[i] = nkctx_load_png(i + IDR_PNG_MIN);

	nkctx_set_style(nk.ctx);

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

	ui_qemu_start(ctx);

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
