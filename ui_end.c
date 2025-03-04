// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"
#include "cmdline.h"

#include <stdbool.h>
#include <shellapi.h>

static bool
check_valid(void)
{
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		switch (nk.ini->qemu_boot_x86)
		{
		case ZEMU_BOOT_X86_VHD:
			return nk.ini->boot_vhd[0] ? true : false;
		case ZEMU_BOOT_X86_ISO:
			return nk.ini->boot_iso[0] ? true : false;
		case ZEMU_BOOT_X86_PD:
			return nk.ini->hd_count ? true : false;
		case ZEMU_BOOT_X86_CD:
			return nk.ini->cd_count ? true : false;
		}
		break;
	case ZEMU_QEMU_ARCH_AA64:
		switch (nk.ini->qemu_boot_arm)
		{
		case ZEMU_BOOT_ARM_VHD:
			return nk.ini->boot_vhd[0] ? true : false;
		case ZEMU_BOOT_ARM_ISO:
			return nk.ini->boot_iso[0] ? true : false;
		case ZEMU_BOOT_ARM_PD:
			return nk.ini->hd_count ? true : false;
		case ZEMU_BOOT_ARM_CD:
			return nk.ini->cd_count ? true : false;
		}
		break;
	}
	return false;
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
		{
			ZeroMemory(nk.ini->output, OUTBUF_SZ);
			nk.ini->output_offset = 0;
		}

		memcpy(nk.ini->output + nk.ini->output_offset, buffer, read_size);
		nk.ini->output_offset += read_size;
		if (nk.ini->output_offset < OUTBUF_SZ)
			nk.ini->output[nk.ini->output_offset] = '\0';
		else
			nk.ini->output[OUTBUF_SZ - 1] = '\0';
	}
	return 0;
}

static inline void
reset_log(void)
{
	ZeroMemory(nk.ini->output, OUTBUF_SZ);
	nk.ini->output_offset = 0;
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

	reset_log();

	SECURITY_ATTRIBUTES sa;
	HANDLE child_out_r = NULL;
	HANDLE child_out_w = NULL;
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

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
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		MessageBoxW(NULL, L"CreateProcess failed", L"Error", MB_OK);
	}
}

void
ui_qemu_end(struct nk_context* ctx)
{
	nk_layout_row(ctx, NK_DYNAMIC, 0, 5, (float[5]) { 0.1f, 0.1f, 0.1f, 0.4f, 0.3f });
	if (nk_button_label(ctx, "Clear"))
		reset_log();
	if (nk_button_label(ctx, "Copy"))
		copy_cmdline();
	if (nk_button_label(ctx, "Save"))
		save_ini();
	nk_spacer(ctx);
	if (check_valid())
	{
		if (nk_button_label(ctx, "Start"))
			run_qemu();
	}
	else
		nk_label(ctx, "Start", NK_TEXT_CENTERED);

	nk_layout_row_dynamic(ctx, 0, 1);
	nk_label(ctx, "Logs", NK_TEXT_LEFT);
	nk_layout_row_dynamic(ctx, 300, 1);
	nk_label_wrap(ctx, nk.ini->output);
}
