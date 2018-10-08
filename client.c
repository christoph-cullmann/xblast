/*
 * file client.c - communication interface for clients
 *
 * $Id: client.c,v 1.69 2006/03/28 11:41:19 fzago Exp $
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

#include "xblast.h"

/*
 * local macros
 */
#define TIME_POLL_QUERY 5		/* unused */

/*
 * local types
 */

/* type for list of network games received, LAN or Central */
typedef struct _xb_network_game_list XBNetworkGameList;
struct _xb_network_game_list
{
	XBNetworkGameList *next;
	XBNetworkGame game;
};

/*
 * local variables
 */

/* Client connections */
static XBComm *comm = NULL;		/* XBCommToServer - TCP connection to server */
static XBComm *dgram = NULL;	/* XBCommDgramServer - UDP connection to server */
static XBComm **query = NULL;	/* List of XBCommQuery's - UDP connections to LAN/Central */

/* list of received network games, for Search LAN/Central */
static XBNetworkGameList *firstGame = NULL;
static XBNetworkGameList *lastGame = NULL;
static XBNetworkGameList *nextGame = NULL;
static unsigned numGames = 0;

/* player actions received for game time and each in-game player */
/* TODO: check player counts when new clients connect ! */
static PlayerAction playerAction[GAME_TIME + 1][MAX_PLAYER];

/* flags for level negotiation status */
static XBBool rejected;
static XBBool fixed;

/* flag for async level result */
static XBBool async;

/*
 * local prototypes
 */
static void PollDatagram (const struct timeval *tv);

/****************************
 * connecting/disconnecting *
 ****************************/

/*
 * try to connect to server: join attempt
 */
XBBool
Client_Connect (CFGGameHost * cfg)
{
	/* clear network and chat data */
	Dbg_Client ("clearing all network and chat data\n");
	Network_Clear ();
	Chat_Clear ();
	Chat_Listen (XBTrue);
	/* create communication */
	Dbg_Client ("trying to join game at %s:%u\n", cfg->name, cfg->port);
	assert (comm == NULL);
	comm = C2S_CreateComm (cfg);
	if (NULL == comm) {
		Dbg_Client ("failed to connect!\n");
		return XBFalse;
	}
	Dbg_Client ("Connected!\n");
	Network_SetType (XBNT_Client);
	return XBTrue;
}								/* Client_Connect */

/*
 * close dgram connection to server
 */
static void
Client_DgramDisconnect (void)
{
	if (dgram != NULL) {
		Dbg_Client ("disconnecting Dgram to server\n");
		/* stop polling */
		GUI_SubtractPollFunction (PollDatagram);
		Dbg_Client ("stopping to poll for pings\n");
		/* delete connection */
		CommDelete (dgram);
		dgram = NULL;
	}
}								/* Client_DgramDisconnect */

/*
 * close stream connection to server
 */
static void
Client_StreamDisconnect (void)
{
	if (comm != NULL) {
		Dbg_Client ("disconnecting Stream to server\n");
		CommDelete (comm);
		comm = NULL;
	}
}								/* Client_StreamDisconnect */

/*
 * close all connections to server
 */
void
Client_Disconnect (void)
{
	Network_Clear ();
	Network_ClearEvents ();
	Client_StreamDisconnect ();
	Client_DgramDisconnect ();
	Chat_Listen (XBFalse);
}								/* Client_Disconnect */

/***************************
 * poll function for dgram *
 ***************************/

/*
 * polling for datagram connections: send pings, disconnect on timeout
 */
static void
PollDatagram (const struct timeval *tv)
{
	if (NULL != dgram && !D2S_Connected (dgram)) {
		Dbg_Client ("queueing connection ping to server\n");
		D2S_SendConnect (dgram);
	}
	if (NULL != dgram && D2S_Timeout (dgram, tv)) {
		/* disconnect from server */
		Dbg_Client ("dgram to server timed out\n");
		Client_Disconnect ();
		Network_QueueEvent (XBNW_Error, 0);
		GUI_SendEventValue (XBE_SERVER, XBSE_ERROR);
	}
}								/* PollDatagram */

/******************
 * error handling *
 ******************/

/*
 * stream event occurred
 */
XBBool
Client_StreamEvent (XBClientConstants code)
{
	switch (code) {
	case XBCC_IDInvalid:
		Dbg_Client ("invalid id received from server, ignoring\n");
		return XBFalse;
	case XBCC_DataInvalid:
		Dbg_Client ("invalid data received from server, ignoring\n");
		return XBFalse;
	case XBCC_COTInvalid:
		Dbg_Client ("invalid cot received from server, disconnecting\n");
		break;
	case XBCC_IOError:
		Dbg_Client ("i/o error on stream to server, disconnecting\n");
		break;
	case XBCC_ExpectedEOF:
		Dbg_Client ("server finishes disconnect announcement\n");
		return XBFalse;
	case XBCC_UnexpectedEOF:
		Dbg_Client ("unexpected server disconnect\n");
		break;
	case XBCC_StreamWaiting:
		/* Dbg_Client("all queued data sent to server\n"); */
		return XBFalse;
	case XBCC_StreamBusy:
		/* Dbg_Client("data remains to be sent to server\n"); */
		return XBFalse;
	case XBCC_StreamClosed:
		Dbg_Client ("connection to server removed\n");
		comm = NULL;
		return XBFalse;
	default:
		Dbg_Client ("unknown error (%u) on stream to server, disconnecting\n", code);
	}
	Client_DgramDisconnect ();
	Network_Clear ();
	Network_ClearEvents ();
	Chat_Listen (XBFalse);
	/* queue event for GUI display */
	Network_QueueEvent (XBNW_Error, 0);
	GUI_SendEventValue (XBE_SERVER, XBSE_ERROR);
	return XBTrue;
}								/* Client_StreamError */

/*
 * handle events on dgram
 */
XBBool
Client_DgramEvent (XBClientConstants code)
{
	switch (code) {
	case XBCC_Loss:
		Dbg_Client ("data loss from server, continuing!\n");
		return XBFalse;
	case XBCC_WriteError:
		Dbg_Client ("write error on dgram to server, disconnecting\n");
		break;
	case XBCC_ConnFailed:
		Dbg_Client ("dgram failed to connect to server, disconnecting\n");
		break;
	case XBCC_DgramClosed:
		Dbg_Client ("dgram to server is removed\n");
		dgram = NULL;
		return XBFalse;
	default:
		Dbg_Client ("unknown event (%u) on dgram to server, disconnecting\n", code);
	}
	Network_Clear ();
	Network_ClearEvents ();
	Chat_Listen (XBFalse);
	Client_StreamDisconnect ();
	/* queue event for GUI display */
	Network_QueueEvent (XBNW_Error, 0);	/* TODO: put in more info here for GUI */
	GUI_SendEventValue (XBE_SERVER, XBSE_ERROR);
	return XBTrue;
}								/* Client_DgramEvent */

/******************
 * receiving data *
 ******************/

/*
 * override game config with local options
 */
static void
OverrideGameConfig (void)
{
	CFGGameHost cfgHost;
	CFGGameSetup cfgLocal;
	CFGGameSetup cfgRemote;
	/* set correct server name */
	if (RetrieveGameHost (CT_Remote, SERVERGAMECONFIG, &cfgHost)) {
		cfgHost.name = C2S_ServerName (comm);
		Dbg_Client ("setting server name to %s\n", cfgHost.name);
		StoreGameHost (CT_Remote, SERVERGAMECONFIG, &cfgHost);
	}
	/* set local overrides to game setup */
	if (RetrieveGameSetup (CT_Remote, SERVERGAMECONFIG, &cfgRemote) &&
		RetrieveGameSetup (CT_Local, atomClient, &cfgLocal)) {
		cfgRemote.recordDemo = cfgLocal.recordDemo;
		cfgRemote.bot = cfgLocal.bot;
		cfgRemote.beep = cfgLocal.beep;
		cfgRemote.Music = cfgLocal.Music;
		StoreGameSetup (CT_Remote, SERVERGAMECONFIG, &cfgRemote);
	}
}								/* OverrideGameConfig */

/*
 * receive game config from server
 */
XBBool
Client_ReceiveGameConfig (unsigned id, const char *line)
{
	unsigned num;
	/* receive the data */
	switch (Network_ReceiveGameConfig (id, line)) {
	case XBGC_Unfinished:
		return XBFalse;
	case XBGC_HostInvalid:
		Client_Disconnect ();
		Network_QueueEvent (XBNW_Error, 0);
		Dbg_Client ("game config for too large host id #%u received, disconnect\n", id);
		GUI_ErrorMessage ("host id=%u too large, probably SMPF-server\n", id);
		return XBFalse;
	case XBGC_Empty:
		Dbg_Client ("server sent empty game config for host #%u\n", id);
		num = Network_CreateLocalPlayers (id);
		if (num != 0) {
			Dbg_Client ("failed to initialize local game config, disconnect\n");
			Client_Disconnect ();
			Network_QueueEvent (XBNW_Error, 0);
			return XBFalse;
		}
		return XBTrue;
	case XBGC_TooManyPlayers:
		Client_Disconnect ();
		Network_QueueEvent (XBNW_Error, 0);
		Dbg_Client ("too many players received, host id #%u, disconnected\n", id);
		GUI_ErrorMessage ("too many players received, disconnected\n", id);
		return XBFalse;
	case XBGC_NoVersion:
		Dbg_Client ("server sent no version, disconnect\n");
		Client_Disconnect ();
		Network_QueueEvent (XBNW_Error, 0);
		return XBFalse;
	case XBGC_Local:
		Dbg_Client ("server sent local game config for host #%u\n", id);
		if (id == 0) {
			OverrideGameConfig ();
		}
		num = Network_CreateLocalPlayers (id);
		if (num > NUM_LOCAL_PLAYER) {
			Dbg_Client ("failed to create local players, disconnect\n");
			Client_Disconnect ();
			Network_QueueEvent (XBNW_Error, 0);
			return XBFalse;
		}
		Dbg_Client ("created %u players at host #%u\n", num, id);
		Network_QueueEvent (XBNW_GameConfig, id);
		return XBTrue;
		break;
	case XBGC_Global:
		Dbg_Client ("server sent global game config for host #%u\n", id);
		if (id > 0) {
			Dbg_Client ("ignoring global game config from client\n");
			DeleteGameConfig (CT_Remote, LOCALGAMECONFIG (id));
			return XBFalse;
		}
		OverrideGameConfig ();
		num = Network_CreateGlobalPlayers (id);
		if (num > MAX_PLAYER) {
			Dbg_Client ("failed to create global players, disconnect\n");
			Client_Disconnect ();
			Network_QueueEvent (XBNW_Error, 0);
			return XBFalse;
		}
		Dbg_Client ("created %u global players from game config #%u\n", num, id);
		num = Network_GetMaskBytes ();
		if (num == 0) {
			Dbg_Client ("failed to set mask bytes, disconnect\n");
			Client_Disconnect ();
			Network_QueueEvent (XBNW_Error, 0);
			return XBFalse;
		}
		D2S_SetMaskBytes (dgram, num);
		Network_QueueEvent (XBNW_GameConfig, id);
		return XBTrue;
	case XBGC_Error:
		Dbg_Client ("server sent invalid game config for host #%u, disconnect\n", id);
		Client_Disconnect ();
		Network_QueueEvent (XBNW_Error, 0);
		return XBFalse;
    /* AbsInt start */
    case XBGC_VersionError:
        Dbg_Client("server and client version are not identical, disconnect\n");
        GUI_ErrorMessage("server and client version are not identical, please restart your xblast client.\n");
        Client_Disconnect();
        Network_QueueEvent (XBNW_Error, 0);
        return XBFalse;
    /* AbsInt end */
	default:
		break;
	}
	return XBFalse;
}								/* Client_ReceiveGameConfig */

/*
 * receive player config from server
 */
void
Client_ReceivePlayerConfig (unsigned id, unsigned player, const char *data)
{
	/* disconnect on range check error */
	if (id >= MAX_HOSTS || player >= NUM_LOCAL_PLAYER) {
		Dbg_Client ("receiving player config for out of range player #%u(%u), disconnect\n", id,
					player);
		Client_Disconnect ();
		Network_QueueEvent (XBNW_Error, 0);
		GUI_ErrorMessage ("cannot handle player id %u(%u)\n", id, player);
	}
	if (ATOM_INVALID != Network_ReceivePlayerConfig (CT_Remote, id, player, data)) {
		Dbg_Client ("received player config for #%u(%u)\n", id, player);
	}
}								/* Client_ReceivePlayerConfig */

/*
 * receive dgram port on server
 */
void
Client_ReceiveDgramPort (unsigned id, unsigned short port)
{
	CFGGameHost cfgSrv, cfgLoc;
	XBBool usingNat = XBFalse;

	assert (comm != NULL);
	assert (dgram == NULL);

	/* this is the first time we learn our id from server */
	Network_ReceiveLocalHostId (id);
	/* first check if we are using NAT to get to the server */
	Dbg_Client ("receiving dgram port #%u on server\n", port);
	if (id != 0 && id < MAX_HOSTS) {
		RetrieveGameHost (CT_Remote, SERVERGAMECONFIG, &cfgSrv);
		Dbg_Client ("server is %s\n", cfgSrv.name);
		RetrieveGameHost (CT_Remote, LOCALGAMECONFIG (id), &cfgLoc);
		Dbg_Client ("server sees me as %s\n", cfgLoc.name);
		Dbg_Client ("local ip for stream connection = %s\n", C2S_ClientName (comm));
		Dbg_Client ("peer ip for stream connection = %s\n", C2S_ServerName (comm));
		if (0 != strcmp (cfgLoc.name, C2S_ClientName (comm))) {
			Dbg_Client ("server does not see local ip -> NAT detected!\n");
			usingNat = XBTrue;
		}
	}
#ifdef DEBUG_NAT
	usingNat = XBTrue;
	Dbg_Client ("forcing NAT by compile flag\n");
#endif
	dgram = D2S_CreateComm (C2S_ClientName (comm), C2S_ServerName (comm), port);
	if (NULL == dgram) {
		Dbg_Client ("failed to open dgram connection\n");
		/* disconnect stream */
		Client_StreamDisconnect ();
		return;
	}
	Client_SendDgramPort (usingNat);
	SetHostType (XBPH_Client1 + id - 1);
	/* poll connection */
	Dbg_Client ("starting to poll for pings\n");
	GUI_AddPollFunction (PollDatagram);
}								/* Client_ReceiveDgramPort */

/*
 * received ping time for one client
 */
void
Client_ReceivePingTime (unsigned id, int ping)
{
	Network_ReceivePing (id, ping);
}								/* Client_ReceivePingTime */

/*
 * receive request for game config
 */
void
Client_ReceiveGameConfigReq (unsigned id)
{
	CFGGamePlayers cfgPlayers;
	CFGGameConst cfgConst;
	Dbg_Client ("server requests game config\n");
	/* copy game players and constants from Local/Client to Remote/Local */
	(void)RetrieveGamePlayers (CT_Local, atomClient, &cfgPlayers);
	StoreGamePlayers (CT_Remote, atomLocal, &cfgPlayers);
	(void)RetrieveGameConst (CT_Local, atomClient, &cfgConst);
	StoreGameConst (CT_Remote, atomLocal, &cfgConst);
	/* send filtered config from database */
	if (!C2S_SendGameConfig (comm, CT_Remote, atomLocal)) {
		Dbg_Client ("failed to queue game config!\n");
		C2S_GameDataNotAvailable (comm);
		return;
	}
	Dbg_Client ("successfully queued game config for %u players\n", cfgPlayers.num);
}								/* Client_ReceiveGameConfigReq */

/*
 * receive request for game config
 */
void
Client_ReceivePlayerConfigReq (unsigned player)
{
	CFGGamePlayers cfgPlayers;

	Dbg_Client ("server requests player config for player %u(%u)\n", Network_LocalHostId (),
				player);
	/* check player index */
	if (player >= NUM_LOCAL_PLAYER) {
		Dbg_Client ("invalid player number !\n");
		C2S_PlayerDataNotAvailable (comm, player);
		return;
	}
	/* get player atom from config */
	if (!RetrieveGamePlayers (CT_Local, atomClient, &cfgPlayers)) {
		Dbg_Client ("no local players !\n");
		C2S_PlayerDataNotAvailable (comm, player);
		return;
	}
	/* check player */
	if (player >= cfgPlayers.num) {
		Dbg_Client ("only %u local players !\n", cfgPlayers.num);
		C2S_PlayerDataNotAvailable (comm, player);
		return;
	}
	if (ATOM_INVALID == cfgPlayers.player[player]) {
		Dbg_Client ("player config empty !\n");
		C2S_PlayerDataNotAvailable (comm, player);
		return;
	}
	/* send config from database */
	if (!C2S_SendPlayerConfig (comm, CT_Local, cfgPlayers.player[player], player, XBFalse)) {
		Dbg_Client ("failed to queue !\n");
		C2S_PlayerDataNotAvailable (comm, player);
		return;
	}
	Dbg_Client ("successfully queued player config for %u(%u)\n", Network_LocalHostId (), player);
}								/* Client_ReceivePlayerConfigReq */

/*
 * receive host state for a host
 */
void
Client_ReceiveHostState (unsigned id, unsigned state)
{
	if (Network_ReceiveHostState (id, state)) {
		Dbg_Client ("received host state #%u = %u\n", id, state);
	}
	else {
		Dbg_Client ("received invalid host state\n");
	}
}								/* Client_ReceiveHostState */

/*
 * receive host state request for a host
 */
void
Client_ReceiveHostStateReq (unsigned who, unsigned id, unsigned state)
{
	if (Network_ReceiveHostStateReq (who, id, state)) {
		Dbg_Client ("received host state request #%u -> %u by %u\n", id, state, who);
	}
	else {
		Dbg_Client ("received invalid host state request\n");
	}
}								/* Client_ReceiveHostStateReq */

/*
 * receive team state for a host/player
 */
void
Client_ReceiveTeamState (unsigned host, unsigned player, unsigned team)
{
	if (Network_ReceiveTeamState (host, player, team)) {
		Dbg_Client ("received team state #%u(%u) = %u\n", host, player, team);
	}
	else {
		Dbg_Client ("received invalid team state\n");
	}
}								/* Client_ReceiveTeamState */

/*
 * receive team state for a host/player
 */
void
Client_ReceiveTeamStateReq (unsigned who, unsigned host, unsigned player, unsigned team)
{
	if (Network_ReceiveTeamStateReq (who, host, player, team)) {
		Dbg_Client ("received team state request #%u(%u) -> %u by %u\n", host, player, team, who);
	}
	else {
		Dbg_Client ("received invalid team state request\n");
	}
}								/* Client_ReceiveTeamStateReq */

/*
 * receive level config from server
 */
void
Client_ReceiveLevelConfig (unsigned iob, const char *data)
{
	if (NULL != data) {
		AddToRemoteLevelConfig (iob, data);
		return;
	}
	/* yes, all data received */
	Dbg_Client ("checking remote level for version\n");
	Client_LevelRejection (!CheckRemoteLevelConfig ());
	Network_QueueEvent (XBNW_LevelConfig, 0);
}								/* Client_ReceiveLevelConfig */

/*
 * receive info about a disconnection
 */
XBBool
Client_ReceiveDisconnect (unsigned id)
{
	if (id == 0) {
		Dbg_Client ("server announces disconnect, partial shutdown!\n");
		/* partial shutdown */
		Client_DgramDisconnect ();
		Network_Clear ();
		Chat_Listen (XBFalse);
		Network_QueueEvent (XBNW_Disconnected, id);
		GUI_SendEventValue (XBE_SERVER, XBSE_ERROR);
		return XBTrue;
	}
	if (id < MAX_HOSTS) {
		Dbg_Client ("host #%u disconnected\n", id);
		Network_ClearHost (id);
	}
	else {
		Dbg_Client ("host id too large, ignoring (probably SMPF server)\n");
		return XBFalse;
	}
	Network_QueueEvent (XBNW_Disconnected, id);
	return XBFalse;
}								/* Client_ReceiveDisconnect */

/*
 * receive chat line
 */
void
Client_ReceiveChat (XBChat * chat)
{
	Chat_Receive (chat);
}								/* Client_ReceiveChat */

/*
 * receive game start signal from server
 */
void
Client_ReceiveStart (unsigned id)
{
	Dbg_Out ("received start of game!\n");
	Network_QueueEvent (XBNW_StartGame, id);
}								/* Client_ReceiveStart */

/*
 * receive seed for random number generator
 */
void
Client_ReceiveRandomSeed (unsigned seed)
{
	Dbg_Out ("receive seed %u\n", seed);
	SeedRandom (seed);
}								/* Client_ReceiveRandomSeed */

/*
 * server notified that all clients reached sync point
 */
void
Client_ReceiveSync (XBNetworkEvent event)
{
	Dbg_Out ("receive sync, raising event %u\n", event);
	/* TODO filter events */
	Network_QueueEvent (event, 0);
}								/* Client_ReceiveSync */

/*
 * receive action keys for a gametime
 */
void
Client_ReceivePlayerAction (int gameTime, const PlayerAction * keys)
{
	assert (gameTime <= GAME_TIME);
	/* inform application */
	GUI_SendEventValue (XBE_SERVER, gameTime);
	/* copy keys (simple version) */
	memcpy (&playerAction[gameTime][0], keys, MAX_PLAYER * sizeof (PlayerAction));
}								/* Client_ReceivePlayerAction */

/*
 * receive level finish
 */
void
Client_ReceiveFinish (void)
{
	/* inform application */
	Dbg_Client ("receive level finish\n");
	GUI_SendEventValue (XBE_SERVER, XBSE_FINISH);
}								/* Client_ReceiveFinish */

/*
 * receive async status from server
 */
void
Client_ReceiveAsync (unsigned ev)
{
	/* ev always equal to XBNW_SyncLevelResult */
	Dbg_Client ("receiving level result async\n");
	async = XBTrue;
	Network_QueueEvent (ev, XBTrue);
}								/* Client_ReceiveAsync */

/************************
 * local data retrieval *
 ************************/

/*
 * retrieve host state for a host
 */
unsigned
Client_GetHostState (unsigned id)
{
	return Network_GetHostState (id);
}								/* Client_GetHostState */

/*
 * retrieve host state requests for a host
 */
unsigned *
Client_GetHostStateReq (unsigned id)
{
	return Network_GetHostStateReq (id);
}								/* Client_GetHostStateReq */

/*
 * retrieve team state for a host/player
 */
unsigned
Client_GetTeamState (unsigned id, unsigned pl)
{
	return Network_GetTeamState (id, pl);
}								/* Client_GetHostState */

/*
 * retrieve team state requests for a host/player
 */
unsigned *
Client_GetTeamStateReq (unsigned id, unsigned pl)
{
	return Network_GetTeamStateReq (id, pl);
}								/* Client_GetTeamStateReq */

/*
 * query rejection status for level config
 */
XBBool
Client_RejectsLevel (void)
{
	return rejected;
}								/* Client_RejectsLevel */

/*
 * query fixed status for a level config
 */
XBBool
Client_FixedLevel (void)
{
	return fixed;
}								/* Client_Fixed level */

/*
 * get actions for given gametime
 */
void
Client_GetPlayerAction (int gameTime, PlayerAction * keys)
{
	assert (gameTime <= GAME_TIME);
	/* copy keys (simple version) */
	memcpy (keys, &playerAction[gameTime][0], MAX_PLAYER * sizeof (PlayerAction));
}								/* Client_PlayerAction */

/*
 * get ping time of server to given host
 */
int
Client_GetPingTime (unsigned clientID)
{
	return Network_GetPingTime (clientID);
}								/* Client_GetPingTime */

/*
 * query async status for level
 */
XBBool
Client_LevelAsynced (void)
{
	return async;
}								/* Client_LevelAsynced */

/**********************
 * local data setting *
 **********************/

/*
 * manually set rejection status for a level config
 */
void
Client_LevelRejection (XBBool rej)
{
	Dbg_Client ("setting level rejection to %u\n", rej);
	rejected = rej;
}								/* Client_LevelRejection */

/*
 * manually set fixed status for a level config
 */
void
Client_LevelFix (XBBool fx)
{
	Dbg_Client ("setting level fix to %u\n", fx);
	fixed = fx;
}								/* Client_LevelFix */

/*
 * reset or fix a level config
 */
void
Client_ActivateLevel (unsigned how)
{
	if (how == 0) {
		Dbg_Client ("received level reset from server\n");
		Client_LevelRejection (XBTrue);
		/* clear level config in game_client: LevelFromServer */
	}
	else {
		Dbg_Client ("received level activation from server\n");
		Client_LevelFix (XBTrue);
	}
	Network_QueueEvent (XBNW_LevelConfig, how);
}								/* Client_Activate level */

/*
 * manually set async status for level
 */
void
Client_SetLevelAsync (XBBool as)
{
	Dbg_Client ("setting async to %u\n", as);
	async = as;
}								/* Client_SetLevelAsync */

/*
 * clear all action data
 */
void
Client_ClearPlayerAction (void)
{
	unsigned gt;
	int player;
	for (gt = 0; gt < GAME_TIME + 1; gt++) {
		for (player = 0; player < MAX_PLAYER; player++) {
			playerAction[gt][player].player = player;
			playerAction[gt][player].dir = GoDefault;
			playerAction[gt][player].bomb = XBFalse;
			playerAction[gt][player].special = XBFalse;
			playerAction[gt][player].pause = XBFalse;
			playerAction[gt][player].suicide = XBFalse;
			playerAction[gt][player].abort = ABORT_NONE;
		}
	}
	/* Dbg_Client("cleared action for all hosts\n"); */
}								/* Client_ClearPlayerAction */

/*
 * reset datagram connection
 */
void
Client_ResetPlayerAction (void)
{
	if (NULL != dgram) {
		Dbg_Client ("resetting dgram player actions\n");
		D2S_Reset (dgram);
	}
}								/* Client_ResetPlayerAction */

/******************
 * queue data out *
 ******************/

/*
 * queue dgram port to server
 */
void
Client_SendDgramPort (XBBool nat)
{
	assert (NULL != comm);
	if (!nat) {
		/* send port to server */
		C2S_SendDgramPort (comm, D2S_Port (dgram));
	}
	else {
		/* send "any port" to notify use of n.a.t.  */
		C2S_SendDgramPort (comm, 0);
	}
	Dbg_Client ("queued dgram port to server\n");
}								/* Client_SendDgramPort */

/*
 * queue game config to server
 */
void
Client_SendGameConfig (void)
{
	/* TODO: currently only for completeness, move from com_to_server.c */
}								/* Client_SendGameConfig */

/*
 * queue player configs to server
 */
void
Client_SendPlayerConfigs (void)
{
	/* TODO: currently only for completeness, move from com_to_server.c */
}								/* Client_SendPlayerConfigs */

/*
 * queue a chat to server
 */
void
Client_SendChat (XBChat * chat)
{
	if (NULL != comm) {
		Dbg_Client ("queueing chat line\n");
		C2S_SendChat (comm, chat);
	}
}								/* Client_SendChat */

/*
 * queue sync point
 */
void
Client_SendSync (XBNetworkEvent event)
{
	if (NULL != comm) {
		Dbg_Client ("queueing sync event #%u on stream\n", event);
		C2S_Sync (comm, event);
	}
}								/* Client_SendSync */

/*
 * queue local host state
 * TODO: remove, only temporarily needed for client toggle button
 */
void
Client_SendHostState (unsigned state)
{
	if (NULL != comm) {
		Dbg_Client ("queueing local host state #%u on stream\n", state);
		C2S_SendHostState (comm, state);
	}
}								/* Client_SendHostState */

/*
 * queue local host state request
 */
void
Client_SendHostStateReq (unsigned id, unsigned state)
{
	if (NULL != comm) {
		Dbg_Client ("queueing local host state request #%u->%u on stream\n", id, state);
		C2S_SendHostStateReq (comm, id, state);
	}
}								/* Client_SendHostStateReq */

/*
 * queue local team state request
 */
void
Client_SendTeamStateReq (unsigned id, unsigned player, unsigned team)
{
	if (NULL != comm) {
		Dbg_Client ("queueing team state request #%u(%u)->%u on stream\n", id, player, team);
		C2S_SendTeamStateReq (comm, id, player, team);
	}
}								/* Client_SendTeamStateReq */

/*
 * queue level negotiation status to server
 */
void
Client_SendLevelCheck (void)
{
	if (NULL != comm) {
		Dbg_Client ("queueing LevelCheck %u on stream\n", rejected);
		C2S_LevelCheck (comm, rejected);
	}
}								/* Client_SendLevelCheck */

/*
 * queue own action at gametime to server
 */
void
Client_SendPlayerAction (int gameTime, const PlayerAction * keys)
{
	D2S_SendPlayerAction (dgram, gameTime, keys);
}								/* Client_SendPlayerAction */

/*
 * queue end of level to server
 */
void
Client_FinishPlayerAction (int gameTime)
{
	Dbg_Client ("queueing finish at %u on dgram\n", gameTime);
	D2S_SendFinish (dgram, gameTime);
}								/* Client_FinishPlayerAction */

/*
 * flush out remaining data
 */
XBBool
Client_FlushPlayerAction (void)
{
	return D2S_Flush (dgram);
}								/* Client_FlushPlayerAction */

/*
 * queue winner team to server
 */
void
Client_SendWinner (unsigned team)
{
	if (NULL != comm) {
		Dbg_Client ("queueing winner team %u\n", team);
		C2S_SendWinner (comm, team);
	}
}								/* Client_SendWinner */

/*------------------------------------------------------------------------*
 *
 * query for local and remote games
 *
 *------------------------------------------------------------------------*/

/*
 * delete current list of network games
 */
static void
DeleteGameList (void)
{
	XBNetworkGameList *next;

	while (firstGame != NULL) {
		next = firstGame->next;
		/* delete data */
		free (firstGame->game.host);
		free (firstGame->game.game);
		free (firstGame->game.version);
		free (firstGame);
		firstGame = next;
	}
	lastGame = NULL;
	nextGame = NULL;
	numGames = 0;
}								/* DeleteGameList */

/*
 * Search lan query
 * try to establish COMM_Query sockets for each interface
 * send query on all created sockets
 */
void
Client_StartQuery (void)
{
	size_t numInter;
	const XBSocketInterface *inter;
	size_t i, j;

	assert (NULL == query);
	inter = Socket_GetInterfaces (&numInter);
	if (NULL == inter) {
		Dbg_Client ("failed to find network interfaces, cancelling LAN query\n");
		return;
	}
	/* alloc maximum possible pointers */
	query = calloc (1 + numInter, sizeof (XBComm *));
	assert (NULL != query);
	/* start query on each broadcast device */
	for (i = 0, j = 0; i < numInter; i++) {
		if (NULL != inter[i].addrBroadcast &&
			NULL != (query[j] =
					 Query_CreateComm (i, inter[i].addrDevice, inter[i].addrBroadcast, 16168,
									   XBTrue))) {
			Dbg_Client ("starting LAN query on #%lu to %s\n", (unsigned long)i, inter[i].addrBroadcast);
			j++;
		}
		else {
			Dbg_Client ("failed to start LAN query on #%lu to %s\n", (unsigned long)i, inter[i].addrBroadcast);
		}
	}
	Client_RestartQuery ();
}								/* Client_StartQuery */

/*
 * XBCC Search central query
 * try to establish COMM_Query sockets for each interface
 * send query on all created sockets
 */
void
Client_StartCentralQuery (void)
{
	size_t numInter;
	const XBSocketInterface *inter;
#ifdef FULL_CENTRAL_QUERY
	size_t i, j;
#endif
	CFGCentralSetup centralSetup;

	assert (NULL == query);
	inter = Socket_GetInterfaces (&numInter);
	if (NULL == inter) {
		Dbg_Client ("failed to find network interfaces, cancelling central query\n");
		return;
	}
	RetrieveCentralSetup (&centralSetup);
	if (NULL == centralSetup.name) {
		Dbg_Client ("failed to find central data, cancelling central query\n");
		return;
	}
#ifdef FULL_CENTRAL_QUERY
	/* alloc comm for each interface plus a NULL pointer (to central) */
	query = calloc (1 + numInter, sizeof (XBComm *));
	assert (NULL != query);
	/* start query on default device */
	for (i = 0, j = 0; i < numInter; i++) {
		if (NULL !=
			(query[j] =
			 Query_CreateComm (i, inter[i].addrDevice, centralSetup.name, centralSetup.port,
							   XBFalse))) {
			Dbg_Client ("starting central query on #%u to %s\n", i, centralSetup.name);
			j++;
		}
		else {
			Dbg_Client ("failed to start central query on #%u to %s\n", i, centralSetup.name);
		}
	}
#else
	/* alloc comm for default interface plus a NULL pointer (to central) */
	query = calloc (1 + 1, sizeof (XBComm *));
	assert (NULL != query);
	/* start query on default device */
	if (NULL !=
		(query[0] = Query_CreateComm (0, NULL, centralSetup.name, centralSetup.port, XBFalse))) {
		Dbg_Client ("starting central query to %s\n", centralSetup.name);
	}
	else {
		Dbg_Client ("failed to start central query to %s\n", centralSetup.name);
	}
#endif
	Client_RestartQuery ();
}								/* Client_StartCentralQuery */

/*
 * send a query on all valid COMM_Query sockets
 */
void
Client_RestartQuery (void)
{
	unsigned cnt = 0;
	DeleteGameList ();
	if (NULL != query) {
		struct timeval tv;
		int i;
		gettimeofday (&tv, NULL);
		for (i = 0; query[i] != NULL; i++) {
			if (!Query_isDeleted (query[i])) {
				Query_Send (query[i], &tv);
				cnt++;
			}
		}
		if (cnt > 0) {
			Dbg_Client ("requeued %u central/lan queries\n", cnt);
		}
		else {
			Dbg_Client ("no valid queries remaining, freeing memory\n");
			Client_StopQuery ();
		}
	}
}								/* Client_RestartQuery */

/*
 * stop query to central/lan
 */
void
Client_StopQuery (void)
{
	size_t i;

	DeleteGameList ();
	/* return if alredy stopped */
	if (NULL == query) {
		return;
	}
	/* delete all valid queries */
	for (i = 0; query[i] != NULL; i++) {
		if (!Query_isDeleted (query[i])) {
			CommDelete (query[i]);
		}
		free (query[i]);
	}
	/* stop */
	free (query);
	query = NULL;
}								/* Client_StopQuery */

/*
 * receive a close event on a query socket
 */
void
Client_ReceiveQueryClose (unsigned id)
{
	Dbg_Client ("query on interface #%u is closed\n", id);	/* nothing else to do */
}								/* Client_ReceiveQueryClose */

/*
 * receive reply from a game server on lan or central
 */
void
Client_ReceiveReply (unsigned id, const char *host, unsigned short port, int ping,
					 const char *version, const char *game, int numLives, int numWins,
					 int frameRate)
{
	XBNetworkGameList *ptr;

	/* alloc data */
	ptr = calloc (1, sizeof (*ptr));
	assert (NULL != ptr);
	/* fill in data */
	ptr->game.id = id;
	ptr->game.host = DupString (host);
	ptr->game.port = port;
	ptr->game.ping = ping;
	ptr->game.version = DupString (version);
	ptr->game.game = DupString (game);
	ptr->game.numLives = numLives;
	ptr->game.numWins = numWins;
	ptr->game.frameRate = frameRate;
	/* append to list */
	if (NULL == lastGame) {
		firstGame = lastGame = nextGame = ptr;
	}
	else {
		lastGame->next = ptr;
		lastGame = ptr;
	}
	Dbg_Client ("received network game data on interface #%u\n", id);
	/* inform application */
	Network_QueueEvent (XBNW_NetworkGame, numGames++);
}								/* Client_ReceiveReply */

#ifdef unused
/*
 * game info for application
 */
static const XBNetworkGame *
Client_FirstNetworkGame (unsigned index)
{
	unsigned i;
	const XBNetworkGame *ptr;

	/* find index-th game */
	nextGame = firstGame;
	for (i = 0; i < index; i++) {
		if (nextGame == NULL) {
			return NULL;
		}
		nextGame = nextGame->next;
	}
	if (nextGame == NULL) {
		return NULL;
	}
	ptr = &nextGame->game;
	nextGame = nextGame->next;
	return ptr;
}								/* Client_FirstNetworkGame */
#endif

/*
 * game info for application
 */
const XBNetworkGame *
Client_NextNetworkGame (void)
{
	const XBNetworkGame *ptr;

	if (NULL == nextGame) {
		return NULL;
	}
	ptr = &nextGame->game;
	nextGame = nextGame->next;
	return ptr;
}								/* Client_NextNetworkGame */

/*
 * end of file client.c
 */
