/*
 * file version.h - tools for version data
 *
 * $Id: version.h,v 1.27 2006/06/16 22:09:08 alfie Exp $
 *
 * Program XBLAST
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will be entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef XBLAST_VERSION_H
#define XBLAST_VERSION_H



/*
 * global macros
 */
#define VERSION_STRING "2.10.4 (Central,epfl,Sky)"
#define VERSION_MAJOR  2
#define VERSION_MINOR  10
#define VERSION_PATCH  4
#define COPYRIGHT_YEAR "1993-2006"

/* version struct */
typedef struct
{
	int major;
	int minor;
	int patch;
} XBVersion;

/* version types */
typedef unsigned char XBVerType;
#define VERSION_JOINT 0xFF

/* constant version strings, please document incompatibility issues here */
extern const XBVersion Ver_None;
extern const XBVersion Ver_Local;
extern const XBVersion Ver_2_10_1;
/* slowMotionBurst key in map section added */
extern const XBVersion Ver_2_10_2;
/* XBTS_Out introduced, earlier versions can't handle it and assert */
/* bug fix for swapcolor, earlier versions might crash (SMPF) */

/* general tools */
extern char *Version_ToString (const XBVersion *);
extern void Version_Clear (XBVersion *);
extern XBBool Version_isDefined (const XBVersion *);
extern int Version_Compare (const XBVersion * v1, const XBVersion * v2);

/* getting local data */
extern void Version_Get (XBVerType, XBVersion *);
extern XBBool Version_AtLeast (XBVerType, const XBVersion *);

/* modifying host versions */
extern void Version_Reset (void);
extern XBBool Version_Join (unsigned char, const XBVersion *);
extern XBBool Version_Remove (unsigned char);

#endif
/*
 * end of file version.h
 */
