// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

static char static_buf_mem[32];

void
ui_qemu_mem_init(void)
{
	nk.ini->qemu_mem_mb = get_ini_num(L"Memory", L"Size", 4096);
	snprintf(static_buf_mem, 32, "%d", nk.ini->qemu_mem_mb);
}

void
ui_qemu_mem_save(void)
{
	set_ini_num(L"Memory", L"Size", nk.ini->qemu_mem_mb);
}

static const char*
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

static const char* static_units[] =
{ "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB" };

void
ui_qemu_mem(struct nk_context* ctx)
{
	char buf[48];
	MEMORYSTATUSEX statex = { .dwLength = sizeof(MEMORYSTATUSEX) };
	GlobalMemoryStatusEx(&statex);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.2f, 0.3f, 0.1f, 0.4f });
	nk_label(ctx, "Memory", NK_TEXT_LEFT);
	if (nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, static_buf_mem, 32, nk_filter_decimal)
		== NK_EDIT_COMMITED)
		nk.ini->qemu_mem_mb = (int)strtol(static_buf_mem, NULL, 10);
	nk_label(ctx, "MB", NK_TEXT_LEFT);
	strcpy_s(buf, 48, get_human_size(statex.ullAvailPhys, static_units, 1024));
	nk_labelf(ctx, NK_TEXT_LEFT, "(%s / %s)",
		buf, get_human_size(statex.ullTotalPhys, static_units, 1024));
}
