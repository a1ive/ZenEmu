// SPDX-License-Identifier: GPL-3.0-or-later

#include "ini.h"
#include "ui.h"
#include "nkctx.h"
#include <stdio.h>
#include <pathcch.h>

#pragma comment(lib, "pathcch.lib")

static CHAR static_u8_buf[CONV_BUFSZ + 1];

LPCSTR ucs2_to_utf8(LPCWSTR src)
{
	size_t i;
	CHAR* p = static_u8_buf;
	ZeroMemory(static_u8_buf, sizeof(static_u8_buf));
	for (i = 0; i < CONV_BUFSZ / 3; i++)
	{
		if (src[i] == 0x0000)
			break;
		else if (src[i] <= 0x007F)
			*p++ = (CHAR)src[i];
		else if (src[i] <= 0x07FF)
		{
			*p++ = (src[i] >> 6) | 0xC0;
			*p++ = (src[i] & 0x3F) | 0x80;
		}
		else if (src[i] >= 0xD800 && src[i] <= 0xDFFF)
		{
			*p++ = 0;
			break;
		}
		else
		{
			*p++ = (src[i] >> 12) | 0xE0;
			*p++ = ((src[i] >> 6) & 0x3F) | 0x80;
			*p++ = (src[i] & 0x3F) | 0x80;
		}
	}
	return static_u8_buf;
}

static WCHAR static_ucs2_buf[CONV_BUFSZ + 1];

LPCWSTR utf8_to_ucs2(LPCSTR src)
{
	size_t i;
	size_t j = 0;
	WCHAR* p = static_ucs2_buf;
	ZeroMemory(static_ucs2_buf, sizeof(static_ucs2_buf));
	for (i = 0; src[i] != '\0' && j < CONV_BUFSZ; )
	{
		if ((src[i] & 0x80) == 0)
		{
			p[j++] = (WCHAR)src[i++];
		}
		else if ((src[i] & 0xE0) == 0xC0)
		{
			p[j++] = (WCHAR)((0x1FU & src[i]) << 6) | (0x3FU & src[i + 1]);
			i += 2;
		}
		else if ((src[i] & 0xF0) == 0xE0)
		{
			p[j++] = (WCHAR)((0x0FU & src[i]) << 12) | ((0x3FU & src[i + 1]) << 6) | (0x3FU & src[i + 2]);
			i += 3;
		}
		else
			break;
	}
	p[j] = L'\0';
	return static_ucs2_buf;
}

const char* human_units[] =
{ "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB" };

const char*
get_human_size(UINT64 size, const char* human_sizes[6], UINT64 base)
{
	UINT64 fsize = size, frac = 0;
	unsigned units = 0;
	static char buf[48];
	const char* umsg;

	while (fsize >= base && units < 5)
	{
		frac = fsize % base;
		fsize = fsize / base;
		units++;
	}

	umsg = human_sizes[units];

	if (units)
	{
		if (frac)
			frac = frac * 100 / base;
		snprintf(buf, sizeof(buf), "%llu.%02llu %s", fsize, frac, umsg);
	}
	else
		snprintf(buf, sizeof(buf), "%llu %s", size, umsg);
	return buf;
}

static ZEMU_INI_DATA static_ini_data;

VOID load_ini(VOID)
{
	nk.ini = &static_ini_data;
	GetModuleFileNameW(NULL, nk.ini->pwd, MAX_PATH);
	PathCchRemoveFileSpec(nk.ini->pwd, MAX_PATH);
	wcscpy_s(nk.ini->ini, MAX_PATH, nk.ini->pwd);
	PathCchAppend(nk.ini->ini, MAX_PATH, L"zemu.ini");

	nk.width = get_ini_num(L"Gui", L"Width", 540);
	nk.height = get_ini_num(L"Gui", L"Height", 600);
	nk.font_size = get_ini_num(L"Gui", L"FontSize", 14);
	GetPrivateProfileStringW(L"Gui", L"Font", L"Courier New", nk.font_name, FONT_NAME_LEN, nk.ini->ini);

	ui_ini_init();
}

VOID save_ini(VOID)
{
	set_ini_num(L"Gui", L"Width", (int)nk.width);
	set_ini_num(L"Gui", L"Height", (int)nk.height);
	set_ini_num(L"Gui", L"FontSize", nk.font_size);
	WritePrivateProfileStringW(L"Gui", L"Font", nk.font_name, nk.ini->ini);
	ui_ini_save();
}

static WCHAR static_ini_value[MAX_PATH];

LPCSTR
get_ini_value(LPCWSTR section, LPCWSTR key, LPCWSTR fallback)
{
	GetPrivateProfileStringW(section, key, fallback, static_ini_value, MAX_PATH, nk.ini->ini);
	return ucs2_to_utf8(static_ini_value);
}

VOID
set_ini_value(LPCWSTR section, LPCWSTR key, LPCSTR value)
{
	WritePrivateProfileStringW(section, key, utf8_to_ucs2(value), nk.ini->ini);
}

int
get_ini_num(LPCWSTR section, LPCWSTR key, int fallback)
{
	WCHAR buf[32];
	swprintf_s(buf, 32, L"%d", fallback);
	GetPrivateProfileStringW(section, key, buf, static_ini_value, MAX_PATH, nk.ini->ini);
	return (int)wcstol(static_ini_value, NULL, 0);
}

VOID
set_ini_num(LPCWSTR section, LPCWSTR key, int value)
{
	WCHAR buf[32];
	swprintf_s(buf, 32, L"%d", value);
	WritePrivateProfileStringW(section, key, buf, nk.ini->ini);
}

static BOOL
is_absolute_path(const WCHAR* path)
{
	if (wcslen(path) >= 3)
	{
		if (iswalpha(path[0]) && path[1] == L':' && path[2] == L'\\')
			return TRUE;
	}
	return FALSE;
}

LPCWSTR
rel_to_abs(LPCSTR path)
{
	static WCHAR abs_path[MAX_PATH];
	LPCWSTR wpath = utf8_to_ucs2(path);

	// If the path is absolute, copy it directly.
	// Otherwise, the path is relative. Combine pwd and path.
	if (is_absolute_path(wpath))
		return wpath;
	swprintf(abs_path, MAX_PATH, L"%s\\%s", nk.ini->pwd, wpath);
	return abs_path;
}

nk_bool check_path_invalid(const char* str)
{
	if (!str)
		return nk_false;
	while (*str)
	{
		if (*str == ',')
			return nk_true;
		if ((unsigned char)*str >= 128)
			return nk_true;
		str++;
	}
	return nk_false;
}
