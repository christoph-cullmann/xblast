/*
 * file com_listen.h - listen for client to connect
 *
 * $Id: com_listen.h,v 1.4 2004/11/05 16:43:17 lodott Exp $
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _COM_LISTEN_H
#define _COM_LISTEN_H

#include "cfg_game.h"
#include "com_base.h"

/*
 * type definitions
 */

/*
 * global prototypes
 */
extern XBComm *CommCreateListen (const CFGGameHost *, XBBool central);
extern void CommFinishListen (XBComm *);

#endif
/*
 * end of file com_listen.h
 */
