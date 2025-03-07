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
	append_cmdline(L"\"%s\\", utf8_to_ucs2(nk.ini->qemu_dir));
	append_cmdline(L"%s\" ", utf8_to_ucs2(nk.ini->qemu_name[nk.ini->qemu_arch]));
}

static void
append_qemu_bios(void)
{
	bool pflash = false;
	LPCWSTR name = utf8_to_ucs2(nk.ini->qemu_fw[nk.ini->cur->fw]);
	if (pflash)
		append_cmdline(L"-drive if=pflash,file=\"%s\\%s\",format=raw ", nk.ini->pwd, name);
	else
		append_cmdline(L"-bios \"%s\\%s\" ", nk.ini->pwd, name);
}

static void
append_qemu_hw(void)
{
	append_cmdline(L"-cpu %s -accel tcg,thread=multi ", utf8_to_ucs2(nk.ini->cur->model));
	append_cmdline(L"-smp %s ", utf8_to_ucs2(nk.ini->cur->smp));
	append_cmdline(L"-M %s,kernel-irqchip=%s", utf8_to_ucs2(nk.ini->cur->machine),
		nk.ini->cur->irqchip ? L"on" : L"off");
	if (nk.ini->qemu_arch)
		append_cmdline(L",virtualization=%s ", nk.ini->cur->virt ? L"true" : L"false");
	else
		append_cmdline(L" ");
	append_cmdline(L"-m %s ", utf8_to_ucs2(nk.ini->cur->mem));
	append_cmdline(L"-device %s ", utf8_to_ucs2(nk.ini->cur->vga));

	if (nk.ini->cur->usb[0])
	{
		append_cmdline(L"-device %s ", utf8_to_ucs2(nk.ini->cur->usb));
		if (nk.ini->cur->usb_kbd)
			append_cmdline(L"-device usb-kbd ");
		if (nk.ini->cur->usb_tablet)
			append_cmdline(L"-device usb-tablet ");
		if (nk.ini->cur->usb_mouse)
			append_cmdline(L"-device usb-mouse ");
	}
	
	append_cmdline(L"-nic user,model=virtio-net-pci ");
}

static void
append_qemu_bootdev(void)
{
	switch (nk.ini->cur->boot)
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
	case ZEMU_BOOT_WIM:
		append_cmdline(L"-kernel \"%s\\%s\" ", nk.ini->pwd, utf8_to_ucs2(nk.ini->qemu_wimldr[nk.ini->cur->fw]));
		append_cmdline(L"-initrd \"%s\" ", utf8_to_ucs2(nk.ini->boot_wim));
		append_cmdline(L"-drive file=\"%s\\%s\",snapshot=on ", nk.ini->pwd, utf8_to_ucs2(nk.ini->qemu_wimhda));
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
