/*
 * file central.h - communication interface for the server
 *
 * $Id: central.h,v 1.9 2006/02/10 15:07:42 fzago Exp $
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

/* possible game stat types */
typedef enum
{
	XBGS_Error,					/* invalid */
	XBGS_Level,					/* level result */
	XBGS_Complete				/* game result */
} XBGameStatType;

/* special pid values */
typedef enum
{
	XBPV_Password = -1,			/* password failed in update */
	XBPV_Unknown = -2,			/* unknown pid in update */
	XBPV_Double = -3,			/* name already registered */
	XBPV_Incomplete = -255		/* still receiving data */
} XBPidValue;

/*
 * global prototypes
 */

/* start/stop */
extern XBBool Central_StartListen (CFGGameHost *);
extern void Central_StopListen (void);

/* accept connection */
extern void Central_Accept (unsigned id, unsigned cnt, const char *host, unsigned port);

/* get data */
extern void Central_GetStatistics (int *nPlayers, int *nGames, int *nGamesPlayed, int *nTotalGames,
								   int *nLogins);
extern int Central_Connected (void);

/* receive data */
extern XBGameStatType Central_ReceiveGameStat (const char *line);
extern int Central_ReceivePlayerConfig (unsigned id, const char *line);
extern void Central_ReceiveDisconnect (unsigned id);
extern void Central_ReceiveFinish (unsigned id);

/* receive a close */
extern void Central_ConnectionClosed (unsigned id, unsigned cnt);

/* queue signals */
extern void Central_QueueDisconnect (unsigned id);
extern void Central_QueueDisconnectAll (void);

#endif

/*
 * end of file server.h
 */
