// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define VC_EXTRALEAN
#include <windows.h>

#define CMDLINE_LEN 65535

void
reset_cmdline(void);

void
append_cmdline(LPCWSTR _Printf_format_string_ format, ...);

LPWSTR
get_cmdline(void);
