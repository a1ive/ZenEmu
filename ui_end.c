// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

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
		}
		break;
	case ZEMU_QEMU_ARCH_AA64:
		switch (nk.ini->qemu_boot_arm)
		{
		case ZEMU_BOOT_ARM_VHD:
			return nk.ini->boot_vhd[0] ? true : false;
		case ZEMU_BOOT_ARM_ISO:
			return nk.ini->boot_iso[0] ? true : false;
		}
		break;
	}
	return false;
}

static LPCWSTR
get_firmware_name(LPCWSTR* bios)
{
	*bios = L"-bios";
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		switch (nk.ini->qemu_fw_x86)
		{
		case ZEMU_FW_X86_BIOS:
			return L"bios.bin";
		case ZEMU_FW_X86_EFI:
			*bios = L"-pflash";
			return L"IA32_EFI.fd";
		case ZEMU_FW_X64_EFI:
			*bios = L"-pflash";
			return L"X64_EFI.fd";
		}
		break;
	case ZEMU_QEMU_ARCH_AA64:
		switch (nk.ini->qemu_fw_arm)
		{
		case ZEMU_FW_AA64_EFI:
			return L"AA64_EFI.fd";
		}
		break;
	}
	return L"linuxboot.bin";
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


static void
run_qemu(void)
{
	LPCWSTR bios = L"-bios";
	LPCWSTR fw = get_firmware_name(&bios);

	ZeroMemory(nk.ini->output, OUTBUF_SZ);
	nk.ini->output_offset = 0;

	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
	{
		swprintf(nk.ini->qemu_path, MAX_PATH, L"%s\\qemu-system-x86_64w.exe",
			utf8_to_ucs2(nk.ini->qemu_dir));
		swprintf(nk.ini->qemu_target, 2ULL * MAX_PATH,
			L"\"%s\"%s \"%s\\%s\" -m %d -smp %d "
			"-mem-prealloc -msg timestamp=off "
			"-cpu max -accel tcg,thread=multi "
			"-nic user,model=virtio "
			"-device nec-usb-xhci -device usb-kbd -device usb-tablet ",
			nk.ini->qemu_path,
			bios, nk.ini->pwd, fw, nk.ini->qemu_mem_mb, nk.ini->qemu_cpu_num);
		switch (nk.ini->qemu_boot_x86)
		{
		case ZEMU_BOOT_X86_VHD:
			swprintf(nk.ini->cmdline, CONV_BUFSZ,
				L"%s -hda \"%s\" -boot c", nk.ini->qemu_target, utf8_to_ucs2(nk.ini->boot_vhd));
			break;
		case ZEMU_BOOT_X86_ISO:
			swprintf(nk.ini->cmdline, CONV_BUFSZ,
				L"%s -cdrom \"%s\" -boot d", nk.ini->qemu_target, utf8_to_ucs2(nk.ini->boot_iso));
			break;
		}
	}
		break;
	case ZEMU_QEMU_ARCH_AA64:
	{
		swprintf(nk.ini->qemu_path, MAX_PATH, L"%s\\qemu-system-aarch64w.exe",
			utf8_to_ucs2(nk.ini->qemu_dir));
		swprintf(nk.ini->qemu_target, 2ULL * MAX_PATH,
			L"\"%s\" %s \"%s\\%s\" -device ramfb -m %d -smp %d "
			"-M virt,virtualization=true,kernel-irqchip=off,mem-merge=off,hmat=off "
			"-mem-prealloc -msg timestamp=off "
			"-cpu cortex-a53 -accel tcg,thread=multi "
			"-nic user,model=virtio "
			"-device nec-usb-xhci -device usb-kbd -device usb-tablet ",
			nk.ini->qemu_path,
			bios, nk.ini->pwd, fw, nk.ini->qemu_mem_mb, nk.ini->qemu_cpu_num);
		switch (nk.ini->qemu_boot_arm)
		{
		case ZEMU_BOOT_ARM_VHD:
			swprintf(nk.ini->cmdline, CONV_BUFSZ,
				L"%s -hda \"%s\" -boot c", nk.ini->qemu_target, utf8_to_ucs2(nk.ini->boot_vhd));
			break;
		case ZEMU_BOOT_ARM_ISO:
			swprintf(nk.ini->cmdline, CONV_BUFSZ,
				L"%s -cdrom \"%s\" -boot d", nk.ini->qemu_target, utf8_to_ucs2(nk.ini->boot_iso));
			break;
		}
	}
		break;
	}

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

	if (CreateProcessW(NULL, nk.ini->cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
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
	nk_layout_row(ctx, NK_DYNAMIC, 0, 3, (float[3]) { 0.3f, 0.4f, 0.3f });
	nk_spacer(ctx);
	if (check_valid())
	{
		if (nk_button_label(ctx, "Start"))
			run_qemu();
	}
	else
		nk_label(ctx, "Start", NK_TEXT_CENTERED);
	nk_spacer(ctx);

	nk_layout_row_dynamic(ctx, 0, 1);
	nk_label(ctx, "Logs", NK_TEXT_LEFT);
	nk_layout_row_dynamic(ctx, 300, 1);
	nk_label_wrap(ctx, nk.ini->output);
}
