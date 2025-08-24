// SPDX-License-Identifier: GPL-3.0-or-later

#include "nkctx.h"
#include "ini.h"

#pragma pack(1)

typedef struct _SMBIOSHEADER_
{
	UINT8 Type;
	UINT8 Length;
	UINT16 Handle;
} SMBIOSHEADER;

enum SMBIOS_BATTERY_CHEMISTRY
{
	BAT_CHEM_OTHER = 0x01,
	BAT_CHEM_UNKNOWN = 0x02,
	BAT_CHEM_PB_ACID = 0x03,
	BAT_CHEM_NI_CD = 0x04,
	BAT_CHEM_NI_MH = 0x05,
	BAT_CHEM_LI_ION = 0x06,
	BAT_CHEM_ZN_AIR = 0x07,
	BAT_CHEM_LI_PO = 0x08,
};

typedef struct _SMBIOS_TYPE_22
{
	SMBIOSHEADER Header;
	UINT8 Location; // String
	UINT8 Manufacturer; // String
	UINT8 Date; // String
	UINT8 SN; // String
	UINT8 DeviceName; // String
	UINT8 DeviceChemistry; // SMBIOS_BATTERY_CHEMISTRY
	UINT16 DesignCapacity; // mW*h
	UINT16 DesignVoltage; // mV
	UINT8 SBDSVer; // String, 0
	UINT8 MaxErr; // Percent
#if 0 // V2.2+
	UINT16 SBDSSerial;
	UINT16 SBDSManufactureDate;
	UINT8 SBDSChemistry;
	UINT8 DesignCapacityMultiplier;
	UINT32 OEMInfo;
#endif
} SMBIOS_BATTERY;

typedef struct _SMBIOS_TYPE_27
{
	SMBIOSHEADER Header;
	UINT16 TempProbeHandle; // 0xFF
	UINT8 DeviceTypeStatus; // 0x65 -> 011-00101 (OK,ChipFan)
	UINT8 CoolingUnitGroup; // 0
	UINT32 OEMDefined; // 0
	UINT16 NominalSpeed; // RPM, 0x8000 -> Unknown/Non-rotating
#if 0 // V2.7+
	UINT8 Description;
#endif
} SMBIOS_COOLING;

typedef struct _ACPI_HEADER
{
	CHAR Signature[4];
	UINT32 Length;
	UINT8 Revision;
	UINT8 Checksum;
	CHAR OemId[6];
	CHAR OemTableId[8];
	UINT32 OemRevision;
	CHAR CreatorId[4];
	UINT32 CreatorRevision;
} ACPI_HEADER;

#pragma pack()

BOOL
write_battery_dmi(LPCWSTR dmi)
{
	SMBIOS_BATTERY bat = { 0 };
	bat.Header.Type = 22;
	bat.Header.Length = sizeof(SMBIOS_BATTERY);
	bat.Header.Handle = 0x1453;
	bat.Location = 1;
	const char* string1 = "Internal";
	bat.Manufacturer = 2;
	const char* string2 = "A1ive";
	bat.Date = 3;
	const char* string3 = "2024/01/01";
	bat.SN = 4;
	const char* string4 = "1453-0529";
	bat.DeviceName = 5;
	const char* string5 = "ZEMU-BAT01";
	bat.DeviceChemistry = BAT_CHEM_LI_ION;
	bat.DesignCapacity = 50000;
	bat.DesignVoltage = 10000;
	bat.SBDSVer = 0;
	bat.MaxErr = 5;

	BOOL rc = FALSE;
	FILE* fp = NULL;
	errno_t err = _wfopen_s(&fp, dmi, L"wb");
	if (err != 0 || !fp)
		return FALSE;
	if (fwrite(&bat, sizeof(bat), 1, fp) != 1)
		goto out;
	if (fwrite(string1, strlen(string1) + 1, 1, fp) != 1)
		goto out;
	if (fwrite(string2, strlen(string2) + 1, 1, fp) != 1)
		goto out;
	if (fwrite(string3, strlen(string3) + 1, 1, fp) != 1)
		goto out;
	if (fwrite(string4, strlen(string4) + 1, 1, fp) != 1)
		goto out;
	if (fwrite(string5, strlen(string5) + 1, 1, fp) != 1)
		goto out;
	if (fwrite("\0", 1, 1, fp) != 1)
		goto out;
	rc = TRUE;
out:
	fclose(fp);
	return rc;
}

/*
	DefinitionBlock ("bat.aml", "SSDT", 2, "ZEMU", "ZEMU_BAT", 0x00000001)
	{
		Scope (_SB)
		{
			Device (BAT0)
			{
				Name (_HID, "PNP0C0A")

				Method (_STA, 0, NotSerialized)
				{
					Return (0x0F)
				}

				Name (BIFP, Package (13)
				{
					0, // [0] Power Unit: 0 for mWh, 1 for mAh
					50000, // [1] Design Capacity
					50000, // [2] Last Full Charge Capacity
					1, // [3] Battery Technology: 1 for rechargeable
					10000, // [4] Design Voltage (mV)
					2500, // [5] Design Capacity of Warning: 5%
					1000, // [6] Design Capacity of Low£º2%
					1, // [7] Battery Capacity Granularity 1 (Low to Warning)
					1, // [8] Battery Capacity Granularity 2 (Warning to Full)
					"ZEMU-BAT01", // [9] Model Number (String)
					"1453-0529", // [10] Serial Number (String)
					"Li-Ion", // [11] Battery Type (String)
					"A1ive" // [12] OEM Information (String)
				})

				Method (_BIF, 0, Serialized)
				{
					Return (BIFP)
				}

				Name (BSTP, Package (4)
				{
					0x00, // [0] Battery State (Bitfield)
					0, // [1] Battery Present Rate (mW)
					50000, // [2] Remaining Capacity (mWh)
					10000 // [3] Battery Present Voltage (mV)
				})

				Method (_BST, 0, Serialized)
				{
					Return (BSTP)
				}
			}
			Device (AC0)
			{
				Name (_HID, "ACPI0003") // AC adapter
				Name (_UID, 0)

				Method (_STA, 0, NotSerialized)
				{
					Return (0x0F)
				}

				Method (_PSR, 0, NotSerialized)
				{
					Return (0x01) // 0x00: Offline, 0x01: Online
				}
			}
		}
	}
*/
static UINT8 BAT_AML[] =
{
	0x53, 0x53, 0x44, 0x54, // "SSDT"
	0xE3, 0x00, 0x00, 0x00, // Length
	0x02, // Revision
	0x4C, // Checksum
	0x5A, 0x45, 0x4D, 0x55, 0x00, 0x00, // OemId
	0x5A, 0x45, 0x4D, 0x55, 0x5F, 0x42, 0x41, 0x54, // OemTableId
	0x01, 0x00, 0x00, 0x00, // OemRevision
	0x49, 0x4E, 0x54, 0x4C, // CreatorId
	0x07, 0x08, 0x25, 0x20, // CreatorRevision
	0x10, 0x4E, 0x0B, 0x5F, 0x53, 0x42, 0x5F, // Scope (_SB)
	0x5B, 0x82, 0x47, 0x08, 0x42, 0x41, 0x54, 0x30, // Device (BAT0)
	0x08, 0x5F, 0x48, 0x49, 0x44, // Name (_HID)
	0x0D, 0x50, 0x4E, 0x50, 0x30, 0x43, 0x30, 0x41, 0x00, // "PNP0C0A"
	0x14, 0x09, 0x5F, 0x53, 0x54, 0x41, // Method (_STA)
	0x00, 0xA4, 0x0A, 0x0F, // NotSerialized, Return (0x0F)
	0x08, 0x42, 0x49, 0x46, 0x50, // Name (BIFP)
	0x12, 0x3B, 0x0D, // Package (13)
	0x00,
	0x0B, 0x50, 0xC3, // 50000
	0x0B, 0x50, 0xC3, // 50000
	0x01,
	0x0B, 0x10, 0x27, // 10000
	0x0B, 0xC4, 0x09, // 2500
	0x0B, 0xE8, 0x03, // 1000
	0x01, 0x01,
	0x0D, 0x5A, 0x45, 0x4D, 0x55, 0x2D, 0x42, 0x41, 0x54, 0x30, 0x31, 0x00, // "ZEMU-BAT01"
	0x0D, 0x31, 0x34, 0x35, 0x33, 0x2D, 0x30, 0x35, 0x32, 0x39, 0x00, // "1453-0529"
	0x0D, 0x4C, 0x69, 0x2D, 0x49, 0x6F, 0x6E, 0x00, // "Li-Ion"
	0x0D, 0x41, 0x31, 0x69, 0x76, 0x65, 0x00, // "A1ive"
	0x14, 0x0B, 0x5F, 0x42, 0x49, 0x46, // Method (_BIF)
	0x08, 0xA4, 0x42, 0x49, 0x46, 0x50, // Serialized, Return (BIFP)
	0x08, 0x42, 0x53, 0x54, 0x50, // Name (BSTP)
	0x12, 0x0A, 0x04, // Package (4)
	0x00, 0x00,
	0x0B, 0x50, 0xC3, // 50000
	0x0B, 0x10, 0x27, // 10000
	0x14, 0x0B, 0x5F, 0x42, 0x53, 0x54, // Method (_BST)
	0x08, 0xA4, 0x42, 0x53, 0x54, 0x50, // Serialized, Return (BSTP)
	0x5B, 0x82, 0x2D, 0x41, 0x43, 0x30, 0x5F, // Device (AC0)
	0x08, 0x5F, 0x48, 0x49, 0x44, // Name (_HID)
	0x0D, 0x41, 0x43, 0x50, 0x49, 0x30, 0x30, 0x30, 0x33, 0x00, // "ACPI0003"
	0x08, 0x5F, 0x55, 0x49, 0x44, 0x00, // Name (_UID, 0)
	0x14, 0x09, 0x5F, 0x53, 0x54, 0x41, // Method (_STA)
	0x00, 0xA4, 0x0A, 0x0F, // NotSerialized, Return (0x0F)
	0x14, 0x08, 0x5F, 0x50, 0x53, 0x52, // Method (_PSR)
	0x00, 0xA4, 0x01 // NotSerialized, Return (0x01)
};

BOOL write_battery_aml(LPCWSTR aml)
{
	BOOL rc = FALSE;
	FILE* fp = NULL;
	errno_t err = _wfopen_s(&fp, aml, L"wb");
	if (err != 0 || !fp)
		return FALSE;
	if (fwrite(BAT_AML, sizeof(BAT_AML), 1, fp) != 1)
		goto out;
	rc = TRUE;
out:
	fclose(fp);
	return rc;
}

