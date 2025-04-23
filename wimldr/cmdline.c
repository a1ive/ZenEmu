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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "wimboot.h"
#include "cmdline.h"

static struct nt_args args = {
	.testmode = NTARG_BOOL_FALSE,
	.minint = NTARG_BOOL_FALSE,
	.rawwim = NTARG_BOOL_FALSE,
	.gui = NTARG_BOOL_FALSE,
	.pause = NTARG_BOOL_FALSE,
	.linear = NTARG_BOOL_FALSE,
	.index = 0,
};

struct nt_args *nt_cmdline;

static uint8_t
convert_bool ( const char *str ) {
	uint8_t value = NTARG_BOOL_FALSE;
	if ( ! str ||
		 strcasecmp ( str, "yes" ) == 0 ||
		 strcasecmp ( str, "on" ) == 0 ||
		 strcasecmp ( str, "true" ) == 0 ||
		 strcasecmp ( str, "1" ) == 0 )
		value = NTARG_BOOL_TRUE;
	return value;
}

/**
 * Process command line
 *
 * @v cmdline		Command line
 */
void process_cmdline ( char *cmdline ) {
	char *tmp = cmdline;
	char *key;
	char *value;
	char *endp;
	char chr;

	nt_cmdline = &args;

	/* Do nothing if we have no command line */
	if ( ( cmdline == NULL ) || ( cmdline[0] == '\0' ) )
		return;

	/* Show command line */
	printf ("Command line: \"%s\"\n", cmdline);

	/* Parse command line */
	while ( *tmp ) {

		/* Skip whitespace */
		while ( isspace ( *tmp ) )
			tmp++;

		/* Find value (if any) and end of this argument */
		key = tmp;
		value = NULL;
		while ( ( chr = *tmp ) ) {
			if ( isspace ( chr ) ) {
				*(tmp++) = '\0';
				break;
			} else if ( ( chr == '=' ) && ( ! value ) ) {
				*(tmp++) = '\0';
				value = tmp;
			} else {
				tmp++;
			}
		}

		/* Process this argument */
		if ( strcmp ( key, "testmode" ) == 0 ) {
			args.testmode = convert_bool ( value );
		} else if ( strcmp ( key, "minint" ) == 0 ) {
			args.minint = convert_bool ( value );
		} else if ( strcmp ( key, "rawwim" ) == 0 ) {
			args.rawwim = convert_bool ( value );
		} else if ( strcmp ( key, "gui" ) == 0 ) {
			args.gui = convert_bool ( value );
		} else if ( strcmp ( key, "pause" ) == 0 ) {
			args.pause = convert_bool ( value );
		} else if ( strcmp ( key, "linear" ) == 0 ) {
			args.linear = convert_bool ( value );
		} else if ( strcmp ( key, "index" ) == 0 ) {
			if ( ( ! value ) || ( ! value[0] ) )
				die ( "Argument \"index\" needs a value\n" );
			args.index = strtoul ( value, &endp, 0 );
			if ( *endp )
				die ( "Invalid index \"%s\"\n", value );
		} else if ( strcmp ( key, "initrd" ) == 0 ) {
			/* Ignore this keyword to allow for use with QEMU */
		} else if ( key == cmdline ) {
			/* Ignore unknown initial arguments, which may
			 * be the program name.
			 */
		} else {
			die ( "Unrecognised argument \"%s%s%s\"\n", key,
			      ( value ? "=" : "" ), ( value ? value : "" ) );
		}

		/* Undo modifications to command line */
		if ( chr )
			tmp[-1] = chr;
		if ( value )
			value[-1] = '=';
	}
}
