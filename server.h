/*
 * file server.h - communication interface for the server
 *
 * $Id: server.h,v 1.33 2006/02/10 15:07:35 fzago Exp $
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
#ifndef XBLAST_SERVER_H
#define XBLAST_SERVER_H

/*
 * global enums
 */
typedef enum
{
	XBSC_GameTime,				/* future actions received, client is async */
	XBSC_IOError,				/* i/o error on stream */
	XBSC_WriteError,			/* write error on dgram */
	XBSC_ConnFailed,			/* connection failed on dgram */
	XBSC_Timeout,				/* timeout on dgram */
	XBSC_COTInvalid,			/* invalid cot received */
	XBSC_IDInvalid,				/* invalid id received */
	XBSC_DataInvalid,			/* invalid data received */
	XBSC_MissingData,			/* client has not requested data */
	XBSC_UnexpectedEOF,			/* client shut down unexpectedly */
	XBSC_ExpectedEOF /* expected eof, possibly due to i/o or parse error */ ,
	XBSC_StreamWaiting,			/* all queued data sent on stream */
	XBSC_StreamBusy,			/* queued data on stream partially sent */
	XBSC_StreamClosed,			/* stream has been removed */
	XBSC_DgramClosed,			/* dgram has been removed */
} XBServerConstants;
/*
 * global prototypes
 */

/* starting/closing server */
extern XBBool Server_StartListen (CFGGameHost *);
extern void Server_StopListen (void);
extern void Server_ReceiveListenClose (XBBool);

/* connecting/disconnecting */
extern void Server_Accept (unsigned id, const char *host, unsigned port);
extern void Server_StreamDisconnect (unsigned id);
extern void Server_DgramDisconnect (unsigned id);
extern void Server_SendDisconnect (unsigned id);
extern void Server_SendDisconnectAll (void);

/* error handling */
extern XBBool Server_StreamEvent (unsigned id, XBServerConstants code);
extern XBBool Server_DgramEvent (unsigned id, XBServerConstants code);

/* receiving data */
extern XBBool Server_ReceiveGameConfig (unsigned id, const char *line);
extern XBBool Server_ReceivePlayerConfig (unsigned id, int player, const char *line);
extern void Server_ReceiveDisconnect (unsigned id);
extern void Server_ReceiveDgramPort (unsigned id, unsigned short port);
extern void Server_ReceiveHostStateReq (unsigned id, unsigned host, XBHostState state);
extern void Server_ReceiveTeamStateReq (unsigned id, unsigned host, unsigned player,
										XBTeamState state);
extern void Server_ReceiveChat (XBChat *);
extern void Server_ReceivePing (unsigned id, int ping);
extern void Server_ReceiveSync (unsigned id, XBNetworkEvent);
extern void Server_ReceiveLevelCheck (unsigned id, unsigned stat);
extern void Server_ReceivePlayerAction (unsigned id, int gameTime,
										const PlayerAction * playerAction);
extern void Server_ReceiveFinish (unsigned id);
extern void Server_ReceiveWinnerTeam (unsigned id, unsigned team);

/* retrieving local data */
extern XBBool GetIsServer (void);
extern void SetIsServer (XBBool value);
extern int Server_GetPingTime (unsigned id);
extern XBHostState Server_GetHostState (unsigned id);
extern XBTeamState Server_GetTeamState (unsigned id, unsigned player);
extern XBHostState *Server_GetHostStateReq (unsigned id);
extern XBTeamState *Server_GetTeamStateReq (unsigned id, unsigned player);
extern XBBool Server_LevelApproved (void);
extern void Server_GetPlayerAction (unsigned id, int player, PlayerAction * action);
extern XBBool Server_LevelAsync (void);

/* setting local data */
extern void Server_ClearLevelStatus (void);
extern void Server_SetLevelStatus (unsigned id, XBBool val);
extern void Server_ResetPlayerAction (void);
extern void Server_ClearPlayerAction (void);
extern void Server_ClearLevelWinners (void);
extern void Server_SetLevelWinners (unsigned id, unsigned team);

/* queueing out data */
extern void Server_SendAllConfigs (unsigned id);
extern void Server_SendDgramPort (unsigned id);
extern void Server_QueryGameConfig (unsigned id);
extern void Server_SendGameConfig (unsigned id);
extern void Server_QueryPlayerConfigs (unsigned id, unsigned cnt);
extern void Server_SendPlayerConfig (unsigned id, unsigned player);
extern void Server_SendHostState (unsigned id, XBHostState state);
extern void Server_SendTeamState (unsigned id, unsigned player, XBTeamState team);
extern void Server_SendHostStateReq (unsigned id, unsigned host, XBHostState state);
extern void Server_SendTeamStateReq (unsigned id, unsigned host, unsigned player,
									 XBTeamState state);
extern void Server_SendDisconnectInfo (unsigned clientID);

extern void Server_SendChat (XBChat * chat);
extern void Server_SysChat (const char *);

extern void Server_SendStart (void);
extern void Server_SendRandomSeed (void);
extern void Server_SendLevel (const DBRoot * level);
extern void Server_SendLevelActivate (void);
extern void Server_SendLevelReset (void);
extern void Server_SendSync (XBNetworkEvent);
extern void Server_SendPlayerAction (int gameTime, const PlayerAction * action);
extern void Server_FinishPlayerAction (int gameTime);
extern XBBool Server_FlushPlayerAction (void);
extern void Server_SendLevelSync (void);
extern void Server_SendLevelAsync (void);

/* connection to central */
extern void Server_StartCentralNewGame (const CFGGameHost * cfg, const CFGGameSetup * setup);
extern void Server_RestartNewGame (int num, const char *score);
extern void Server_CloseNewGame (void);
extern void Server_StopNewGame (void);
extern void Server_ReceiveNewGameClose (void);

#endif
/*
 * end of file server.h
 */
