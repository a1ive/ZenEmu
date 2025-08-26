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
		return nk.ini->net_file[0] ? true : false;
	case ZEMU_BOOT_LINUX:
		return nk.ini->boot_linux[0] ? true : false;
	case ZEMU_BOOT_WIM:
		return nk.ini->boot_wim[0] ? true : false;
	case ZEMU_BOOT_DIR:
		return true;
	}
	return false;
}

void
ui_reset_log(void)
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
			ui_reset_log();

		memcpy(nk.ini->output + nk.ini->output_offset, buffer, read_size);
		nk.ini->output_offset += read_size;
		if (nk.ini->output_offset < OUTBUF_SZ)
			nk.ini->output[nk.ini->output_offset] = '\0';
		else
			nk.ini->output[OUTBUF_SZ - 1] = '\0';
	}
	return 0;
}

nk_bool ui_is_qemu_running(void)
{
	DWORD rc = 0;
	if (!nk.ini->output_handle || nk.ini->output_handle == INVALID_HANDLE_VALUE)
		goto fail;
	if (GetExitCodeProcess(nk.ini->output_handle, &rc))
	{
		if (rc == STILL_ACTIVE)
			return nk_true;
	}
fail:
	if (nk.ini->ews)
	{
		ews_stop(nk.ini->ews);
		nk.ini->ews = NULL;
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
	if (nk.ini->cur->boot == ZEMU_BOOT_PXE && nk.ini->net_http)
	{
		int port = atoi(nk.ini->net_http_port);
		if (port <= 0 || port > 65535)
			port = 80;
		nk.ini->ews = ews_start((uint16_t)port, nk.ini->net_tftp);
		if (!nk.ini->ews)
		{
			MessageBoxW(NULL, L"Could not start HTTP server", L"ERROR", MB_OK | MB_ICONERROR);
			return;
		}
	}

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

	if (CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, nk.ini->pwd, &si, &pi))
	{
		CloseHandle(child_out_w);
		HANDLE hThread = CreateThread(NULL, 0, read_pipe_thread, child_out_r, 0, NULL);
		if (hThread == NULL)
		{
			MessageBoxW(NULL, L"CreateThread failed", L"Error", MB_OK);
		}
		nk.ini->output_handle = pi.hProcess;
		nk.ini->output_pid = pi.dwProcessId;
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
ui_qemu_start(struct nk_context* ctx)
{
	nk_bool is_running = ui_is_qemu_running();

	nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f });
	
	if (nk_button_ex(ctx, GET_PNG(IDR_PNG_COPY), ZTXT(ZTXT_COPY)))
	{
		copy_cmdline();
		ui_popup_msg(ZTXT(ZTXT_MSG_COPIED), IDR_PNG_INFO);
	}
	if (nk_button_ex(ctx, GET_PNG(IDR_PNG_FLOPPY), ZTXT(ZTXT_SAVE)))
	{
		save_ini();
		ui_popup_msg(ZTXT(ZTXT_MSG_SAVED), IDR_PNG_INFO);
	}
	nk_spacer(ctx);
	if (is_running)
	{
		if (nk_button_ex(ctx, GET_PNG(IDR_PNG_HALT), ZTXT(ZTXT_STOP)))
			kill_qemu();
	}
	else
		nk_spacer(ctx);
	if (is_running)
		nk_widget_disable_begin(ctx);
	if (nk_button_ex(ctx, GET_PNG(IDR_PNG_START), ZTXT(ZTXT_START)))
	{
		ui_reset_log();
		if (check_valid())
			run_qemu();
		else
			ui_popup_msg(ZTXT(ZTXT_MSG_MISSING_ARGS), IDR_PNG_WARN);
	}
	if (is_running)
		nk_widget_disable_end(ctx);
}
