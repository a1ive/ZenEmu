// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

static char static_buf_cpu[32];
static char static_buf_core[32];

void
ui_qemu_cpu_init(void)
{
	nk.ini->qemu_cpu_num = get_ini_num(L"CPU", L"Smp", 4);
	snprintf(static_buf_cpu, 32, "%d", nk.ini->qemu_cpu_num);
	//nk.ini->qemu_cpu_core = get_ini_num(L"CPU", L"Cores", 4);
	//snprintf(static_buf_core, 32, "%d", nk.ini->qemu_cpu_core);
}

void
ui_qemu_cpu_save(void)
{
	set_ini_num(L"CPU", L"Smp", nk.ini->qemu_cpu_num);
}

void
ui_qemu_cpu(struct nk_context* ctx)
{
	nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { 0.2f, 0.2f, 0.6f });
	nk_label(ctx, "CPU", NK_TEXT_LEFT);
	if (nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, static_buf_cpu, 32, nk_filter_decimal)
		== NK_EDIT_COMMITED)
		nk.ini->qemu_cpu_num = (int) strtol(static_buf_cpu, NULL, 10);
	//nk_label(ctx, "cpus", NK_TEXT_LEFT);
	//if (nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, static_buf_core, 32, nk_filter_decimal)
	//	== NK_EDIT_COMMITED)
	//	nk.ini->qemu_cpu_core = (int)strtol(static_buf_core, NULL, 10);
	//nk_label(ctx, "cores", NK_TEXT_LEFT);
	nk_spacer(ctx);
}
