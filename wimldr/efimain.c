/*
 * Copyright (C) 2014 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/**
 * @file
 *
 * EFI entry point
 *
 */

#include <stdio.h>
#include "wimboot.h"
#include "cmdline.h"
#include "efi.h"
#include "efifile.h"
#include "efiblock.h"
#include "efiboot.h"

/** SBAT section attributes */
#define __sbat __attribute__ (( section ( ".sbat" ), aligned ( 512 ) ))

/** SBAT metadata */
#define SBAT_CSV							\
	/* SBAT version */						\
	"sbat,1,SBAT Version,sbat,1,"					\
	"https://github.com/rhboot/shim/blob/main/SBAT.md"		\
	"\n"								\
	/* wimboot version */						\
	"wimboot," SBAT_GENERATION ",iPXE,wimboot," VERSION ","		\
	"https://ipxe.org/wimboot"					\
	"\n"

/** SBAT metadata (with no terminating NUL) */
const char sbat[ sizeof ( SBAT_CSV ) - 1 ] __sbat = SBAT_CSV;

/**
 * Process command line
 *
 * @v loaded		Loaded image protocol
 */
static void efi_cmdline ( EFI_LOADED_IMAGE_PROTOCOL *loaded ) {
	size_t cmdline_len = ( loaded->LoadOptionsSize / sizeof ( wchar_t ) );
	char cmdline[ cmdline_len + 1 /* NUL */ ];
	const wchar_t *wcmdline = loaded->LoadOptions;

	/* Convert command line to ASCII */
	snprintf ( cmdline, sizeof ( cmdline ), "%ls", wcmdline );

	/* Process command line */
	process_cmdline ( cmdline );
}

static void efi_connect ( void )
{
	EFI_STATUS status;
	UINTN count;
	EFI_HANDLE *buf;
	UINTN i;
	EFI_BOOT_SERVICES *bs = efi_systab->BootServices;
	DBG ( "Connecting ...\n" );
	status = bs->LocateHandleBuffer ( AllHandles,
									  NULL, NULL,
									  &count,
									  &buf );

	if (status != EFI_SUCCESS)
		return;

	for ( i = 0 ; i < count ; i++ )
		bs->ConnectController ( buf[i], NULL, NULL, TRUE );

	if ( buf )
		bs->FreePool ( buf );
}

/**
 * EFI entry point
 *
 * @v image_handle	Image handle
 * @v systab		EFI system table
 * @ret efirc		EFI status code
 */
EFI_STATUS EFIAPI efi_main ( EFI_HANDLE image_handle,
			     EFI_SYSTEM_TABLE *systab ) {
	EFI_BOOT_SERVICES *bs;
	union {
		EFI_LOADED_IMAGE_PROTOCOL *image;
		void *interface;
	} loaded;
	EFI_HANDLE vdisk = NULL;
	EFI_HANDLE vpartition = NULL;
	EFI_STATUS efirc;

	/* Record EFI handle and system table */
	efi_image_handle = image_handle;
	efi_systab = systab;
	bs = systab->BootServices;

	/* Initialise stack cookie */
	init_cookie();

	/* Print welcome banner */
	printf ( "\n\nwimldr\n\n" );

	/* Get loaded image protocol */
	if ( ( efirc = bs->OpenProtocol ( image_handle,
					  &efi_loaded_image_protocol_guid,
					  &loaded.interface, image_handle, NULL,
					  EFI_OPEN_PROTOCOL_GET_PROTOCOL ))!=0){
		die ( "Could not open loaded image protocol: %#lx\n",
		      ( ( unsigned long ) efirc ) );
	}

	/* Process command line */
	efi_cmdline ( loaded.image );

	efi_connect ();

	/* Extract files from file system */
	efi_extract_wim ( loaded.image->DeviceHandle );
	efi_extract_hda ();

	/* Install virtual disk */
	efi_install ( &vdisk, &vpartition );

	/* Invoke boot manager */
	efi_boot ( bootmgfw, bootmgfw_path, vpartition );

	return 0;
}
