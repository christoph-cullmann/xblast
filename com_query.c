/*
 * file com_query.c - client queryes for local network game
 *
 * $Id: com_query.c,v 1.16 2005/01/23 14:30:48 lodott Exp $
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
#include "com_query.h"
#include "com_browse.h"
#include "version.h"
#include "client.h"
#include "gui.h"
#include "str_util.h"
#include "timeval.h"

/*
 * local types
 */
typedef struct {
  XBCommBrowse   browse;
  char          *addrRemote;         /* address to connect to */
  unsigned short port;               /* port of remote address */
  unsigned char  serial;             /* datagram id */
  struct timeval tvSend;             /* time of last send */
  unsigned       id;                 /* interface id */
  XBBool         broadcast;          /* broadcast flag */
  XBBool         deleted;            /* deletion flag */
} XBCommQuery;

/*
 * handle replies, from either a server or from central
 */
static void
HandleReply (XBCommQuery *qComm, const XBBrowseTeleReply *tele, const char *host)
{
  long  msec;
  struct timeval tv;
  char version[32];

  /* calculate ping time */
  gettimeofday (&tv, NULL);
  msec = (tv.tv_sec - qComm->tvSend.tv_sec) * 1000L + (tv.tv_usec - qComm->tvSend.tv_usec) / 1000L;
  /* create remote version string */
  sprintf (version, "V%u.%u.%u", (unsigned) tele->version[0], (unsigned) tele->version[1], (unsigned) tele->version[2]);
  /* check if it is response to latest query */
  if ( tele->any.serial != qComm->serial ) {
    Dbg_Query("received reply ignored, got serial %u, expected serial %u\n", tele->any.serial, qComm->serial);
    return;
  }
  /* check version and source, inform client */
  if ( ( (tele->version[0]==VERSION_MAJOR) && (tele->version[1]==VERSION_MINOR) ) || (tele->frameRate==0)) {
    /* versions match or message */
    if (strlen(tele->host)<=0) {
      /* server reply: host entry is empty/NULL */
      Dbg_Query ("received reply for #%u - game \"%s\" at %s:%hu (%ld msec)\n", tele->any.serial, tele->game, host, tele->port, msec);
      Client_ReceiveReply (qComm->id, host, tele->port, msec, version, tele->game, tele->numLives, tele->numWins, tele->frameRate);
    } else {
      /* central reply: host entry not empty*/
      Dbg_Query ("received reply for #%u - game \"%s\" at %s:%hu (from central %ld msec)\n", tele->any.serial, tele->game, tele->host, tele->port, msec);
      Client_ReceiveReply (qComm->id, tele->host, tele->port, msec, version, tele->game, tele->numLives, tele->numWins, tele->frameRate);
    }
  } else {
    /* versions don't match and not message */
    if (strlen(tele->host)<=0) {
      /* server reply */
      Dbg_Query ("received reply for #%u - game \"%s\" at %s:%hu (%ld msec), WRONG VERSION %s\n", tele->any.serial, tele->game, host, tele->port, msec, version);
    } else {
      /* central reply */
      Dbg_Query ("received reply for #%u - game \"%s\" at %s:%hu (from central %ld msec), WRONG VERSION %s\n", tele->any.serial, tele->game, tele->host, tele->port, msec, version);
    }
  }
} /* HandleReply */

/*
 * handle received data
 */
static void
ReceiveQuery (XBCommBrowse *bComm, const XBBrowseTele *tele, const char *host, unsigned short port)
{
  assert (NULL != bComm);
  assert (NULL != tele);
  assert (NULL != host);
  switch (tele->type) {
  case XBBT_Reply: HandleReply ((XBCommQuery *) bComm, &tele->reply, host); break;
  default: Dbg_Query("receiving invalid signal %u, ignoring\n", tele->type); break;
  }
} /* ReceiveQuery */

/*
 * handle browse event
 */
static XBBool
EventQuery (XBCommBrowse *bComm, XBBrowseEvent ev)
{
#if defined(DEBUG_QUERY) || defined(WMS)
  XBCommQuery *qComm = (XBCommQuery *) bComm;
  unsigned id = qComm->id;
#endif
  switch (ev) {
  case XBBE_Wait:  Dbg_Query("all data sent on #%u\n",id); break;
  case XBBE_Dgram: Dbg_Query("received invalid datagram on #%u, ignoring\n",id); break;
  case XBBE_Browse: Dbg_Query("received invalid browse on #%u, ignoring\n",id); break;
  case XBBE_Write: Dbg_Query("new data waits to be sent on #%u\n",id); break;
  case XBBE_Close: Dbg_Query("browse shutdown complete for #%u\n",id); break;
  default: Dbg_Query("unknown browse event on #%u, ignoring!\n");
  }
  return XBFalse;
} /* EventQuery */

/*
 * Delete handler for COMM_Query sockets
 * frees contents of structure but not the structure itself
 * deleted entry indicates state of the structure
 */
static XBCommResult
DeleteQuery (XBComm *comm)
{
  XBCommQuery *qComm = (XBCommQuery *) comm;
  assert (NULL != qComm);
  assert(! qComm->deleted);
  Browse_Finish (&qComm->browse);
  assert (NULL != qComm->addrRemote);
  free (qComm->addrRemote);
  /* mark as deleted, do *not* free qComm yet */
  qComm->deleted = XBTrue;
  /* inform client */
  Client_ReceiveQueryClose(qComm->id);
  return XCR_OK;
} /* DeleteQuery  */

/*
 * create broadcast socket to query for network games/central games
 */
XBComm *
Query_CreateComm (unsigned id, const char *local, const char *remote, unsigned short port, XBBool broadcast)
{
  XBSocket    *pSocket;
  XBCommQuery *qComm;

  assert (NULL != remote);
  /* create socket */
  pSocket = Net_BindUdp (local, 0);
  if (NULL == pSocket) {
    Dbg_Query("failed to establish query socket on %s to %s:%u\n", (local==NULL) ? "auto" : local, remote, port);
    return NULL;
  }
  Dbg_Query("established udp socket on %s:%u\n", Socket_HostName(pSocket, XBFalse), Socket_HostPort(pSocket,XBFalse));
  /* create communication data structure */
  qComm = calloc (1, sizeof (*qComm));
  assert (NULL != qComm);
  /* set values */
  Browse_CommInit (&qComm->browse, COMM_Query, pSocket, ReceiveQuery, EventQuery, DeleteQuery);
  qComm->addrRemote     = DupString (remote);
  qComm->port           = port;
  qComm->serial         = 0;
  qComm->tvSend.tv_sec  = 0;
  qComm->tvSend.tv_usec = 0;
  qComm->id             = id;
  qComm->broadcast      = broadcast;
  qComm->deleted        = XBFalse;
  /* that's all ? */
  return &qComm->browse.comm;
} /* Query_CreateComm */

/*
 * return deletion flag
 */
XBBool
Query_isDeleted(XBComm *comm)
{
  XBCommQuery *qComm = (XBCommQuery *) comm;
  assert (comm->type == COMM_Query);
  assert (NULL != qComm);
  return qComm->deleted;
} /* Query_isDeleted */

/*
 * query central/lan for games
 */
void
Query_Send (XBComm *comm, const struct timeval *tv)
{
  XBBrowseTeleQuery tele;
  XBCommQuery *qComm = (XBCommQuery *) comm;

  assert (NULL != qComm);
  assert (NULL != tv);
  /* create browse datagram */
  tele.any.type   = XBBT_Query;
  tele.any.serial = ++ qComm->serial;
  /* queue data */
  Browse_Send (&qComm->browse, qComm->addrRemote, qComm->port, qComm->broadcast, &tele.any);
  /* mark time */
  qComm->tvSend = *tv;
  Dbg_Query("queued query #%u to %s:%hu\n",qComm->serial,qComm->addrRemote, qComm->port);
} /* Query_Send */

/*
 * end of file com_query.c
 */
