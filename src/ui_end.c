// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

#include "lodepng.h"

#pragma pack(1)
typedef struct _RGBA_PIXEL
{
	UINT8 b;
	UINT8 g;
	UINT8 r;
	UINT8 a;
} RGBA_PIXEL;
#pragma pack()

static BOOL
bmp_to_png(HANDLE hf, HDC hdc, HBITMAP hbitmap, UINT w, UINT h)
{
	BOOL rc = FALSE;
	DWORD dwpng = 0;
	size_t i, szimg, szpng = 0;
	RGBA_PIXEL* raw = NULL;
	UCHAR* png = NULL;
	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = w;
	bmi.biHeight = -((LONG)h);
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;
	szimg = ((UINT64)w) * h;
	raw = calloc(szimg, sizeof(RGBA_PIXEL));
	if (!raw)
		return FALSE;
	GetDIBits(hdc, hbitmap, 0, h, raw, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
	for (i = 0; i < szimg; i++)
	{
		UINT8 tmp = raw[i].b;
		raw[i].b = raw[i].r;
		raw[i].r = tmp;
		raw[i].a = 0xFF;
	}
	if (lodepng_encode32(&png, &szpng, (const UINT8*)raw, w, h) != 0)
	{
		free(raw);
		return FALSE;
	}
	free(raw);
	dwpng = (DWORD)szpng;
	rc = WriteFile(hf, png, dwpng, &dwpng, NULL);
	free(png);
	return rc;
}

typedef struct _FIND_WINDOWS_DATA
{
	DWORD pid;
	HWND hwnd;
} FIND_WINDOWS_DATA;

static BOOL CALLBACK
enum_windows_proc(HWND hwnd, LPARAM lParam)
{
	FIND_WINDOWS_DATA* data = (FIND_WINDOWS_DATA*)lParam;
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == data->pid && IsWindowVisible(hwnd))
	{
		data->hwnd = hwnd;
		return FALSE;
	}
	return TRUE;
}

static HWND find_window_by_pid(DWORD pid)
{
	FIND_WINDOWS_DATA data;
	data.pid = pid;
	data.hwnd = NULL;
	EnumWindows(enum_windows_proc, (LPARAM)&data);
	return data.hwnd;
}

static BOOL
save_screenshot(ZEMU_SCREEN_SAVE save_to, HDC hdc, HBITMAP hbitmap, UINT w, UINT h)
{
	SetForegroundWindow(nk.wnd);
	switch (save_to)
	{
	case ZEMU_SCREEN_TO_CLIP:
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hbitmap);
		CloseClipboard();
		return TRUE;
	case ZEMU_SCREEN_TO_FILE:
	{
		HANDLE hf = INVALID_HANDLE_VALUE;
		WCHAR path[MAX_PATH];
		SYSTEMTIME st;
		GetSystemTime(&st);
		swprintf(path, MAX_PATH, L"zemu-%u%u%u%u%u%u.png",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		if (!ui_save_file(path, MAX_PATH, FILTER_PNG, L"png"))
			return FALSE;
		hf = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (!hf || hf == INVALID_HANDLE_VALUE)
			return FALSE;
		BOOL rc = bmp_to_png(hf, hdc, hbitmap, w, h);
		CloseHandle(hf);
		return rc;
	}
	}
	return FALSE;
}

static BOOL get_screenshot(HWND hwnd)
{
	int x = 0, y = 0, w = 0, h = 0;
	HDC screen = NULL;
	HDC hdc = NULL;
	HBITMAP hbitmap = NULL;
	BOOL rc = FALSE;
	RECT rec = { 0 };

	if (!hwnd)
	{
		MessageBoxW(NULL, L"Could not find QEMU window", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	// restore window if its minimized.
	if (IsIconic(hwnd))
		ShowWindow(hwnd, SW_RESTORE);
	SetForegroundWindow(hwnd);
	if (!GetWindowRect(hwnd, &rec))
	{
		MessageBoxW(NULL, L"Could not get QEMU window size", L"ERROR", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	x = rec.left;
	y = rec.top;
	w = rec.right - rec.left;
	h = rec.bottom - rec.top;

	screen = GetDC(NULL);
	hdc = CreateCompatibleDC(screen);
	if (!hdc)
	{
		MessageBoxW(NULL, L"Could not get DC", L"ERROR", MB_OK | MB_ICONERROR);
		goto out;
	}
	hbitmap = CreateCompatibleBitmap(screen, w, h);
	if (!hbitmap)
	{
		MessageBoxW(NULL, L"Could not create bitmap", L"ERROR", MB_OK | MB_ICONERROR);
		goto out;
	}
	SelectObject(hdc, hbitmap);
	rc = BitBlt(hdc, 0, 0, w, h, screen, x, y, SRCCOPY);
	if (rc != TRUE)
	{
		MessageBoxW(NULL, L"BitBlt failed", L"ERROR", MB_OK | MB_ICONERROR);
		goto out;
	}

	rc = save_screenshot(nk.ini->qemu_screenshot, hdc, hbitmap, w, h);

out:
	if (hdc)
		DeleteDC(hdc);
	if (hbitmap)
		DeleteObject(hbitmap);
	ReleaseDC(NULL, screen);
	if (rc)
		ui_popup_msg(ZTXT(ZTXT_MSG_PNG_OK), IDR_PNG_INFO);
	else
		ui_popup_msg(ZTXT(ZTXT_MSG_PNG_ERR), IDR_PNG_WARN);
	return rc;
}

void
ui_qemu_end(struct nk_context* ctx)
{
	nk_bool is_running = ui_is_qemu_running();
	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.3f, 0.2f, 0.2f, 0.3f });
	nk_image_label(ctx, GET_PNG(IDR_PNG_VM), ZTXT(ZTXT_SAVE_TO));
	UI_OPTION(ZTXT(ZTXT_FILE), nk.ini->qemu_screenshot, ZEMU_SCREEN_TO_FILE);
	UI_OPTION(ZTXT(ZTXT_CLIPBOARD), nk.ini->qemu_screenshot, ZEMU_SCREEN_TO_CLIP);
	if (!is_running)
		nk_widget_disable_begin(ctx);
	if (nk_button_ex(ctx, GET_PNG(IDR_PNG_CAMERA), ZTXT(ZTXT_SCREENSHOT)))
		get_screenshot(find_window_by_pid(nk.ini->output_pid));
	if (!is_running)
		nk_widget_disable_end(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 2, (float[2]) { 1.0f - nk.sq, nk.sq });
	nk_image_label(ctx, GET_PNG(IDR_PNG_INFO), ZTXT(ZTXT_LOGS));
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_REFRESH)))
		ui_reset_log();
	nk_layout_row_dynamic(ctx, 300, 1);
	nk_label_wrap(ctx, nk.ini->output);
}