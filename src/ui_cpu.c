// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "gettext.h"

static const char* edit_cpu_x86[] =
{
	"486", "Broadwell", "Conroe", "Cooperlake",
	"Dhyana", "EPYC", "Haswell", "Ivybridge",
	"Skylake-Client", "Skylake-Server", "Snowridge",
	"athlon", "core2duo", "max",
};

static const char* edit_cpu_arm[] =
{
	"cortex-a53", "cortex-a57", "cortex-a72", "cortex-a76", "max",
};

static const char* edit_machine_x86[] =
{
	"pc", "q35", "isapc",
};

static const char* edit_machine_arm[] =
{
	"virt",
};

void
ui_qemu_cpu(struct nk_context* ctx)
{
	nk_layout_row_dynamic(ctx, 0, 1);
	nk_image_label(ctx, GET_PNG(IDR_PNG_CPU), ZTXT(ZTXT_CPU));
	nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { 0.2f, 0.2f, 0.1f, 0.25f, 0.25f });
	nk_space_label(ctx, ZTXT(ZTXT_SMP));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->smp, OPT_SZ, nk_filter_decimal);
	nk_spacer(ctx);
	nk_checkbox_label(ctx, ZTXT(ZTXT_KERNEL_IRQCHIP), &nk.ini->cur->irqchip);
	if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		nk_checkbox_label(ctx, ZTXT(ZTXT_VIRT), &nk.ini->cur->virt);
	else if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_X64)
		nk_checkbox_label(ctx, ZTXT(ZTXT_HYPER_V), &nk.ini->cur->whpx);
	else
		nk_spacer(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 6,
		(float[6]) { 0.2f, 0.3f - nk.sq, nk.sq, 0.2f, 0.3f - nk.sq, nk.sq });
	nk_space_label(ctx, ZTXT(ZTXT_MODEL));
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->model, OPT_SZ, nk_filter_ascii);
	if (nk_menu_begin_image_ex(ctx, "#EDIT_CPU", GET_PNG(IDR_PNG_DOWN), nk_vec2(200, 600)))
	{
		const char** list = edit_cpu_x86;
		size_t count = ARRAYSIZE(edit_cpu_x86);
		if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		{
			list = edit_cpu_arm;
			count = ARRAYSIZE(edit_cpu_arm);
		}
		nk_layout_row_dynamic(ctx, 0, 1);
		for (size_t i = 0; i < count; i++)
		{
			if (nk_menu_item_label(ctx, list[i], NK_TEXT_LEFT))
				strcpy_s(nk.ini->cur->model, OPT_SZ, list[i]);
		}
		nk_menu_end(ctx);
	}
	nk_label(ctx, ZTXT(ZTXT_MACHINE), NK_TEXT_RIGHT);
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, nk.ini->cur->machine, OPT_SZ, nk_filter_ascii);
	if (nk_menu_begin_image_ex(ctx, "#EDIT_MACHINE", GET_PNG(IDR_PNG_DOWN), nk_vec2(200, 300)))
	{
		const char** list = edit_machine_x86;
		size_t count = ARRAYSIZE(edit_machine_x86);
		if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64)
		{
			list = edit_machine_arm;
			count = ARRAYSIZE(edit_machine_arm);
		}
		nk_layout_row_dynamic(ctx, 0, 1);
		for (size_t i = 0; i < count; i++)
		{
			if (nk_menu_item_label(ctx, list[i], NK_TEXT_LEFT))
				strcpy_s(nk.ini->cur->machine, OPT_SZ, list[i]);
		}
		nk_menu_end(ctx);
	}
}
