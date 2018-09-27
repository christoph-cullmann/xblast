/*
 * file client.h - communication interface for clients
 *
 * $Id: client.h,v 1.22 2005/01/11 17:37:34 lodott Exp $
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
#ifndef XBLAST_CLIENT_H
#define XBLAST_CLIENT_H

#include "player.h"
#include "network.h"
#include "chat.h"

/*
 * global types
 */
typedef struct _xb_network_game {
  unsigned        id;        /* interface id */
  char           *host;      /* address of server */
  unsigned short  port;      /* port for game */
  int             ping;      /* ping time */
  char           *version;   /* version string */
  char           *game;      /* name of the game */
  int             numLives;  /* number of lives */
  int             numWins;   /* number of levels to win */
  int             frameRate; /* frame per second */
} XBNetworkGame;

typedef enum {
  XBCC_Loss,            /* data loss on dgram */
  XBCC_IOError,         /* i/o error on stream */
  XBCC_WriteError,      /* write error on dgram */
  XBCC_ConnFailed,      /* connection failed on dgram */
  XBCC_COTInvalid,      /* invalid cot received */
  XBCC_IDInvalid,       /* invalid id received */
  XBCC_DataInvalid,     /* invalid data received */
  XBCC_ExpectedEOF,     /* expected close on stream */
  XBCC_UnexpectedEOF,   /* unexpected server disconnect */
  XBCC_StreamWaiting,   /* complete send of queued data  */
  XBCC_StreamBusy,      /* partial send of queued data */
  XBCC_StreamClosed,    /* stream closed */
  XBCC_DgramClosed,     /* dgram closed */
} XBClientConstants;
/*
 * global prototypes
 */

/* connecting/disconnecting */
extern XBBool Client_Connect (CFGGameHost *);
extern void Client_Disconnect (void);
extern void Client_SetDisconnected (XBBool);

/* error handling */
extern XBBool Client_StreamEvent (XBClientConstants);
extern XBBool Client_DgramEvent (XBClientConstants);

/* receiving data */
extern void Client_ReceiveGameConfig (unsigned, const char *data);
extern void Client_ReceivePlayerConfig (unsigned, unsigned, const char *data);
extern void Client_ReceiveDgramPort (unsigned id, unsigned short);
extern void Client_ReceiveId (unsigned id);
extern void Client_ReceivePingTime (unsigned clientID, int ping);
extern void Client_ReceiveGameConfigReq(unsigned);
extern void Client_ReceivePlayerConfigReq(unsigned);
extern void Client_ReceiveHostState (unsigned id, unsigned state);
extern void Client_ReceiveTeamState (unsigned host, unsigned player, unsigned team);
extern void Client_ReceiveHostStateReq (unsigned who, unsigned id, unsigned state);
extern void Client_ReceiveTeamStateReq (unsigned who, unsigned host, unsigned player, unsigned team);
extern void Client_ReceiveChat(XBChat *);
extern void Client_ReceiveStart (unsigned);
extern void Client_ReceiveSync (XBNetworkEvent);
extern void Client_ReceiveLevelConfig (unsigned, const char *data);
extern void Client_ReceiveRandomSeed (unsigned);
extern void Client_ReceivePlayerAction (int, const PlayerAction *);
extern void Client_ReceiveFinish (void);
extern void Client_ReceiveAsync(XBBool as);
extern void Client_ReceiveDisconnect (unsigned);

/* retrieving local data */
extern unsigned Client_ID();
extern unsigned Client_GetHostState (unsigned id);
extern int  Client_GetPingTime (unsigned clientID);
extern unsigned Client_GetHostState (unsigned id);
extern unsigned * Client_GetHostStateReq (unsigned id);
extern unsigned Client_GetTeamState (unsigned id, unsigned pl);
extern unsigned * Client_GetTeamStateReq (unsigned id, unsigned pl);
extern XBBool Client_RejectsLevel(void);
extern XBBool Client_FixedLevel(void);
extern void Client_GetPlayerAction (int, PlayerAction *);
extern XBBool Client_LevelAsynced(void);

/* setting local data */
extern void Client_LevelRejection(XBBool);
extern void Client_LevelFix(XBBool);
extern void Client_ClearPlayerAction (void);
extern void Client_ResetPlayerAction (void);
extern void Client_ActivateLevel (unsigned);
extern void Client_SetLevelAsync(XBBool);

/* queueing data */
extern void Client_SendDgramPort(XBBool nat);
extern void Client_SendGameConfig();
extern void Client_SendPlayerConfigs();
extern void Client_SendHostState (unsigned state);
extern void Client_SendHostStateReq (unsigned id, unsigned state);
extern void Client_SendTeamStateReq (unsigned id, unsigned player, unsigned team);
extern void Client_SendChat(unsigned fp, unsigned th, unsigned tp, unsigned how, char *chat);
extern void Client_SendSync (XBNetworkEvent);
extern void Client_SendLevelCheck ();
extern void Client_SendPlayerAction (int, const PlayerAction *);
extern void Client_FinishPlayerAction (int gameTime);
extern XBBool Client_FlushPlayerAction (void);
extern void Client_SendWinner(unsigned team);

/* central queries */
extern void Client_StartQuery (void);
extern void Client_StartCentralQuery (void); // XBCC
extern void Client_RestartQuery (void);
extern void Client_StopQuery (void);
extern void Client_ReceiveQueryClose(unsigned);
extern void Client_ReceiveReply (unsigned id, const char *host, unsigned short port, int ping, const char *version, const char *game, int numLives , int numWins , int frameRate);
extern const XBNetworkGame * Client_NextNetworkGame (void);

#endif
/*
 * end of file client.h
 */
