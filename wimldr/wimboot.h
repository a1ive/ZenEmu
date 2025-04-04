#ifndef _WIMBOOT_H
#define _WIMBOOT_H

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
 * WIM boot loader
 *
 */

/** Debug switch */
#ifndef DEBUG
#define DEBUG 1
#endif

#ifndef ASSEMBLY

#include <stdint.h>
#include <cmdline.h>

/** Construct wide-character version of a string constant */
#define L( x ) _L ( x )
#define _L( x ) L ## x

/** Page size */
#define PAGE_SIZE 4096

/**
 * Calculate start page number
 *
 * @v address		Address
 * @ret page		Start page number
 */
static inline unsigned int page_start ( const void *address ) {
	return ( ( ( intptr_t ) address ) / PAGE_SIZE );
}

/**
 * Calculate end page number
 *
 * @v address		Address
 * @ret page		End page number
 */
static inline unsigned int page_end ( const void *address ) {
	return ( ( ( ( intptr_t ) address ) + PAGE_SIZE - 1 ) / PAGE_SIZE );
}

/**
 * Calculate page length
 *
 * @v start		Start address
 * @v end		End address
 * @ret num_pages	Number of pages
 */
static inline unsigned int page_len ( const void *start, const void *end ) {
	return ( page_end ( end ) - page_start ( start ) );
}

/**
 * Bochs magic breakpoint
 *
 */
static inline void bochsbp ( void ) {
	__asm__ __volatile__ ( "xchgw %bx, %bx" );
}

/** Debugging output */
#define DBG(...) do {						\
		if ( ( DEBUG & 1 ) && ( ! cmdline_quiet ) ) {	\
			printf ( __VA_ARGS__ );			\
		}						\
	} while ( 0 )

/** Verbose debugging output */
#define DBG2(...) do {						\
		if ( ( DEBUG & 2 ) && ( ! cmdline_quiet ) ) {	\
			printf ( __VA_ARGS__ );			\
		}						\
	} while ( 0 )

/* Branch prediction macros */
#define likely( x ) __builtin_expect ( !! (x), 1 )
#define unlikely( x ) __builtin_expect ( (x), 0 )

/* Mark parameter as unused */
#define __unused __attribute__ (( unused ))

extern void __attribute__ (( noreturn, format ( printf, 1, 2 ) ))
die ( const char *fmt, ... );

extern unsigned long __stack_chk_guard;
extern void init_cookie ( void );

#endif /* ASSEMBLY */

#endif /* _WIMBOOT_H */
