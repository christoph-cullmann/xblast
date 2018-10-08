/*
 * file w32_keysym.h
 *
 * $Id: w32_keysym.h,v 1.3 2006/02/09 21:21:25 fzago Exp $
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef W32_KEYSYM_H
#define W32_KEYSYM_H

#include "w32_common.h"


#include "event.h"

/*
 * global prototypes
 */
extern XBBool InitKeysym (void);
extern void FinishKeysym (void);
extern UINT StringToVirtualKey (const char *name);
extern XBAtom VirtualKeyToAtom (UINT code);

#endif
/*
 * end of file w32_keysym.h
 */
