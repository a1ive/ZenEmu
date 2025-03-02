// SPDX-License-Identifier: Unlicense

#include "nkctx.h"

int APIENTRY
wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	nkctx_init(hInstance, 200, 200, 600, 800, L"NkWindowClass", L"ZenEMU", L"Courier New", 16);
	nkctx_loop();
	nkctx_fini(0);
	return 0;
}
