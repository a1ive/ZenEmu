// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "ui.h"

static inline nk_bool
get_ini_bool(LPCWSTR section, LPCWSTR key, nk_bool fallback)
{
	int value = get_ini_num(section, key, (int)fallback);
	return value ? nk_true : nk_false;
}

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
		machine = L"pc";
		display = L"vmware-svga";
		fw = ZEMU_FW_X64_EFI;
		fw_min = ZEMU_FW_X86_MIN;
		fw_max = ZEMU_FW_X86_MAX;
		break;
	case ZEMU_QEMU_ARCH_AA64:
		section = L"Arm";
		model = L"cortex-a72";
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
	strcpy_s(p->vga, OPT_SZ, get_ini_value(section, L"Display", display));
	p->pflash = get_ini_bool(section, L"Pflash", nk_false);
	strcpy_s(p->usb, OPT_SZ, get_ini_value(section, L"Usb", L"usb-ehci"));
	p->usb_kbd = get_ini_bool(section, L"UsbKeyboard", nk_true);
	p->usb_tablet = get_ini_bool(section, L"UsbTablet", nk_true);
	p->usb_mouse = get_ini_bool(section, L"UsbMouse", nk_false);
	
	p->fw = get_ini_num(section, L"Firmware", fw);
	if (p->fw < fw_min || p->fw > fw_max)
		p->fw = fw;

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

	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_X64_EFI], MAX_PATH, get_ini_value(L"Qemu", L"X64_EFI", L"X64_EFI.fd"));
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_X86_BIOS], MAX_PATH, get_ini_value(L"Qemu", L"X86_BIOS", L"bios.bin"));
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_X86_EFI], MAX_PATH, get_ini_value(L"Qemu", L"X86_EFI", L"IA32_EFI.fd"));
	
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_AA64_EFI], MAX_PATH, get_ini_value(L"Qemu", L"AA64_EFI", L"AA64_EFI.fd"));
	strcpy_s(nk.ini->qemu_fw[ZEMU_FW_ARM32_EFI], MAX_PATH, get_ini_value(L"Qemu", L"ARM32_EFI", L"ARM_EFI.fd"));

	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_X64_EFI], OPT_SZ, get_ini_value(L"Wim", L"X64_EFI", L"wimldr.x64"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_X86_BIOS], OPT_SZ, get_ini_value(L"Wim", L"X86_BIOS", L"grub.exe"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_X86_EFI], OPT_SZ, get_ini_value(L"Wim", L"X86_EFI", L"wimldr.ia32"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_AA64_EFI], OPT_SZ, get_ini_value(L"Wim", L"AA64_EFI", L"wimldr.aa64"));
	strcpy_s(nk.ini->qemu_wimldr[ZEMU_FW_ARM32_EFI], OPT_SZ, get_ini_value(L"Wim", L"ARM32_EFI", L"wimldr.arm"));
	strcpy_s(nk.ini->qemu_wimhda, OPT_SZ, get_ini_value(L"Wim", L"Hda", L"wim.qcow2"));

	nk.ini->boot_vhd_attr.snapshot = get_ini_bool(L"Vhd", L"Snapshot", nk_false);
	nk.ini->boot_hd_attr.snapshot = get_ini_bool(L"Pd", L"Snapshot", nk_true);

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
	set_ini_value(section, L"Display", p->vga);
	set_ini_num(section, L"Pflash", p->pflash);
	set_ini_value(section, L"Usb", p->usb);
	set_ini_num(section, L"UsbKeyboard", p->usb_kbd);
	set_ini_num(section, L"UsbTablet", p->usb_tablet);
	set_ini_num(section, L"UsbMouse", p->usb_mouse);
	set_ini_num(section, L"Firmware", p->fw);
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

	set_ini_value(L"Wim", L"X64_EFI", nk.ini->qemu_wimldr[ZEMU_FW_X64_EFI]);
	set_ini_value(L"Wim", L"X86_BIOS", nk.ini->qemu_wimldr[ZEMU_FW_X86_BIOS]);
	set_ini_value(L"Wim", L"X86_EFI", nk.ini->qemu_wimldr[ZEMU_FW_X86_EFI]);
	set_ini_value(L"Wim", L"AA64_EFI", nk.ini->qemu_wimldr[ZEMU_FW_AA64_EFI]);
	set_ini_value(L"Wim", L"ARM32_EFI", nk.ini->qemu_wimldr[ZEMU_FW_ARM32_EFI]);
	set_ini_value(L"Wim", L"Hda", nk.ini->qemu_wimhda);

	set_ini_num(L"Vhd", L"Snapshot", nk.ini->boot_vhd_attr.snapshot);
	set_ini_num(L"Pd", L"Snapshot", nk.ini->boot_hd_attr.snapshot);

	set_profile(ZEMU_QEMU_ARCH_X64);
	set_profile(ZEMU_QEMU_ARCH_AA64);
}
