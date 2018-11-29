/*
 * file version.h - strings and ints with version info
 *
 * $Id: version.h,v 1.12 2005/01/13 18:58:04 lodott Exp $
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
#define VERSION_STRING "2.10.0 (Central,epfl,Sky,AbsInt)"
#define VERSION_MAJOR  2
#define VERSION_MINOR  10
#define VERSION_PATCH  0
#define COPYRIGHT_YEAR "1993-2005"

#include "ini_file.h"

typedef struct {
  int major;
  int minor;
  int patch;
} XBVersion;

extern const XBVersion Ver_2_10_1;
/* slowMotionBurst key in map section added */

extern void Version_GetLocal(XBVersion *);
extern void Version_GetJoint(XBVersion *);

extern char * Version_ToString(const XBVersion *);
extern void Version_ShowData();

extern void Version_Clear(XBVersion *);
extern XBBool Version_isDefined(const XBVersion *);

extern void Version_Reset();
extern int Version_Compare(const XBVersion *v1, const XBVersion *v2);
extern void Version_Join(const XBVersion *);
extern XBBool Version_AtLeast(const XBVersion *);

#endif
/*
 * end of file version.h
 */
