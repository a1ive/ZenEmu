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
	ZEMU_FW fw;
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		fw = nk.ini->qemu_fw_x86;
		break;
	case ZEMU_QEMU_ARCH_AA64:
		fw = nk.ini->qemu_fw_arm;
		break;
	default:
		fw = ZEMU_BOOT_MAX;
	}
	switch (fw)
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
	case ZEMU_FW_AA64_EFI:
		name = L"AA64_EFI.fd";
		break;
	case ZEMU_FW_ARM32_EFI:
		name = L"ARM_EFI.fd";
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
	LPCWSTR accel = L"-accel tcg,thread=multi";
	LPCWSTR device = L"-device usb-ehci -device usb-kbd -device usb-tablet";
	LPCWSTR nic = L"-nic user,model=virtio-net-pci";
	LPCWSTR extra = L"";
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		cpu = utf8_to_ucs2(nk.ini->qemu_cpu_x86);
		extra = L"-M pc,kernel-irqchip=off -device vmware-svga";
		break;
	case ZEMU_QEMU_ARCH_AA64:
		cpu = utf8_to_ucs2(nk.ini->qemu_cpu_arm);
		extra = L"-M virt,virtualization=true,kernel-irqchip=off -device ramfb";
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
	ZEMU_BOOT_TARGET target;
	switch (nk.ini->qemu_arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		target = nk.ini->qemu_boot_x86;
		break;
	case ZEMU_QEMU_ARCH_AA64:
		target = nk.ini->qemu_boot_arm;
		break;
	default:
		target = ZEMU_BOOT_MAX;
	}
	switch (target)
	{
	case ZEMU_BOOT_VHD:
		append_cmdline(L"-hda \"%s\" -boot c ", utf8_to_ucs2(nk.ini->boot_vhd));
		break;
	case ZEMU_BOOT_ISO:
		append_cmdline(L"-cdrom \"%s\" -boot d ", utf8_to_ucs2(nk.ini->boot_iso));
		break;
	case ZEMU_BOOT_PD:
		append_cmdline(L"-drive file=\\\\.\\PhysicalDrive%lu,format=raw,index=0,media=disk -boot c ",
			nk.ini->hd_info[nk.ini->boot_hd].index);
		break;
	case ZEMU_BOOT_CD:
		append_cmdline(L"-drive file=\\\\.\\CdRom%lu,format=raw,index=0,media=cdrom -boot d ",
			nk.ini->cd_info[nk.ini->boot_cd].index);
		break;
	case ZEMU_BOOT_VFD:
		append_cmdline(L"-fda \"%s\" -boot a ", utf8_to_ucs2(nk.ini->boot_vfd));
		break;
	case ZEMU_BOOT_PXE:
		append_cmdline(L"-net user,tftp=\"%s\",", utf8_to_ucs2(nk.ini->net_tftp));
		append_cmdline(L",bootfile=\"%s\" -boot n ", utf8_to_ucs2(nk.ini->net_file));
		break;
	case ZEMU_BOOT_LINUX:
		append_cmdline(L"-kernel \"%s\" ", utf8_to_ucs2(nk.ini->boot_linux));
		if (nk.ini->boot_initrd[0])
			append_cmdline(L"-initrd \"%s\" ", utf8_to_ucs2(nk.ini->boot_initrd));
		if (nk.ini->boot_kcmd[0])
			append_cmdline(L"-append \"%s\" ", utf8_to_ucs2(nk.ini->boot_kcmd));
		if (nk.ini->boot_dtb[0])
			append_cmdline(L"-dtb \"%s\" ", utf8_to_ucs2(nk.ini->boot_dtb));
		if (nk.ini->boot_shim[0])
			append_cmdline(L"-shim \"%s\" ", utf8_to_ucs2(nk.ini->boot_shim));
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

	return static_cmdline;
}
