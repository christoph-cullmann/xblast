/*
 * file server.c - communication interface for the server
 *
 * $Id: server.c,v 1.52 2005/01/18 15:33:04 lodott Exp $
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
#include "server.h"
#include "timeval.h"
#include "cfg_player.h"
#include "com_listen.h"
#include "com_to_client.h"
#include "com_dg_client.h"
#include "com_reply.h"
#include "random.h"
#include "com_newgame.h"
#include "cfg_xblast.h"
#include "chat.h"
#include "version.h"

#include "game.h" /* GetNumOfPlayers */
#include "status.h" /* SetChat */


/*
 * local variables
 */

/* Server connections */
static XBComm *listenComm   = NULL;  /* XBCommListen - TCP listen for clients */
static XBComm *replyComm    = NULL;  /* XBCommReply - UDP LAN reply */
static XBComm **query       = NULL;  /* XBCommNewGame - UDP connection to central */

/* default server options */
static XBBool  fixedUdpPort = XBFalse;  /* UDP range in 16168+ ? */
static XBBool  allowNat     = XBFalse;  /* allow clients with NAT ?*/
static XBBool  beep         = XBFalse;  /* beep on connect ? */
static XBBool  teamMode     = XBFalse;  /* start in team mode? */

/* player actions received for expected gametime */
static PlayerAction playerAction[MAX_HOSTS][MAX_PLAYER];

/* level negotiation states per host */
static unsigned  levelStatus[MAX_HOSTS];

/* levelwinners per host */
static unsigned   levelWinner[MAX_HOSTS];

/* server flag, unneeded ? */
static XBBool     isServer   = XBFalse;

/*******************
 * initializations *
 *******************/

/*
 * init local server data
 */
static void
InitServerData(CFGGame *cfgGame)
{
  CFGPlayer cfgPlayer;
  XBVersion ver;
  unsigned id;
  /* network data */
  Dbg_Server("clearing all host data\n");
  Network_Clear();
  Network_ClearEvents();
  /* local game config and host state */
  Dbg_Server("setting up server game config\n");
  Version_GetLocal(&ver);
  StoreGameVersion(CT_Remote, atomLocal, &ver);
  Server_ReceiveHostStateReq(0,0, XBHS_Server);
  Network_ReceiveVersion(0,&ver);
  /* local players and teamstates */
  Dbg_Server("setting up server players\n");
  teamMode = cfgGame->setup.teamMode;
  for (id = 0; id < cfgGame->players.num; id ++) {
    Network_SetPlayer (0, id, cfgGame->players.player[id]);
    if (ATOM_INVALID != cfgGame->players.player[id]) {
      if (RetrievePlayer (CT_Local, cfgGame->players.player[id], cfgGame->players.teamColor[id], &cfgPlayer)) {
	StorePlayer (CT_Remote, cfgGame->players.player[id], &cfgPlayer);
	Server_ReceiveTeamStateReq(0,0,id, teamMode ? XBTS_Red : XBTS_None);
      }
    }
  }
} /* InitServerData */

/***************************
 * starting/closing server *
 ***************************/

/*
 * start listening for clients
 *
 * should always be start for server!
 */
XBBool
Server_StartListen (CFGGameHost *cfg)
{
  CFGGame   cfgGame;
  /* get game data */
  if (RetrieveGame (CT_Local, atomServer, &cfgGame) ) {
    StoreGame (CT_Remote, atomLocal, &cfgGame);
  }
  /* set server options from cfg */
  fixedUdpPort = cfg->fixedUdpPort;
  allowNat     = cfg->allowNat;
  beep         = cfg->beep;
  /* init */
  InitServerData(&cfgGame);
  /* listen on tcp-port for clients */
  assert (listenComm == NULL);
  listenComm = CommCreateListen (cfg, XBFalse);
  if (NULL == listenComm) {
    Dbg_Server("failed to listen for clients on TCP port %u\n", cfg->port);
    return XBFalse;
  }
  Dbg_Server("listening for clients on port %u\n", cfg->port);
  /* allow clients to browse for games */
  if (cfg->browseLan) {
    assert (NULL == replyComm);
    replyComm = Reply_CreateComm (16168, cfg, &cfgGame.setup);
    if (NULL == replyComm) {
      Dbg_Server ("failed to open reply socket, not visible in LAN\n");
    }
  }
  Dbg_Server("listening for LAN queries on UDP port %u\n", 16168);
  /* force stop on old query sockets - unlikely that they are still active, but not impossible */
  if (query!=NULL) {
    Dbg_Server("removing old query sockets\n");
    Server_StopNewGame();
  }
  /* registering game at central */
  if (cfg->central) {
    Dbg_Server("attempting to register game\n");
    Server_StartCentralNewGame(cfg, &cfgGame.setup);
  }
  Dbg_Server("marking as server\n");
  isServer = XBTrue; /* needed for old code, remove some time */
  assert( XBNT_None == Network_GetType() );
  Network_SetType(XBNT_Server);
  /* that's all */
  return XBTrue;
} /* Server_StartListen */

/*
 * remove listen ports
 */
void
Server_StopListen (void)
{
  /* delete listen port */
  assert (NULL != listenComm);
  CommFinishListen (listenComm);
  /* should trigger Server_ReceiveListenClose() */
  assert( listenComm == NULL);
  /* delete reply socket */
  if (NULL != replyComm) {
    CommDelete (replyComm);
    replyComm = NULL;
    Dbg_Server("stopped listening for LAN queries\n");
  }
} /* Server_StopListen */

/*
 * listen socket is closed down
 */
void
Server_ReceiveListenClose(XBBool exp)
{
  listenComm = NULL;
  if (exp) {
    Dbg_Server("stopped listening for clients\n");
  } else {
    Dbg_Server("unexpected shutdown of listen socket\n");
    /* do something appropriate */
  }
} /* Server_ReceiveListenClose */

/****************************
 * connecting/disconnecting *
 ****************************/

/*
 * a client has connected to server
 */
void
Server_Accept (unsigned id, const char *hostName, unsigned port)
{
  CFGGameHost cfg;
  time_t ltime;

  assert (hostName != NULL);
  assert (id > 0);
  assert (id < MAX_HOSTS);
  /* produce beep if wanted */
  time(&ltime);
  if (beep) {
    fprintf (stderr,"\n%s client adr=%s:%u id=%u connected \a\n",ctime(&ltime),hostName, port, id);
  } else {
    Dbg_Server ("%s client adr=%s:%u id=%u connected\n",ctime(&ltime),hostName, port, id);
  }
  /* store in database */
  Dbg_Server("setting up initial game config for host #%u\n", id);
  memset (&cfg, 0, sizeof (cfg));
  cfg.name = hostName;
  cfg.port = port;
  StoreGameHost (CT_Remote, atomArrayHost0[id], &cfg);
  /* attempt to start dgram */
  if (NULL == D2C_CreateComm (id, S2C_LocalName (id), fixedUdpPort) ) {
    Dbg_Server("failed to create udp socket for host #%u, disconnecting !!\n", id);
    Server_StreamDisconnect(id);
    return;
  }
  Dbg_Server("created udp socket for host #%u on port %u\n", id, D2C_Port(id));
  /* respond to client now */
  Server_SendAllConfigs(id);
  Server_SendDgramPort(id);
  Server_QueryGameConfig (id);
  Network_QueueEvent (XBNW_Accepted, id);
} /* Server_Accept */

/*
 * initiate stream shut down - send the disconnect signal and mark waiting EOF
 * the stream and its host slot will be freed on receivng eof/error
 */
void
Server_StreamDisconnect (unsigned clientID)
{
  assert (clientID > 0);
  assert (clientID < MAX_HOSTS);
  if (S2C_Connected (clientID) ) {
    Dbg_Server("queueing TCP disconnect sequence to host #%u\n", clientID);
    S2C_Disconnect (clientID);
  }
} /* Server_StreamDisconnect */

/*
 * makes dgram slot available for new clients
 * the dgram data structure is completely removed
 */
void
Server_DgramDisconnect (unsigned clientID)
{
  assert (clientID > 0);
  assert (clientID < MAX_HOSTS);
  if (D2C_Connected (clientID) ) {
    Dbg_Server("disconnecting UDP to host #%u\n",  clientID);
    D2C_Disconnect (clientID);
  }
} /* Server_DgramDisconnect */

/*
 * active freeing of the client slot
 * informs other clients and local GUI
 */
void
Server_SendDisconnect (unsigned clientID)
{
  Server_StreamDisconnect(clientID);
  Server_DgramDisconnect(clientID);
  Server_SendDisconnectInfo(clientID);
  Network_ClearHost(clientID);
  Network_QueueEvent(XBNW_Disconnected, clientID);
} /* Server_SendDisconnect */

/*
 * active shutdown of server
 * all slots are cleared
 */
void
Server_SendDisconnectAll (void)
{
  unsigned clientID;
  /* disconnect from clients */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    Server_StreamDisconnect (clientID);
    Server_DgramDisconnect (clientID);
  }
  Network_Clear();
  if (NULL != query) {
    Dbg_Server("queuing close to central, mark for shutdown after send\n");
    Server_CloseNewGame();
  }
  if (listenComm == NULL) {
    Network_SetType(XBNT_None);
  }
  Dbg_Server("*** all data completely cleared ***\n");
} /* Server_SendDisconnectAll */

/***************
 * comm events *
 ***************/

/*
 * handle event on stream
 */
XBBool
Server_StreamEvent (unsigned clientID, XBServerConstants code)
{
  assert (clientID > 0);
  assert (clientID < MAX_HOSTS);
  switch (code) {
  case XBSC_IDInvalid:
    Dbg_Server("invalid id received from host #%u, ignoring\n", clientID);
    return XBFalse;
  case XBSC_DataInvalid:
    Dbg_Server("invalid data received from host #%u, ignoring\n", clientID);
    return XBFalse;
  case XBSC_COTInvalid:
    Dbg_Server("invalid cot received from host #%u, sending disconnect\n", clientID);
    Server_StreamDisconnect(clientID);
    Server_SendDisconnectInfo(clientID);
    return XBFalse;
  case XBSC_MissingData:
    Dbg_Server("host #%u has not requested data\n", clientID);
    return XBFalse;
  case XBSC_IOError:
    Dbg_Server("i/o error on stream to host #%u, disconnecting\n", clientID);
    Server_SendDisconnectInfo(clientID);
    break;
  case XBSC_UnexpectedEOF:
    Dbg_Server("unexpected eof on stream to host #%u, disconnecting\n", clientID);
    Server_SendDisconnectInfo(clientID);
    break;
  case XBSC_StreamWaiting:
    Dbg_Server("all queued data sent to host #%u\n", clientID);
    return XBFalse;
  case XBSC_StreamBusy:
    /* Dbg_Server("data waits to be sent to host #%u\n", clientID); */
    return XBFalse;
  case XBSC_StreamClosed:
    Dbg_Server("stream to host #%u fully disconnected\n", clientID);
    return XBFalse;
  default:
    Dbg_Server("unknown event (%u) on stream to host #%u, disconnecting\n",code, clientID);
  }
  /* remove dgram if still there */
  Server_DgramDisconnect(clientID);
  /* clear host data */
  Network_ClearHost(clientID);
  /* queue event for GUI display */
  Network_QueueEvent (XBNW_Error, clientID); /* more info for GUI */
  return XBTrue;
} /* Server_StreamEvent */

/*
 * handle event on dgram
 */
XBBool
Server_DgramEvent (unsigned clientID, XBServerConstants code)
{
  assert (clientID > 0);
  assert (clientID < MAX_HOSTS);
  switch (code) {
  case XBSC_GameTime:
    Dbg_Server("future gametimes from host #%u received, ignoring\n", clientID);
    return XBFalse;
  case XBSC_WriteError:
    Dbg_Server("write error on dgram to host #%u, disconnecting\n", clientID);
    break;
  case XBSC_ConnFailed:
    Dbg_Server("dgram failed to connect to host #%u, disconnecting\n", clientID);
    break;
  case XBSC_Timeout:
    Dbg_Server("dgram to host #%u timed out, disconnecting\n", clientID);
    Server_DgramDisconnect(clientID);
    break;
  case XBSC_DgramClosed:
    Dbg_Server("dgram to host #%u removed\n", clientID);
    return XBFalse;
  default:
    Dbg_Server("unknown event (%u) on dgram to host #%u, disconnecting\n",code ,clientID);
  }
  /* initiate disconnection of stream */
  Server_StreamDisconnect(clientID);
  Server_SendDisconnectInfo(clientID);
  /* clear host data */
  Network_ClearHost(clientID);
  /* queue event for GUI display */
  Network_QueueEvent (XBNW_Error, clientID);
  return XBTrue;
} /* Server_DgramDisconnect */

/******************
 * receiving data *
 ******************/

/*
 * check version of connecting client, local
 */
static XBBool
CheckRemoteVersionServer(unsigned id, XBAtom atom) {
  XBVersion ver;
  if (! RetrieveGameVersion( CT_Remote, atom, &ver ) ) {
    Dbg_Server("host #%u sent no version, is < 2.10\n", id);
    return XBFalse;
  }
  Network_ReceiveVersion(id,&ver);
  Dbg_Server("host #%u version = %s\n", id, Version_ToString(&ver));
  Network_ShowVersions();
  Version_ShowData();
  return XBTrue;
} /* CheckRemoteVersion */

/*
 * game config received from client
 */
void
Server_ReceiveGameConfig (unsigned id, const char *line)
{
  XBAtom   atom;
  CFGGamePlayers cfg;

  atom = Network_ReceiveGameConfig (id, line, &cfg);
  /* check if data is complete */
  if (ATOM_INVALID != atom) {
    Dbg_Server("game config for host #%u received\n",id);
    /* check versions first */
    if (CheckRemoteVersionServer(id, atom)) {
      Network_ApplyGameConfig(id, atom, &cfg);
      /* send game config of id to other clients */
      Server_SendGameConfig(id);
      /* query player configs according to received game config */
      Server_QueryPlayerConfigs(id, cfg.num);
    } else {
      Server_SendDisconnect(id);
    }
  }
} /* Server_ReceiveGameConfig */

/*
 * player config received from client
 */
void
Server_ReceivePlayerConfig (unsigned id, int player, const char *line)
{
  XBAtom   atom;
  /* store player for config */
  atom = Network_ReceivePlayerConfig (CT_Remote, id, player, line);
  /* if atom is valid, data is complete */
  if (ATOM_INVALID != atom) {
    Dbg_Server("player config for host #%u:%u received\n",id,player);
    /* send player config of id to other clients */
    Server_SendPlayerConfig(id, player);
    /* initial team state */
    Server_ReceiveTeamStateReq(0,id,player, teamMode ? XBTS_Red : XBTS_None);
  }
} /* Server_ReceivePlayerConfig */

/*
 * client has sent dgram port
 */
void
Server_ReceiveDgramPort (unsigned id, unsigned short port)
{
  /* set port for datagram conection */
  if (0 != port || allowNat) {
    if (port==0) {
      Dbg_Server("client at %s reports NAT, setting up dgram\n", S2C_HostName(id));
    } else {
      Dbg_Server("received dgram port %u for host #%u, connecting dgram\n", port, id);
    }
    if (! D2C_Connect (id, S2C_HostName (id), port) ) {
      Dbg_Server("failed to connect dgram\n");
      Server_SendDisconnect (id);
    }
  } else {
    Dbg_Server("client has NAT, disallowed\n");
    Server_SendDisconnect (id);
  }
} /* Server_ReceiveDgramPort */

/*
 * receive and resend host state changes, local use only
 */
static void
Server_ReceiveHostState (unsigned id, XBHostState state)
{
  if (Network_ReceiveHostState (id, state) ) {
    Server_SendHostState(id, state);
  } else {
    Dbg_Server("received invalid host state\n");
  }
} /* Server_ReceiveHostState */

/*
 * return if host state request from id for host to state is granted
 */
static XBBool
Server_GrantsHostRequest(unsigned id, unsigned host, XBHostState state)
{
  /* never grant the None request */
  if (state == XBHS_None) {
    return XBFalse;
  }
  /* distinguish server and client requests */
  if (id==0) {
    /* grant any server request unless request equals current */
    return (Network_GetHostState(host)!=state);
  } else if (id == host) {
    /* grant certain client requests for self */
    switch (Network_GetHostState(host)) {
    case XBHS_Ready: return (state == XBHS_In);
    case XBHS_In:  return (state == XBHS_Ready);
    default: return XBFalse;
    }
  }
  return XBFalse;
} /* Server_GrantsHostRequest */

/*
 * receive host state request from id for host to state
 */
void
Server_ReceiveHostStateReq (unsigned id, unsigned host, XBHostState state)
{
  XBHostState *req;
  /* check if request is valid */
  if (Network_ReceiveHostStateReq (id, host, state) ) {
    /* first send request data to all clients */
    Server_SendHostStateReq(id, host, state);
    /* check if request is granted */
    if (Server_GrantsHostRequest(id,host,state)) {
      Dbg_Server("granting host request #%u->%u from #%u\n", host, state, id);
      Server_ReceiveHostState(host, state);
    }
    /* reprocess host's request */
    req = Network_GetHostStateReq(host);
    if (Server_GrantsHostRequest(host,host,req[host])) {
      Dbg_Server("granting reprocessed host request #%u->%u from #%u\n", host, req[host], host);
      Server_ReceiveHostState(host, req[host]);
    }
    /* clients agree on a host state */
    if (Network_HostReqClientsAgree(host,state)) {
	/* all clients!=host request out for host, grant unless host = server */
      if (state == XBHS_Out && id!=0) {
	Dbg_Server("granting disconnect request of clients for host #%u!!\n",host);
	Server_SendDisconnect(host);
	return;
      }
    }
    /* check if all clients are ready */
    if (Network_ClientsReady() && Network_GetHostState(0)==XBHS_Ready) {
      Dbg_Server("all hosts are ready, starting game!!\n");
      Network_QueueEvent(XBNW_StartGame,0);
    }
  } else {
    Dbg_Server("received invalid host state request\n");
  }
} /* Server_ReceiveHostStateReq */

/*
 * receive and resend team state change, local use only
 */
static void
Server_ReceiveTeamState (unsigned id, unsigned player, XBTeamState state)
{
  if (Network_ReceiveTeamState (id, player, state) ) {
    Server_SendTeamState(id, player, state);
  } else {
    Dbg_Server("received invalid team state\n");
  }
} /* Server_ReceiveTeamState */

/*
 * return if team request from id for host to state is granted
 */
static XBBool
Server_GrantsTeamRequest(unsigned id, unsigned host, unsigned player, XBTeamState state)
{
  if (id==0) {
    /* grant any server request for server */
    Dbg_Server("granting team request #%u: #%u(%u)->%u\n", id, host, player, state);
    Server_ReceiveTeamState(host, player, state);
    return XBTrue;
  } else if (Network_TeamReqClientsAgree(host,player,state)) {
    /* grant client request if they agree */
    Dbg_Server("granting joint team request #%u(%u)->%u\n", host, player, state);
    Server_ReceiveTeamState(host, player, state);
    return XBTrue;
  }
  return XBFalse;
} /* Server_GrantsTeamRequest */

/*
 * receive team state request from id for host:player to state
 */
void
Server_ReceiveTeamStateReq (unsigned id, unsigned host, unsigned player, XBTeamState state)
{
  XBTeamState *req;
  unsigned i, p;
  if (Network_ReceiveTeamStateReq (id, host, player, state) ) {
    /* send request data to all hosts */
    Server_SendTeamStateReq(id, host, player, state);
    /* process request */
    if (Server_GrantsTeamRequest(id, host, player, state)) {
      Dbg_Server("team #%u(%u) changed to %u, mode %s\n",host,player,state, teamMode ? "teams" : "chaos");
      /* check for mode change */
      if (teamMode && (state == XBTS_None) ) {
	/* change to chaos */
	for (i=0; i<MAX_HOSTS; i++) {
	  for (p=0; p<NUM_LOCAL_PLAYER; p++) {
	    if (Network_GetTeamState(i,p) > XBTS_None) {
	      Server_ReceiveTeamState(i, p, XBTS_None);
	    }
	  }
	}
	Dbg_Server("changed to chaos mode!\n");
	teamMode = XBFalse;
      } else if (!teamMode && (state > XBTS_None) ) {
	/* change to teams */
	for (i=0; i<MAX_HOSTS; i++) {
	  for (p=0; p<NUM_LOCAL_PLAYER; p++) {
	    if (Network_GetTeamState(i,p) == XBTS_None) {
	      req = Network_GetTeamStateReq(i,p);
	      Server_ReceiveTeamState(i, p, (req[i]<=XBTS_None) ? XBTS_Red : req[i]);
	    }
	  }
	}
	Dbg_Server("changed to team mode!\n");
	teamMode = XBTrue;
      }
    }
  } else {
    Dbg_Server("received invalid team state request\n");
  }
} /* Server_ReceiveTeamStateReq */

/*
 * server received chat from a client
 */
void
Server_ReceiveChat(XBChat *chat) {
  BMPlayer *ps;
  int numOfPlayers = 0;
  int counter = 0,team = 0;
  numOfPlayers = GetNumOfPlayers();
  Dbg_Server("receiving chat mode %u\n",chat->how);
  if (chat->how != numOfPlayers && chat->how < 255) {
    if (chat->how > numOfPlayers) {
      /*team msg */
      for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers; ps ++,counter++) {
	if(chat->fh==counter) {
	  team=ps->team;
	  for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers; ps ++,counter++) {
	    if (ps->team==team && ps->local) {
	      Dbg_Server("seetin  ting chat2\n");
	      SetChat(chat->txt,XBTrue);
	      Chat_Receive(chat);
	      break;
	    }
	  }
	  break;
	}
      }
    } else {
      /*private */
      for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers; ps ++,counter++)  {
	if (chat->how == ps->localDisplay) {
	  Dbg_Server("seetin  ting chat1 how %i localdisplay %i\n",chat->how,ps->localDisplay); 
	  SetChat(chat->txt,XBTrue);
	  Chat_Receive(chat);
	}
      }
    }
  } else {
    /* public */
    SetChat(chat->txt,XBTrue);
    Chat_Receive(chat);
    Dbg_Server("seetin  ting chat0\n"); 
  }
  Server_ReSendChat(chat);
} /* Server_ReceiveChat */

/*
 * ping received, tell network
 */
void
Server_ReceivePing (unsigned id, int ping)
{
  Dbg_Server("received ping from host #%u\n",id);
  Network_ReceivePing(id,ping);
} /* Server_ReceivePing */

/*
 * client has reached a sync point
 */
void
Server_ReceiveSync (unsigned id, XBNetworkEvent event)
{
  /* inform application */
  Dbg_Server("host #%u reached sync point %u\n",id,event);
  Network_QueueEvent (event, id);
} /* Server_ReceiveSync */

/*
 * receive a level check result from a client
 */
void
Server_ReceiveLevelCheck(unsigned id, unsigned stat)
{
  Dbg_Server("receiving level status at %u = %u\n",id,stat);
  levelStatus[id] = stat;
  Network_QueueEvent (XBNW_LevelConfig, id);
} /* Server_ReceiveLevelCheck */

/*
 * received keys from one client
 */
void
Server_ReceivePlayerAction (unsigned id, int gameTime, const PlayerAction *keys)
{
  int i;

  assert (id > 0);
  assert (id < MAX_HOSTS);
  assert (playerAction != NULL);

  /* Dbg_Server("received action from host #%u\n", id); */
  for (i = 0; i < MAX_PLAYER; i ++) {
    if (keys[i].dir != GoDefault) {
      playerAction[id][i].dir     = keys[i].dir;
    }
    if (keys[i].bomb) {
      playerAction[id][i].bomb    = XBTrue;
    }
    if (keys[i].special) {
      playerAction[id][i].special = XBTrue;
    }
    if (keys[i].pause) {
      playerAction[id][i].pause   = XBTrue;
    }
    if (keys[i].abort != ABORT_NONE) {
      playerAction[id][i].abort   = keys[i].abort;
    }
    if (keys[i].suicide) {
      playerAction[id][i].suicide = XBTrue;
    }
    /* Skywalker */
    if (keys[i].laola) {
      playerAction[id][i].laola  = XBTrue;
    } else {
      playerAction[id][i].laola  = XBFalse;
      if (keys[i].looser) {
	playerAction[id][i].looser  = XBTrue;
      } else {
	playerAction[id][i].looser  = XBFalse;
      }
    }
    /* */
  }
} /* Server_ReceivePlayerAction */

/*
 * received level finish from clients
 */
void
Server_ReceiveFinish (unsigned id)
{
  Dbg_Server("FINISH from host #%u\n", id);
} /* Server_ReceiveFinish */

/*
 * receive the winner team from a client
 */
void
Server_ReceiveWinnerTeam(unsigned id, unsigned team)
{
  assert(id > 0);
  assert(id < MAX_HOSTS);
  Dbg_Server("receiving winner at %u = %u\n", id, team);
  levelWinner[id] = team;
  /* queue event for game_server sync*/
  Network_QueueEvent (XBNW_SyncLevelResult, id);
} /* Server_ReceiveWinnerTeam */

/************************
 * local data retrieval *
 ************************/

/*
 * get server flag
 */
XBBool GetIsServer(){
  return isServer;
} /* GetIsServer */

/*
 * set server flag
 */
void SetIsServer(XBBool value){
  isServer=value;
} /* SetIsServer */

/*
 * retrieve host state from client
 */
XBHostState
Server_GetHostState (unsigned id)
{
  return Network_GetHostState(id);
} /* Server_GetHostState */

/*
 * retrieve host state requests for client
 */
XBHostState *
Server_GetHostStateReq (unsigned id)
{
  return Network_GetHostStateReq(id);
} /* Server_GetHostStateReq */

/*
 * retrieve team state form client, player
 */
XBTeamState
Server_GetTeamState (unsigned id, unsigned player)
{
  return Network_GetTeamState(id,player);
} /* Server_GetTeamState */

/*
 * retrieve host state requests from client
 */
XBTeamState *
Server_GetTeamStateReq (unsigned id, unsigned player)
{
  return Network_GetTeamStateReq(id,player);
} /* Server_GetTeamStateReq */

/*
 * last ping time of client
 */
int
Server_GetPingTime (unsigned clientID)
{
  return Network_GetPingTime(clientID);
} /* Server_GetPingTime */

/*
 * check level status list for rejections
 */
XBBool
Server_LevelApproved(void)
{
  unsigned i;
  for (i = 0; i<MAX_HOSTS; i++) {
    if (levelStatus[i] != 255) {
      if (levelStatus[i] == 0) {
	Dbg_Server("level rejected by clients\n");
	return XBFalse;
      }
    }
  }
  Dbg_Server("level approved by clients\n");
  return XBTrue;
} /* Server_LevelApproved */

/*
 * read player keys from local buffer
 */
void
Server_GetPlayerAction (unsigned id, int player, PlayerAction *action)
{
  assert (id > 0);
  assert (id < MAX_HOSTS);
  assert (player < MAX_PLAYER);
  assert (playerAction != NULL);
  /* copy data */
  *action = playerAction[id][player];
} /* Server_GetPlayerAction */

/*
 * check winner list for async
 */
XBBool
Server_LevelAsync()
{
  unsigned i;
  unsigned win = 255;
  for (i = 0; i< MAX_HOSTS; i ++) {
    if (levelWinner[i] != 255) {
      if (win == 255) {
	win = levelWinner[i];
      } else if (win != levelWinner[i]) {
	Dbg_Server("level results asynced!\n");
	return XBTrue;
      }
    }
  }
  Dbg_Server("level results match!\n");
  return XBFalse;
} /* Server_LevelAsync */

/**********************
 * local data setting *
 **********************/

/*
 * clear level status list for level checking
 */
void
Server_ClearLevelStatus(void)
{
  unsigned i;
  Dbg_Server("clearing level negotiation status\n");
  for (i=0; i<MAX_HOSTS; i++) { levelStatus[i] = 255; } /* undefined value */
} /* Server_ClearLevelStatus */

/*
 * set the level status for a single client
 */
void
Server_SetLevelStatus (unsigned id, XBBool val)
{
  assert(id < MAX_HOSTS);
  levelStatus[id] = val ? 1 : 0;
  Dbg_Server("level is %s by %u\n", val ? "accepted" : "rejected", id);
} /* Server_SetLevelStatus */

/*
 * reset datagrams connections for new level
 */
void
Server_ResetPlayerAction (void)
{
  unsigned client;

  for (client = 1; client < MAX_HOSTS; client ++) {
    if (D2C_Connected (client) ) {
      D2C_Reset (client);
    }
  }
  Dbg_Server("reset frames for all hosts\n");
} /* Server_ResetPlayerAction */

/*
 * clear player action data
 */
void
Server_ClearPlayerAction (void)
{
  unsigned id;
  int      player;

  for (id = 0; id < MAX_HOSTS; id ++) {
    for (player = 0; player < MAX_PLAYER; player ++) {
      playerAction[id][player].player  = player;
      playerAction[id][player].dir     = GoDefault;
      playerAction[id][player].bomb    = XBFalse;
      playerAction[id][player].special = XBFalse;
      playerAction[id][player].pause   = XBFalse;
      playerAction[id][player].suicide = XBFalse;
      playerAction[id][player].abort   = ABORT_NONE;
    }
  }
  /* Dbg_Server("cleared action for all hosts\n"); */
} /* Server_ClearPlayerAction */

/*
 * clear the winner list for async checking
 */
void
Server_ClearLevelWinners(void)
{
  unsigned i;
  Dbg_Server("clearing winner list\n");
  for (i=0; i<MAX_HOSTS; i++) { levelWinner[i] = 255; } /* undefined value */
} /* Server_ClearLevelWinners */

/*
 * set the winner team on given host
 */
void
Server_SetLevelWinners (unsigned id, unsigned team)
{
  assert(id < MAX_HOSTS);
  Dbg_Server("winner at %u set to %u\n", id, team);
  levelWinner[id] = team;
} /* Server_SetLevelWinners */


/******************
 * queue data out *
 ******************/

/*
 * queue all current game configs to client
 */
void
Server_SendAllConfigs (unsigned id)
{
  unsigned    client;
  unsigned    player;
  unsigned    rclient;
  XBHostState  *hReq;
  XBTeamState  *tReq;
  XBAtom      atom;
  assert (id > 0);
  assert (id < MAX_HOSTS);
  for (client = 0; client < MAX_HOSTS; client ++) {
    if (client == 0 || S2C_Connected (client) ) {
      /* send game configs to client */
      S2C_SendGameConfig (id, client, client ? atomArrayHost0[client] : atomLocal);
      S2C_SendHostState(id, client, Network_GetHostState(client));
      hReq = Network_GetHostStateReq(client);
      for (rclient=0; rclient<MAX_HOSTS; rclient++) {
	if (rclient == 0|| S2C_Connected(rclient)) {
	  S2C_SendHostStateReq(id, rclient, client, hReq[rclient]);
	}
      }
      /* send player configs to client*/
      for (player = 0; player < NUM_LOCAL_PLAYER; player ++) {
	atom = Network_GetPlayer (client, player);
	if (ATOM_INVALID != atom) {
	  S2C_SendPlayerConfig (id, client, player, atom);
	  S2C_SendTeamState(id, client, player, Network_GetTeamState(client,player));
	  tReq = Network_GetTeamStateReq(client,player);
	  for (rclient=0; rclient<MAX_HOSTS; rclient++) {
	    if (rclient == 0|| S2C_Connected(rclient)) {
	      S2C_SendTeamStateReq(id, rclient, client, player, tReq[rclient]);
	    }
	  }
	}
      }
      Dbg_Server("Queued data for host #%u to %u\n", client, id);
    }
  }
} /* Server_SendAllConfigs */


/*
 * queue dgram port to client
 */
void
Server_SendDgramPort(unsigned id)
{
  assert (id > 0);
  assert (id < MAX_HOSTS);
  S2C_SendDgramPort (id, D2C_Port (id));
  Dbg_Server("queueing dgram port number to host #%u\n",id);
} /* Server_SendDgramPort */

/*
 * queue request for game config to client
 */
void
Server_QueryGameConfig (unsigned id)
{
  assert (id > 0);
  assert (id < MAX_HOSTS);
  S2C_QueryGameConfig (id);
  Dbg_Server("queueing game config request to host #%u\n",id);
} /* Server_QueryGameConfig */

/*
 * queue game config of client to all hosts
 */
void
Server_SendGameConfig (unsigned id) {
  unsigned client;
  assert(id < MAX_HOSTS);
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (S2C_Connected (client) ) {
      S2C_SendGameConfig (client, id, atomArrayHost0[id]);
    }
  }
  Dbg_Server("queueing game config #%u to hosts\n", id);
} /* Server_SendGameConfig */

/*
 * queue request for player config to client
 */
void
Server_QueryPlayerConfigs (unsigned id, unsigned cnt)
{
  unsigned player;
  assert (id > 0);
  assert (id < MAX_HOSTS);
  for (player = 0; player < cnt; player ++) {
    DeletePlayerConfig (CT_Demo, Network_GetPlayer (id, player));
    S2C_QueryPlayerConfig (id, player);
  }
  Dbg_Server("queueing %u player config requests to host #%u\n", cnt, id);
} /* Server_QueryPlayerConfigs */

/*
 * queue player config of client id:player to all hosts
 */
void
Server_SendPlayerConfig (unsigned id, unsigned player) {
  unsigned client;
  XBAtom atom;
  assert(id < MAX_HOSTS);
  atom = Network_GetPlayer (id, player);
  assert(ATOM_INVALID != atom);
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (S2C_Connected (client) ) {
      S2C_SendPlayerConfig (client, id, player, atom);
    }
  }
  Dbg_Server("queueing player config #%u:%u to hosts\n",id,player);
} /* Server_SendPlayerConfig */

/*
 * queue chat to clients
 * TODO: do not ignore how settings!
 */
void
Server_SendChat (unsigned fp, unsigned th, unsigned tp, unsigned how, char *txt)
{
  XBChat *chat;
  unsigned client;

  chat = Chat_Create(0x00,fp,th,tp,how,txt);
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (S2C_Connected (client) ) {
      S2C_SendChat (client, chat);
    }
  }
  Dbg_Server("queued chat to clients -%s-\n", chat->txt);
} /* Server_SendChat */

/*
 * requeue a received chat
 */
void
Server_ReSendChat (XBChat *chat)
{
  unsigned client;
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (S2C_Connected (client) ) {
      S2C_SendChat (client, chat);
    }
  }
  Dbg_Server("requeuing chat to clients -%s-\n", chat->txt);
} /* Server_ReSendChat */

/*
 * queue host state to all clients and sync local data
 */
void
Server_SendHostState (unsigned id, XBHostState state)
{
  unsigned clientID;
  assert(id < MAX_HOSTS);
  Network_ReceiveHostState(id, state);
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendHostState (clientID, id, state);
    }
  }
  Dbg_Server("queued host state for host #%u=%u to hosts\n", id, state);
} /* Server_SendHostState */

/*
 * queue team state to all clients and sync local data
 */
void
Server_SendTeamState (unsigned id, unsigned player, XBTeamState team)
{
  unsigned clientID;
  assert(id < MAX_HOSTS);
  assert(player < MAX_HOSTS);
  Network_ReceiveTeamState(id, player, team);
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendTeamState (clientID, id, player, team);
    }
  }
  Dbg_Server("queued team state for host #%u(%u)=%u to hosts\n", id, player, team);
} /* Server_SendTeamState */

/*
 * queue host state request from id for host to state and sync local data
 */
void
Server_SendHostStateReq (unsigned id, unsigned host, XBHostState state)
{
  unsigned clientID;
  assert(id < MAX_HOSTS);
  assert(host < MAX_HOSTS);
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendHostStateReq (clientID, id, host, state);
    }
  }
  Dbg_Server("queued host request #%u->%u by #%u to hosts\n", host, state, id);
} /* Server_SendHostStateReq */

/*
 * queue team state request from id for host:player to state and sync local data
 */
void
Server_SendTeamStateReq (unsigned id, unsigned host, unsigned player, XBTeamState team)
{
  unsigned clientID;
  assert(id < MAX_HOSTS);
  assert(host < MAX_HOSTS);
  assert(player < MAX_HOSTS);
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendTeamStateReq (clientID, id, host, player, team);
    }
  }
  Dbg_Server("queued requested team state for player #%u(%u)->%u by #%u to hosts\n", host, player, team, id);
} /* Server_SendTeamStateReq */

/*
 * queue disconnect info to clients
 */
void
Server_SendDisconnectInfo(unsigned clientID)
{
  unsigned id;
  unsigned cnt = 0;
  for (id = 1; id < MAX_HOSTS; id ++) {
    if (S2C_Connected (id) ) {
      cnt ++;
      S2C_HostDisconnected (id, clientID);
    }
  }
  if (cnt) {
    Dbg_Server("queued disconnect of host #%u to %u hosts\n",clientID, cnt);
  }
} /* SendDisconnectInfo */

/*
 * queue start game to single client
 */
void
Server_SendStart (unsigned id)
{
  /* send game config to client */
  S2C_SendGameConfig (id, 0, atomArrayHost0[0]);
  Dbg_Server("queued final game config to host #%u\n",id);
  /* send start signal to client */
  S2C_StartGame (id);
  Dbg_Server("queued start to host #%u\n",id);
} /* Server_SendStart */

/*
 * queue random seed to clients
 */
void
Server_SendRandomSeed ()
{
  unsigned client;
  unsigned seed = GetRandomSeed ();
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (S2C_Connected (client) ) {
      S2C_SendRandomSeed (client, seed);
    }
  }
  Dbg_Server("queued random seed %u to hosts\n", seed);
} /* Server_SendRandomSeed */

/*
 * queue level data to all clients
 */
void
Server_SendLevel (const DBRoot *level)
{
  unsigned client;
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (S2C_Connected (client) ) {
      S2C_SendLevelConfig (client, level);
    }
  }
  Dbg_Server("queued level data to hosts\n");
} /* Server_SendLevel */

/*
 * queue level activation to all clients
 */
void
Server_SendLevelActivate()
{
  unsigned clientID;
  /* to all connected client */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendLevelActivate (clientID);
    }
  }
  Dbg_Server("queued level activation to hosts\n");
} /* Server_SendLevelActivate */

/*
 * queue level reset to all clients
 */
void
Server_SendLevelReset()
{
  unsigned clientID;
  /* to all connected client */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendLevelReset (clientID);
    }
  }
  Dbg_Server("queued level reset to hosts\n");
} /* Server_SendLevelReset */

/*
 * queue player action to client
 */
void
Server_SendPlayerAction (int gameTime, const PlayerAction *playerAction)
{
  unsigned client;
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (D2C_Connected (client) ) {
      D2C_SendPlayerAction (client, gameTime, playerAction);
    }
  }
  /* Dbg_Server("queued actions for gt=%u to hosts\n", gameTime); */
} /* Server_SendPlayerAction */

/*
 * queue finish player actions (= end of level) to clients
 */
void
Server_FinishPlayerAction (int gameTime)
{
  unsigned client;
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (D2C_Connected (client) ) {
      D2C_SendFinish (client, gameTime);
    }
  }
  Dbg_Server("queued FINISH to hosts\n");
} /* Server_FinishPlayerAction */

/*
 * flush last player actions
 */
XBBool
Server_FlushPlayerAction (void)
{
  XBBool   result;
  unsigned client;

  result = XBTrue;
  for (client = 1; client < MAX_HOSTS; client ++) {
    if (D2C_Connected (client) ) {
      if (! D2C_Flush (client)) {
	result = XBFalse;
      }
    }
  }
  Dbg_Server("flushing actions: (%s needed)\n", result ? "not" : "" );
  return result;
} /* Server_FlushPlayerAction */

/*
 * tell clients that all clients have reached sync point
 */
void
Server_SendSync (XBNetworkEvent event)
{
  unsigned clientID;

  /* to all connected client */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_Sync (clientID, event);
    }
  }
  Dbg_Server("queued sync %u to hosts\n", event);
} /* Server_SendSync */

/*
 * queue a level sync result to all clients
 */
void
Server_SendLevelSync()
{
  unsigned clientID;

  /* to all connected client */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendLevelSync (clientID);
    }
  }
  Dbg_Server("queued LevelSync to clients\n");
} /* Server_SendLevelSync */

/*
 * queue a level async signal to all clients
 */
void
Server_SendLevelAsync()
{
  unsigned clientID;

  /* to all connected client */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    if (S2C_Connected (clientID) ) {
      S2C_SendLevelAsync (clientID);
    }
  }
  Dbg_Server("queued LevelAsync to clients\n");
} /* Server_SendLevelAsync */


/***********************
 * central connections *
 ***********************/

/*
 * create connection to central for game info
 */
void
Server_StartCentralNewGame (const CFGGameHost *cfg, const CFGGameSetup *setup)
{
#ifdef OLDIFCODE
  size_t                   numInter;
  const XBSocketInterface *inter;
  size_t                   i, j;
#endif
  CFGCentralSetup          centralSetup;

  assert (NULL == query);
  Dbg_Server("attempting to register game\n");
#ifndef OLDIFCODE
  RetrieveCentralSetup (&centralSetup);
  if (NULL == centralSetup.name) {
    Dbg_Server("no central defined!\n");
    return;
  }
  /* alloc a single communication to central */
  query = calloc (1, sizeof (XBComm *));
  assert (NULL != query);
  Dbg_Server("connecting to central %s:%u\n", centralSetup.name, centralSetup.port);
  /* let system choose the interface */
  query[0] = NewGame_CreateComm (NULL, centralSetup.port, centralSetup.name, cfg, setup);
  if (NULL == query[0]) {
    Dbg_Server("failed to establish socket to central, game not visible in central\n");
    return;
  }
  Dbg_Server("socket to central established\n");
#else
  inter = Socket_GetInterfaces (&numInter);
  if (NULL == inter) {
    Dbg_Server("no interfaces found!\n");
    return;
  }
  Dbg_Server("interfaces found = %u\n",numInter);
  RetrieveCentralSetup (&centralSetup);
  if (NULL == centralSetup.name) {
    Dbg_Server("no central defined!\n");
    return;
  }
  /* alloc 1 pointer (to central) */
  query = calloc (1 + numInter, sizeof (XBComm *));
  assert (NULL != query);
  Dbg_Server("connecting to central %s:%u\n", centralSetup.name, centralSetup.port);
  /* start query on at most one device */
  /* FIXXX used to be i=1 worked on linux */
#ifdef W32
  Dbg_Server("W32\n");
#ifdef CYG
  for (i = 0, j = 0; (j == 0) && (i < numInter); i ++) {
#else
  for (i = 1, j = 0; (j == 0) && (i < numInter); i ++) {
#endif
#else
  Dbg_Server("Linux\n");
  for (i = 1, j = 0; (j == 0) && (i < numInter); i ++) {
#endif
    if (NULL != (query[j] = NewGame_CreateComm (inter[i].addrDevice, centralSetup.port, centralSetup.name, cfg, setup) ) ) {
      Dbg_Server ("established query to %s on interface %u\n", centralSetup.name, i);
      j ++;
    }
  }
  /* make sure not more than one device is used, would clash with close routines */
  assert(j==1);
#endif
  /* queue initial data to central */
  Server_RestartNewGame(1,"");
} /* Server_StartCentralNewGame */

/*
 * queue game info to central
 */
/* GAMEONFIX */
void
Server_RestartNewGame (int num, const char *score)
{
  if (NULL != query) {
    struct timeval tv;
    int    i;
    gettimeofday (&tv, NULL);
    for (i = 0; query[i] != NULL; i ++) {
      NewGame_Send (query[i], &tv, num, score);
      Dbg_Server("queued game data to central\n");
    }
  }
} /* Server_RestartQuery */
/* GAMEONFIX */

/*
 * queue request to central for removing game info
 */
void
Server_CloseNewGame (void)
{
  if (NULL != query) {
    struct timeval tv;
    int    i;
    gettimeofday (&tv, NULL);
    for (i = 0; query[i] != NULL; i ++) {
      NewGame_Close (query[i], &tv);
      Dbg_Server("queued game close to central\n");
    }
  }
} /* Server_CloseNewGame */

/*
 * delete open connection to central
 */
void
Server_StopNewGame (void)
{
  size_t i;
  /* delete communications */
  assert (NULL != query);
  for (i = 0; query[i] != NULL; i ++) {
    assert(i==0);
    Dbg_Server("forcing stop of previous newgame\n");
    CommDelete (query[i]);
  }
} /* Server_StopQuery */

/*
 * receive close event from query
 */
void
Server_ReceiveNewGameClose (void)
{
  assert (NULL != query);
  free (query);
  query = NULL;
  Dbg_Server ("removed newgame sockets to central\n");
} /**/

/*
 * end of file server.c
 */
