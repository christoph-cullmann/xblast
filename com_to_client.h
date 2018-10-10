/*
 * file com_to_client.h - handle communications with clients
 *
 * $Id: com_to_client.h,v 1.8 2004/10/06 23:33:28 lodott Exp $
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
#ifndef _COM_TO_CLIENT_H
#define _COM_TO_CLIENT_H

#include "atom.h"
#include "com_base.h"
#include "net_socket.h"
#include "network.h"
#include "ini_file.h"
#include "chat.h"

/*
 * global prototypes
 */

/* constructor */
extern XBComm *S2C_CreateComm (const XBSocket *socket);

/* get local data */
extern XBBool S2C_Connected (unsigned id);
extern void S2C_ShowConnected (void);
extern const char *S2C_HostName (unsigned id);
extern const char *S2C_LocalName (unsigned id);

/* queue data */
extern void S2C_SendGameConfig (unsigned id, unsigned hostId, XBAtom atom);
extern void S2C_SendPlayerConfig (unsigned id, unsigned hostId, int player, XBAtom);
extern void S2C_SendDgramPort (unsigned id, unsigned short port);
extern void S2C_QueryGameConfig (unsigned id);
extern void S2C_QueryPlayerConfig (unsigned id, int player);
extern void S2C_SendHostState (unsigned id, unsigned hostId, unsigned state);
extern void S2C_SendTeamState (unsigned id, unsigned host, unsigned player, unsigned team);
extern void S2C_SendHostStateReq (unsigned id, unsigned who, unsigned hostId, unsigned state);
extern void S2C_SendTeamStateReq (unsigned id, unsigned who, unsigned hostId, unsigned player, unsigned team);
extern void S2C_SendChat (unsigned id, XBChat *chat);
extern void S2C_HostDisconnected (unsigned id, unsigned hostID);
extern void S2C_Disconnect (unsigned id);
extern void S2C_StartGame (unsigned id);
extern void S2C_SendRandomSeed (unsigned id, unsigned seed);
extern void S2C_Sync (unsigned id, XBNetworkEvent event);
extern void S2C_SendLevelConfig (unsigned id, const DBRoot *db);
extern void S2C_SendLevelActivate (unsigned id);
extern void S2C_SendLevelReset (unsigned id);
extern void S2C_SendLevelAsync (unsigned id);
extern void S2C_SendLevelSync (unsigned id);

#endif
/*
 * end of file com_to_client.h
 */





