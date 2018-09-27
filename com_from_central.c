/*
 * file com_from_central.c - handle communications with clients
 *
 * $Id: com_from_central.c,v 1.4 2004/11/06 02:00:41 lodott Exp $
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
#include "com_from_central.h"

#include "central.h"
#include "com_stream.h"
#include "net_tele.h"
#include "server.h"
#include "cfg_level.h"

/*
 * local types
 */
typedef struct {
  XBCommStream stream;
  unsigned     serial;
} XBCommFromCentral;

/*
 * local variables
 */
static XBCommFromCentral *commList[MAX_HOSTS] = {
  /* up to 6 clients can coonect */
  NULL, NULL, NULL, NULL, NULL, NULL,
};

/*
 * a client requests rankings
 */
static void
HandleRequestPlayerConfig (XBSndQueue *sndQueue, const char *line)
{
  XBAtom atom;
  unsigned int    i,j,k,l=0;

  assert (NULL != sndQueue);
  /* get player atom from config  */
  j=GetNumPlayerConfigs (CT_Central);
  for (i=0; i<j; i++) {
    atom=GetPlayerAtom (CT_Central, i);
    if (atom == ATOM_INVALID) {
      Dbg_C2X("invalid player atom, cancelling reply\n");
      return;
    }
    k=GUI_AtomToInt(atom);
    if (k>0) {
      Dbg_C2X("Sending player config of player %u\n", k);
      if (! SendPlayerConfig (CT_Central, sndQueue, XBT_COT_DataAvailable, 0, atom, XBFalse) ) {
	Dbg_C2X("Unable to send player config of player %u\n", k);
      } else {
	l++;
      }
    }
  }
  /* that's all */
  Dbg_C2X("%u players queued\n",l);
  /* data not available means no more entries */
  Net_SendTelegram (sndQueue, Net_CreateTelegram (XBT_COT_DataNotAvailable, XBT_ID_PlayerConfig, 0, NULL, 0) );
  /* send now */
  return;
} /* HandleRequestPlayerConfig */

/*
 * client requests data
 */
static XBCommResult
HandleRequestData (XBCommFromCentral *fromCentral, const XBTelegram *tele)
{
  const char *data;
  size_t      len;
  data = Net_TeleData (tele, &len);
  /* now retrieve data */
  switch (Net_TeleID (tele)) {
  case XBT_ID_PlayerConfig:
    Dbg_C2X("client %u requests players\n", fromCentral->serial);
    HandleRequestPlayerConfig (fromCentral->stream.sndQueue, data);
    Socket_RegisterWrite (CommSocket (&commList[fromCentral->serial]->stream.comm));
    break;
  default:
    break;
  }
  return XCR_OK;
} /* HandleRequestData */

/*
 * client sends data
 */
static XBCommResult
HandleSendData (XBCommFromCentral *fromCentral, const XBTelegram *tele)
{
  const char *data;
  size_t      len;
  XBTeleIOB   iob;
  data = Net_TeleData (tele, &len);
  iob  = Net_TeleIOB (tele);
  switch (Net_TeleID (tele)) {
  case XBT_ID_PlayerConfig:
    Dbg_C2X("client %u registers player\n", fromCentral->serial);
    Central_ReceivePlayerConfig (fromCentral->serial, data);
    /* TODO: check if there is data to send! */
    Socket_RegisterWrite (CommSocket (&commList[fromCentral->serial]->stream.comm));
    break;
  case XBT_ID_GameStat:
    Dbg_C2X("client %u sends game stat\n", fromCentral->serial);
    Central_ReceiveGameStat (data);
    break;
  default:
    return XCR_OK;
  }
  return XCR_OK;
} /* HandleSendData */

/*
 * client sends command
 */
static XBCommResult
HandleActivate (XBCommFromCentral *fromCentral, const XBTelegram *tele)
{
  const void *data;
  size_t      len;
  data = Net_TeleData (tele, &len);
  switch (Net_TeleID (tele)) {
  case XBT_ID_RequestDisconnect:
    /* just close the socket */
    Dbg_C2X("client %u requests disconnect\n", fromCentral->serial);
    return XCR_Finished;
  default:
    break;
  }
  return XCR_OK;
} /* HandleActivate */

/*
 * client sends spontaneous info
 */
static XBCommResult
HandleSpontaneous (XBCommFromCentral *fromCentral, const XBTelegram *tele)
{
  switch (Net_TeleID (tele)) {
  case XBT_ID_HostDisconnected:
    Dbg_C2X("client %u prepares to disconnect\n", fromCentral->serial);
    Central_ReceiveDisconnect (fromCentral->serial);
    return XCR_OK;
  default:
    break;
  }
  return XCR_OK;
} /* HandleSpontaneous */

/*
 * handle client telegrams
 */
static XBCommResult
HandleTelegram (XBCommStream *stream, const XBTelegram *tele)
{
  XBCommFromCentral *fromCentral = (XBCommFromCentral *) stream;
  assert (fromCentral != NULL);
  switch (Net_TeleCOT (tele)) {
    /* user requests data from central */
  case XBT_COT_RequestData:
    return HandleRequestData (fromCentral, tele);
    /* user sends data to central */
  case XBT_COT_SendData:
    return HandleSendData (fromCentral, tele);
    /* server activate command on client */
  case XBT_COT_Activate:
    return HandleActivate (fromCentral, tele);
    /* server send spontaneous status change */
  case XBT_COT_Spontaneous:
    return HandleSpontaneous (fromCentral, tele);
  default:
    return XCR_Error;
  }
} /* HandleTelegram */

/*
 * delete handler
 */
static XBCommResult
DeleteFromCentral (XBComm *comm)
{
  XBCommFromCentral *fromcentral = (XBCommFromCentral *) comm;
  assert (comm != NULL);
  assert (fromcentral == commList[fromcentral->serial]);
  /* unmark client */
  commList[fromcentral->serial] = NULL;
  /* clean up */
  Stream_CommFinish (&fromcentral->stream);
  /* make sure application is informed */
  Central_ReceiveDisconnect (fromcentral->serial);
  /* free memory */
  Dbg_C2X("tcp connection to client %u is removed\n", fromcentral->serial);
  free (comm);
  return XCR_OK;
} /* DeleteFromCentral */

/*
 * handle stream events
 */
static XBBool
EventFromCentral (XBCommStream *comm, const XBStreamEvent ev)
{
  switch (ev) {
  case XBST_IOREAD: return XBTrue; break;
  case XBST_IOWRITE: return XBTrue; break;
  case XBST_EOF: return XBTrue; break;
  case XBST_WAIT: return XBFalse; break;
  case XBST_BUSY: return XBFalse; break;
  case XBST_CLOSE: return XBFalse; break;
  default: return XBFalse;
  }
  /* TODO: implement this!
  return Central_StreamEvent(code);
  */
} /* EventFromCentral */

/*
 * create listeneing communication
 */
XBComm *
C2X_CreateComm (const XBSocket *socket)
{
  unsigned        serial;
  XBSocket       *pSocket;
  XBCommFromCentral *fromcentral;

  assert (socket != NULL);
  /* get free serial */
  for (serial = 0; serial < MAX_HOSTS; serial ++) {
    if (NULL == commList[serial]) {
      break;
    }
  }
  if (serial >= MAX_HOSTS) {
    Dbg_C2X("failed to find serial, rejecting connection\n");
    return NULL;
  }
  Dbg_C2X("assigned serial %u\n", serial);
  /* create listen socket */
  pSocket = Net_Accept (socket);
  if (NULL == pSocket) {
    Dbg_C2X("failed to accept socket, rejecting connection\n");
    return NULL;
  }
  /* create communication data structure */
  fromcentral = calloc (1, sizeof (XBCommFromCentral) );
  assert (NULL != fromcentral);
  /* set values */
  Stream_CommInit (&fromcentral->stream, COMM_FromCentral, pSocket, HandleTelegram, EventFromCentral, DeleteFromCentral);
  fromcentral->serial = serial;
  /* add to inernal list */
  commList[serial] = fromcentral;
  /* inform application */
  Central_Accept (serial, Net_RemoteName (pSocket), Net_RemotePort (pSocket));
  /* that's all */
  return &fromcentral->stream.comm;
} /* C2X_CreateComm */

/************
 * get data *
 ************/

/*
 * check if client is connected
 */
XBBool
C2X_Connected (unsigned id)
{
  assert (id < MAX_HOSTS);
  return (commList[id] != NULL);
} /* C2X_Connected */

/*
 * hostname of client
 */
const char *
C2X_HostName (unsigned id)
{
  assert (id <= MAX_HOSTS);
  assert (commList[id] != NULL);
  /* get name from socket */
  return Net_RemoteName (commList[id]->stream.comm.socket);
} /* C2X_HostName */

/*
 * hostname of client
 */
const char *
C2X_LocalName (unsigned id)
{
  assert (id <= MAX_HOSTS);
  assert (commList[id] != NULL);
  /* get name from socket */
  return Net_LocalName (commList[id]->stream.comm.socket);
} /* C2X_LocalName */

/**************
 * queue data *
 **************/

/*
 * send game config to client
 */
void
C2X_SendPlayerConfig (unsigned id, unsigned hostId, int player, XBAtom atom)
{
  XBTeleIOB iob;
  assert (id < MAX_HOSTS);
  assert (commList[id] != NULL);
  assert (commList[id]->stream.sndQueue != NULL);
  /* convert id and player to iob */
  iob = ((XBTeleIOB) hostId << 4) + (XBTeleIOB) player;
  /* send database section */
  Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
  SendPlayerConfig (CT_Remote, commList[id]->stream.sndQueue, XBT_COT_SendData, iob, atom, XBFalse);
} /* C2X_SendPlayerConfig */

/*
 * send random seed to client
 */
void
C2X_SendUserPID (unsigned id, int PID)
{
  char          tmp[16];
  /* sanity check */
  assert (id < MAX_HOSTS);
  assert (commList[id] != NULL);
  assert (commList[id]->stream.sndQueue != NULL);
  /* send seed as ascii */
  sprintf (tmp, "%i", PID);
  /* send data */
  Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
  Net_SendTelegram (commList[id]->stream.sndQueue, Net_CreateTelegram (XBT_COT_DataAvailable, XBT_ID_PID, 0, tmp, strlen (tmp) + 1) );
} /* S2C_SendUserPID */

/*
 * send disconnect message to client
 */
void
C2X_HostDisconnected (unsigned id, unsigned hostID)
{
  assert (id < MAX_HOSTS);
  assert (commList[id] != NULL);
  Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
  Net_SendTelegram (commList[id]->stream.sndQueue, Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostDisconnected, hostID, NULL, 0) );
} /* C2X_HostDisconnected */

/*
 * send request for disconnect to given client
 */
void
C2X_Disconnect (unsigned id)
{
  assert (id < MAX_HOSTS);
  assert (commList[id] != NULL);
  /* inform host about disconnect request */
  Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
  Net_SendTelegram (commList[id]->stream.sndQueue, Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostDisconnected, 0, NULL, 0) );
  Net_SendTelegram (commList[id]->stream.sndQueue, Net_CreateTelegram (XBT_COT_Activate, XBT_ID_RequestDisconnect, 0, NULL, 0) );
  /* clear slot, prepare to finish */
  commList[id]->stream.prepFinish = XBTrue;
  commList[id]=NULL;
} /* C2X_Disconnect */

/*
 * end of file com_from_central.c
 */
