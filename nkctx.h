﻿// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "resource.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#include <assert.h>
#define NK_ASSERT(expr) assert(expr)

#include <nuklear.h>
#include <nuklear_gdip.h>

GdipFont*
nk_gdip_load_font(LPCWSTR name, int size);

struct _PHY_DRIVE_INFO;
DWORD
nk_disk_list(struct nk_context* ctx, struct _PHY_DRIVE_INFO* items, DWORD count,
	DWORD selected, int item_height, float width);

void
nk_image_label(struct nk_context* ctx, struct nk_image img, const char* str);

void
nk_space_label(struct nk_context* ctx, const char* str);

#define NK_COLOR_YELLOW     {0xFF, 0xEA, 0x00, 0xff}
#define NK_COLOR_RED        {0xFF, 0x17, 0x44, 0xff}
#define NK_COLOR_GREEN      {0x00, 0xE6, 0x76, 0xff}
#define NK_COLOR_CYAN       {0x03, 0xDA, 0xC6, 0xff}
#define NK_COLOR_BLUE       {0x29, 0x79, 0xFF, 0xff}
#define NK_COLOR_WHITE      {0xFF, 0xFF, 0xFF, 0xff}
#define NK_COLOR_BLACK      {0x00, 0x00, 0x00, 0xff}
#define NK_COLOR_GRAY       {0x1E, 0x1E, 0x1E, 0xff}
#define NK_COLOR_LIGHT      {0xBF, 0xBF, 0xBF, 0xff}
#define NK_COLOR_DARK       {0x2D, 0x2D, 0x2D, 0xFF}

struct _ZEMU_INI_DATA;

#define FONT_NAME_LEN 64

typedef struct _NK_GUI_CTX
{
	HINSTANCE inst;
	HINSTANCE prev;
	LPWSTR cmdline;
	int cmdshow;
	HWND wnd;
	WNDCLASSW wc;
	WCHAR font_name[FONT_NAME_LEN];
	GdipFont* font;
	struct nk_context* ctx;
	unsigned width;
	unsigned height;
	unsigned font_size;
	float title_height;
	float sq;
	nk_bool show_warning;

	struct nk_image image[IDR_PNG_MAX - IDR_PNG_MIN];

	MEMORYSTATUSEX statex;

	struct _ZEMU_INI_DATA* ini;
} NK_GUI_CTX;
extern NK_GUI_CTX nk;

void
nkctx_init(int x, int y, LPCWSTR class_name, LPCWSTR title);

void
nkctx_loop(void);

void
nkctx_update(WPARAM wparam);

_Noreturn void
nkctx_fini(int code);
