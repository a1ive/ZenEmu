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

static UINT8
get_acpi_checksum(void* base, size_t size)
{
	UINT8* ptr;
	UINT8 ret = 0;
	for (ptr = (UINT8*)base; ptr < ((UINT8*)base) + size; ptr++)
		ret += *ptr;
	return ret;
}

#define AML_NOTSERIALIZED 0
#define AML_SERIALIZED 1

typedef struct
{
	uint8_t* buffer;
	size_t max;
	size_t pos;
} AML_BUFFER;

static inline void aml_init_buffer(AML_BUFFER* ctx, uint8_t* buffer, size_t size)
{
	ctx->buffer = buffer;
	ctx->max = size;
	ctx->pos = 0;
}

static inline size_t aml_get_size(const AML_BUFFER* ctx)
{
	return ctx->pos;
}

static inline int aml_write_byte(AML_BUFFER* ctx, uint8_t val)
{
	if (ctx->pos >= ctx->max)
	{
		assert(!"AML buffer overflow");
		return 0;
	}
	ctx->buffer[ctx->pos++] = val;
	return 1;
}

static inline int aml_write_data(AML_BUFFER* ctx, const void* data, size_t len)
{
	if (ctx->pos + len > ctx->max)
	{
		assert(!"AML buffer overflow");
		return 0;
	}
	memcpy(ctx->buffer + ctx->pos, data, len);
	ctx->pos += len;
	return 1;
}

static void aml_write_nameseg(AML_BUFFER* ctx, const char* seg)
{
	char padded_name[4] = { '_', '_', '_', '_' };
	size_t len = strlen(seg);
	if (len > 4)
		len = 4;
	memcpy(padded_name, seg, len);
	aml_write_data(ctx, padded_name, 4);
}

static void aml_write_namestring(AML_BUFFER* ctx, const char* name)
{
	// '\', '^' or '.' are not supported
	size_t len = strlen(name);
	const char* p = name;
	size_t num_segs = (len + 3) / 4;

	if (num_segs == 1)
		aml_write_nameseg(ctx, p);
	else if (num_segs == 2)
	{
		aml_write_byte(ctx, 0x2E); // DualNamePrefix
		aml_write_nameseg(ctx, p);
		aml_write_nameseg(ctx, p + 4);
	}
	else
	{
		if (num_segs > 255)
		{
			assert(!"Name is too long, exceeds 255 segments");
			num_segs = 255;
		}
		aml_write_byte(ctx, 0x2F); // MultiNamePrefix
		aml_write_byte(ctx, (uint8_t)num_segs);
		for (size_t i = 0; i < num_segs; ++i)
			aml_write_nameseg(ctx, p + i * 4);
	}
}

static void aml_int(AML_BUFFER* ctx, uint64_t value)
{
	if (value == 0)
		aml_write_byte(ctx, 0x00); // ZeroOp
	else if (value == 1)
		aml_write_byte(ctx, 0x01); // OneOp
	else if (value <= 0xFF)
	{
		aml_write_byte(ctx, 0x0A); // BytePrefix
		aml_write_data(ctx, &value, 1);
	}
	else if (value <= 0xFFFF)
	{
		aml_write_byte(ctx, 0x0B); // WordPrefix
		aml_write_data(ctx, &value, 2);
	}
	else if (value <= 0xFFFFFFFF)
	{
		aml_write_byte(ctx, 0x0C); // DWordPrefix
		aml_write_data(ctx, &value, 4);
	}
	else
	{
		aml_write_byte(ctx, 0x0E); // QWordPrefix
		aml_write_data(ctx, &value, 8);
	}
}

static size_t aml_encode_pkg_length(size_t length, uint8_t out_buf[4])
{
	if (length < 64)
	{
		// 1 byte, [5:0] = length
		out_buf[0] = (uint8_t)length;
		return 1;
	}
	else if (length < 4096)
	{
		// 2 bytes, [3:0] of byte 0 = L1, [7:4]=1
		out_buf[0] = (uint8_t)(((length & 0x0F) | 0x40));
		out_buf[1] = (uint8_t)(length >> 4);
		return 2;
	}
	else if (length < 2097152)
	{
		// 3 bytes, [3:0] of byte 0 = L1, [7:4]=2
		out_buf[0] = (uint8_t)(((length & 0x0F) | 0x80));
		out_buf[1] = (uint8_t)(length >> 4);
		out_buf[2] = (uint8_t)(length >> 12);
		return 3;
	}
	else
	{
		// 4 bytes, [3:0] of byte 0 = L1, [7:4]=3
		out_buf[0] = (uint8_t)(((length & 0x0F) | 0xC0));
		out_buf[1] = (uint8_t)(length >> 4);
		out_buf[2] = (uint8_t)(length >> 12);
		out_buf[3] = (uint8_t)(length >> 20);
		return 4;
	}
}

static void aml_finalize_package(AML_BUFFER* ctx, size_t package_start_pos, int is_ext_op)
{
	size_t content_end_pos = ctx->pos;
	size_t content_len = content_end_pos - package_start_pos;

	uint8_t pkg_len_bytes[4];
	size_t pkg_len_size = aml_encode_pkg_length(content_len, pkg_len_bytes);

	size_t move_size = content_len;
	if (ctx->pos + pkg_len_size > ctx->max)
	{
		assert(!"AML buffer overflow while finalizing package");
		return;
	}

	memmove(ctx->buffer + package_start_pos + pkg_len_size,
		ctx->buffer + package_start_pos,
		move_size);

	memcpy(ctx->buffer + package_start_pos, pkg_len_bytes, pkg_len_size);

	ctx->pos += pkg_len_size;

	size_t total_package_len = ctx->pos - package_start_pos + (is_ext_op ? 2 : 1);

	size_t final_content_len = content_len + pkg_len_size;
	pkg_len_size = aml_encode_pkg_length(final_content_len, pkg_len_bytes);

	memcpy(ctx->buffer + package_start_pos, pkg_len_bytes, pkg_len_size);
}

static size_t aml_start_scope(AML_BUFFER* ctx, const char* name)
{
	aml_write_byte(ctx, 0x10); // ScopeOp
	size_t pkg_start = ctx->pos;
	aml_write_namestring(ctx, name);
	return pkg_start;
}

static void aml_end_scope(AML_BUFFER* ctx, size_t pkg_start_pos)
{
	aml_finalize_package(ctx, pkg_start_pos, 0);
}

static size_t aml_start_device(AML_BUFFER* ctx, const char* name)
{
	aml_write_byte(ctx, 0x5B); // ExtOpPrefix
	aml_write_byte(ctx, 0x82); // DeviceOp
	size_t pkg_start = ctx->pos;
	aml_write_namestring(ctx, name);
	return pkg_start;
}

static void aml_end_device(AML_BUFFER* ctx, size_t pkg_start_pos)
{
	aml_finalize_package(ctx, pkg_start_pos, 1);
}

static void aml_name_decl(AML_BUFFER* ctx, const char* name)
{
	aml_write_byte(ctx, 0x08); // NameOp
	aml_write_namestring(ctx, name);
}

static size_t aml_start_method(AML_BUFFER* ctx, const char* name, int arg_count, int sflag)
{
	aml_write_byte(ctx, 0x14); // MethodOp
	size_t pkg_start = ctx->pos;

	uint8_t method_flags = (uint8_t)(arg_count & 0x07);
	if (sflag == AML_SERIALIZED)
		method_flags |= (1 << 3);

	aml_write_namestring(ctx, name);
	aml_write_byte(ctx, method_flags);
	return pkg_start;
}

static void aml_end_method(AML_BUFFER* ctx, size_t pkg_start_pos)
{
	aml_finalize_package(ctx, pkg_start_pos, 0);
}

static void aml_return(AML_BUFFER* ctx)
{
	aml_write_byte(ctx, 0xA4); // ReturnOp
}

static size_t aml_start_package(AML_BUFFER* ctx, uint8_t num_elements)
{
	aml_write_byte(ctx, 0x12); // PackageOp
	size_t pkg_start = ctx->pos;
	aml_write_byte(ctx, num_elements);
	return pkg_start;
}

static void aml_end_package(AML_BUFFER* ctx, size_t pkg_start_pos)
{
	aml_finalize_package(ctx, pkg_start_pos, 0);
}

static void aml_string(AML_BUFFER* ctx, const char* str)
{
	aml_write_byte(ctx, 0x0D); // StringPrefix
	aml_write_data(ctx, str, strlen(str) + 1); // including null terminator
}

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
	const char* string2 = "ZEMU";
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
	DefinitionBlock ("bat.aml", "SSDT", 2, "ZEMU", "ZEMUBAT", 0x00000001)
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
					"BAT01", // [9] Model Number (String)
					"14530529", // [10] Serial Number (String)
					"Li-Ion", // [11] Battery Type (String)
					"A1ive" // [12] OEM Information (String)
				})

				Method (_BIF, 0, NotSerialized)
				{
					Return (BIFP)
				}

				Name (BSTP, Package (4)
				{
					0x02, // [0] Battery State (Bitfield)
					0x10, // [1] Battery Present Rate (mW)
					50000, // [2] Remaining Capacity (mWh)
					10000 // [3] Battery Present Voltage (mV)
				})

				Method (_BST, 0, NotSerialized)
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

#define MAX_AML_SZ 512

BOOL write_battery_aml(LPCWSTR aml)
{
	UINT8 BAT_AML[MAX_AML_SZ] = { 0 };

	AML_BUFFER ctx;
	aml_init_buffer(&ctx, BAT_AML + sizeof(ACPI_HEADER), MAX_AML_SZ - sizeof(ACPI_HEADER));

	size_t scope_sb = aml_start_scope(&ctx, "_SB");
	{
		size_t dev_bat0 = aml_start_device(&ctx, "BAT0");
		{
			aml_name_decl(&ctx, "_HID");
			aml_string(&ctx, "PNP0C0A");

			aml_name_decl(&ctx, "_UID");
			aml_int(&ctx, 0);

			size_t method_sta = aml_start_method(&ctx, "_STA", 0, AML_NOTSERIALIZED);
			{
				aml_return(&ctx);
				aml_int(&ctx, 0x1F);
			}
			aml_end_method(&ctx, method_sta);

			size_t method_bif = aml_start_method(&ctx, "_BIF", 0, AML_NOTSERIALIZED);
			{
				aml_return(&ctx);
				size_t pkg_bif = aml_start_package(&ctx, 13);
				{
					aml_int(&ctx, 0); // [0] Power Unit
					aml_int(&ctx, 50000); // [1] Design Capacity
					aml_int(&ctx, 50000); // [2] Last Full Charge Capacity
					aml_int(&ctx, 1); // [3] Battery Technology
					aml_int(&ctx, 10000); // [4] Design Voltage
					aml_int(&ctx, 2500); // [5] Design Capacity of Warning
					aml_int(&ctx, 1000); // [6] Design Capacity of Low
					aml_int(&ctx, 1); // [7] Granularity 1
					aml_int(&ctx, 1); // [8] Granularity 2
					aml_string(&ctx, "T1000"); // [9] Model Number
					aml_string(&ctx, "1453-0529"); // [10] Serial Number
					aml_string(&ctx, "Li-Ion"); // [11] Battery Type
					aml_string(&ctx, "ZEMU"); // [12] OEM Info
				}
				aml_end_package(&ctx, pkg_bif);
			}
			aml_end_method(&ctx, method_bif);

			size_t method_bst = aml_start_method(&ctx, "_BST", 0, AML_NOTSERIALIZED);
			{
				aml_return(&ctx);
				size_t pkg_bst = aml_start_package(&ctx, 4);
				{
					uint64_t bat_remain = (uint64_t)((nk.ini->cur->battery_percent * 50000) / 100);
					aml_int(&ctx, (nk.ini->cur->ac_power) ? 0x02 : 0x01); // [0] Battery State
					aml_int(&ctx, 0x10); // [1] Battery Present Rate
					aml_int(&ctx, bat_remain); // [2] Remaining Capacity
					aml_int(&ctx, 10000); // [3] Battery Present Voltage
				}
				aml_end_package(&ctx, pkg_bst);
			}
			aml_end_method(&ctx, method_bst);
		}
		aml_end_device(&ctx, dev_bat0);

		if (nk.ini->cur->ac_power)
		{
			size_t dev_ac0 = aml_start_device(&ctx, "AC0");
			{
				aml_name_decl(&ctx, "_HID");
				aml_string(&ctx, "ACPI0003");

				aml_name_decl(&ctx, "_UID");
				aml_int(&ctx, 0);

				size_t method_sta = aml_start_method(&ctx, "_STA", 0, AML_NOTSERIALIZED);
				{
					aml_return(&ctx);
					aml_int(&ctx, 0x0F);
				}
				aml_end_method(&ctx, method_sta);

				size_t method_psr = aml_start_method(&ctx, "_PSR", 0, AML_NOTSERIALIZED);
				{
					aml_return(&ctx);
					aml_int(&ctx, 0x01);
				}
				aml_end_method(&ctx, method_psr);
			}
			aml_end_device(&ctx, dev_ac0);
		}
	}
	aml_end_scope(&ctx, scope_sb);

	ACPI_HEADER* hdr = (ACPI_HEADER*)BAT_AML;
	memcpy(hdr->Signature, "SSDT", 4);
	hdr->Length = (UINT32)(sizeof(ACPI_HEADER) + aml_get_size(&ctx));
	hdr->Revision = 2;
	hdr->Checksum = 0;
	memcpy(hdr->OemId, "ZEMU\0\0", 6);
	memcpy(hdr->OemTableId, "ZEMUBAT\0", 8);
	hdr->OemRevision = 1;
	memcpy(hdr->CreatorId, "INTL", 4);
	hdr->CreatorRevision = 0x250807;

	hdr->Checksum = 1 + ~get_acpi_checksum(BAT_AML, hdr->Length);

	BOOL rc = FALSE;
	FILE* fp = NULL;
	errno_t err = _wfopen_s(&fp, aml, L"wb");
	if (err != 0 || !fp)
		return FALSE;
	if (fwrite(BAT_AML, hdr->Length, 1, fp) != 1)
		goto out;
	rc = TRUE;
out:
	fclose(fp);
	return rc;
}

