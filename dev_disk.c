// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"
#include "dev.h"

#include <stdio.h>
#include <setupapi.h>
#include <winioctl.h>

typedef struct _PHY_DRIVE_INFO
{
	DWORD index;
	PARTITION_STYLE partmap; // 0:MBR 1:GPT 2:RAW
	UINT64 size;
	CHAR prefix[3];
	CHAR name[MAX_PATH];
	STORAGE_BUS_TYPE bus;
	UINT32 mnt;
}PHY_DRIVE_INFO;

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

#define LAYOUT_BUFSZ 0x10000
static CHAR static_layout[LAYOUT_BUFSZ];
static PARTITION_STYLE
get_disk_partmap(HANDLE hd)
{
	DRIVE_LAYOUT_INFORMATION_EX* pLayout = (DRIVE_LAYOUT_INFORMATION_EX*)static_layout;
	DWORD dw = LAYOUT_BUFSZ;
	if (!DeviceIoControl(hd, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0,
		pLayout, dw, &dw, NULL))
		goto fail;
	if (dw < sizeof(DRIVE_LAYOUT_INFORMATION_EX) - sizeof(PARTITION_INFORMATION_EX))
		goto fail;
	return pLayout->PartitionStyle;
fail:
	return PARTITION_STYLE_RAW;
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

static size_t
get_info_list(BOOL is_cd, PHY_DRIVE_INFO** drive_list)
{
	size_t i;
	BOOL rc;
	DWORD ret_bytes;
	PHY_DRIVE_INFO* info;

	size_t count = 0;
	SP_DEVICE_INTERFACE_DATA if_data = { .cbSize = sizeof(SP_DEVICE_INTERFACE_DATA) };
	MY_DEVIF_DETAIL_DATA my_data = { .cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W) };
	SP_DEVINFO_DATA devinfo_data = { .cbSize = sizeof(SP_DEVINFO_DATA) };
	GUID dev_guid = is_cd ? GUID_DEVINTERFACE_CDROM : GUID_DEVINTERFACE_DISK;
	HDEVINFO dev_info_handle = SetupDiGetClassDevsW(&dev_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (dev_info_handle == INVALID_HANDLE_VALUE)
		return 0;

	while (SetupDiEnumDeviceInterfaces(dev_info_handle, NULL, &dev_guid, (DWORD)count, &if_data))
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
		if (is_cd)
			strcpy_s(info[i].prefix, 3, "CD");
		else
			strcpy_s(info[i].prefix, 3, sdd->RemovableMedia ? "RM" : "HD");
		info[i].bus = sdd->BusType;

		if (sdd->VendorIdOffset)
		{
			strcpy_s(info[i].name, MAX_PATH,
				(char*)sdd + sdd->VendorIdOffset);
			trim_str(info[i].name);
		}

		if (sdd->ProductIdOffset)
		{
			size_t len = strlen(info[i].name);
			if (len && len < MAX_PATH - 1)
				info[i].name[len++] = ' ';
			strcpy_s(info[i].name + len, MAX_PATH - len,
				(char*)sdd + sdd->ProductIdOffset);
			trim_str(info[i].name);
		}

		info[i].partmap = get_disk_partmap(hd);

	next_drive:
		if (sdd)
			free(sdd);
		if (hd && hd != INVALID_HANDLE_VALUE)
			CloseHandle(hd);
	}
	SetupDiDestroyDeviceInfoList(dev_info_handle);

	qsort(info, count, sizeof(PHY_DRIVE_INFO), compare_disk_id);
	return count;
}

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

CHAR**
get_disk_list(BOOL is_cd, size_t* count)
{
	size_t i;
	CHAR** ret = NULL;
	PHY_DRIVE_INFO* pd_info;
	*count = get_info_list(is_cd, &pd_info);
	if (*count == 0)
		return NULL;
	ret = calloc(*count, sizeof(CHAR*));
	if (ret == NULL)
	{
		*count = 0;
		free(pd_info);
		return NULL;
	}

	for (i = 0; i < *count; i++)
	{
		ret[i] = malloc(MAX_PATH);
		if (ret[i] == NULL)
			continue;
		snprintf(ret[i], MAX_PATH, "%s%lu:%s %s (%s)",
			pd_info[i].prefix, pd_info[i].index, get_bus_name(pd_info[i].bus),
			pd_info[i].name,
			get_human_size(pd_info[i].size, human_units, 1024));
	}
	free(pd_info);
	return ret;
}

VOID
free_disk_list(CHAR** disk_list, size_t count)
{
	size_t i;
	for (i = 0; i < count; i++)
	{
		free(disk_list[i]);
	}
	free(disk_list);
}
