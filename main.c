// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"

int APIENTRY
wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	OleInitialize(NULL);
	ZeroMemory(&nk, sizeof(nk));
	nk.inst = hInstance;
	nk.prev = hPrevInstance;
	nk.cmdline = lpCmdLine;
	nk.cmdshow = nCmdShow;
	load_ini();
	nkctx_init(100, 100, L"NkWindowClass", L"ZenEMU");
	nkctx_loop();
	nkctx_fini(0);
	return 0;
}
