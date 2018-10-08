/*
 * net_tele.h - telegrams for cleint/server communication
 *
 * $Id: net_tele.h,v 1.12 2006/02/09 21:21:24 fzago Exp $
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
#ifndef _NET_TELE_H
#define _NET_TELE_H

/*
 * type declarations
 */
typedef struct _xb_telegram XBTelegram;
typedef struct _xb_snd_queue XBSndQueue;
typedef struct _xb_rcv_queue XBRcvQueue;

/*
 * type definitions
 */

/* cause of transmission */
typedef enum
{
	XBT_COT_Activate,			/* server demands activation */
	XBT_COT_Spontaneous,		/* client message */
	XBT_COT_SendData,			/* send data to client */
	XBT_COT_RequestData,		/* request data from client */
	XBT_COT_DataNotAvailable,	/* client does not have data */
	XBT_COT_DataAvailable,		/* client has data */
	/* --- */
	NUM_XBT_COT
} XBTeleCOT;

/* telegram object id */
typedef enum
{
	XBT_ID_GameConfig,			/* game data */
	XBT_ID_PlayerConfig,		/* player data */
	XBT_ID_RequestDisconnect,	/* host wants disconnect */
	XBT_ID_HostDisconnected,	/* message host has disconnected */
	XBT_ID_StartGame,			/* server starts the game */
	XBT_ID_RandomSeed,			/* seed for random numer generator */
	XBT_ID_LevelConfig,			/* level data */
	XBT_ID_DgramPort,			/* port for datagram connection */
	XBT_ID_Sync,				/* synchronisation between client and server */
	XBT_ID_HostIsIn,			/* host is in game */
	XBT_ID_HostIsOut,			/* host is out of game */
	XBT_ID_TeamChange,			/* Team Change */
	XBT_ID_GameStat,			/* Host sends game statistics */
	XBT_ID_PID,					/* Central sends PID */
	XBT_ID_WinnerTeam,			/* Client sends winner team for level */
	XBT_ID_Async,				/* client and server asynced */
	XBT_ID_Chat,				/* Server/Client sends chat line */
	XBT_ID_HostChange,			/* host state change by server */
	XBT_ID_HostChangeReq,		/* host change request */
	XBT_ID_TeamChangeReq,		/* team change request */
	/* --- */
	NUM_XBT_ID
} XBTeleID;

/* result of i/o operations */
typedef enum
{
	XBT_R_IOError,				/* an error has occured during i/o operation */
	XBT_R_TeleError,			/* an error has occured during parsing */
	XBT_R_Continue,				/* telegram has been partially send/received */
	XBT_R_Complete,				/* telegram has been completely send/received */
	XBT_R_EndOfFile				/* stream was closed */
} XBTeleResult;

/* information object adress */
typedef unsigned char XBTeleIOB;

/*
 * global prototypes
 */
extern XBTelegram *Net_CreateTelegram (XBTeleCOT cot, XBTeleID id, XBTeleIOB iob, const void *data,
									   size_t len);
extern void Net_DeleteTelegram (XBTelegram * tele);

extern XBSndQueue *Net_CreateSndQueue (XBBool server);
extern void Net_DeleteSndQueue (XBSndQueue * list);
extern XBTeleResult Net_Send (XBSndQueue * list, const XBSocket * pSocket);
extern void Net_SendTelegram (XBSndQueue * list, XBTelegram * tele);

extern XBRcvQueue *Net_CreateRcvQueue (XBBool server);
extern void Net_DeleteRcvQueue (XBRcvQueue * list);
extern XBTeleResult Net_Receive (XBRcvQueue * list, const XBSocket * pSocket);
extern XBTelegram *Net_ReceiveTelegram (XBRcvQueue * list);

extern XBTeleCOT Net_TeleCOT (const XBTelegram *);
extern XBTeleID Net_TeleID (const XBTelegram *);
extern XBTeleIOB Net_TeleIOB (const XBTelegram *);
extern const void *Net_TeleData (const XBTelegram *, size_t * len);

#endif
/*
 * end of file net_tele.h
 */
