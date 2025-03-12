// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "resource.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#include <assert.h>
#define NK_ASSERT(expr) assert(expr)

#include <nuklear.h>
#include <nuklear_gdip.h>

#include "nkctx_priv.h"

void
nkctx_init(int x, int y, LPCWSTR class_name, LPCWSTR title);

void
nkctx_loop(void);

void
nkctx_update(WPARAM wparam);

_Noreturn void
nkctx_fini(int code);
