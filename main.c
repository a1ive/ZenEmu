// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"

int APIENTRY
wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	OleInitialize(NULL);
	nkctx_init(hInstance, 200, 200, 800, 800, L"NkWindowClass", L"ZenEMU", L"Courier New", 18);
	nkctx_loop();
	nkctx_fini(0);
	return 0;
}
