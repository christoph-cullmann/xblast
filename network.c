/*
 * file network.c - shared functions for server and clients
 *
 * $Id: network.c,v 1.19 2005/01/12 15:50:42 lodott Exp $
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
#include "network.h"

/* AddToPlayerConfig */
#include "cfg_player.h"

/*
 * local types
 */
typedef struct _network_event {
  struct _network_event *next;
  XBNetworkEvent         msg;
  unsigned                id;
} XBNetworkEventQueue;

/*
 * local variables
 */

/* events */
static XBNetworkEventQueue *queueFirst = NULL;
static XBNetworkEventQueue *queueLast  = NULL;

/* network type */
static XBNetworkType nettype = XBNT_None;

/* player atoms */
static XBAtom hostPlayer[MAX_HOSTS][MAX_PLAYER];
static XBAtom hostPlayer2[MAX_HOSTS][MAX_PLAYER];

/* ping times */
static unsigned   hostPing[MAX_HOSTS];

/* host/team states per host */
static XBHostState   hostState[MAX_HOSTS];
static XBTeamState   teamState[MAX_HOSTS][NUM_LOCAL_PLAYER];

/* requested raw host/team states received for each host */
static XBHostState   hostStateReq[MAX_HOSTS][MAX_HOSTS];
static XBTeamState   teamStateReq[MAX_HOSTS][NUM_LOCAL_PLAYER][MAX_HOSTS];

/* host versions */
static XBVersion hostVersion[MAX_HOSTS];

/******************
 * initialization *
 ******************/

/*
 * clear host data
 */
void
Network_ClearHost(unsigned id)
{
  unsigned host;
  unsigned pl;
  /* clear data for host id */
  hostPing[id] = -1;
  hostState[id] = XBHS_None;
  for (pl=0; pl<NUM_LOCAL_PLAYER; pl++) {
    hostPlayer[id][pl] = ATOM_INVALID;
    hostPlayer2[id][pl] = ATOM_INVALID;
#ifndef OLDMENUS
    teamState[id][pl] = XBTS_Invalid;
#endif
  }
  Version_Clear(&hostVersion[id]);
  Version_Reset();
  for (host = 0; host<MAX_HOSTS; host++) {
    hostStateReq[id][host] = XBHS_None;
    hostStateReq[host][id] = XBHS_None;
    Version_Join(&hostVersion[host]);
    for (pl=0; pl<NUM_LOCAL_PLAYER; pl++) {
#ifndef OLDMENUS
      teamStateReq[id][pl][host] = XBTS_Invalid;
      teamStateReq[host][pl][id] = XBTS_Invalid;
#endif
    }
  }
  DeleteGameConfig (CT_Remote, atomArrayHost0[id]);
#ifdef DEBUG_NETWORK
  Dbg_Network("cleared host data #%u\n",id);
#endif
} /* Network_ClearHost */

/*
 * clear all host and event data
 */
void
Network_Clear()
{
  unsigned id;
  /* clear data */
  memset(hostPlayer, 0, sizeof(hostPlayer));
  memset(hostPlayer2, 0, sizeof(hostPlayer2));
  memset(hostPing, -1, sizeof(hostPing));
  memset(hostState, 0, sizeof(hostState));
  memset(hostStateReq, 0, sizeof(hostStateReq));
  memset(teamState, 0, sizeof(teamState));
  memset(teamStateReq, 0, sizeof(teamStateReq));
  /* clear remote game configs and versions in database */
  for (id = 1; id < MAX_HOSTS; id++) {
    DeleteGameConfig (CT_Remote, atomArrayHost0[id]);
    Version_Clear(&hostVersion[id]);
  }
  Version_Clear(&hostVersion[0]);
  Version_Reset();
  Network_SetType(XBNT_None);
#ifdef DEBUG_NETWORK
  Dbg_Network("cleared all host data\n");
#endif
} /* Network_Clear */

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
  case XBNW_None:              return "None";
  case XBNW_Accepted:          return "Accepted";
  case XBNW_GameConfig:        return "GameConfig";
  case XBNW_RightPlayerConfig: return "RightPlayerConfig";
  case XBNW_LeftPlayerConfig:  return "LeftPlayerConfig";
  case XBNW_Joy1PlayerConfig:  return "Joy1PlayerConfig";
  case XBNW_Joy2PlayerConfig:  return "Joy2PlayerConfig";
  case XBNW_Disconnected:      return "Disconnected";
  case XBNW_StartGame:	       return "StartGame";
  case XBNW_EndOfInit:	       return "EndOfInit";
  case XBNW_LevelConfig:       return "LevelConfig";
  case XBNW_SyncEndOfInit:     return "SyncEndOfInit";
  case XBNW_SyncLevelIntro:    return "SyncLevelIntro";
  case XBNW_SyncLevelResult:   return "SyncLevelResult";
  case XBNW_SyncLevelEnd:      return "SyncLevelEnd";
  case XBNW_SyncScoreboard:    return "SyncScoreboard";
  case XBNW_HostIsIn:          return "HostIsIn";
  case XBNW_HostIsOut:         return "HostIsOut";
  case XBNW_Error:             return "Error";
  case XBNW_PingReceived:      return "PingReceived";
  case XBNW_NetworkGame:       return "NetworkGame";
  case XBNW_TeamChange:        return "TeamChange";
  case XBNW_TeamChangeData:    return "TeamChangeData";
  case XBNW_HostChange:        return "HostChange";
  default:                     return "unknown";
  }
} /* EventName */
#endif

/*
 * clear event queue
 */
void
Network_ClearEvents()
{
  unsigned id;
  while (NULL != queueFirst) {
    (void) Network_GetEvent(&id);
  }
#ifdef DEBUG_NETWORK
  Dbg_Network("clearing all network events\n");
#endif
} /* Network_ClearEvents */

/*
 * add event to queue
 */
void
Network_QueueEvent (XBNetworkEvent msg, unsigned id)
{
  /* alloc data */
  XBNetworkEventQueue *ptr = calloc (1, sizeof (*ptr) );
  assert (ptr != NULL);
  /* set values */
  ptr->msg = msg;
  ptr->id  = id;
  /* put in queue */
  if (queueLast != NULL) {
    queueLast->next = ptr;
  } else {
    queueFirst = ptr;
  }
  queueLast = ptr;
#ifdef DEBUG_NETWORK
  Dbg_Network ("queue network event %s %u\n", NWEventName (msg), id);
#endif
} /* QueueEvent */

/*
 * check for event in queue
 */
XBNetworkEvent
Network_GetEvent (unsigned *pId)
{
  XBNetworkEventQueue *ptr;
  XBNetworkEvent       msg;

  assert (NULL != pId);
  if (NULL == queueFirst) {
    return XBNW_None;
  }
  /* take element from list */
  ptr        = queueFirst;
  queueFirst = queueFirst->next;
  if (NULL == queueFirst) {
    queueLast = NULL;
  }
  /* set results */
  msg  = ptr->msg;
  *pId = ptr->id;
#ifdef DEBUG_NETWORK
  Dbg_Network ("get network event %s %u\n", NWEventName (msg), *pId);
#endif
  /* free element */
  free (ptr);
  /* that's all */
  return msg;
} /* Network_GetEvent */

/*******************
 * type of network *
 *******************/

/*
 * get network type
 */
XBNetworkType
Network_GetType ()
{
  return nettype;
} /* Network_GetType */

/*
 * set network type
 */
void
Network_SetType (XBNetworkType type)
{
  nettype = type;
#ifdef DEBUG_NETWORK
  Dbg_Network("setting type to %u\n", type);
#endif
} /* Network_SetType */

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
} /* Network_GetPingTime */

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
} /* Network_ReceivePing */

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
} /* Network_GetPlayer */

/*
 * get player atom
 */
XBAtom
Network_GetPlayer2 (unsigned id, int player)
{
  assert (id < MAX_HOSTS);
  assert (player < MAX_PLAYER);
  return hostPlayer2[id][player];
} /* Network_GetPlayer2 */

/*
 * set player atom
 */
void
Network_SetPlayer (unsigned id, int player, XBAtom atom)
{
  assert (id < MAX_HOSTS);
  assert (player < MAX_PLAYER);
  hostPlayer[id][player] = atom;
} /* Network_SetPlayer */

/*
 * set player atom
 */
void
Network_SetPlayer2 (unsigned id, int player, XBAtom atom)
{
  assert (id < MAX_HOSTS);
  assert (player < MAX_PLAYER);
  hostPlayer2[id][player] = atom;
} /* Network_SetPlayer2 */

/***************
 * game config *
 ***************/

/*
 * store game config from server or client in database
 */
XBAtom
Network_ReceiveGameConfig (unsigned id, const char *data, CFGGamePlayers *cfg)
{
  XBAtom         atom;

  assert (id < MAX_HOSTS);
  assert (cfg != NULL);

  atom = atomArrayHost0[id];
  if (NULL != data) {
    AddToGameConfig (CT_Remote, atom, data);
    return ATOM_INVALID;
  }
  /* yes, all data received, extract to cfg */
  (void) RetrieveGamePlayers (CT_Remote, atom, cfg);
#ifdef DEBUG_NETWORK
  Dbg_Network ("received game config from host #%u\n", id);
#endif
  return atom;
}

/*
 * configure game data from game config struct
 */
void
Network_ApplyGameConfig(unsigned id, XBAtom atom, CFGGamePlayers *cfg)
{
  unsigned       i;
  char           name[256];
  char           tmp[256];
  /* init player configs */
#ifdef DEBUG_NETWORK
  Dbg_Network("expecting %u players\n", cfg->num);
#endif
  for (i = 0; i < cfg->num; i ++) {
    assert (ATOM_INVALID != cfg->player[i]);
    strcpy (name, GUI_AtomToString (cfg->player[i]) );
    strcpy (tmp, name);
    if (id != 0) {
      strcat (tmp, "@");
      strcat (tmp, GetHostName (CT_Remote, atom) );
    }
    Network_SetPlayer (id, i, GUI_StringToAtom (tmp) );
    Network_SetPlayer2 (id, i, GUI_StringToAtom (name) );
#ifdef DEBUG_NETWORK
    Dbg_Network ("hostPlayer[%u][%d] = \"%s\" \"%s\"\n", id, i, tmp, name);
#endif
    /* set name */
  }
  /* clear other players */
  for (; i < NUM_LOCAL_PLAYER; i ++) {
    Network_SetPlayer (id, i, ATOM_INVALID);
  }
  /* inform user interface */
  Network_QueueEvent (XBNW_GameConfig, id);
  /* that's all */
  return;
} /* Network_ReceiveGameConfig */


/*****************
 * player config *
 *****************/

/*
 * player config received from client
 */
XBAtom
Network_ReceivePlayerConfig (CFGType cfgType, unsigned id, int player, const char *line) // XBCC
{
  XBAtom   atom;

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
  /* create message */
  if(cfgType!=CT_Central) {
#ifdef DEBUG_NETWORK
    Dbg_Network ("received player config for %u(%u)\n", id, player);
#endif
    switch (player) {
    case 0:  Network_QueueEvent (XBNW_RightPlayerConfig, id); break;
    case 1:  Network_QueueEvent (XBNW_LeftPlayerConfig,  id); break;
    case 2:  Network_QueueEvent (XBNW_Joy1PlayerConfig,  id); break;
    case 3:  Network_QueueEvent (XBNW_Joy2PlayerConfig,  id); break;
    default: break;
    }
  } else {
#ifdef DEBUG_NETWORK
    Dbg_Network ("received player config for %u(%u)\n", id, player);
#endif
  }
  return atom;
} /* Network_ReceivePlayerConfig */

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
} /* Network_ReceiveHostState */

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
} /* Network_ReceiveHostStateReq */

/*
 * receive team state for a host/player
 */
XBBool
Network_ReceiveTeamState (unsigned host, unsigned player, XBTeamState team)
{
  if (host < MAX_HOSTS && player < MAX_PLAYER) {
    teamState[host][player] = team;
#ifdef OLDMENUS
    Network_QueueEvent (XBNW_TeamChange, host * NUM_LOCAL_PLAYER + player);
    Network_QueueEvent (XBNW_TeamChangeData, team);
#endif
    return XBTrue;
  }
  return XBFalse;
} /* Network_ReceiveTeamState */

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
} /* Network_ReceiveTeamStateReq */

/*
 * return host state
 */
XBHostState
Network_GetHostState (unsigned id)
{
  assert(id < MAX_HOSTS);
  return hostState[id];
} /* Network_GetHostState */

/*
 * return team state
 */
XBTeamState
Network_GetTeamState (unsigned id, unsigned player)
{
  assert(id < MAX_HOSTS);
  assert(player < NUM_LOCAL_PLAYER);
  return teamState[id][player];
} /* Network_GetTeamState */

/*
 * return host state requests
 */
XBHostState *
Network_GetHostStateReq (unsigned id)
{
  assert(id < MAX_HOSTS);
  return &hostStateReq[id][0];
} /* Network_GetHostStateReq */

/*
 * return team state requests
 */
XBTeamState *
Network_GetTeamStateReq (unsigned id, unsigned player)
{
  assert(id < MAX_HOSTS);
  assert(player < NUM_LOCAL_PLAYER);
  return &teamStateReq[id][player][0];
} /* Network_GetTeamStateReq */

/*
 * check if all clients agree on a state for a host (at least two)
 */
XBBool
Network_HostReqClientsAgree(unsigned host, XBHostState state)
{
  unsigned id;
  unsigned count = 0;
  for (id=1; id<MAX_HOSTS; id++) {
    if (id != host) {
      switch (hostState[id]) {
      case XBHS_In:
      case XBHS_Ready:
	if (state != hostStateReq[host][id]) {
	  return XBFalse;
	}
	count ++;
	break;
      default:
	break;
      }
    }
  }
  return (count>1);
} /* Network_HostReqClientsAgree */

/*
 * check if all clients are ready (at least one)
 */
XBBool
Network_ClientsReady()
{
  unsigned id;
  unsigned count = 0;
  for (id=1; id<MAX_HOSTS; id++) {
    switch (hostState[id]) {
    case XBHS_Ready:
      count++;
    case XBHS_None:
      break;
    default:
      return XBFalse;
    }
  }
  return (count>0);
} /* Network_ClientsReady */

/*
 * check if clients agree on team state (at least two)
 */
XBBool
Network_TeamReqClientsAgree(unsigned host, unsigned player, unsigned state)
{
  unsigned id;
  unsigned count = 0;
  for (id=1; id<MAX_HOSTS; id++) {
    if (id != host) {
      switch (hostState[id]) {
      case XBHS_In:
      case XBHS_Ready:
	if (state != teamStateReq[host][player][id]) {
	  return XBFalse;
	}
	count ++;
	break;
      default:
	break;
      }
    }
  }
  return (count>1);
} /* Network_TeamReqClientsAgree */

/************
 * versions *
 ************/

/*
 * show currently stored versions
 */
void
Network_ShowVersions()
{
  unsigned id;
  for (id=0; id<MAX_HOSTS; id++) {
    if (Version_isDefined(&hostVersion[id])) {
      fprintf(stderr,"host version #%2u: %s\n", id, Version_ToString(&hostVersion[id]));
    }
  }
} /* Network_ShowVersions */

/*
 * receive version data from a host
 */
void
Network_ReceiveVersion(unsigned host, const XBVersion *ver)
{
  assert(host<MAX_HOSTS);
  hostVersion[host] = *ver;
  Version_Join(ver);
  Dbg_Network("receive version for host %u\n", host);
} /* Network_ReceiveVersion */

/*
 * end of file network.c
 */
