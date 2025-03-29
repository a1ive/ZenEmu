// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

GdipFont*
nk_gdip_load_font(LPCWSTR name, int size);

nk_bool nk_button_ex(struct nk_context* ctx, struct nk_image img, const char* label);

nk_bool
nk_begin_ex(struct nk_context* ctx, const char* title,
	struct nk_rect bounds, nk_flags flags);

struct _PHY_DRIVE_INFO;
DWORD
nk_disk_list(struct nk_context* ctx, struct _PHY_DRIVE_INFO* items, DWORD count,
	DWORD selected, int item_height);

void
nk_image_label(struct nk_context* ctx, struct nk_image img, const char* str);

void
nk_space_label(struct nk_context* ctx, const char* str);

nk_bool
nk_menu_begin_image_ex(struct nk_context* ctx, const char* id, struct nk_image img,
	struct nk_vec2 size);

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
	nk_bool dpi_scaling;
	double dpi_factor;
	struct nk_color color[NK_COLOR_COUNT];
	struct nk_context* ctx;
	unsigned width;
	unsigned height;
	unsigned font_size;
	float title_height;
	float sq;
	nk_bool show_warning;
	struct nk_style_button button_style;
	UINT64 tilck;

	struct nk_image image[IDR_PNG_MAX - IDR_PNG_MIN];

	MEMORYSTATUSEX statex;

	struct _ZEMU_INI_DATA* ini;
} NK_GUI_CTX;
extern NK_GUI_CTX nk;

