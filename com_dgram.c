/*
 * file com_dgram.c - base struct und functions for datagram connections
 *
 * $Id: com_dgram.c,v 1.9 2005/01/15 12:40:38 lodott Exp $
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
#include "com_dgram.h"
#include "timeval.h"

/*
 * local macros
 */
#define GAME_TIME_PING     0xFFFF
#define GAME_TIME_RESERVED 0xFFFE
#define PLAYER_MASK_FINISH 0xFF

/*
 * local variables
 */
static unsigned char buffer[MAX_DGRAM_SIZE];

/*
 * pack player action
 */
static size_t
PackPlayerAction (PackedPlayerAction *dst, const PlayerAction *src)
{
  size_t        i, j;
  unsigned char action;

  assert (NULL != dst);
  assert (NULL != src);

  dst->mask = 0;
  for (i = 0, j = 0; i < MAX_PLAYER; i ++) {
    action = PlayerActionToByte (src + i);
    if (0 != action) {
      dst->mask      |= (1u << i);
      dst->action[j]  = action;
      j ++;
    }
  }
  dst->numBytes = j + 1;
  return dst->numBytes;
} /* PackPlayerAction */

/*
 * unpack player action
 */
static size_t
UnpackPlayerAction (PlayerAction *dst, const unsigned char *buf)
{
  size_t        i, j;
  unsigned char mask;

  assert (NULL != dst);
  assert (NULL != buf);

  for (i = 0, j = 1, mask = 1; i < MAX_PLAYER; i ++, mask <<= 1u) {
    if (buf[0] & mask) {
      PlayerActionFromByte (dst + i, buf[j]);
      j ++;
    } else {
      PlayerActionFromByte (dst + i, 0x00);
    }
  }
  return j;
} /* UnpackPlayerAction */

/*
 * show current status
 */
void
CurrentStatus(XBCommDgram *dComm) {
  assert(NULL != dComm);
  Dbg_Dgram ("rcv=(%u,%u), buf=(%u,%u), snd=(%u,%u), exp=%u, queue=%u\n",
	     dComm->rcvfirst, dComm->rcvfirst ? dComm->rcvnext-1 : 0,
	     dComm->buffirst, dComm->buffirst ? dComm->bufnext-1 : 0,
	     dComm->sndfirst, dComm->sndfirst ? dComm->sndnext-1 : 0,
	     dComm->expect, dComm->queue);
} /* CurrentStatus */

/*
 * handle a received ping time
 */
static void
HandlePing (XBCommDgram *dComm, const unsigned char *data, size_t len)
{
  size_t i, j;

  assert (NULL != dComm);
  /* call pingFunc once for each received time */
  if (len > 0) {
    assert (NULL != data);
    for (i = 1, j = 0; i < MAX_HOSTS && j < len; i ++, j += 2) {
      (*dComm->pingFunc) (dComm, i, (data[j+1] << 8) + data[j]);
    }
  }
  /* call pingFunc for empty ping */
  (*dComm->pingFunc) (dComm, 0, 0);
  Dbg_Dgram ("handle pings, len=%u\n", len);
} /* HandlePing */

/*
 * handle received frames data
 */
static void
HandleFrames (XBCommDgram *dComm, unsigned gameTime, const unsigned char *data, size_t len)
{
  size_t i;
  XBBool ignored;
  static PlayerAction playerAction[MAX_PLAYER];

  /* this frame is in the future ... */
  assert (NULL != dComm->infoFunc);
  if (gameTime > dComm->expect) {
    Dbg_Out ("DGRAM: handle frames %d-%d lost\n", dComm->expect, gameTime-1);
    if ((*dComm->infoFunc) (dComm, XBDI_LOSS)) {
      return;
    }
  }
  /* set first frame to send for client */
  /* TODO: why is that needed? if needed, move to com_dg_server.c!
  if (! dComm->primary) {
    dComm->queue = gameTime;
  }
  */
  dComm->rcvfirst = gameTime;
  i = 0;
  while (i < len) {
    ignored = XBFalse;
    if (PLAYER_MASK_FINISH == data[i]) {
      Dbg_Dgram("FINISH received for gt=%u\n",gameTime);
      (void) ( (*dComm->infoFunc) (dComm,XBDI_FINISH) );
      i ++;
    } else {
      i += UnpackPlayerAction (playerAction, data + i);
      if (gameTime != dComm->expect) {
	Dbg_Dgram ("ignoring action for gt=%u, expected gt=%u\n",gameTime, dComm->expect);
	dComm->ignore = gameTime;
	(void) ( (*dComm->infoFunc) (dComm,XBDI_IGNORE) );
	ignored = XBTrue;
      } else {
	Dbg_Dgram ("accepting action for gt=%u\n",gameTime);
	assert (dComm->actionFunc != NULL);
	(*dComm->actionFunc) (dComm, gameTime, playerAction);
      }
    }
    /* adjust datagrams to send */
    if (! ignored) {
      dComm->expect ++;
    }
    /* ready for next frame */
    gameTime ++;
  }
  dComm->rcvnext = gameTime;
  (void) ((*dComm->infoFunc) (dComm, XBDI_PARSED));
  /*
  if (dComm->primary) {
    dComm->queue = gameTime;
  }
  */
} /* HandleFrames */

/*
 * receive datagram from readable socket
 */
static XBCommResult
ReadDgram (XBComm *comm)
{
  XBDatagram *rcv;
  const unsigned char *data;
  size_t               len;
  unsigned             gameTime;
  const char *         host;
  unsigned short       port;
  XBCommDgram         *dComm = (XBCommDgram *) comm;

  assert (NULL != dComm);
  if (NULL == dComm->host) {
    /* get datagram for connected socket */
    rcv = Net_ReceiveDatagram (comm->socket);
    Dbg_Dgram("rcv datagram on connected socket\n");
  } else {
    /* get datagram plus sender for unconnected socket */
    rcv = Net_ReceiveDatagramFrom (comm->socket, &host, &port);
    Dbg_Dgram("rcv datagram from %s:%u\n",host,port);
    /* match hosts */
    if (0 == strcmp (host, dComm->host)) {
      dComm->connected  = Net_ConnectUdp (comm->socket, host, port);
      if (dComm->connected) {
	Dbg_Dgram("successfully connected!\n");
	/* no further hostname checking */
	dComm->host       = NULL;
	(void) (*dComm->infoFunc) (dComm, XBDI_CONSUCC);
	return XCR_OK;
      } else {
	Dbg_Dgram("failed to connect!\n");
	assert(NULL != dComm->infoFunc);
	if ( (*dComm->infoFunc) (dComm, XBDI_CONFAIL)) {
	  return XCR_Error;
	}
      }
    } else {
      Dbg_Dgram("unexpected host!\n");
      return XCR_OK;
    }
  }
  if (NULL != rcv) {
    /* save reception time for ping calculation */
    gettimeofday (&dComm->lastRcv, NULL);
    /* copy data for application */
    data = Net_DgramData (rcv, &len);
    if (len == 0) {
      /* no data -> empty ping */
      HandlePing (dComm, data, 0);
    } else if (len >= 2) {
      /* first two bytes determine type otherwise */
      gameTime = (data[1] << 8) + data[0];
      if (GAME_TIME_PING == gameTime) {
	/* list of ping times */
	HandlePing (dComm, data + 2, len - 2);
      } else if (GAME_TIME_RESERVED == gameTime) {
	/* reserved type for future extensions */
      } else {
	/* frame data for given gametime */
	HandleFrames (dComm, gameTime, data + 2, len - 2);
      }
    }
    /* received datagram is parsed now */
    Net_DeleteDatagram (rcv);
  }
  CurrentStatus(dComm);
  return XCR_OK;
} /* ReadDgram */

/*
 * write current datagram to writeable socket
 */
static XBCommResult
WriteDgram (XBComm *comm)
{
  XBCommDgram *dComm = (XBCommDgram *) comm;

  assert (NULL != comm);
  /* unregister for next socket loop */
  Socket_UnregisterWrite (CommSocket (comm));
  /* buffer should be non-NULL, but assert fails occasionally */
  /* assert(NULL != dComm->snd); */
  if (NULL != dComm->snd) {
    /* try to send */
    if (! Net_SendDatagram (dComm->snd, comm->socket) ) {
      Dbg_Dgram("failed to send datagram!\n");
      assert(NULL != dComm->infoFunc);
      return (*dComm->infoFunc) (dComm, XBDI_WRITEERR) ? XCR_Error : XCR_OK;
    }
    /* success, update send info for frames */
    dComm->sndfirst = dComm->buffirst;
    dComm->sndnext = dComm->bufnext;
    /* clear buffer */
    Net_DeleteDatagram (dComm->snd);
    dComm->snd = NULL;
    dComm->buffirst = 0;
    dComm->bufnext = 0;
    /* store send time for ping calculations */
    gettimeofday (&dComm->lastSnd, NULL);
    if (dComm->sndfirst<dComm->sndnext) {
      Dbg_Dgram("sent frames [%u,%u]\n", dComm->sndfirst, dComm->sndnext-1);
    } else {
      Dbg_Dgram("sent pings\n");
    }
  }
  CurrentStatus(dComm);
  return XCR_OK;
} /* WriteDgram */

/*
 * free the XBComm structure
 */
static XBCommResult
DeleteDgram (XBComm *comm)
{
  XBCommDgram *dgram = (XBCommDgram *) comm;
  assert (dgram != NULL);
  CurrentStatus(dgram);
  if (NULL != dgram->snd) {
    Net_DeleteDatagram (dgram->snd);
  }
  (void) (dgram->infoFunc) (dgram, XBDI_CLOSE);
  CommFinish (&dgram->comm);
  free (dgram);
  Dbg_Dgram("instance removed\n");
  return XCR_OK;
} /* DeleteDgram */

/*
 * create datagram communication structure
 */
XBComm *
Dgram_CommInit (XBCommDgram *dComm, XBCommType commType, XBSocket *pSocket,
		DgramPingFunc pingFunc, DgramInfoFunc infoFunc,
		DgramActionFunc actionFunc)
{
  assert (NULL != dComm);
  assert (NULL != pingFunc);
  assert (NULL != infoFunc);
  assert (NULL != actionFunc);
  /* set values */
  CommInit (&dComm->comm, commType, pSocket, ReadDgram, WriteDgram, DeleteDgram);
  dComm->snd	 = NULL;
  dComm->port	 = Net_LocalPort (pSocket);
  dComm->host            = NULL;
  dComm->connected       = XBFalse;
  dComm->rcvfirst	 = 0;
  dComm->rcvnext	 = 0;
  dComm->buffirst	 = 0;
  dComm->bufnext	 = 0;
  dComm->sndfirst	 = 0;
  dComm->sndnext	 = 0;
  dComm->ignore          = 0;
  dComm->queue           = 0;
  dComm->expect	         = 0;
  dComm->pingFunc	 = pingFunc;
  dComm->infoFunc	 = infoFunc;
  dComm->actionFunc      = actionFunc;
  dComm->lastSnd.tv_sec  = 0;
  dComm->lastSnd.tv_usec = 0;
  dComm->lastRcv.tv_sec  = 0;
  dComm->lastRcv.tv_usec = 0;
  memset (dComm->ppa, 0, sizeof (dComm->ppa));
  /* that's all */
  Dbg_Dgram("created at local port %u\n",dComm->port);
  CurrentStatus(dComm);
  return &dComm->comm;
} /* D2C_CreateComm */

/*
 * get port for client
 */
unsigned short
Dgram_Port (const XBCommDgram *dComm)
{
  /* sanity checks */
  assert (dComm != NULL);
  /* get value */
  return dComm->port;
} /* D2C_Port */

/*
 * reset read/write parameters
 */
void
Dgram_Reset (XBCommDgram *dComm)
{
  assert (dComm != NULL);
  Dbg_Dgram("resetting\n");
  /* clear any old datagrams */
  if (NULL != dComm->snd) {
    Dbg_Dgram ("clear buffer [%u:%u]\n", dComm->buffirst, dComm->bufnext - 1);
    Net_DeleteDatagram (dComm->snd);
    dComm->snd = NULL;
  }
  dComm->buffirst     = 0;
  dComm->bufnext      = 0;
  dComm->rcvfirst     = 0;
  dComm->rcvnext      = 0;
  dComm->expect       = 1;
  dComm->queue        = 1;
  memset (dComm->ppa, 0, sizeof (dComm->ppa));
} /* Dgram_Reset */

/*
 * write a ping to buffer unless buffer is occupied
 */
void
Dgram_SendPing (XBCommDgram *dComm)
{
  if (NULL == dComm->snd) {
    dComm->snd = Net_CreateDatagram (NULL, 0);
    Socket_RegisterWrite (CommSocket (&dComm->comm));
    Dbg_Dgram("Queueing ping\n");
  } else {
    Dbg_Dgram("Buffer occupied, discarding ping\n");
  }
} /* Dgram_SendPing */

/*
 * write ping data to buffer unless buffer is occupied
 */
void
Dgram_SendPingData (XBCommDgram *dComm, const int pingTime[])
{
  size_t        i;
  unsigned char pingData[2*MAX_HOSTS];

  if (NULL == dComm->snd) {
    assert (NULL != pingTime);
    /* setup buffer with ping times */
    pingData[0] = 0xFF & (GAME_TIME_PING);
    pingData[1] = 0xFF & (GAME_TIME_PING >> 8);
    for (i = 1; i < MAX_HOSTS; i++) {
      pingData[2*i]   = 0xFF & ((unsigned) pingTime[i]);
      pingData[2*i+1] = 0xFF & ((unsigned) pingTime[i] >> 8);
    }
    dComm->snd = Net_CreateDatagram (pingData, 2*MAX_HOSTS);
    Socket_RegisterWrite (dComm->comm.socket);
    Dbg_Dgram("Queued ping times\n");
  } else {
    Dbg_Dgram("Buffer occupied, discarding ping times\n");
  }
} /* Dgram_SendPingData */

/*
 * queue player actions or finish to buffer
 */
static void
CreateBuffer (XBCommDgram  *dComm, int first, int next)
{
  int i;
  size_t len;

  /* sanity check */
  assert (dComm != NULL);
  /* clear any old datagrams */
  if (NULL != dComm->snd) {
    Dbg_Dgram ("clear buffer [%u:%u]\n", dComm->buffirst, dComm->bufnext - 1);
    Net_DeleteDatagram (dComm->snd);
    dComm->snd = NULL;
  }
  /* set time stamp */
  memset (buffer, 0, sizeof (buffer));
  buffer[0] = 0xFF &  first;
  buffer[1] = 0xFF & (first >> 8);
  /* fill buffer */
  len = 2;
  for (i = first; i < next; i ++) {
    /* check for space in datagram buffer */
    if (dComm->ppa[i].numBytes + len > sizeof (buffer) ) {
      break;
    }
    /* copy to buffer */
    memcpy (buffer + len, &dComm->ppa[i].mask, dComm->ppa[i].numBytes);
    len += dComm->ppa[i].numBytes;
  }
  /* store current buffer parameters */
  dComm->buffirst = first;
  dComm->bufnext = i;
  /* prepare sending */
  Socket_RegisterWrite (dComm->comm.socket);
  assert(dComm->snd == NULL);
  dComm->snd =  Net_CreateDatagram (buffer, len);
  CurrentStatus(dComm);
} /* Dgram_SendPlayerAction */

/*
 * send player action to client
 */
void
Dgram_SendPlayerAction (XBCommDgram  *dComm, int gameTime, const PlayerAction *playerAction)
{
  /* sanity check */
  assert (dComm != NULL);
  assert (gameTime < NUM_PLAYER_ACTION-1);
  assert (playerAction != NULL);
  /* pack data */
  PackPlayerAction (dComm->ppa + gameTime, playerAction);
  /* try to send it */
  CreateBuffer (dComm, dComm->queue, gameTime+1);
} /* Dgram_SendPlayerAction */

/*
 * acknowledge level finish
 */
void
Dgram_SendFinish (XBCommDgram *dComm, int gameTime)
{
  /* sanity check */
  assert (dComm != NULL);
  assert (gameTime < NUM_PLAYER_ACTION);
  /* pack data */
  dComm->ppa[gameTime].numBytes = 1;
  dComm->ppa[gameTime].mask     = PLAYER_MASK_FINISH;
  /* try to send it */
  CreateBuffer (dComm, dComm->queue, gameTime+1);
} /* Dgram_SendFinish */

/*
 * flush - resend last batch of data
 */
XBBool
Dgram_Flush (XBCommDgram *dComm)
{
  /* requeue last send */
  CreateBuffer (dComm, dComm->sndfirst,dComm->sndnext);
  return XBTrue;
} /* Dgram_Flush */

/*
 * end of file com_dgram.c
 */
