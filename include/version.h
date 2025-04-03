// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

#define NKGUI_MAJOR_VERSION 0
#define NKGUI_MINOR_VERSION 1
#define NKGUI_MICRO_VERSION 0
#define NKGUI_BUILD_VERSION 0

#define NKGUI_VERSION      NKGUI_MAJOR_VERSION,NKGUI_MINOR_VERSION,NKGUI_MICRO_VERSION,NKGUI_BUILD_VERSION
#define NKGUI_VERSION_STR  QUOTE(NKGUI_MAJOR_VERSION.NKGUI_MINOR_VERSION.NKGUI_MICRO_VERSION.NKGUI_BUILD_VERSION)

#define NKGUI_COMPANY      "A1ive"
#define NKGUI_COPYRIGHT    "Copyright (c) 2025 A1ive"
#define NKGUI_FILEDESC     "ZenEMU"

#define NKGUI_NAME         "ZenEMU"
