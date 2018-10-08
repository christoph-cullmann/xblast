/*
 * file w32_config.h - config data for win32 graphics engine
 *
 * Program XBLAST
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * $Id: w32_config.h,v 1.4 2006/02/09 21:21:25 fzago Exp $
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
#ifndef _W32_CONFIG_H
#define _W32_CONFIG_H

#include "w32_common.h"


/*
 * global prototypes
 */
extern XBBool RetrieveWindowRect (RECT * rect);
extern void StoreWindowRect (const RECT * cfg);

#endif
/*
 * end fo file w32_config.h
 */
