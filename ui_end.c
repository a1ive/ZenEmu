// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "cmdline.h"
#include "gettext.h"

#include <stdbool.h>
#include <shellapi.h>

static bool
check_valid(void)
{
	switch (nk.ini->cur->boot)
	{
	case ZEMU_BOOT_VHD:
		return nk.ini->boot_vhd[0] ? true : false;
	case ZEMU_BOOT_ISO:
		return nk.ini->boot_iso[0] ? true : false;
	case ZEMU_BOOT_PD:
		return nk.ini->d_count[ZEMU_DEV_HD] ? true : false;
	case ZEMU_BOOT_CD:
		return nk.ini->d_count[ZEMU_DEV_CD] ? true : false;
	case ZEMU_BOOT_VFD:
		return nk.ini->boot_vfd[0] ? true : false;
	case ZEMU_BOOT_PXE:
		return (nk.ini->net_file[0] && nk.ini->net_tftp[0]) ? true : false;
	case ZEMU_BOOT_LINUX:
		return nk.ini->boot_linux[0] ? true : false;
	case ZEMU_BOOT_WIM:
		return nk.ini->boot_wim[0] ? true : false;
	case ZEMU_BOOT_DIR:
		return nk.ini->boot_dir[0] ? true : false;
	}
	return false;
}

static inline void
reset_log(void)
{
	ZeroMemory(nk.ini->output, OUTBUF_SZ);
	nk.ini->output_offset = 0;
}

DWORD WINAPI
read_pipe_thread(LPVOID lparam)
{
	HANDLE pp = (HANDLE)lparam;
	char buffer[OUTBUF_SZ];
	DWORD read_size;

	while (ReadFile(pp, buffer, OUTBUF_SZ - 1, &read_size, NULL) && read_size > 0)
	{
		buffer[read_size] = '\0';

		if (nk.ini->output_offset + read_size >= OUTBUF_SZ)
			reset_log();

		memcpy(nk.ini->output + nk.ini->output_offset, buffer, read_size);
		nk.ini->output_offset += read_size;
		if (nk.ini->output_offset < OUTBUF_SZ)
			nk.ini->output[nk.ini->output_offset] = '\0';
		else
			nk.ini->output[OUTBUF_SZ - 1] = '\0';
	}
	return 0;
}

static nk_bool is_qemu_running()
{
	DWORD exitCode = 0;
	if (!nk.ini->output_handle || nk.ini->output_handle == INVALID_HANDLE_VALUE)
		return nk_false;
	if (GetExitCodeProcess(nk.ini->output_handle, &exitCode))
	{
		return (exitCode == STILL_ACTIVE);
	}
	return nk_false;
}

static void
copy_cmdline(void)
{
	reset_cmdline();
	LPCWSTR str = get_cmdline();

	if (!OpenClipboard(NULL))
	{
		MessageBoxW(NULL, L"Could not open clipboard", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}
	EmptyClipboard();
	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, CMDLINE_LEN * sizeof(WCHAR));
	if (!hmem)
	{
		CloseClipboard();
		MessageBoxW(NULL, L"Could not allocate memory", L"ERROR", MB_OK | MB_ICONERROR);
		return;
	}
	LPWSTR pmem = (LPWSTR)GlobalLock(hmem);
	if (pmem)
	{
		wcscpy_s(pmem, CMDLINE_LEN, str);
		GlobalUnlock(hmem);
	}
	SetClipboardData(CF_UNICODETEXT, hmem);
	CloseClipboard();
}

static void
run_qemu(void)
{
	reset_cmdline();
	LPWSTR cmdline = get_cmdline();

	SECURITY_ATTRIBUTES sa;
	HANDLE child_out_r = NULL;
	HANDLE child_out_w = NULL;
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if (nk.ini->output_handle && nk.ini->output_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(nk.ini->output_handle);
		nk.ini->output_handle = NULL;
	}

	if (!CreatePipe(&child_out_r, &child_out_w, &sa, 0))
	{
		MessageBoxW(NULL, L"Stdout pipe creation failed", L"Error", MB_OK);
		return;
	}

	if (!SetHandleInformation(child_out_r, HANDLE_FLAG_INHERIT, 0))
	{
		MessageBoxW(NULL, L"SetHandleInformation failed", L"Error", MB_OK);
		return;
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdOutput = child_out_w;
	si.hStdError = child_out_w;
	si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(child_out_w);
		HANDLE hThread = CreateThread(NULL, 0, read_pipe_thread, child_out_r, 0, NULL);
		if (hThread == NULL)
		{
			MessageBoxW(NULL, L"CreateThread failed", L"Error", MB_OK);
		}
		nk.ini->output_handle = pi.hProcess;
		CloseHandle(pi.hThread);
	}
	else
	{
		MessageBoxW(NULL, L"CreateProcess failed", L"Error", MB_OK);
	}
}

static void
kill_qemu(void)
{
	if (!nk.ini->output_handle || nk.ini->output_handle == INVALID_HANDLE_VALUE)
		return;
	if (TerminateProcess(nk.ini->output_handle, 0))
	{
		CloseHandle(nk.ini->output_handle);
		nk.ini->output_handle = NULL;
		ui_popup_msg(ZTXT(ZTXT_MSG_KILLED), IDR_PNG_INFO);
	}
	else
		ui_popup_msg(ZTXT(ZTXT_MSG_KILL_FAILED), IDR_PNG_WARN);
}

void
ui_qemu_end(struct nk_context* ctx)
{
	nk_layout_row(ctx, NK_DYNAMIC, 0, 4, (float[4]) { 0.2f, 0.2f, 0.3f, 0.3f });
	
	if (nk_button_image_label(ctx, GET_PNG(IDR_PNG_COPY), ZTXT(ZTXT_COPY), NK_TEXT_RIGHT))
	{
		copy_cmdline();
		ui_popup_msg(ZTXT(ZTXT_MSG_COPIED), IDR_PNG_INFO);
	}
	if (nk_button_image_label(ctx, GET_PNG(IDR_PNG_FLOPPY), ZTXT(ZTXT_SAVE), NK_TEXT_RIGHT))
	{
		save_ini();
		ui_popup_msg(ZTXT(ZTXT_MSG_SAVED), IDR_PNG_INFO);
	}
	if (is_qemu_running())
	{
		if (nk_button_image_label(ctx, GET_PNG(IDR_PNG_HALT), ZTXT(ZTXT_STOP), NK_TEXT_RIGHT))
			kill_qemu();
	}
	else
		nk_spacer(ctx);
	if (is_qemu_running())
		nk_widget_disable_begin(ctx);
	if (nk_button_image_label(ctx, GET_PNG(IDR_PNG_START), ZTXT(ZTXT_START), NK_TEXT_RIGHT))
	{
		reset_log();
		if (check_valid())
			run_qemu();
		else
			ui_popup_msg(ZTXT(ZTXT_MSG_MISSING_ARGS), IDR_PNG_WARN);
	}
	if (is_qemu_running())
		nk_widget_disable_end(ctx);

	nk_layout_row(ctx, NK_DYNAMIC, 0, 2, (float[2]) { 1.0f - nk.sq, nk.sq });
	nk_image_label(ctx, GET_PNG(IDR_PNG_INFO), ZTXT(ZTXT_LOGS));
	if (nk_button_image(ctx, GET_PNG(IDR_PNG_REFRESH)))
		reset_log();
	nk_layout_row_dynamic(ctx, 300, 1);
	nk_label_wrap(ctx, nk.ini->output);
}
