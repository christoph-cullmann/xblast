/*
 * file user.h - communication interface for users
 *
 * $Id: user.h,v 1.5 2004/11/11 14:58:15 lodott Exp $
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
#ifndef XBLAST_USER_H
#define XBLAST_USER_H

#include "player.h"
#include "network.h"
#include "cfg_xblast.h"
#include "com_to_central.h"

/*
 * global prototypes
 */

/* connect/disconnect */
extern XBBool User_Connect (CFGCentralSetup *);
extern void User_Disconnect (void);
extern XBBool User_EventToCentral(const XBEventToCentral);

/* get data */
extern XBBool User_Connected();
extern int    User_GetPID();
extern int    User_Received();
extern XBBool User_Complete();
extern void   User_NoMorePlayers();

/* receive data */
extern void User_ReceivePlayerPID (const char *data);
extern void User_ReceivePlayerConfig (const char *data);

/* queue data */
extern void User_SendDisconnect(void);
extern void User_RequestUpdate();
extern void User_SendRegisterPlayer (XBAtom);
extern void User_SendUnregisterPlayer (XBAtom);
extern void User_SendGameStat (int numPlayers, BMPlayer *playerStat, int *pa);

#endif
/*
 * end of file user.h
 */
