// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "dev.h"
#include "gettext.h"

static struct nk_image imgs[ZEMU_DEV_MAX];
static const char* names[ZEMU_DEV_MAX];
static LPCWSTR filters[ZEMU_DEV_MAX] = { FILTER_VHD, FILTER_ISO };

void
ui_qemu_hdb(struct nk_context* ctx)
{
	imgs[ZEMU_DEV_HD] = GET_PNG(IDR_PNG_DISK),
	imgs[ZEMU_DEV_CD] = GET_PNG(IDR_PNG_CD);
	names[ZEMU_DEV_HD] = ZTXT(ZTXT_HARD_DISK);
	names[ZEMU_DEV_CD] = ZTXT(ZTXT_CD_ROM);
	if (nk.ini->add_dev_count > MAX_ADD_DEV)
		nk.ini->add_dev_count = MAX_ADD_DEV;
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 1.0f - 2 * nk.sq, nk.sq, nk.sq });
	nk_image_label(ctx, GET_PNG(IDR_PNG_STORAGE), ZTXT(ZTXT_ADDITIONAL));
	if (nk.ini->add_dev_count == MAX_ADD_DEV)
		nk_widget_disable_begin(ctx);
	if (nk_menu_begin_image_ex(ctx, "#ADD_DEV", GET_PNG(IDR_PNG_ADD), nk_vec2(200,200)))
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		if (nk_menu_item_image_label(ctx, GET_PNG(IDR_PNG_DISK), ZTXT(ZTXT_HARD_DISK), NK_TEXT_RIGHT))
		{
			nk.ini->add_dev[nk.ini->add_dev_count].type = ZEMU_DEV_HD;
			nk.ini->add_dev[nk.ini->add_dev_count].is_active = nk_true;
			nk.ini->add_dev_count++;
		}
		if (nk_menu_item_image_label(ctx, GET_PNG(IDR_PNG_CD), ZTXT(ZTXT_CD_ROM), NK_TEXT_RIGHT))
		{
			nk.ini->add_dev[nk.ini->add_dev_count].type = ZEMU_DEV_CD;
			nk.ini->add_dev[nk.ini->add_dev_count].is_active = nk_true;
			nk.ini->add_dev_count++;
		}
		nk_menu_end(ctx);
	}
	if (nk.ini->add_dev_count == MAX_ADD_DEV)
		nk_widget_disable_end(ctx);
	if (nk.ini->add_dev_count == 0)
		nk_widget_disable_begin(ctx);
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_REMOVE)))
		nk.ini->add_dev_count--;
	if (nk.ini->add_dev_count == 0)
		nk_widget_disable_end(ctx);

	for (size_t i = 0; i < nk.ini->add_dev_count; i++)
	{
		ZEMU_ADD_DEV* dev = &nk.ini->add_dev[i];
		nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { nk.sq, 0.2f - nk.sq, 0.2f, 0.2f, 0.4f });
		ui_dev_button(ctx, imgs[dev->type], names[dev->type], &dev->is_active);

		if (!dev->is_active)
			nk_widget_disable_begin(ctx);

		UI_OPTION(ZTXT(ZTXT_FILE), dev->is_device, nk_false);
		UI_OPTION(ZTXT(ZTXT_DEVICE), dev->is_device, nk_true);
		nk_checkbox_label(ctx, ZTXT(ZTXT_SNAPSHOT), &dev->attr.snapshot);
		
		nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.2f, 0.8f - nk.sq, nk.sq });
		if (dev->is_device)
		{
			BOOL is_cd = (dev->type == ZEMU_DEV_CD) ? TRUE : FALSE;
			if (nk.ini->d_info[dev->type] == NULL)
				nk.ini->d_count[dev->type] = get_disk_list(is_cd, &nk.ini->d_info[dev->type]);

			if (dev->id >= nk.ini->d_count[dev->type])
				dev->id = 0;

			nk_space_label(ctx, ZTXT(ZTXT_DEVICE));

			if (nk.ini->d_count[dev->type] == 0)
				nk_label(ctx, ZTXT(ZTXT_NO_DEVICE), NK_TEXT_CENTERED);
			else
				dev->id = nk_disk_list(ctx, nk.ini->d_info[dev->type], nk.ini->d_count[dev->type],
					dev->id, (int)nk.title_height);
			if (nk_button_image(ctx, GET_PNG(IDR_PNG_REFRESH)))
			{
				free(nk.ini->d_info[dev->type]);
				nk.ini->d_info[dev->type] = NULL;
				nk.ini->d_count[dev->type] = 0;
			}
		}
		else
		{
			nk_space_label(ctx, ZTXT(ZTXT_FILE));
			nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, dev->path, MAX_PATH, NULL);
			if (nk_button_image(ctx, GET_PNG(IDR_PNG_DIR)))
				ui_open_file(dev->path, MAX_PATH, filters[dev->type]);

			if (nk.show_warning == nk_false)
				nk.show_warning = check_path_invalid(dev->path);
		}

		if (!dev->is_active)
			nk_widget_disable_end(ctx);
	}
}
