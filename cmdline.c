// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "cmdline.h"
#include "dev.h"

#include <stdbool.h>

static WCHAR static_cmdline[CMDLINE_LEN + 1];
static int static_offset = 0;

void
reset_cmdline(void)
{
	ZeroMemory(static_cmdline, CMDLINE_LEN + 1);
	static_offset = 0;
}

void
append_cmdline(LPCWSTR _Printf_format_string_ format, ...)
{
	int sz;
	WCHAR* buf = static_cmdline + static_offset;
	int available = (int)CMDLINE_LEN - static_offset;
	va_list ap;
	va_start(ap, format);
	if (available <= 0)
		goto fail;
	sz = _vsnwprintf_s(buf, available, available, format, ap);
	if (sz <= 0)
	{
		va_end(ap);
		goto fail;
	}
	if (sz > available)
		sz = available;
	static_offset += sz;
	va_end(ap);
	return;
fail:
	MessageBoxW(NULL, L"Set cmdline failed", L"Error", MB_ICONERROR);
}

static void
append_qemu_path(void)
{
	LPCWSTR name = L"";
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		name = L"qemu-system-x86_64w.exe";
		break;
	case ZEMU_QEMU_ARCH_AA64:
		name = L"qemu-system-aarch64w.exe";
		break;
	}
	append_cmdline(L"\"%s\\%s\" ", utf8_to_ucs2(nk.ini->qemu_dir), name);
}

static void
append_qemu_bios(void)
{
	bool pflash = false;
	LPCWSTR name = L"";
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		switch (nk.ini->qemu_fw_x86)
		{
		case ZEMU_FW_X86_BIOS:
			name = L"bios.bin";
			break;
		case ZEMU_FW_X86_EFI:
			name = L"IA32_EFI.fd";
			break;
		case ZEMU_FW_X64_EFI:
			name = L"X64_EFI.fd";
			break;
		}
		break;
	case ZEMU_QEMU_ARCH_AA64:
		switch (nk.ini->qemu_fw_arm)
		{
		case ZEMU_FW_AA64_EFI:
			name = L"AA64_EFI.fd";
			break;
		case ZEMU_FW_ARM32_EFI:
			name = L"ARM_EFI.fd";
			break;
		}
		break;
	}
	if (pflash)
		append_cmdline(L"-drive if=pflash,file=\"%s\\%s\",format=raw ", nk.ini->pwd, name);
	else
		append_cmdline(L"-bios \"%s\\%s\" ", nk.ini->pwd, name);
}

static void
append_qemu_hw(void)
{
	LPCWSTR cpu = L"";
	LPCWSTR accel = L"-msg timestamp=off -accel tcg,thread=multi";
	LPCWSTR device = L"-device nec-usb-xhci";
	LPCWSTR nic = L"-nic user,model=virtio-net-pci";
	LPCWSTR extra = L"";
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		cpu = utf8_to_ucs2(nk.ini->qemu_cpu_x86);
		extra = L"-M pc";
		break;
	case ZEMU_QEMU_ARCH_AA64:
		cpu = utf8_to_ucs2(nk.ini->qemu_cpu_arm);
		extra = L"-M virt,virtualization=true -device ramfb -device usb-kbd -device usb-tablet";
		break;
	}
	if (cpu[0] == '\0')
		cpu = L"max";
	append_cmdline(L"-m %d -smp %d -cpu %s %s %s %s %s ",
		nk.ini->qemu_mem_mb, nk.ini->qemu_cpu_num, cpu, accel, device, nic, extra);
}

static void
append_qemu_bootdev(void)
{
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
	{
		switch (nk.ini->qemu_boot_x86)
		{
		case ZEMU_BOOT_X86_VHD:
			append_cmdline(L"-hda \"%s\" -boot c ", utf8_to_ucs2(nk.ini->boot_vhd));
			break;
		case ZEMU_BOOT_X86_ISO:
			append_cmdline(L"-cdrom \"%s\" -boot d ", utf8_to_ucs2(nk.ini->boot_iso));
			break;
		case ZEMU_BOOT_X86_PD:
			append_cmdline(L"-drive file=\\\\.\\PhysicalDrive%lu,format=raw,index=0,media=disk -boot c ",
				nk.ini->hd_info[nk.ini->boot_hd].index);
			break;
		case ZEMU_BOOT_X86_CD:
			append_cmdline(L"-drive file=\\\\.\\CdRom%lu,format=raw,index=0,media=cdrom -boot d ",
				nk.ini->cd_info[nk.ini->boot_cd].index);
			break;
		case ZEMU_BOOT_X86_VFD:
			append_cmdline(L"-fda \"%s\" -boot a ", utf8_to_ucs2(nk.ini->boot_vfd));
			break;
		case ZEMU_BOOT_X86_PXE:
			append_cmdline(L"-net user,tftp=\"%s\",", utf8_to_ucs2(nk.ini->net_tftp));
			append_cmdline(L",bootfile=\"%s\" -boot n ", utf8_to_ucs2(nk.ini->net_file));
			break;
		}
	}
	break;
	case ZEMU_QEMU_ARCH_AA64:
	{
		switch (nk.ini->qemu_boot_arm)
		{
		case ZEMU_BOOT_ARM_VHD:
			append_cmdline(L"-hda \"%s\" -boot c ", utf8_to_ucs2(nk.ini->boot_vhd));
			break;
		case ZEMU_BOOT_ARM_ISO:
			append_cmdline(L"-cdrom \"%s\" -boot d ", utf8_to_ucs2(nk.ini->boot_iso));
			break;
		case ZEMU_BOOT_ARM_PD:
			append_cmdline(L"-drive file=\\\\.\\PhysicalDrive%lu,format=raw,index=0,media=disk -boot c ",
				nk.ini->hd_info[nk.ini->boot_hd].index);
			break;
		case ZEMU_BOOT_ARM_CD:
			append_cmdline(L"-drive file=\\\\.\\CdRom%lu,format=raw,index=0,media=cdrom -boot d ",
				nk.ini->cd_info[nk.ini->boot_cd].index);
			break;
		case ZEMU_BOOT_ARM_PXE:
			append_cmdline(L"-net user,tftp=\"%s\",", utf8_to_ucs2(nk.ini->net_tftp));
			append_cmdline(L",bootfile=\"%s\" -boot n ", utf8_to_ucs2(nk.ini->net_file));
			break;
		}
	}
	break;
	}
}

LPWSTR
get_cmdline(void)
{
	append_qemu_path();
	append_qemu_bios();
	append_qemu_hw();
	append_qemu_bootdev();
	//MessageBoxW(nk.wnd, static_cmdline, L"Command", MB_OK);
	return static_cmdline;
}
