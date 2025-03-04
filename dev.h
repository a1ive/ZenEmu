// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VC_EXTRALEAN
#include <windows.h>

CHAR**
get_disk_list(BOOL is_cd, size_t* count);

VOID
free_disk_list(CHAR** disk_list, size_t count);
