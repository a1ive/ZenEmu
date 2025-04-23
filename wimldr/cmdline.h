#ifndef _CMDLINE_H
#define _CMDLINE_H

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
 * Command line
 *
 */

#include <stdint.h>

#define NTARG_BOOL_TRUE  1
#define NTARG_BOOL_FALSE 0

struct nt_args {
	uint8_t testmode;
	uint8_t minint;
	uint8_t rawwim;
	uint8_t gui;
	uint8_t pause;
	uint8_t linear;
	unsigned int index;
	/* QEMU Loader Data */
	void *ldr_data;
	intptr_t ldr_addr;
	unsigned int ldr_len;
};

extern struct nt_args *nt_cmdline;

extern void process_cmdline ( char *cmdline );

#endif /* _CMDLINE_H */
