// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VC_EXTRALEAN
#include <windows.h>

typedef struct _PHY_DRIVE_INFO
{
	DWORD index;
	UINT64 size;
	LPCSTR prefix;
	CHAR hw[MAX_PATH];
	LPCSTR bus;
	UINT32 mnt;
	CHAR text[MAX_PATH];
}PHY_DRIVE_INFO;

DWORD
get_disk_list(BOOL is_cd, PHY_DRIVE_INFO** drive_list);
