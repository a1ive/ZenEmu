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

static inline void
fix_dir_path(char* path, size_t path_size)
{
	size_t len = strlen(path);
	if (len == 0)
	{
		strcpy_s(path, path_size, ucs2_to_utf8(nk.ini->pwd));
	}
	else if (path[len - 1] == '\\')
	{
		path[len - 1] = '\0';
	}
}

static void
append_qemu_path(void)
{
	append_cmdline(L"\"%s\\", rel_to_abs(nk.ini->qemu_dir));
	append_cmdline(L"%s\" ", utf8_to_ucs2(nk.ini->qemu_name[nk.ini->qemu_arch]));
}

static void
append_qemu_bios(void)
{
	LPCWSTR name = rel_to_abs(nk.ini->qemu_fw[nk.ini->cur->fw]);
	if (nk.ini->cur->pflash)
		append_cmdline(L"-drive if=pflash,file=\"%s\",snapshot=on ", name);
	else
		append_cmdline(L"-bios \"%s\" ", name);
}

static void
append_qemu_hw(void)
{
	append_cmdline(L"-cpu %s ", utf8_to_ucs2(nk.ini->cur->model));
	if (nk.ini->cur->whpx)
		append_cmdline(L"-accel whpx ");
	else
		append_cmdline(L"-accel tcg,thread=multi ");
	if (nk.ini->cur->smp[0])
		append_cmdline(L"-smp %s ", utf8_to_ucs2(nk.ini->cur->smp));
	append_cmdline(L"-M %s,kernel-irqchip=%s", utf8_to_ucs2(nk.ini->cur->machine),
		nk.ini->cur->irqchip ? L"on" : L"off");
	if (nk.ini->cur->audio && nk.ini->cur->audio_spk)
		append_cmdline(L",pcspk-audiodev=snd0");
	if (nk.ini->qemu_arch == ZEMU_QEMU_ARCH_AA64
		&& nk.ini->cur->fw == ZEMU_FW_AA64_EFI
		&& nk.ini->cur->virt)
		append_cmdline(L",virtualization=true ");
	else
		append_cmdline(L" ");
	if (nk.ini->cur->mem[0])
		append_cmdline(L"-m %s ", utf8_to_ucs2(nk.ini->cur->mem));

	if (nk.ini->cur->graphics && nk.ini->cur->vgadev[0])
		append_cmdline(L"-device %s ", utf8_to_ucs2(nk.ini->cur->vgadev));
	if (nk.ini->qemu_fullscreen)
		append_cmdline(L"-full-screen ");

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
		append_cmdline(L"-nic user,model=%s", utf8_to_ucs2(nk.ini->cur->netdev));
		if (nk.ini->cur->netmac[0])
			append_cmdline(L",mac=%s", utf8_to_ucs2(nk.ini->cur->netmac));
		if (nk.ini->cur->boot != ZEMU_BOOT_PXE)
			append_cmdline(L" ");
		else
		{
			fix_dir_path(nk.ini->net_tftp, MAX_PATH);
			append_cmdline(L",tftp=\"%s\"", rel_to_abs(nk.ini->net_tftp));
			append_cmdline(L",bootfile=\"%s\" ", utf8_to_ucs2(nk.ini->net_file));
		}
	}

	if (nk.ini->cur->audio)
	{
		if (nk.ini->cur->audiodev[0])
			append_cmdline(L"-audiodev %s,id=snd0 ", utf8_to_ucs2(nk.ini->cur->audiodev));
		if (nk.ini->cur->audio_hda)
			append_cmdline(L"-device intel-hda -device hda-output,audiodev=snd0 ");
	}
	if (nk.ini->cur->battery)
	{
		LPCWSTR dmi = utf8_to_ucs2(nk.ini->qemu_batdmi);
		if (write_battery_dmi(dmi))
			append_cmdline(L"-smbios file=\"%s\" ", dmi);
		LPCWSTR aml = utf8_to_ucs2(nk.ini->qemu_bataml); // absolute path not supported
		if (write_battery_aml(aml))
			append_cmdline(L"-acpitable file=\"%s\" ", aml);
	}
}

static inline void
append_disk_attr(ZEMU_DEV_ATTR* attr)
{
	if (attr->snapshot)
		append_cmdline(L",snapshot=on");
	switch (attr->devif)
	{
	case ZEMU_DEV_IF_IDE:
		append_cmdline(L",if=ide");
		break;
	case ZEMU_DEV_IF_SCSI:
		append_cmdline(L",if=scsi");
		break;
	case ZEMU_DEV_IF_SD:
		append_cmdline(L",if=sd");
		break;
	case ZEMU_DEV_IF_MTD:
		append_cmdline(L",if=mtd");
		break;
	case ZEMU_DEV_IF_VIRTIO:
		append_cmdline(L",if=virtio");
		break;
	case ZEMU_DEV_IF_FLOPPY:
		append_cmdline(L",if=floppy");
		break;
	default:
		break;
	}
}

static void
append_qemu_bootdev(void)
{
	switch (nk.ini->cur->boot)
	{
	case ZEMU_BOOT_VHD:
		append_cmdline(L"-drive file=\"%s\"",
			rel_to_abs(nk.ini->boot_vhd));
		append_disk_attr(&nk.ini->boot_vhd_attr);
		append_cmdline(L",index=0,media=disk ");
		break;
	case ZEMU_BOOT_ISO:
		append_cmdline(L"-drive file=\"%s\",index=0,media=cdrom ",
			rel_to_abs(nk.ini->boot_iso));
		break;
	case ZEMU_BOOT_PD:
		append_cmdline(L"-drive file=\\\\.\\PhysicalDrive%lu",
			nk.ini->d_info[ZEMU_DEV_HD][nk.ini->boot_hd].index);
		append_disk_attr(&nk.ini->boot_hd_attr);
		append_cmdline(L",format=raw,index=0,media=disk ");
		break;
	case ZEMU_BOOT_CD:
		append_cmdline(L"-drive file=\\\\.\\CdRom%lu,format=raw,index=0,media=cdrom ",
			nk.ini->d_info[ZEMU_DEV_CD][nk.ini->boot_cd].index);
		break;
	case ZEMU_BOOT_VFD:
		append_cmdline(L"-drive file=\"%s\"",
			rel_to_abs(nk.ini->boot_vfd));
		append_disk_attr(&nk.ini->boot_vfd_attr);
		append_cmdline(L",index=0,media=disk ");
		break;
	case ZEMU_BOOT_PXE:
		break;
	case ZEMU_BOOT_LINUX:
		append_cmdline(L"-kernel \"%s\" ", rel_to_abs(nk.ini->boot_linux));
		if (nk.ini->boot_initrd[0])
			append_cmdline(L"-initrd \"%s\" ", rel_to_abs(nk.ini->boot_initrd));
		if (nk.ini->boot_kcmd[0])
			append_cmdline(L"-append \"%s\" ", utf8_to_ucs2(nk.ini->boot_kcmd));
		if (nk.ini->boot_dtb[0])
			append_cmdline(L"-dtb \"%s\" ", rel_to_abs(nk.ini->boot_dtb));
		if (!IS_BIOS && nk.ini->boot_shim[0])
			append_cmdline(L"-shim \"%s\" ", rel_to_abs(nk.ini->boot_shim));
		break;
	case ZEMU_BOOT_WIM:
		append_cmdline(L"-kernel \"%s\" ", rel_to_abs(nk.ini->qemu_wimldr[nk.ini->cur->fw]));
		append_cmdline(L"-initrd \"%s\" ", rel_to_abs(nk.ini->boot_wim));
		if (IS_BIOS)
		{
			const wchar_t* wimcpio = rel_to_abs(nk.ini->qemu_wimcpio);
			append_cmdline(L"-append \"rawwim gui index=%d addr=0x%lx len=%llu\" ",
				nk.ini->boot_wim_index, nk.ini->qemu_wimaddr, get_file_size(wimcpio));
			append_cmdline(L"-device loader,file=\"%s\",addr=0x%lx,force-raw=on ",
				wimcpio, nk.ini->qemu_wimaddr);
		}
		else
		{
			append_cmdline(L"-append \"rawwim gui index=%d\" ", nk.ini->boot_wim_index);
			append_cmdline(L"-drive file=\"%s\",snapshot=on ", rel_to_abs(nk.ini->qemu_wimhda));
		}
		break;
	case ZEMU_BOOT_DIR:
		fix_dir_path(nk.ini->boot_dir, MAX_PATH);
		append_cmdline(L"-drive file=fat%s\"%s\"",
			nk.ini->boot_dir_attr.snapshot ? L":" : L":rw:",
			rel_to_abs(nk.ini->boot_dir));
		append_disk_attr(&nk.ini->boot_dir_attr);
		append_cmdline(L",format=raw,index=0,media=disk ");
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
	if (nk.ini->cur->fw_menu)
		append_cmdline(L"menu=on,");
	append_cmdline(L"splash-time=%s,strict=on ", utf8_to_ucs2(nk.ini->cur->fw_timeout));
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
		append_disk_attr(&dev->attr);
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
