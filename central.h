/*
 * file central.h - communication interface for the server
 *
 * $Id: central.h,v 1.4 2005/01/23 14:21:28 lodott Exp $
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
#ifndef XBLAST_CENTRAL_H
#define XBLAST_CENTRAL_H

#include "player.h"
#include "network.h"
#include "com_central.h"

/*
 * global prototypes
 */
extern void Central_GetStatistics (int *nPlayers, int *nGames, int *nGamesPlayed, int *nTotalGames, int *nLogins);

extern XBBool Central_StartListen (CFGGameHost *);
extern void   Central_StopListen (void);
extern void   Central_Accept (unsigned id, const char *host, unsigned port);

extern void Central_SendDisconnect (unsigned id);
extern void Central_SendDisconnectAll (void);

extern void Central_ReceiveGameStat (const char *line);
extern void Central_ReceivePlayerConfig (unsigned id, const char *line);
extern void Central_ReceiveDisconnect (unsigned id);
#endif

/*
 * end of file server.h
 */
