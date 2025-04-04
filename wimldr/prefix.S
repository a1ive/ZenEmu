/*
 * Copyright (C) 2012 Michael Brown <mbrown@fensystems.co.uk>.
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
 * bzImage prefix
 *
 */

#include "wimboot.h"

/** Standard number of setup sectors */
#define SETUP_SECTS 4

/** Sector size */
#define SECTOR_SIZE 512

/** Setup code length */
#define SETUP_LEN ( ( SETUP_SECTS + 1 ) * SECTOR_SIZE )

/** Protected-mode code will be loaded high */
#define LOADED_HIGH 0x01

/** Protected-mode code will be loaded to this linear address */
#define LOADED_HIGH_ADDRESS 0x100000

	.section ".note.GNU-stack", "", @progbits
	.text
	.section ".prefix", "ax", @progbits
	.org	0
_prefix:

#if defined(__i386__) || defined(__x86_64__)

	.code32

	.org	0x1f1
setup_sects:
	.byte	SETUP_SECTS

	.org	0x1fe
boot_flag:
	.word	0xaa55

	.org	0x200
jump:
	.byte	0xeb, setup - 1f
1:

	.org	0x202
header:
	.ascii	"HdrS"

	.org	0x206
version:
	.word	0x206	/* Version 2.06 */

	.org	0x20e
kernel_version:
	.word	( version_string - _prefix - 0x200 )

	.org	0x211
loadflags:
	.byte	LOADED_HIGH

	.org	0x218
ramdisk_image:
	.long	0	/* Filled in by boot loader */

	.org	0x21c
ramdisk_size:
	.long	0	/* Filled in by boot loader */

	.org	0x228
cmd_line_ptr:
	.long	0	/* Filled in by boot loader */

	.org	0x22c
ramdisk_max:
	.long	0xffffffff

	.org	0x238
cmdline_size:
	.long	4096

version_string:
	.asciz	VERSION

setup:

#endif

	.org	SETUP_LEN
