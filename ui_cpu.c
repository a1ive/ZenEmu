// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

static char static_buf_cpu[32];

void
ui_qemu_cpu_init(void)
{
	nk.ini->qemu_cpu_num = get_ini_num(L"Cpu", L"Smp", 4);
	snprintf(static_buf_cpu, 32, "%d", nk.ini->qemu_cpu_num);
	strcpy_s(nk.ini->qemu_cpu_x86, 32, get_ini_value(L"Cpu", L"X86", L"max"));
	strcpy_s(nk.ini->qemu_cpu_arm, 32, get_ini_value(L"Cpu", L"Arm", L"cortex-a76"));
}

void
ui_qemu_cpu_save(void)
{
	set_ini_num(L"CPU", L"Smp", nk.ini->qemu_cpu_num);
	set_ini_value(L"Cpu", L"X86", L"%s", utf8_to_ucs2(nk.ini->qemu_cpu_x86));
	set_ini_value(L"Cpu", L"Arm", L"%s", utf8_to_ucs2(nk.ini->qemu_cpu_arm));
}

void
ui_qemu_cpu(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_CPU), ZTXT(ZTXT_CPU));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.2f, 0.6f });
	nk_space_label(ctx, ZTXT(ZTXT_SMP));
	if (nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, static_buf_cpu, 32, nk_filter_decimal)
		== NK_EDIT_COMMITED)
		nk.ini->qemu_cpu_num = (int) strtol(static_buf_cpu, NULL, 10);
	nk_spacer(ctx);
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.3f, 0.5f });
	nk_space_label(ctx, ZTXT(ZTXT_NAME));
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->qemu_cpu_x86, 32, NULL);
		break;
	case ZEMU_QEMU_ARCH_AA64:
		nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->qemu_cpu_arm, 32, NULL);
		break;
	default:
		nk_spacer(ctx);
	}
	nk_spacer(ctx);
}
