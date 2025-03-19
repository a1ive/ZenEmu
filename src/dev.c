// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "dev.h"

#include <stdio.h>
#include <setupapi.h>
#include <winioctl.h>

static inline LPCSTR
get_bus_name(STORAGE_BUS_TYPE bus)
{
	switch (bus)
	{
	case BusTypeUnknown: return "Unknown";
	case BusTypeScsi: return "SCSI";
	case BusTypeAtapi: return "Atapi";
	case BusTypeAta: return "ATA";
	case BusType1394: return "1394";
	case BusTypeSsa: return "SSA";
	case BusTypeFibre: return "Fibre";
	case BusTypeUsb: return "USB";
	case BusTypeRAID: return "RAID";
	case BusTypeiScsi: return "iSCSI";
	case BusTypeSas: return "SAS";
	case BusTypeSata: return "SATA";
	case BusTypeSd: return "SD";
	case BusTypeMmc: return "MMC";
	case BusTypeVirtual: return "Virtual";
	case BusTypeFileBackedVirtual: return "File";
	case BusTypeSpaces: return "Spaces";
	case BusTypeNvme: return "NVMe";
	case BusTypeSCM: return "SCM";
	case BusTypeUfs: return "UFS";
	}
	return "Unknown";
}

static UINT64
get_disk_size(HANDLE hd)
{
	DWORD dw;
	UINT64 size = 0;
	GET_LENGTH_INFORMATION LengthInfo = { 0 };
	if (DeviceIoControl(hd, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0,
		&LengthInfo, sizeof(LengthInfo), &dw, NULL))
		size = LengthInfo.Length.QuadPart;
	return size;
}

static HANDLE
get_disk_handle(BOOL is_cd, DWORD index)
{
	WCHAR path[28]; // L"\\\\.\\PhysicalDrive4294967295"
	if (is_cd)
		swprintf(path, 28, L"\\\\.\\CdRom%u", index);
	else
		swprintf(path, 28, L"\\\\.\\PhysicalDrive%u", index);
	return CreateFileW(path, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
}

static VOID
trim_str(CHAR* str)
{
	char* p1 = str;
	char* p2 = str;
	size_t len = strlen(str);
	while (len > 0)
	{
		if (!isblank(str[len - 1]))
			break;
		str[len - 1] = '\0';
		len--;
	}
	while (isblank(*p1))
		p1++;
	while (*p1)
	{
		if (!isprint(*p1))
			*p1 = '?';
		*p2++ = *p1++;
	}
	*p2++ = 0;
}

static int __cdecl
compare_disk_id(const void* a, const void* b)
{
	return ((int)((const PHY_DRIVE_INFO*)a)->index) - ((int)((const PHY_DRIVE_INFO*)b)->index);
}

typedef struct
{
	DWORD  cbSize;
	WCHAR  DevicePath[512];
} MY_DEVIF_DETAIL_DATA;

DWORD
get_disk_list(BOOL is_cd, PHY_DRIVE_INFO** drive_list)
{
	DWORD i;
	BOOL rc;
	DWORD ret_bytes;
	PHY_DRIVE_INFO* info;

	DWORD count = 0;
	SP_DEVICE_INTERFACE_DATA if_data = { .cbSize = sizeof(SP_DEVICE_INTERFACE_DATA) };
	MY_DEVIF_DETAIL_DATA my_data = { .cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W) };
	SP_DEVINFO_DATA devinfo_data = { .cbSize = sizeof(SP_DEVINFO_DATA) };
	GUID dev_guid = is_cd ? GUID_DEVINTERFACE_CDROM : GUID_DEVINTERFACE_DISK;
	HDEVINFO dev_info_handle = SetupDiGetClassDevsW(&dev_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (dev_info_handle == INVALID_HANDLE_VALUE)
		return 0;

	while (SetupDiEnumDeviceInterfaces(dev_info_handle, NULL, &dev_guid, count, &if_data))
		count++;
	if (count == 0)
	{
		SetupDiDestroyDeviceInfoList(dev_info_handle);
		return 0;
	}

	*drive_list = calloc(count, sizeof(PHY_DRIVE_INFO));
	if (!*drive_list)
	{
		SetupDiDestroyDeviceInfoList(dev_info_handle);
		return 0;
	}

	info = *drive_list;

	for (i = 0; i < count; i++)
	{
		HANDLE hd = INVALID_HANDLE_VALUE;
		STORAGE_PROPERTY_QUERY spq = { 0 };
		STORAGE_DESCRIPTOR_HEADER sdh = { 0 };
		STORAGE_DEVICE_DESCRIPTOR* sdd = NULL;
		STORAGE_DEVICE_NUMBER sdn;
		HANDLE if_dev_handle;

		info[i].index = (DWORD)-1;
		info[i].prefix = is_cd ? "CD" : "HD";
		info[i].bus = "Unknown";

		if (!SetupDiEnumDeviceInterfaces(dev_info_handle, NULL, &dev_guid, (DWORD)i, &if_data))
			goto next_drive;
		if (!SetupDiGetDeviceInterfaceDetailW(dev_info_handle, &if_data, (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)&my_data,
			sizeof(MY_DEVIF_DETAIL_DATA), NULL, &devinfo_data))
			goto next_drive;
		if_dev_handle = CreateFileW(my_data.DevicePath,
			GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (if_dev_handle == INVALID_HANDLE_VALUE || if_dev_handle == NULL)
			goto next_drive;
		rc = DeviceIoControl(if_dev_handle, IOCTL_STORAGE_GET_DEVICE_NUMBER,
			NULL, 0, &sdn, (DWORD)(sizeof(STORAGE_DEVICE_NUMBER)),
			&ret_bytes, NULL);
		CloseHandle(if_dev_handle);
		if (rc == FALSE)
			goto next_drive;
		info[i].index = sdn.DeviceNumber;

		hd = get_disk_handle(is_cd, info[i].index);

		if (!hd || hd == INVALID_HANDLE_VALUE)
			goto next_drive;

		spq.PropertyId = StorageDeviceProperty;
		spq.QueryType = PropertyStandardQuery;

		rc = DeviceIoControl(hd, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq),
			&sdh, sizeof(STORAGE_DESCRIPTOR_HEADER), &ret_bytes, NULL);
		if (!rc || sdh.Size < sizeof(STORAGE_DEVICE_DESCRIPTOR))
			goto next_drive;

		sdd = (STORAGE_DEVICE_DESCRIPTOR*)malloc(sdh.Size);
		if (!sdd)
			goto next_drive;

		rc = DeviceIoControl(hd, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq),
			sdd, sdh.Size, &ret_bytes, NULL);
		if (!rc)
			goto next_drive;

		info[i].size = get_disk_size(hd);
		if (!is_cd && sdd->RemovableMedia)
			info[i].prefix = "RM";
		info[i].bus = get_bus_name(sdd->BusType);

		if (sdd->VendorIdOffset)
		{
			strcpy_s(info[i].hw, MAX_PATH,
				(char*)sdd + sdd->VendorIdOffset);
			trim_str(info[i].hw);
		}

		if (sdd->ProductIdOffset)
		{
			size_t len = strlen(info[i].hw);
			if (len && len < MAX_PATH - 1)
				info[i].hw[len++] = ' ';
			strcpy_s(info[i].hw + len, MAX_PATH - len,
				(char*)sdd + sdd->ProductIdOffset);
			trim_str(info[i].hw);
		}

	next_drive:
		if (sdd)
			free(sdd);
		if (hd && hd != INVALID_HANDLE_VALUE)
			CloseHandle(hd);
	}
	SetupDiDestroyDeviceInfoList(dev_info_handle);

	qsort(info, count, sizeof(PHY_DRIVE_INFO), compare_disk_id);
	for (i = 0; i < count; i++)
	{
		if (info[i].index == (DWORD)-1)
			break;
		snprintf(info[i].text, MAX_PATH, "%s%lu:%s %s (%s)",
			info[i].prefix, info[i].index, info[i].bus, info[i].hw,
			get_human_size(info[i].size, human_units, 1024));

	}
	return i;
}
