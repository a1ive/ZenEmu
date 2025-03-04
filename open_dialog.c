// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ui.h"
#include "ini.h"

#include <commdlg.h>
#include <shlobj.h>

void
ui_open_file(CHAR* path, size_t len, LPCWSTR filter)
{
	WCHAR buf[MAX_PATH] = L"";
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nk.wnd;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	if (GetOpenFileNameW(&ofn) == TRUE)
		strcpy_s(path, len, ucs2_to_utf8(buf));
}

static int CALLBACK
browse_callback_fn(HWND wnd, UINT msg, LPARAM lparam, LPARAM data)
{
	int rc = 0;
	switch (msg)
	{
	case BFFM_INITIALIZED:
		SendMessageW(wnd, BFFM_SETSELECTION, TRUE, (LPARAM)nk.ini->pwd);
		break;
	case BFFM_VALIDATEFAILED:
		MessageBoxW(wnd, L"Invalid Path", L"ERROR", MB_OK);
		rc = 1;
		break;
	}
	return rc;
}

void
ui_open_dir(CHAR* path, size_t len)
{
	WCHAR dir[MAX_PATH];
	BROWSEINFOW bi =
	{
		.hwndOwner = nk.wnd,
		.pidlRoot = NULL,
		.pszDisplayName = dir,
		.lpszTitle = L"Select Folder",
		.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_USENEWUI | BIF_VALIDATE,
		.lpfn = browse_callback_fn,
	};
	LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
	if (pidl != NULL)
	{
		SHGetPathFromIDListW(pidl, dir);
		strcpy_s(path, len, ucs2_to_utf8(dir));
		CoTaskMemFree(pidl);
	}
}
