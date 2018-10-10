/*
 * file com_to_server.c - client's communication with server
 *
 * $Id: com_to_server.c,v 1.18 2004/11/06 01:59:04 lodott Exp $
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
#include "com_to_server.h"

#include "atom.h"
#include "client.h"
#include "net_tele.h"
#include "net_socket.h"

/*
 * local types
 */
typedef struct {
  XBCommStream stream;
} XBCommToServer;

/************
 * handlers *
 ************/

/*
 * server requests data
 */
static XBCommResult
HandleRequestData (XBCommStream *stream, const XBTelegram *tele)
{
  switch (Net_TeleID (tele)) {
  case XBT_ID_PlayerConfig:
    Dbg_C2S("receiving playe config request from server\n");
    Client_ReceivePlayerConfigReq(Net_TeleIOB(tele));
    break;
  case XBT_ID_GameConfig:
    Dbg_C2S("receiving game config request from server\n");
    Client_ReceiveGameConfigReq(Net_TeleIOB(tele));
    break;
  default:
    Dbg_C2S("server sends unrecognized data !!!\n");
    return Client_StreamEvent(XBCC_IDInvalid) ? XCR_Error : XCR_OK;
  }
  return XCR_OK;
} /* HandleRequestData */

/*
 * server sends data
 */
static XBCommResult
HandleSendData (XBCommStream *stream, const XBTelegram *tele)
{
  const char *data;
  size_t      len;
  XBTeleIOB   iob;
  XBChat      *chat;

  data = Net_TeleData (tele, &len);
  iob  = Net_TeleIOB (tele);
  switch (Net_TeleID (tele)) {
  case XBT_ID_GameConfig:
    Dbg_C2S("receiving game config from server\n");
    Client_ReceiveGameConfig (iob, data);
    break;
  case XBT_ID_PlayerConfig:
    Dbg_C2S("receiving player config from server\n");
    Client_ReceivePlayerConfig (iob >> 4, iob & 0x0F, data);
    break;
  case XBT_ID_LevelConfig:
    Dbg_C2S("receiving level config from server\n");
    Client_ReceiveLevelConfig (iob, data);
    break;
  case XBT_ID_Chat:
    if (len > 2) {
      chat = Chat_Create(data[0] >> 4, data[0] & 0x0F, data[1] >> 4, data[1] & 0x0F, iob, data+2);
      Client_ReceiveChat(chat);
      Dbg_C2S("receiving chat from server\n");
    } else {
      Dbg_C2S("receiving invalid chat data\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    return XCR_OK;
  default:
    Dbg_C2S("server sent unrecognized data\n");
    return Client_StreamEvent(XBCC_IDInvalid) ? XCR_Error : XCR_OK;
  }
  /* achknowledge reception, needed ?! */
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  return XCR_OK;
} /* HandleSendData */

/*
 * server sends a command
 */
static XBCommResult
HandleActivate (XBCommStream *stream, const XBTelegram *tele)
{
  const char *data;
  size_t      len;
  unsigned    value;
  unsigned    tmp;

  data = Net_TeleData (tele, &len);
  switch (Net_TeleID (tele)) {
  case XBT_ID_RequestDisconnect:
    Dbg_C2S("server has disconnected, shutting down stream\n");
    (void) Client_StreamEvent(XBCC_ExpectedEOF);
    return XCR_Finished;
  case XBT_ID_StartGame:
    Dbg_C2S("server has started the game!\n");
    Client_ReceiveStart (Net_TeleIOB (tele));
    break;
  case XBT_ID_RandomSeed:
    if (NULL != data && 1 == sscanf (data, "%u", &value) ) {
      Dbg_C2S("server has sent random seed\n");
      Client_ReceiveRandomSeed (value);
    } else {
      Dbg_C2S("server send invalid seed!\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    break;
  case XBT_ID_DgramPort:
    if (NULL != data && 1 == sscanf (data, "%u", &value) ) {
      Dbg_C2S("server has sent dgram port\n");
      Client_ReceiveDgramPort (Net_TeleIOB (tele), value);
    } else {
      Dbg_C2S("server has sent invalid dgram port!\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    break;
  case XBT_ID_Sync:
    Dbg_C2S("server has sent sync\n");
    Client_ReceiveSync (Net_TeleIOB (tele));
    break;
  case XBT_ID_Async:
    Dbg_C2S("server has sent async!\n");
    Client_ReceiveAsync (Net_TeleIOB (tele));
    break;
  case XBT_ID_HostChange:
    if (NULL != data && 1 == sscanf(data, "%u", &value) ) {
      Dbg_C2S("server has sent host state\n");
      Client_ReceiveHostState (Net_TeleIOB (tele), value);
    } else {
      Dbg_C2S("server has sent invalid host state!\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    break;
  case XBT_ID_TeamChange:
    if (NULL != data && 1 == sscanf (data, "%u", &value) ) {
      Dbg_C2S("server has sent team state!\n");
      tmp = Net_TeleIOB(tele);
      Client_ReceiveTeamState (tmp / NUM_LOCAL_PLAYER, tmp % NUM_LOCAL_PLAYER, value);
    } else {
      Dbg_C2S("server has sent invalid team state!\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    break;
  case XBT_ID_HostChangeReq:
    if (len == 2) {
      Dbg_C2S("server has sent host state\n");
      Client_ReceiveHostStateReq (Net_TeleIOB(tele), data[0], data[1]);
    } else {
      Dbg_C2S("server has sent invalid host state!\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    break;
  case XBT_ID_TeamChangeReq:
    if (len == 3) {
      Dbg_C2S("server has sent team state request!\n");
      Client_ReceiveTeamStateReq (Net_TeleIOB(tele), data[0], data[1], data[2]);
    } else {
      Dbg_C2S("server has sent invalid team state!\n");
      return Client_StreamEvent(XBCC_DataInvalid) ? XCR_Error : XCR_OK;
    }
    break;
  case XBT_ID_LevelConfig:
    Dbg_C2S("server sends level activation\n");
    Client_ActivateLevel( Net_TeleIOB (tele));
    break;
  default:
    Dbg_C2S("server sends unrecognized command !!!\n");
    return Client_StreamEvent(XBCC_IDInvalid) ? XCR_Error : XCR_OK;
  }
  return XCR_OK;
} /* HandleSendData */

/*
 * handle infos from server
 */
static XBCommResult
HandleSpontaneous (XBCommStream *stream, const XBTelegram *tele)
{
  switch (Net_TeleID (tele)) {
  case XBT_ID_HostDisconnected:
    Dbg_C2S("server sends disconnect info\n");
    Client_ReceiveDisconnect (Net_TeleIOB (tele));
    return XCR_OK;
  default:
    Dbg_C2S("server sends unrecognized info !!!\n");
    return Client_StreamEvent(XBCC_IDInvalid) ? XCR_Error : XCR_OK;
  }
} /* HandleSpontaneous */


/*
 * handle telegrams from server
 */
static XBCommResult
HandleTelegram (XBCommStream *stream, const XBTelegram *tele)
{
  switch (Net_TeleCOT (tele)) {
    /* server requests data from client */
  case XBT_COT_RequestData:
    return HandleRequestData (stream, tele);
    /* server sends data to client */
  case XBT_COT_SendData:
    return HandleSendData (stream, tele);
    /* server activate command on client */
  case XBT_COT_Activate:
    return HandleActivate (stream, tele);
    /* server send spontaneous status change */
  case XBT_COT_Spontaneous:
    return HandleSpontaneous (stream, tele);
    /* unknown cause of transmission */
  default:
    Dbg_C2S("server sends invalid COT !\n");
    return Client_StreamEvent(XBCC_COTInvalid) ? XCR_Error : XCR_OK;
  }
} /* HandleTelegram */

/*
 * handle delete: triggered by eof on com_stream, local parse errors
 */
static XBCommResult
DeleteToServer (XBComm *comm)
{
  XBCommStream *stream = (XBCommStream *) comm;
  /* delete communication */
  Stream_CommFinish (stream);
  free (comm);
  Dbg_C2S("removed stream to server\n");
  return XCR_OK;
} /* DeleteToServer */

/*
 * handle stream events
 */
static XBBool
EventToServer (XBCommStream *comm, const XBStreamEvent ev)
{
  XBCommToServer *toServer = (XBCommToServer *) comm;
  XBClientConstants code;
  assert (toServer != NULL);
  switch (ev) {
  case XBST_IOREAD: code = XBCC_IOError; break;
  case XBST_IOWRITE: code = XBCC_IOError; break;
  case XBST_EOF: code = XBCC_UnexpectedEOF; break;
  case XBST_WAIT: code = XBCC_StreamWaiting; break;
  case XBST_BUSY: code = XBCC_StreamBusy; break;
  case XBST_CLOSE: code = XBCC_StreamClosed; break;
  default: return XBFalse;
  }
  return Client_StreamEvent(code);
} /* EventToServer */

/***************
 * constructor *
 ***************/

/*
 * create a stream to server
 */
XBComm *
C2S_CreateComm (const CFGGameHost *cfg)
{
  XBSocket       *pSocket;
  XBCommToServer *toServer;

  assert (cfg != NULL);
  /* create connection to server */
  pSocket = Net_ConnectInet (cfg->name, cfg->port);
  if (NULL == pSocket) {
    Dbg_C2S("failed to connect stream to server!\n");
    return NULL;
  }
  Dbg_C2S("connected stream to server!\n");
  /* create communication data structure */
  toServer = calloc (1, sizeof (*toServer));
  assert (NULL != toServer);
  /* set values */
  Stream_CommInit (&toServer->stream, COMM_ToServer, pSocket, HandleTelegram, EventToServer, DeleteToServer);
  /* that'S all */
  Dbg_C2S("handlers established!\n");
  return &toServer->stream.comm;
} /* CommCreateToServer */

/******************
 * get local data *
 ******************/

/*
 * return address of server in dot-representation
 */
const char *
C2S_ServerName (XBComm *comm)
{
  return Net_RemoteName (comm->socket);
} /* C2S_ServerName */

/*
 * return address of client (local host) in dot-representation
 */
const char *
C2S_ClientName (XBComm *comm)
{
  return Net_LocalName (comm->socket);
} /* C2S_ClientName */

/**************
 * queue data *
 **************/

/*
 * queue random seed to client
 */
void
C2S_SendDgramPort (XBComm *comm, unsigned short port)
{
  XBCommStream *stream = (XBCommStream *) comm;
  char          tmp[16];
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  /* send seed as ascii */
  sprintf (tmp, "%hu", port);
  /* send data */
  Socket_RegisterWrite (CommSocket (&stream->comm));
  Net_SendTelegram (stream->sndQueue,
		    Net_CreateTelegram (XBT_COT_Activate, XBT_ID_DgramPort, 0, tmp, strlen (tmp) + 1) );
  Dbg_C2S("queued dgram port to server!\n");
} /* C2S_SendDgramPort */

/*
 * queue a DataNotAvailable response, local
 */
static void
DataNotAvailable (XBComm *comm, XBTeleID id, XBTeleIOB iob)
{
  XBCommStream *stream = (XBCommStream *) comm;
  Socket_RegisterWrite (CommSocket (&stream->comm));
  Net_SendTelegram (stream->sndQueue,
		    Net_CreateTelegram (XBT_COT_DataNotAvailable, id, iob, NULL, 0) );
  Dbg_C2S("queued data not available to server\n");
} /* DataNotAvailable */

/*
 * queue game config not available
 */
void
C2S_GameDataNotAvailable(XBComm *comm)
{
  DataNotAvailable(comm, XBT_ID_GameConfig, 0);
} /* C2S_PlayerDataNotAvailable */

/*
 * queue game config to client
 */
XBBool
C2S_SendGameConfig (XBComm *comm, CFGType cfgType, XBAtom atom)
{
  XBCommStream *stream = (XBCommStream *) comm;
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  /* send data */
  if (! SendGameConfig(cfgType, stream->sndQueue, XBT_COT_DataAvailable, 0, atom)) {
    Dbg_C2S("failed to queue game config!\n");
    return XBFalse;
  }
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  Dbg_C2S("queued game config to server!\n");
  return XBTrue;
} /* C2S_SendGameConfig */

/*
 * queue player config not available
 */
void
C2S_PlayerDataNotAvailable(XBComm *comm, unsigned id)
{
  DataNotAvailable(comm, XBT_ID_PlayerConfig, id);
} /* C2S_PlayerDataNotAvailable */

/*
 * queue player config to client
 */
XBBool
C2S_SendPlayerConfig (XBComm *comm, CFGType cfgType, XBAtom atom, unsigned player, XBBool how)
{
  XBCommStream *stream = (XBCommStream *) comm;
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  /* send data */
  if (! SendPlayerConfig(cfgType, stream->sndQueue, XBT_COT_DataAvailable, player, atom, how)) {
    Dbg_C2S("failed to queue player config!\n");
    return XBFalse;
  }
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  Dbg_C2S("queued game config to server!\n");
  return XBTrue;
} /* C2S_SendPlayerConfig */

/*
 * queue host state to server
 * TODO: replace with requests below
 */
void
C2S_SendHostState (XBComm *comm, unsigned state)
{
  XBCommStream *stream = (XBCommStream *) comm;
  char tmp[16];
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  /* send state as ascii */
  sprintf (tmp, "%hu", state);
  Socket_RegisterWrite (CommSocket (&stream->comm));
  Net_SendTelegram (stream->sndQueue,
		    Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostChange, 0, tmp, strlen (tmp) + 1) );
  Dbg_C2S("queued host state to server!\n");
} /* C2S_SendHostState */

/*
 * queue host state request to server
 */
void
C2S_SendHostStateReq (XBComm *comm, unsigned host, unsigned state)
{
  XBCommStream *stream = (XBCommStream *) comm;
  char tmp[2];
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  tmp[0] = host & 0xff;
  tmp[1] = state & 0xff;
  Socket_RegisterWrite (CommSocket (&stream->comm));
  Net_SendTelegram (stream->sndQueue, Net_CreateTelegram (XBT_COT_Activate, XBT_ID_HostChangeReq, 0, tmp, sizeof (tmp)) );
  Dbg_C2S("queued host state request host #%u->%u to server!\n", host, state);
} /* C2S_SendHostStateReq */

/*
 * queue team state request to server
 */
void
C2S_SendTeamStateReq (XBComm *comm, unsigned host, unsigned player, unsigned team)
{
  XBCommStream *stream = (XBCommStream *) comm;
  char tmp[3];
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  tmp[0] = host & 0xff;
  tmp[1] = player & 0xff;
  tmp[2] = team & 0xff;
  Socket_RegisterWrite (CommSocket (&stream->comm));
  Net_SendTelegram (stream->sndQueue, Net_CreateTelegram (XBT_COT_Activate, XBT_ID_TeamChangeReq, 0, tmp, sizeof (tmp)) );
  Dbg_C2S("queued team state request host #%u(%u)->%u to server!\n", host, player, team);
} /* C2S_SendHostStateReq */

/*
 * send a chat line to server
 */
void
C2S_SendChat (XBComm *comm, XBChat *chat)
{
  XBCommStream *stream = (XBCommStream *) comm;
  static char data[CHAT_LINE_SIZE+2];
  /* sanity check */
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  assert (chat != NULL);
  /* build chat package */
  data[0] = 0xFF & ( (chat->fh << 4) + (chat->fp & 0x0F) );
  data[1] = 0xFF & ( (chat->th << 4) + (chat->tp & 0x0F) );
  memcpy(data+2, chat->txt, chat->len);
  data[chat->len+2]=(char)'\0';
  /* prepare for writing */
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  Net_SendTelegram (stream->sndQueue, Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_Chat, chat->how & 0xFF, data, chat->len+3) );
  Dbg_C2S("queueing chat -%s- (%u,%u)->(%u,%u), how=%u\n",data+2,chat->fh,chat->fp,chat->th,chat->tp,chat->how);
} /* C2S_SendChat */

/*
 * queue sync to server
 */
void
C2S_Sync (XBComm *comm, XBNetworkEvent event)
{
  XBCommStream *stream = (XBCommStream *) comm;
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  Net_SendTelegram (stream->sndQueue, Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_Sync, (XBTeleIOB) event, NULL, 0) );
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  Dbg_C2S("queued sync #%u to server!\n", event);
} /* C2S_Sync */

/*
 * queue level check result to server
 */
void
C2S_LevelCheck (XBComm *comm, XBBool rej)
{
  XBCommStream *stream = (XBCommStream *) comm;
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  Net_SendTelegram (stream->sndQueue, Net_CreateTelegram (XBT_COT_Activate, XBT_ID_LevelConfig, rej? 0 : 1, NULL, 0) );
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  Dbg_C2S("queued level check result to server = %s\n", rej? "reject" : "accept");
} /* C2S_LevelCheck */

/*
 * queue level winner to server
 */
void
C2S_SendWinner (XBComm *comm, unsigned team)
{
  XBCommStream *stream = (XBCommStream *) comm;
  assert (stream != NULL);
  assert (stream->sndQueue != NULL);
  Net_SendTelegram (stream->sndQueue, Net_CreateTelegram (XBT_COT_Activate, XBT_ID_WinnerTeam, team, NULL, 0) );
  Socket_RegisterWrite (CommSocket (&stream->comm) );
  Dbg_C2S("queued level winner to server = %u\n", team);
} /* C2S_SendWinner */

/*
 * end of file com_to_server.c
 */
