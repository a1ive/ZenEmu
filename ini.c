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

	ui_qemu_dir_init();
	ui_qemu_cpu_init();
	ui_qemu_mem_init();
	ui_qemu_fw_init();
	ui_qemu_boot_init();
	ui_qemu_obj_init();
}

VOID save_ini(VOID)
{
	ui_qemu_dir_save();
	ui_qemu_cpu_save();
	ui_qemu_mem_save();
	ui_qemu_fw_save();
	ui_qemu_boot_save();
	ui_qemu_obj_save();
}

static WCHAR static_ini_value[MAX_PATH];

LPCSTR
get_ini_value(LPCWSTR section, LPCWSTR key, LPCWSTR fallback)
{
	GetPrivateProfileStringW(section, key, fallback, static_ini_value, MAX_PATH, nk.ini->ini);
	return ucs2_to_utf8(static_ini_value);
}

VOID
set_ini_value(LPCWSTR section, LPCWSTR key, LPCWSTR _Printf_format_string_ format, ...)
{
	int sz;
	WCHAR* buf = NULL;
	va_list ap;
	va_start(ap, format);
	sz = _vscwprintf(format, ap) + 1;
	if (sz <= 0)
	{
		va_end(ap);
		goto fail;
	}
	buf = calloc(sizeof(WCHAR), sz);
	if (!buf)
	{
		va_end(ap);
		goto fail;
	}
	_vsnwprintf_s(buf, sz, _TRUNCATE, format, ap);
	va_end(ap);
	WritePrivateProfileStringW(section, key, buf, nk.ini->ini);
	free(buf);
	return;
fail:
	MessageBoxW(NULL, L"Set ini value failed", L"Error", MB_ICONERROR);
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
