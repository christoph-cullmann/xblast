/*
 * file com_central.h - central answers to browse from clients
 *
 * $Id: com_central.h,v 1.4 2005/01/23 14:19:59 lodott Exp $
 *
 * Program XBLAST
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 * Added by Koen De Raedt for central support
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
#ifndef XBLAST_COM_CENTRAL_H
#define XBLAST_COM_CENTRAL_H

#include "com.h"
#include "cfg_game.h"

/*
 * type definitions
 */

/*
 * global prototypes
 */
extern int C2B_GetOpenGames();
extern void C2B_ClearOldGames();
extern XBComm* C2B_CreateComm (unsigned short centralPort);

#endif
/*
 * end of file com_central.h
 */
