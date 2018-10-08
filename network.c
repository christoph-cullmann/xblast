/*
 * file network.c - shared functions for server and clients
 *
 * $Id: network.c,v 1.42 2006/03/28 11:41:19 fzago Exp $
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
 * local types
 */
typedef struct _network_event
{
	struct _network_event *next;
	XBNetworkEvent msg;
	unsigned id;
} XBNetworkEventQueue;

typedef struct
{
	unsigned char host;
	unsigned char player;
} XBHostPlayerId;

/*
 * local variables
 */

/* events */
static XBNetworkEventQueue *queueFirst = NULL;
static XBNetworkEventQueue *queueLast = NULL;

/* network type */
static XBNetworkType nettype = XBNT_None;
static unsigned char localId = MAX_HOSTS;

/* player atoms and player count */
static unsigned players = 0;
static XBAtom hostPlayer[MAX_HOSTS][NUM_LOCAL_PLAYER];
static XBAtom hostPlayer2[MAX_HOSTS][NUM_LOCAL_PLAYER];

/* local player counts */
static unsigned localPlayers[MAX_HOSTS];

/* global config and conversion to local */
static XBBool global = XBFalse;
static CFGGame globalcfg;
static XBHostPlayerId s2l[MAX_PLAYER];
static unsigned char l2s[MAX_HOSTS][NUM_LOCAL_PLAYER];

/* ping times */
static unsigned hostPing[MAX_HOSTS];

/* host/team states per host */
static XBHostState hostState[MAX_HOSTS];
static XBTeamState teamState[MAX_HOSTS][NUM_LOCAL_PLAYER];

/* default team states */
static XBTeamState defTeam[MAX_HOSTS][NUM_LOCAL_PLAYER];

/* requested raw host/team states received for each host */
static XBHostState hostStateReq[MAX_HOSTS][MAX_HOSTS];
static XBTeamState teamStateReq[MAX_HOSTS][NUM_LOCAL_PLAYER][MAX_HOSTS];

/* constant team color assignment for team states */
const XBColor teamColors[NUM_XBTS] = {
	COLOR_INVALID, COLOR_INVALID, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_INVALID,
};

/***************
 * local stuff *
 ***************/

/*
 * reset host data
 */
static void
ClearHostLocalData (unsigned id)
{
	unsigned host;
	unsigned pl;
	/* update total player count first */
	players -= localPlayers[id];
	/* clear data for host id */
	hostPing[id] = -1;
	hostState[id] = XBHS_None;
	localPlayers[id] = 0;
	for (pl = 0; pl < NUM_LOCAL_PLAYER; pl++) {
		hostPlayer[id][pl] = ATOM_INVALID;
		hostPlayer2[id][pl] = ATOM_INVALID;
		teamState[id][pl] = XBTS_Invalid;
	}
	for (host = 0; host < MAX_HOSTS; host++) {
		hostStateReq[id][host] = XBHS_None;
		hostStateReq[host][id] = XBHS_None;
		for (pl = 0; pl < NUM_LOCAL_PLAYER; pl++) {
			defTeam[id][pl] = XBTS_Red;
			teamStateReq[id][pl][host] = XBTS_Invalid;
			teamStateReq[host][pl][id] = XBTS_Invalid;
		}
	}
}								/* ClearHostLocalData */

/******************
 * initialization *
 ******************/

/*
 * clear all host data
 */
void
Network_ClearHost (unsigned id)
{
	ClearHostLocalData (id);
	Version_Remove (id);
	DeleteGameConfig (CT_Remote, LOCALGAMECONFIG (id));
	Dbg_Network ("cleared host data #%u\n", id);
}								/* Network_ClearHost */

/*
 * clear all host, event, chat and version data
 */
void
Network_Clear (void)
{
	unsigned id, pl;
	/* clear data */
	localId = MAX_HOSTS;
	players = 0;
	memset (hostPlayer, 0, sizeof (hostPlayer));
	memset (hostPlayer2, 0, sizeof (hostPlayer2));
	memset (localPlayers, 0, sizeof (localPlayers));
	memset (hostPing, -1, sizeof (hostPing));
	memset (hostState, 0, sizeof (hostState));
	memset (hostStateReq, 0, sizeof (hostStateReq));
	memset (teamState, 0, sizeof (teamState));
	memset (teamStateReq, 0, sizeof (teamStateReq));
	/* clear remote game configs and versions in database */
	for (id = 0; id < MAX_HOSTS; id++) {
		DeleteGameConfig (CT_Remote, LOCALGAMECONFIG (id));
		for (pl = 0; pl < NUM_LOCAL_PLAYER; pl++) {
			defTeam[id][pl] = XBTS_Red;
		}
	}
	/* clear global game config and links */
	global = XBFalse;
	DeleteGameConfig (CT_Remote, SERVERGAMECONFIG);
	memset (s2l, 0xFF, sizeof (s2l));
	memset (l2s, 0xFF, sizeof (l2s));
	/* reset version management */
	Version_Reset ();
	Network_SetType (XBNT_None);
#ifdef DEBUG_NETWORK
	Dbg_Network ("cleared all host data\n");
#endif
}								/* Network_Clear */

/************************************
 * network events for client/server *
 ************************************/

#ifdef DEBUG_NETWORK
/*
 * network event to string
 */
static const char *
NWEventName (XBNetworkEvent msg)
{
	switch (msg) {
	case XBNW_None:
		return "None";
	case XBNW_Accepted:
		return "Accepted";
	case XBNW_GameConfig:
		return "GameConfig";
	case XBNW_RightPlayerConfig:
		return "RightPlayerConfig";
	case XBNW_LeftPlayerConfig:
		return "LeftPlayerConfig";
	case XBNW_Joy1PlayerConfig:
		return "Joy1PlayerConfig";
	case XBNW_Joy2PlayerConfig:
		return "Joy2PlayerConfig";
	case XBNW_Disconnected:
		return "Disconnected";
	case XBNW_StartGame:
		return "StartGame";
	case XBNW_EndOfInit:
		return "EndOfInit";
	case XBNW_LevelConfig:
		return "LevelConfig";
	case XBNW_SyncEndOfInit:
		return "SyncEndOfInit";
	case XBNW_SyncLevelIntro:
		return "SyncLevelIntro";
	case XBNW_SyncLevelResult:
		return "SyncLevelResult";
	case XBNW_SyncLevelEnd:
		return "SyncLevelEnd";
	case XBNW_SyncScoreboard:
		return "SyncScoreboard";
	case XBNW_HostIsIn:
		return "HostIsIn";
	case XBNW_HostIsOut:
		return "HostIsOut";
	case XBNW_Error:
		return "Error";
	case XBNW_PingReceived:
		return "PingReceived";
	case XBNW_NetworkGame:
		return "NetworkGame";
	case XBNW_TeamChange:
		return "TeamChange";
	case XBNW_TeamChangeData:
		return "TeamChangeData";
	case XBNW_HostChange:
		return "HostChange";
	default:
		return "unknown";
	}
}								/* EventName */
#endif

/*
 * clear event queue
 */
void
Network_ClearEvents (void)
{
	unsigned id;
	while (NULL != queueFirst) {
		(void)Network_GetEvent (&id);
	}
#ifdef DEBUG_NETWORK
	Dbg_Network ("clearing all network events\n");
#endif
}								/* Network_ClearEvents */

/*
 * add event to queue
 */
void
Network_QueueEvent (XBNetworkEvent msg, unsigned id)
{
	/* alloc data */
	XBNetworkEventQueue *ptr = calloc (1, sizeof (XBNetworkEventQueue));
	assert (ptr != NULL);
	/* set values */
	ptr->msg = msg;
	ptr->id = id;
	/* put in queue */
	if (queueLast != NULL) {
		queueLast->next = ptr;
	}
	else {
		queueFirst = ptr;
	}
	queueLast = ptr;
#ifdef DEBUG_NETWORK
	Dbg_Network ("queue network event %s %u\n", NWEventName (msg), id);
#endif
}								/* QueueEvent */

/*
 * check for event in queue
 */
XBNetworkEvent
Network_GetEvent (unsigned *pId)
{
	XBNetworkEventQueue *ptr;
	XBNetworkEvent msg;

	assert (NULL != pId);
	if (NULL == queueFirst) {
		return XBNW_None;
	}
	/* take element from list */
	ptr = queueFirst;
	queueFirst = queueFirst->next;
	if (NULL == queueFirst) {
		queueLast = NULL;
	}
	/* set results */
	msg = ptr->msg;
	*pId = ptr->id;
#ifdef DEBUG_NETWORK
	Dbg_Network ("get network event %s %u\n", NWEventName (msg), *pId);
#endif
	/* free element */
	free (ptr);
	/* that's all */
	return msg;
}								/* Network_GetEvent */

/*******************
 * type of network *
 *******************/

/*
 * get network type
 */
XBNetworkType
Network_GetType (void)
{
	return nettype;
}								/* Network_GetType */

/*
 * set network type
 */
void
Network_SetType (XBNetworkType type)
{
	nettype = type;
#ifdef DEBUG_NETWORK
	Dbg_Network ("setting type to %u\n", type);
#endif
}								/* Network_SetType */

/************
 * local id *
 ************/

/*
 * receive local host id
 */
void
Network_ReceiveLocalHostId (unsigned id)
{
	Dbg_Network ("receiving local host id = %u\n", id);
	localId = id & 0xFF;
}								/* Network_ReceiveLocalHostId */

/*
 * get local host id
 */
unsigned char
Network_LocalHostId (void)
{
	return localId;
}								/* Network_LocalHostId */

/**************
 * ping times *
 **************/

/*
 * get ping time of host
 */
int
Network_GetPingTime (unsigned id)
{
	assert (id < MAX_HOSTS);
	return hostPing[id];
}								/* Network_GetPingTime */

/*
 * ping received
 */
void
Network_ReceivePing (unsigned id, int ping)
{
	assert (id < MAX_HOSTS);
	if (hostPing[id] != ping) {
		Network_QueueEvent (XBNW_PingReceived, id);
	}
	hostPing[id] = ping;
}								/* Network_ReceivePing */

/****************
 * player atoms *
 ****************/

/*
 * get player atom
 */
XBAtom
Network_GetPlayer (unsigned id, int player)
{
	assert (id < MAX_HOSTS);
	assert (player < MAX_PLAYER);
	return hostPlayer[id][player];
}								/* Network_GetPlayer */

/*
 * get player atom
 */
XBAtom
Network_GetPlayer2 (unsigned id, int player)
{
	assert (id < MAX_HOSTS);
	assert (player < MAX_PLAYER);
	return hostPlayer2[id][player];
}								/* Network_GetPlayer2 */

/*
 * set player atom
 */
void
Network_SetPlayer (unsigned id, int player, XBAtom atom)
{
	assert (id < MAX_HOSTS);
	assert (player < MAX_PLAYER);
	hostPlayer[id][player] = atom;
	if (atom != ATOM_INVALID) {
		players += 1;
		localPlayers[id] += 1;
	}
}								/* Network_SetPlayer */

/*
 * set player atom
 */
void
Network_SetPlayer2 (unsigned id, int player, XBAtom atom)
{
	assert (id < MAX_HOSTS);
	assert (player < MAX_PLAYER);
	hostPlayer2[id][player] = atom;
}								/* Network_SetPlayer2 */

/*
 * get first player different from given one
 */
XBBool
Network_GetFirstOtherPlayer (unsigned char id, unsigned char pl, unsigned char *h, unsigned char *p)
{
	*h = 0;
	*p = 0;
	if (hostPlayer2[0][0] != ATOM_INVALID) {
		if (id != 0 || pl < NUM_LOCAL_PLAYER) {
			return XBTrue;
		}
	}
	return Network_GetNextOtherPlayer (id, pl, h, p);
}								/* Network_GetFirstOtherPlayer */

/*
 * get next player different from given one
 */
XBBool
Network_GetNextOtherPlayer (unsigned char id, unsigned char pl, unsigned char *h, unsigned char *p)
{
	*p += 1;
	while (*h < MAX_HOSTS) {
		if (id != *h || pl < NUM_LOCAL_PLAYER) {
			while (*p < NUM_LOCAL_PLAYER) {
				if ((hostPlayer2[*h][*p] != ATOM_INVALID) && (*h != id || *p != pl)) {
					Dbg_Chat ("next = %s (%u,%u)\n", GUI_AtomToString (hostPlayer2[*h][*p]), *h,
							  *p);
					return XBTrue;
				}
				*p += 1;
			}
		}
		*h += 1;
		*p = 0;
	}
	return XBFalse;
}								/* Network_GetNextOtherPlayer */

/*************
 * host data *
 *************/

/*
 * return player max for host, for check against global game config
 */
unsigned
Network_HostPlayerMax (unsigned id)
{
	CFGGameConst con;
	unsigned max = -1;
	assert (id < MAX_HOSTS);
	(void)RetrieveGameConst (CT_Remote, LOCALGAMECONFIG (id), &con);
	max = 8 * con.maxbytes - 1;
	max = (max < con.maxplayers) ? max : con.maxplayers;
	return max;
}								/* Network_HostPlayerMax */

/***************
 * game config *
 ***************/

/*
 * create global game config from current setup
 */
XBGameConfigResult
Network_CreateGlobalGameConfig (CFGGame * cfg)
{
	CFGGamePlayers localcfg;
	unsigned char t[NUM_XBTS];
	unsigned char p, save;
	XBTeamState team = XBTS_Invalid;
	unsigned char id, pl, host;
	assert (cfg != NULL);
	Dbg_Network ("creating global game config\n");
	memset (t, 0, sizeof (t));
	p = 0;
	for (id = 0, host = XBPH_Server; id < MAX_HOSTS; id++, host++) {
		/* first check if host is allowed to take part */
		if (!Network_HostIsIn (id)) {
			Dbg_Network ("host #%u ignored, is out\n", id);
			continue;
		}
		/* now get player data for host */
		if (!RetrieveGamePlayers (CT_Remote, LOCALGAMECONFIG (id), &localcfg)) {
			Dbg_Network ("failed to get players for host #%u, failure\n", id);
			return XBGC_Error;
		}
		/* check number of players */
		if (localcfg.num == 0) {
			Dbg_Network ("no players for host #%u, failure\n", id);
			return XBGC_Error;
		}
		/* add each player */
		save = p;
		for (pl = 0; pl < localcfg.num; pl++) {
			team = Network_GetTeamState (id, pl);
			if (team == XBTS_Out) {
				Dbg_Network ("player %u(%u) is out, ignoring\n", id, pl);
				continue;
			}
			if (p >= MAX_PLAYER) {
				Dbg_Network ("more players than I can handle (=%u)!\n", MAX_PLAYER);
				return XBGC_TooManyPlayers;
			}
			cfg->players.player[p] = Network_GetPlayer (id, pl);
			if (ATOM_INVALID == cfg->players.player[p]) {
				Dbg_Network ("player #%u(%u) has no name, failure\n", id, pl);
				return XBGC_Error;
			}
			cfg->players.control[p] = localcfg.control[pl];
			if (XBPC_None == cfg->players.control[p]) {
				Dbg_Network ("warning, player #%u(%u) has no control\n", id, pl);
			}
			cfg->players.playerID[p] = localcfg.playerID[pl];
			cfg->players.host[p] = host;
			cfg->players.team[p] = team;
			Dbg_Network ("adding player #%u(%u)=%s, team %u\n", id, pl,
						 GUI_AtomToString (cfg->players.player[p]), team);
			p += 1;
			t[team] += 1;
		}
		if (save == p) {
			Dbg_Network ("host %u has no active players, will only watch/chat!\n", id);
		}
	}
	cfg->players.num = p;
	cfg->setup.maskBytes = (int)(1 + p / 8);
	Dbg_Network ("--- Results of game config creation ---\n");
	/* check for invalid teams */
	if (t[XBTS_Invalid] > 0) {
		Dbg_Network ("%u invalid teams were assigned, failure", t[XBTS_Invalid]);
		return XBGC_Error;
	}
	/* check player count */
	if (p <= 1) {
		Dbg_Network ("at least two players are required!\n");
		return XBGC_SingleTeam;
	}
	/* check for chaos mode */
	if (t[XBTS_None] == p) {
		Dbg_Network ("created game config with %u players, chaos mode\n", p);
		cfg->setup.teamMode = XBTM_None;
		return XBGC_Global;
	}
	else if (t[XBTS_None] > 0) {
		Dbg_Network ("invalid team mode, failure\n");
		return XBGC_Error;
	}
	/* check for multiple teams */
	for (team = XBTS_Red; team < NUM_XBTS; team++) {
		if (t[team] < p) {
			Dbg_Network ("%u players assigned to team %u\n", t[team], team);
		}
		else {
			Dbg_Network ("all players assigned to team %u\n", team);
			return XBGC_SingleTeam;
		}
	}
	Dbg_Network ("created game config with %u players, team mode\n", p);
	cfg->setup.teamMode = XBTM_Team;
	return XBGC_Global;
}								/* Network_CreateGameConfig */

/*
 * store game config from server or client in database
 */
XBGameConfigResult
Network_ReceiveGameConfig (unsigned id, const char *data)
{
	XBAtom atom;
	CFGGamePlayers cfg;
	CFGGameConst con;
	XBVersion ver;
	XBBool loc;
	unsigned char p;
	/* check if valid id was received */
	if (id >= MAX_HOSTS) {
		return XBGC_HostInvalid;
	}
	/* set section atom to write to */
	atom = (id == 0) ? SERVERGAMECONFIG : LOCALGAMECONFIG (id);
	if (NULL != data) {
		/* write data and return */
		AddToGameConfig (CT_Remote, atom, data);
		return XBGC_Unfinished;
	}
	/* game config complete */
	Dbg_Network ("received game config from host #%u\n", id);
	/* extract player data in struct, shouldn't fail */
	if (!RetrieveGamePlayers (CT_Remote, atom, &cfg)) {
		Dbg_Network ("error in game config\n");
		return XBGC_Error;
	}
	/* check if empty */
	if (cfg.num == 0) {
		Dbg_Network ("no players found\n");
		return XBGC_Empty;
	}
	Dbg_Network ("game config contains %u players\n", cfg.num);
	/* check for too many players */
	if (cfg.num > MAX_PLAYER) {
		Dbg_Network ("too many players found\n");
		return XBGC_TooManyPlayers;
	}
	/* extract constants, should not fail */
	if (!RetrieveGameConst (CT_Remote, atom, &con)) {
		Dbg_Network ("failed to get constants\n");
		return XBGC_Error;
	}
	/* check for existing version */
	if (!Version_isDefined (&con.version)) {
		Dbg_Network ("no version sent\n");
		return XBGC_NoVersion;
	}
	Dbg_Network ("remote constants: MH=%u, MP=%u, ML=%u, MB=%u\n", con.maxhosts, con.maxplayers,
				 con.maxlocals, con.maxbytes);
	/* register version */
    /* AbsInt start */
    Version_Get(VERSION_JOINT,&ver);
    /* fprintf(stderr, "con.version=%d.%d.%d, ver=%d.%d.%d\n", con.version.major, con.version.minor, con.version.patch, ver.major, ver.minor, ver.patch); */
    if (Version_Compare(&con.version, &ver)) {
        Server_SendDisconnect (id);
        return XBGC_VersionError;
    }
    /* AbsInt end */
	if (Version_Join (id & 0xFF, &con.version)) {
		Version_Get (VERSION_JOINT, &ver);
		Dbg_Network ("joint version updated to %s\n", Version_ToString (&ver));
	}
	/* check type, at least one player has been sent */
	loc = (cfg.host[0] == XBPH_Local);
	for (p = 0; p < cfg.num; p++) {
		if (ATOM_INVALID == cfg.player[p]) {
			Dbg_Network ("player #%u has no name, error\n", p);
			return XBGC_Error;
		}
		if (loc != (cfg.host[p] == XBPH_Local)) {
			Dbg_Network ("error in game config\n");
			return XBGC_Error;
		}
	}
	Dbg_Network ("game config type = %s\n", loc ? "local" : "global");
	return loc ? XBGC_Local : XBGC_Global;
}								/* Network_ReceiveGameConfig */

/*
 * create local players from game config id
 */
unsigned
Network_CreateLocalPlayers (unsigned id)
{
	CFGGame cfg;
	XBVersion ver;
	XBAtom atom;
	unsigned p;
	char name[256];
	assert (id < MAX_HOSTS);
	/* get remote atom with received data */
	atom = (id == 0) ? SERVERGAMECONFIG : LOCALGAMECONFIG (id);
	/* extract data in struct, shouldn't fail */
	if (!RetrieveGame (CT_Remote, atom, &cfg)) {
		Dbg_Network ("error in game config!?\n");
		return NUM_LOCAL_PLAYER;
	}
	/* check local player max */
	if (cfg.players.num > NUM_LOCAL_PLAYER) {
		Dbg_Network ("create failed, too many local players\n");
		return NUM_LOCAL_PLAYER;
	}
	/* clear local data */
	ClearHostLocalData (id);
	/* for server data, store as local game config */
	if (id == 0) {
		StoreGame (CT_Remote, LOCALGAMECONFIG (0), &cfg);
		if (RetrieveGameVersion (CT_Remote, atom, &ver)) {
			StoreGameVersion (CT_Remote, LOCALGAMECONFIG (0), &ver);
		}
	}
	/* add players from game config */
	for (p = 0; p < cfg.players.num; p++) {
		/* first add name atom, must be valid */
		assert (ATOM_INVALID != cfg.players.player[p]);
		Network_SetPlayer2 (id, p, cfg.players.player[p]);
		/* use plain name as section name for server */
		atom = cfg.players.player[p];
		/* add @host:port for non-server, to make it unique */
		if (id != 0) {
			sprintf (name, "%s@%s:%d", GUI_AtomToString (atom), cfg.host.name, cfg.host.port);
			atom = GUI_StringToAtom (name);
		}
		/* now store player section atom */
		Network_SetPlayer (id, p, atom);
		Dbg_Network ("hostPlayer[%u][%d] = %s\n", id, p, GUI_AtomToString (atom));
		/* set chat controls */
		if (localId == id) {
			switch (cfg.players.control[p]) {
			case XBPC_RightKeyboard:
				Dbg_Network ("local player #%u on right keyboard, enabling chat\n", id);
				Chat_AddEventCode (p, XBE_KEYB_1);
				break;
			case XBPC_LeftKeyboard:
				Dbg_Network ("local player #%u on left keyboard, enabling chat\n", id);
				Chat_AddEventCode (p, XBE_KEYB_2);
				break;
			default:
				Dbg_Network ("local player #%u has no keyboard control, no chatting\n", id);
				break;
			}
		}
	}
	Dbg_Network ("created %u players, total %u\n", cfg.players.num, players);
	return cfg.players.num;
}								/* Network_CreateLocalPlayers */

/*
 * create global players from game config, only allowed from server
 */
unsigned
Network_CreateGlobalPlayers (unsigned id)
{
	unsigned h, p, pl;
	assert (id == 0);
	/* extract data in struct, shouldn't fail */
	if (!RetrieveGame (CT_Remote, SERVERGAMECONFIG, &globalcfg)) {
		Dbg_Network ("error in game config!?\n");
		return MAX_PLAYER;
	}
	/* check max player */
	if (globalcfg.players.num > MAX_PLAYER) {
		Dbg_Network ("create failed, too many global players\n");
		return MAX_PLAYER;
	}
	/* check mask bytes */
	if (globalcfg.setup.maskBytes > MAX_MASK_BYTES) {
		Dbg_Network ("create failed, too many mask bytes\n");
		return MAX_PLAYER;
	}
	/* now link with local */
	memset (s2l, 0xFF, sizeof (s2l));
	memset (l2s, 0xFF, sizeof (l2s));
	for (pl = 0; pl < globalcfg.players.num; pl++) {
		for (h = 0; h < MAX_HOSTS; h++) {
			for (p = 0; p < localPlayers[h]; p++) {
				if (globalcfg.players.player[pl] == hostPlayer[h][p]) {
					Dbg_Network ("linking global player %u with local player #%u(%u)\n", pl, h, p);
					s2l[pl].host = h;
					s2l[pl].player = p;
					l2s[h][p] = pl;
					break;
				}
			}
		}
		if (s2l[pl].host >= MAX_HOSTS) {
			Dbg_Network ("failed to match global player with local\n");
			return MAX_PLAYER;
		}
	}
	/* success */
	global = XBTrue;
	return globalcfg.players.num;
}								/* Network_CreateGlobalPlayers */

/*
 * getting current mask bytes
 */
unsigned
Network_GetMaskBytes (void)
{
	/* no mask bytes if no global game config */
	if (!global) {
		return 0;
	}
	return globalcfg.setup.maskBytes;
}								/* Network_GetMaskBytes */

/*****************
 * player config *
 *****************/

/*
 * player config received from client
 */
XBAtom
Network_ReceivePlayerConfig (CFGType cfgType, unsigned id, int player, const char *line)
{
	XBAtom atom;

	assert (id < MAX_HOSTS);
	assert (player < NUM_LOCAL_PLAYER);
	/* get player for config */
	atom = Network_GetPlayer (id, player);
	if (ATOM_INVALID == atom) {
		return ATOM_INVALID;
	}
	/* check if there is any data */
	if (NULL != line) {
		AddToPlayerConfig (cfgType, atom, line);
		/* ok that's all for now */
		return ATOM_INVALID;
	}
	/* all data received */
	Dbg_Network ("received player config for %u(%u)\n", id, player);
	switch (player) {
	case 0:
		Network_QueueEvent (XBNW_RightPlayerConfig, id);
		break;
	case 1:
		Network_QueueEvent (XBNW_LeftPlayerConfig, id);
		break;
	case 2:
		Network_QueueEvent (XBNW_Joy1PlayerConfig, id);
		break;
	case 3:
		Network_QueueEvent (XBNW_Joy2PlayerConfig, id);
		break;
	default:
		break;
	}
	return atom;
}								/* Network_ReceivePlayerConfig */

/******************
 * state requests *
 ******************/

/*
 * receive host state for a host
 */
XBBool
Network_ReceiveHostState (unsigned id, XBHostState state)
{
	if (id < MAX_HOSTS) {
		hostState[id] = state;
		Network_QueueEvent (XBNW_HostChange, id);
		return XBTrue;
	}
	return XBFalse;
}								/* Network_ReceiveHostState */

/*
 * receive host state request for a host
 */
XBBool
Network_ReceiveHostStateReq (unsigned who, unsigned id, XBHostState state)
{
	if (who < MAX_HOSTS && id < MAX_HOSTS) {
		hostStateReq[id][who] = state;
		return XBTrue;
	}
	return XBFalse;
}								/* Network_ReceiveHostStateReq */

/*
 * receive team state for a host/player
 */
XBBool
Network_ReceiveTeamState (unsigned host, unsigned player, XBTeamState team)
{
	if (host < MAX_HOSTS && player < MAX_PLAYER) {
		teamState[host][player] = team;
		return XBTrue;
	}
	return XBFalse;
}								/* Network_ReceiveTeamState */

/*
 * receive team state for a host/player
 */
XBBool
Network_ReceiveTeamStateReq (unsigned who, unsigned host, unsigned player, XBTeamState team)
{
	if (who < MAX_HOSTS && host < MAX_HOSTS && player < MAX_PLAYER) {
		teamStateReq[host][player][who] = team;
		return XBTrue;
	}
	return XBFalse;
}								/* Network_ReceiveTeamStateReq */

/*
 * return host state
 */
XBHostState
Network_GetHostState (unsigned id)
{
	assert (id < MAX_HOSTS);
	return hostState[id];
}								/* Network_GetHostState */

/*
 * return if host is in
 */
XBBool
Network_HostIsIn (unsigned id)
{
	switch (Network_GetHostState (id)) {
	case XBHS_Server:
	case XBHS_In:
	case XBHS_Ready:
		return XBTrue;
	default:
		return XBFalse;
	}
}								/* Network_HostIsIn */

/*
 * store current teams as default
 */
void
Network_SetDefaultTeams (unsigned host, unsigned player)
{
	unsigned id, pl;
	for (id = 0; id < MAX_HOSTS; id++) {
		for (pl = 0; pl < NUM_LOCAL_PLAYER; pl++) {
			if (teamState[id][pl] > XBTS_None) {
				defTeam[id][pl] = teamState[id][pl];
			}
		}
	}
	defTeam[host][player] = XBTS_Red;
}								/* Network_SetDefaultTeams */

/*
 * return default team state
 */
XBTeamState
Network_GetDefaultTeam (unsigned id, unsigned player)
{
	assert (id < MAX_HOSTS);
	assert (player < NUM_LOCAL_PLAYER);
	return defTeam[id][player];
}								/* Network_GetTeamState */

/*
 * return team state
 */
XBTeamState
Network_GetTeamState (unsigned id, unsigned player)
{
	assert (id < MAX_HOSTS);
	assert (player < NUM_LOCAL_PLAYER);
	return teamState[id][player];
}								/* Network_GetTeamState */

/*
 * return host state requests
 */
XBHostState *
Network_GetHostStateReq (unsigned id)
{
	assert (id < MAX_HOSTS);
	return &hostStateReq[id][0];
}								/* Network_GetHostStateReq */

/*
 * return team state requests
 */
XBTeamState *
Network_GetTeamStateReq (unsigned id, unsigned player)
{
	assert (id < MAX_HOSTS);
	assert (player < NUM_LOCAL_PLAYER);
	return &teamStateReq[id][player][0];
}								/* Network_GetTeamStateReq */

/*
 * check if all clients agree on a state for a host (at least two)
 */
XBBool
Network_HostReqClientsAgree (unsigned host, XBHostState state)
{
	unsigned id;
	unsigned count = 0;
	for (id = 1; id < MAX_HOSTS; id++) {
		if (id != host) {
			switch (hostState[id]) {
			case XBHS_In:
			case XBHS_Ready:
				if (state != hostStateReq[host][id]) {
					return XBFalse;
				}
				count++;
				break;
			default:
				break;
			}
		}
	}
	return (count > 1);
}								/* Network_HostReqClientsAgree */

/*
 * check if all clients are ready (at least one)
 */
XBBool
Network_ClientsReady (void)
{
	unsigned id;
	unsigned count = 0;
	for (id = 1; id < MAX_HOSTS; id++) {
		switch (hostState[id]) {
		case XBHS_Ready:
			count++;
		case XBHS_None:
			break;
		default:
			return XBFalse;
		}
	}
	return (count > 0);
}								/* Network_ClientsReady */

/*
 * check if clients agree on team state (at least two)
 */
XBBool
Network_TeamReqClientsAgree (unsigned host, unsigned player, unsigned state)
{
	unsigned id;
	unsigned count = 0;
	for (id = 1; id < MAX_HOSTS; id++) {
		if (id != host) {
			switch (hostState[id]) {
			case XBHS_In:
			case XBHS_Ready:
				if (state != teamStateReq[host][player][id]) {
					return XBFalse;
				}
				count++;
				break;
			default:
				break;
			}
		}
	}
	return (count > 1);
}								/* Network_TeamReqClientsAgree */

/*
 * end of file network.c
 */
