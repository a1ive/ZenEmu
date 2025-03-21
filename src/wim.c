// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ui.h"
#include "ini.h"

#pragma pack(1)

struct wim_resource_header
{
	uint64_t zlen_flags;
	uint64_t offset;
	uint64_t len;
};

enum wim_header_flags
{
	WIM_HDR_COMPRESSED = 0x00000002,
	WIM_HDR_XPRESS = 0x00020000,
	WIM_HDR_LZX = 0x00040000,
	WIM_HDR_LZMS = 0x00080000,
};

struct wim_header
{
	uint8_t signature[8]; // "MSWIM\0\0"
	uint32_t header_len;
	uint32_t version;
	uint32_t flags;
	uint32_t chunk_len;
	uint8_t guid[16];
	uint16_t part;
	uint16_t parts;
	uint32_t images;
	struct wim_resource_header lookup;
	struct wim_resource_header xml;
	struct wim_resource_header boot;
	uint32_t boot_index;
	struct wim_resource_header integrity;
	uint8_t reserved[60];
};

#pragma pack()

#define INITRD_MAX_SIZE (2ULL * 1024 * 1024 * 1024)

nk_bool
ui_check_wim_header(uint32_t* num_images, uint32_t* boot)
{
	*num_images = 0;
	*boot = 0;
	if (!nk.ini->boot_wim[0])
		return nk_true;
	struct wim_header hdr = { 0 };
	uint64_t size = get_file_header(rel_to_abs(nk.ini->boot_wim),
		&hdr, sizeof(struct wim_header));
	if (size == 0 || size >= INITRD_MAX_SIZE)
		return nk_false;
	if (memcmp(hdr.signature, "MSWIM\0\0", 8) != 0)
		return nk_false;
	if (hdr.flags & WIM_HDR_LZMS)
		return nk_false;
	*num_images = hdr.images;
	*boot = hdr.boot_index;
	if (hdr.boot_index == 0)
		return nk_false;
	return nk_true;
}
