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
	LPCWSTR name = rel_to_abs(nk.ini->qemu_fw[nk.ini->cur->fw]);
	if (nk.ini->cur->pflash)
		append_cmdline(L"-drive if=pflash,file=\"%s\",format=raw ", name);
	else
		append_cmdline(L"-bios \"%s\" ", name);
}

static void
append_qemu_hw(void)
{
	append_cmdline(L"-cpu %s -accel tcg,thread=multi ", utf8_to_ucs2(nk.ini->cur->model));
	append_cmdline(L"-smp %s ", utf8_to_ucs2(nk.ini->cur->smp));
	append_cmdline(L"-M %s,kernel-irqchip=%s", utf8_to_ucs2(nk.ini->cur->machine),
		nk.ini->cur->irqchip ? L"on" : L"off");
	if (nk.ini->cur->audio && nk.ini->cur->audio_spk)
		append_cmdline(L",pcspk-audiodev=snd0");
	if (nk.ini->qemu_arch)
		append_cmdline(L",virtualization=%s ", nk.ini->cur->virt ? L"true" : L"false");
	else
		append_cmdline(L" ");
	append_cmdline(L"-m %s ", utf8_to_ucs2(nk.ini->cur->mem));

	if (nk.ini->cur->graphics && nk.ini->cur->vgadev[0])
		append_cmdline(L"-device %s ", utf8_to_ucs2(nk.ini->cur->vgadev));
	//if (!nk.ini->cur->graphics)
		//append_cmdline(L"-nographic ");

	if (nk.ini->cur->usb && nk.ini->cur->usbctrl[0])
	{
		append_cmdline(L"-device %s ", utf8_to_ucs2(nk.ini->cur->usbctrl));
		if (nk.ini->cur->usb_kbd)
			append_cmdline(L"-device usb-kbd ");
		if (nk.ini->cur->usb_tablet)
			append_cmdline(L"-device usb-tablet ");
		if (nk.ini->cur->usb_mouse)
			append_cmdline(L"-device usb-mouse ");
	}
	
	if (nk.ini->cur->net && nk.ini->cur->netdev[0])
	{
		append_cmdline(L"-net nic,model=%s ", utf8_to_ucs2(nk.ini->cur->netdev));
		if (nk.ini->cur->boot != ZEMU_BOOT_PXE)
			append_cmdline(L"-net user ");
	}

	if (nk.ini->cur->audio)
	{
		append_cmdline(L"-audiodev %s,id=snd0 ", utf8_to_ucs2(nk.ini->cur->audiodev));
		if (nk.ini->cur->audio_hda)
			append_cmdline(L"-device intel-hda -device hda-output,audiodev=snd0 ");
	}
}

static inline void
append_hd_attr(ZEMU_DEV_ATTR* attr)
{
	if (attr->snapshot)
		append_cmdline(L",snapshot=on");
}

static void
append_qemu_bootdev(void)
{
	switch (nk.ini->cur->boot)
	{
	case ZEMU_BOOT_VHD:
		append_cmdline(L"-drive file=\"%s\"",
			rel_to_abs(nk.ini->boot_vhd));
		append_hd_attr(&nk.ini->boot_vhd_attr);
		append_cmdline(L",index=0,media=disk ");
		break;
	case ZEMU_BOOT_ISO:
		append_cmdline(L"-drive file=\"%s\",index=0,media=cdrom ",
			rel_to_abs(nk.ini->boot_iso));
		break;
	case ZEMU_BOOT_PD:
		append_cmdline(L"-drive file=\\\\.\\PhysicalDrive%lu",
			nk.ini->d_info[ZEMU_DEV_HD][nk.ini->boot_hd].index);
		append_hd_attr(&nk.ini->boot_hd_attr);
		append_cmdline(L",format=raw,index=0,media=disk ");
		break;
	case ZEMU_BOOT_CD:
		append_cmdline(L"-drive file=\\\\.\\CdRom%lu,format=raw,index=0,media=cdrom ",
			nk.ini->d_info[ZEMU_DEV_CD][nk.ini->boot_cd].index);
		break;
	case ZEMU_BOOT_VFD:
		append_cmdline(L"-fda \"%s\" ", rel_to_abs(nk.ini->boot_vfd));
		break;
	case ZEMU_BOOT_PXE:
		append_cmdline(L"-net user,tftp=\"%s\",", rel_to_abs(nk.ini->net_tftp));
		append_cmdline(L",bootfile=\"%s\" ", rel_to_abs(nk.ini->net_file));
		break;
	case ZEMU_BOOT_LINUX:
		append_cmdline(L"-kernel \"%s\" ", rel_to_abs(nk.ini->boot_linux));
		if (nk.ini->boot_initrd[0])
			append_cmdline(L"-initrd \"%s\" ", rel_to_abs(nk.ini->boot_initrd));
		if (nk.ini->boot_kcmd[0])
			append_cmdline(L"-append \"%s\" ", rel_to_abs(nk.ini->boot_kcmd));
		if (nk.ini->boot_dtb[0])
			append_cmdline(L"-dtb \"%s\" ", rel_to_abs(nk.ini->boot_dtb));
		if (!IS_BIOS && nk.ini->boot_shim[0])
			append_cmdline(L"-shim \"%s\" ", rel_to_abs(nk.ini->boot_shim));
		break;
	case ZEMU_BOOT_WIM:
		append_cmdline(L"-kernel \"%s\" ", rel_to_abs(nk.ini->qemu_wimldr[nk.ini->cur->fw]));
		append_cmdline(L"-initrd \"%s\" ", rel_to_abs(nk.ini->boot_wim));
		append_cmdline(L"-drive file=\"%s\",snapshot=on ", rel_to_abs(nk.ini->qemu_wimhda));
		break;
	case ZEMU_BOOT_DIR:
		append_cmdline(L"-drive file=fat:rw:\"%s\",format=raw,media=disk,snapshot=on ",
			rel_to_abs(nk.ini->boot_dir));
		break;
	}
}

static void
append_qemu_bootorder(void)
{
	switch (nk.ini->cur->boot)
	{
	case ZEMU_BOOT_VHD:
	case ZEMU_BOOT_PD:
	case ZEMU_BOOT_DIR:
		append_cmdline(L"-boot c,");
		break;
	case ZEMU_BOOT_ISO:
	case ZEMU_BOOT_CD:
		append_cmdline(L"-boot d,");
		break;
	case ZEMU_BOOT_VFD:
		append_cmdline(L"-boot a,");
		break;
	case ZEMU_BOOT_PXE:
		append_cmdline(L"-boot n,");
		break;
	case ZEMU_BOOT_LINUX:
	case ZEMU_BOOT_WIM:
	default:
		append_cmdline(L"-boot ");
	}
	append_cmdline(L"strict=on ");
}

static void
append_qemu_hdb(void)
{
	for (size_t i = 0; i < nk.ini->add_dev_count; i++)
	{
		ZEMU_ADD_DEV* dev = &nk.ini->add_dev[i];
		if (!dev->is_active)
			continue;
		if (dev->is_device)
		{
			append_cmdline(L"-drive file=\\\\.\\PhysicalDrive%lu,format=raw", dev->id);
		}
		else
		{
			if (!dev->path[0])
				continue;
			append_cmdline(L"-drive file=\"%s\"", rel_to_abs(dev->path));
		}
		append_hd_attr(&dev->attr);
		if (dev->type == ZEMU_DEV_CD)
			append_cmdline(L",media=cdrom ");
		else
			append_cmdline(L",media=disk ");
	}
}

LPWSTR
get_cmdline(void)
{
	append_qemu_path();
	append_qemu_bios();
	append_qemu_hw();
	append_qemu_bootdev();
	append_qemu_hdb();
	append_qemu_bootorder();

	return static_cmdline;
}
