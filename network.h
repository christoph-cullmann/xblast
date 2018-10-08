/*
 * file network.h - shared functions for server and clients
 *
 * $Id: network.h,v 1.36 2006/02/10 15:07:42 fzago Exp $
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
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef XBLAST_NETWORK_H
#define XBLAST_NETWORK_H

/*
 * game config atoms in remote database
 */
#define LOCALGAMECONFIG(id) atomArrayHost0[id]
#define SERVERGAMECONFIG atomServer
/*
 * type definition for network events
 */
typedef enum
{
	XBNW_None = 0,
	XBNW_Accepted,				/* connection to client accepted */
	XBNW_GameConfig,			/* client has send game config */
	XBNW_RightPlayerConfig,		/* client has send player config */
	XBNW_LeftPlayerConfig,		/* client has send player config */
	XBNW_Joy1PlayerConfig,		/* client has send player config */
	XBNW_Joy2PlayerConfig,		/* client has send player config */
	XBNW_Disconnected,			/* connection to client accepted */
	XBNW_StartGame,				/* server wants to start game */
	XBNW_EndOfInit,				/* client is initialized */
	XBNW_LevelConfig,			/* server has sent level data 10 */
	XBNW_SyncEndOfInit,			/* sync after level intro */
	XBNW_SyncLevelIntro,		/* sync after level intro */
	XBNW_SyncLevelResult,		/* sync before level results are calculated */
	XBNW_SyncLevelEnd,			/* sync after level end */
	XBNW_SyncScoreboard,		/* sync after scoreboard 15 */
	XBNW_HostIsIn,				/* host is in game */
	XBNW_HostIsOut,				/* host is out of game */
	XBNW_TeamChange,			/* Team Change */
	XBNW_TeamChangeData,		/* Team Change Data */
	XBNW_Error,					/* error while writing to host */
	XBNW_PingReceived,			/* received ping from client  */
	XBNW_NetworkGame,			/* a new network game was added to the list */
	XBNW_HostChange,			/* host state changed */
	/* no new message behind this line */
	XBNW_MAX
} XBNetworkEvent;

/*
 * type definition for network events
 */
typedef enum
{
	XBNT_None = 0,				/* no networking going on */
	XBNT_Server,				/* acting as server */
	XBNT_Client,				/* acting as client */
	XBNT_MAX
} XBNetworkType;

/* client state */
typedef enum
{
	XBHS_None,
	XBHS_Wait,					/* waiting for client to send player data */
	XBHS_In,					/* client is in the game */
	XBHS_Out,					/* client is out of the game */
	XBHS_Server,				/* host is server */
	XBHS_Ready,					/* host is ready to start */
  /*---*/
	NUM_XBHS
} XBHostState;

/* team state */
typedef enum
{
	XBTS_Invalid,				/* invalid team tag */
	XBTS_None,					/* no team tag */
	XBTS_Red,					/* unknown if this is good */
	XBTS_Green,
	XBTS_Blue,
	XBTS_Out,
  /*---*/
	NUM_XBTS
} XBTeamState;

/* special codes for winning team */
#define TEAM_NOWINNER 255
#define TEAM_LOCALASYNC 254
#define TEAM_ASYNC 253
#define TEAM_UNDEF 252

/* team color assignment */
const XBColor teamColors[NUM_XBTS];

/* results of game config receive/create */
typedef enum
{
	XBGC_Unfinished,			/* still waiting for data */
	XBGC_HostInvalid,			/* invalid host id */
	XBGC_Empty,					/* game config with no players received */
	XBGC_TooManyPlayers,		/* game config contains too many players to handle */
	XBGC_NoVersion,				/* no version found */
	XBGC_Local,					/* local game config received */
	XBGC_Global,				/* global game config with all players received */
	XBGC_Error,					/* game config received is invalid */
	XBGC_SingleTeam,			/* single team for all players when creating */
    /* AbsInt start */
    XBGC_VersionError,          /* Server and client version are not identical */
    /* AbsInt end */
} XBGameConfigResult;

/*
 * global prototypes
 */

/* init */
extern void Network_ClearHost (unsigned id);
extern void Network_Clear (void);

/* events */
extern void Network_ClearEvents (void);
extern XBNetworkEvent Network_GetEvent (unsigned *pId);
extern void Network_QueueEvent (XBNetworkEvent msg, unsigned id);

/* type of network */
extern XBNetworkType Network_GetType (void);
extern void Network_SetType (XBNetworkType type);

/* local host id */
extern void Network_ReceiveLocalHostId (unsigned id);
extern unsigned char Network_LocalHostId (void);

/* ping times */
extern int Network_GetPingTime (unsigned id);
extern void Network_ReceivePing (unsigned id, int ping);

/* player atoms */
extern XBAtom Network_GetPlayer (unsigned id, int player);
extern XBAtom Network_GetPlayer2 (unsigned id, int player);
extern void Network_SetPlayer (unsigned id, int player, XBAtom atom);
extern void Network_SetPlayer2 (unsigned id, int player, XBAtom atom);
extern XBBool Network_GetFirstOtherPlayer (unsigned char id, unsigned char pl, unsigned char *host,
										   unsigned char *player);
extern XBBool Network_GetNextOtherPlayer (unsigned char id, unsigned char pl, unsigned char *host,
										  unsigned char *player);
/* host data */
extern unsigned Network_HostPlayerMax (unsigned);

/* game configs */
extern XBGameConfigResult Network_CreateGlobalGameConfig (CFGGame * cfg);
extern XBGameConfigResult Network_ReceiveGameConfig (unsigned id, const char *data);
extern unsigned Network_CreateLocalPlayers (unsigned);
extern unsigned Network_CreateGlobalPlayers (unsigned);
extern unsigned Network_GetMaskBytes (void);

/* player configs */
extern XBAtom Network_ReceivePlayerConfig (CFGType cfgType, unsigned id, int player,
										   const char *line);

/* state requests */
extern XBBool Network_ReceiveHostState (unsigned id, XBHostState state);
extern XBBool Network_ReceiveTeamState (unsigned host, unsigned player, XBTeamState team);
extern XBBool Network_ReceiveHostStateReq (unsigned who, unsigned id, XBHostState state);
extern XBBool Network_ReceiveTeamStateReq (unsigned who, unsigned host, unsigned player,
										   XBTeamState team);
extern void Network_SetDefaultTeams (unsigned host, unsigned player);
extern XBTeamState Network_GetDefaultTeam (unsigned id, unsigned player);
extern XBHostState Network_GetHostState (unsigned id);
extern XBBool Network_HostIsIn (unsigned id);
extern XBHostState *Network_GetHostStateReq (unsigned id);
extern XBTeamState Network_GetTeamState (unsigned id, unsigned pl);
extern XBTeamState *Network_GetTeamStateReq (unsigned id, unsigned pl);
extern XBBool Network_HostReqClientsAgree (unsigned host, XBHostState state);
extern XBBool Network_ClientsReady (void);
extern XBBool Network_TeamReqClientsAgree (unsigned host, unsigned player, unsigned state);

#endif
/*
 * end of file network.h
 */
