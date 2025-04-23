// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

static void
get_profile(ZEMU_QEMU_ARCH arch)
{
	int num;
	LPCWSTR section, model, machine, display;
	ZEMU_FW fw, fw_min, fw_max;
	ZEMU_INI_PROFILE* p = &nk.ini->profile[arch];
	switch (arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		section = L"X86";
		model = L"max";
		machine = L"q35";
		display = L"vmware-svga";
		fw = ZEMU_FW_X64_EFI;
		fw_min = ZEMU_FW_X86_MIN;
		fw_max = ZEMU_FW_X86_MAX;
		break;
	case ZEMU_QEMU_ARCH_AA64:
		section = L"Arm";
		model = L"max";
		machine = L"virt";
		display = L"ramfb";
		fw = ZEMU_FW_AA64_EFI;
		fw_min = ZEMU_FW_ARM_MIN;
		fw_max = ZEMU_FW_ARM_MAX;
		break;
	default:
		section = L"Unknown";
		model = L"max";
		machine = L"virt";
		display = L"VGA";
		fw = 0;
		fw_min = ZEMU_FW_MAX;
		fw_max = ZEMU_FW_MAX;
	}

	num = get_ini_num(section, L"Smp", 4);
	if (num <= 0)
		num = 1;
	snprintf(p->smp, OPT_SZ, "%d", num);

	strcpy_s(p->model, OPT_SZ, get_ini_value(section, L"Model", model));

	num = get_ini_num(section, L"Memory", 4096);
	if (num <= 0)
		num = 1;
	snprintf(p->mem, OPT_SZ, "%d", num);

	strcpy_s(p->machine, OPT_SZ, get_ini_value(section, L"Machine", machine));
	p->irqchip = get_ini_bool(section, L"KernelIrqchip", nk_true);
	p->virt = get_ini_bool(section, L"Virtualization", nk_true);
	p->graphics = get_ini_bool(section, L"GuiWindow", nk_true);
	strcpy_s(p->vgadev, OPT_SZ, get_ini_value(section, L"Display", display));
	p->pflash = get_ini_bool(section, L"Pflash", nk_true);
	p->net = get_ini_bool(section, L"Network", nk_true);
	strcpy_s(p->netdev, OPT_SZ, get_ini_value(section, L"NetworkDevice", L"e1000"));
	strcpy_s(p->netmac, MAC_SZ, get_ini_value(section, L"NetworkMacAddr", L""));
	p->usb = get_ini_bool(section, L"Usb", nk_true);
	strcpy_s(p->usbctrl, OPT_SZ, get_ini_value(section, L"UsbController", L"usb-ehci"));
	p->usb_kbd = get_ini_bool(section, L"UsbKeyboard", nk_true);
	p->usb_tablet = get_ini_bool(section, L"UsbTablet", nk_true);
	p->usb_mouse = get_ini_bool(section, L"UsbMouse", nk_false);
	p->audio = get_ini_bool(section, L"Audio", nk_false);
	p->audio_hda = get_ini_bool(section, L"IntelHDA", nk_false);
	p->audio_spk = get_ini_bool(section, L"PcSpeaker", nk_false);
	strcpy_s(p->audiodev, OPT_SZ, get_ini_value(section, L"AudioBackend", L"dsound"));
	
	p->fw = get_ini_num(section, L"Firmware", fw);
	if (p->fw < fw_min || p->fw > fw_max)
		p->fw = fw;
	p->fw_menu = get_ini_bool(section, L"BootMenu", nk_false);
	num = get_ini_num(section, L"Timeout", 1);
	snprintf(p->fw_timeout, OPT_SZ, "%d", num);

	p->boot = get_ini_num(section, L"BootTarget", ZEMU_BOOT_VHD);
	if (p->boot < 0 || p->boot >= ZEMU_BOOT_MAX)
		p->boot = ZEMU_BOOT_VHD;
}

void
ui_ini_init(void)
{
	strcpy_s(nk.ini->qemu_dir, MAX_PATH, get_ini_value(L"Qemu", L"Dir", L"qemu"));
	nk.ini->qemu_arch = get_ini_num(L"Qemu", L"Arch", ZEMU_QEMU_ARCH_X64);
	if (nk.ini->qemu_arch >= ZEMU_QEMU_ARCH_MAX ||
		nk.ini->qemu_arch < 0)
		nk.ini->qemu_arch = ZEMU_QEMU_ARCH_X64;

	strcpy_s(nk.ini->qemu_name[ZEMU_QEMU_ARCH_X64], OPT_SZ, get_ini_value(L"Qemu", L"X64", L"qemu-system-x86_64w.exe"));
	strcpy_s(nk.ini->qemu_name[ZEMU_QEMU_ARCH_AA64], OPT_SZ, get_ini_value(L"Qemu", L"AA64", L"qemu-system-aarch64w.exe"));

	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_X64_EFI], MAX_PATH, get_ini_value(L"Qemu", L"X64_EFI", L"X64_EFI.qcow2"));
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_X86_BIOS], MAX_PATH, get_ini_value(L"Qemu", L"X86_BIOS", L"bios.qcow2"));
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_X86_EFI], MAX_PATH, get_ini_value(L"Qemu", L"X86_EFI", L"IA32_EFI.qcow2"));
	
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_AA64_EFI], MAX_PATH, get_ini_value(L"Qemu", L"AA64_EFI", L"AA64_EFI.qcow2"));
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_ARM32_EFI], MAX_PATH, get_ini_value(L"Qemu", L"ARM32_EFI", L"ARM_EFI.qcow2"));

	nk.ini->qemu_screenshot = get_ini_num(L"Qemu", L"Screenshot", ZEMU_SCREEN_TO_FILE);
	if (nk.ini->qemu_screenshot < 0 || nk.ini->qemu_screenshot >= ZEMU_SCREEN_TO_MAX)
		nk.ini->qemu_screenshot = ZEMU_SCREEN_TO_FILE;
	nk.ini->qemu_fullscreen = get_ini_bool(L"Qemu", L"FullScreen", nk_false);

	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_X64_EFI], OPT_SZ, get_ini_value(L"Wim", L"X64_EFI", L"wimldr.x64"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_X86_BIOS], OPT_SZ, get_ini_value(L"Wim", L"X86_BIOS", L"wimldr"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_X86_EFI], OPT_SZ, get_ini_value(L"Wim", L"X86_EFI", L"wimldr.ia32"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_AA64_EFI], OPT_SZ, get_ini_value(L"Wim", L"AA64_EFI", L"wimldr.aa64"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_ARM32_EFI], OPT_SZ, get_ini_value(L"Wim", L"ARM32_EFI", L"wimldr.arm"));
	strcpy_s(nk.ini->qemu_wimhda, OPT_SZ, get_ini_value(L"Wim", L"Hda", L"wim.qcow2"));
	strcpy_s(nk.ini->qemu_wimcpio, OPT_SZ, get_ini_value(L"Wim", L"Cpio", L"wim.cpio"));
	nk.ini->qemu_wimaddr = get_ini_num(L"Wim", L"Addr", 0x1000000); // address: 0x1000000 (16MB)
	nk.ini->boot_wim_index = get_ini_num(L"Wim", L"Index", 0);

	nk.ini->boot_vhd_attr.snapshot = get_ini_bool(L"Vhd", L"Snapshot", nk_true);
	nk.ini->boot_hd_attr.snapshot = get_ini_bool(L"Pd", L"Snapshot", nk_true);
	nk.ini->boot_vfd_attr.devif = ZEMU_DEV_IF_FLOPPY;
	nk.ini->boot_vfd_attr.snapshot = get_ini_bool(L"Vfd", L"Snapshot", nk_true);
	nk.ini->boot_dir_attr.snapshot = get_ini_bool(L"Vvfat", L"Snapshot", nk_true);
	nk.ini->net_http = get_ini_bool(L"Pxe", L"Http", nk_true);
	int num = get_ini_num(L"Pxe", L"HttpPort", 80);
	if (num <= 0 || num > 65535)
		num = 80;
	snprintf(nk.ini->net_http_port, OPT_SZ, "%d", num);

	nk.ini->cur = &nk.ini->profile[nk.ini->qemu_arch];
	get_profile(ZEMU_QEMU_ARCH_X64);
	get_profile(ZEMU_QEMU_ARCH_AA64);
}

void
set_profile(ZEMU_QEMU_ARCH arch)
{
	ZEMU_INI_PROFILE* p = &nk.ini->profile[arch];
	LPCWSTR section;
	switch (arch)
	{
	case ZEMU_QEMU_ARCH_X64:
		section = L"X86";
		break;
	case ZEMU_QEMU_ARCH_AA64:
		section = L"Arm";
		break;
	default:
		section = L"Unknown";
		break;
	}
	set_ini_value(section, L"Smp", p->smp);
	set_ini_value(section, L"Model", p->model);
	set_ini_value(section, L"Memory", p->mem);
	set_ini_value(section, L"Machine", p->machine);
	set_ini_num(section, L"KernelIrqchip", p->irqchip);
	set_ini_num(section, L"Virtualization", p->virt);
	set_ini_num(section, L"GuiWindow", p->graphics);
	set_ini_value(section, L"Display", p->vgadev);
	set_ini_num(section, L"Pflash", p->pflash);
	set_ini_num(section, L"Network", p->net);
	set_ini_value(section, L"NetworkDevice", p->netdev);
	if (p->netmac[0])
		set_ini_value(section, L"NetworkMacAddr", p->netmac);
	set_ini_num(section, L"Usb", p->usb);
	set_ini_value(section, L"UsbController", p->usbctrl);
	set_ini_num(section, L"UsbKeyboard", p->usb_kbd);
	set_ini_num(section, L"UsbTablet", p->usb_tablet);
	set_ini_num(section, L"UsbMouse", p->usb_mouse);
	set_ini_num(section, L"Audio", p->audio);
	set_ini_num(section, L"IntelHDA", p->audio_hda);
	set_ini_num(section, L"PcSpeaker", p->audio_spk);
	set_ini_value(section, L"AudioBackend", p->audiodev);
	set_ini_num(section, L"Firmware", p->fw);
	set_ini_num(section, L"BootMenu", p->fw_menu);
	set_ini_value(section, L"Timeout", p->fw_timeout);
	set_ini_num(section, L"BootTarget", p->boot);
}

void
ui_ini_save(void)
{
	set_ini_value(L"Qemu", L"Dir", nk.ini->qemu_dir);
	set_ini_num(L"Qemu", L"Arch", nk.ini->qemu_arch);
	set_ini_value(L"Qemu", L"X64", nk.ini->qemu_name[ZEMU_QEMU_ARCH_X64]);
	set_ini_value(L"Qemu", L"AA64", nk.ini->qemu_name[ZEMU_QEMU_ARCH_AA64]);

	set_ini_value(L"Qemu", L"X64_EFI", nk.ini->qemu_fw[ZEMU_FW_X64_EFI]);
	set_ini_value(L"Qemu", L"X86_BIOS", nk.ini->qemu_fw[ZEMU_FW_X86_BIOS]);
	set_ini_value(L"Qemu", L"X86_EFI", nk.ini->qemu_fw[ZEMU_FW_X86_EFI]);

	set_ini_value(L"Qemu", L"AA64_EFI", nk.ini->qemu_fw[ZEMU_FW_AA64_EFI]);
	set_ini_value(L"Qemu", L"ARM32_EFI", nk.ini->qemu_fw[ZEMU_FW_ARM32_EFI]);

	set_ini_num(L"Qemu", L"Screenshot", nk.ini->qemu_screenshot);
	set_ini_num(L"Qemu", L"FullScreen", nk.ini->qemu_fullscreen);

	set_ini_value(L"Wim", L"X64_EFI", nk.ini->qemu_wimldr[ZEMU_FW_X64_EFI]);
	set_ini_value(L"Wim", L"X86_BIOS", nk.ini->qemu_wimldr[ZEMU_FW_X86_BIOS]);
	set_ini_value(L"Wim", L"X86_EFI", nk.ini->qemu_wimldr[ZEMU_FW_X86_EFI]);
	set_ini_value(L"Wim", L"AA64_EFI", nk.ini->qemu_wimldr[ZEMU_FW_AA64_EFI]);
	set_ini_value(L"Wim", L"ARM32_EFI", nk.ini->qemu_wimldr[ZEMU_FW_ARM32_EFI]);
	set_ini_value(L"Wim", L"Hda", nk.ini->qemu_wimhda);
	set_ini_num(L"Wim", L"Index", nk.ini->boot_wim_index);

	set_ini_num(L"Vhd", L"Snapshot", nk.ini->boot_vhd_attr.snapshot);
	set_ini_num(L"Pd", L"Snapshot", nk.ini->boot_hd_attr.snapshot);
	set_ini_num(L"Vfd", L"Snapshot", nk.ini->boot_vfd_attr.snapshot);
	set_ini_num(L"Vvfat", L"Snapshot", nk.ini->boot_dir_attr.snapshot);
	set_ini_num(L"Pxe", L"Http", nk.ini->net_http);
	set_ini_value(L"Pxe", L"HttpPort", nk.ini->net_http_port);

	set_profile(ZEMU_QEMU_ARCH_X64);
	set_profile(ZEMU_QEMU_ARCH_AA64);
}

void
ui_dev_button(struct nk_context* ctx,
	struct nk_image img, const char* label, nk_bool* value)
{
	if (*value)
	{
		nk.button_style.normal = nk_style_item_color(nk.color[NK_COLOR_BUTTON]);
		nk.button_style.hover = nk_style_item_color(nk.color[NK_COLOR_WINDOW]);
		nk.button_style.active = nk_style_item_color(nk.color[NK_COLOR_WINDOW]);
	}
	else
	{
		nk.button_style.normal = nk_style_item_color(nk.color[NK_COLOR_WINDOW]);
		nk.button_style.hover = nk_style_item_color(nk.color[NK_COLOR_BUTTON]);
		nk.button_style.active = nk_style_item_color(nk.color[NK_COLOR_BUTTON]);
	}
	if (nk_button_image_styled(ctx, &nk.button_style, img))
		*value = !*value;
	nk_label(ctx, label, NK_TEXT_LEFT);
}

static UINT64 popup_tilck;
static const char* popup_msg;
static DWORD popup_icon = IDR_PNG_INFO;

void
ui_popup_window(struct nk_context* ctx, float width, float height)
{
	if (popup_tilck < nk.tilck || popup_msg == NULL)
		return;

	if (nk_begin_ex(ctx, "#POPUP_WIN",
		nk_rect(0.1f * width, 0.5f * height, 0.8f * width, 2.0f * nk.title_height),
		NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 0, 1);
		nk_image_label(ctx, GET_PNG(popup_icon), popup_msg);
	}

	nk_end(ctx);
}

void
ui_popup_msg(const char* msg, DWORD icon)
{
	popup_tilck = nk.tilck + 1;
	popup_msg = msg;
	popup_icon = icon;
}
