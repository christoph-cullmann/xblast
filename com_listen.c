/*
 * file com_listen.c - listen for client to connect
 *
 * $Id: com_listen.c,v 1.8 2006/02/09 21:21:23 fzago Exp $
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
typedef struct
{
	XBComm comm;
	XBBool shutdown;
	XBBool central;
} XBCommListen;

/*
 * handle readability = connection on listen socket
 */
static XBCommResult
ReadListen (XBComm * comm)
{
	XBCommListen *lComm = (XBCommListen *) comm;
	if (lComm->central) {
		if (NULL != (C2X_CreateComm (comm->socket))) {
			Dbg_Listen ("central user has connected\n");
		}
	}
	else {
		if (NULL != (S2C_CreateComm (comm->socket))) {
			Dbg_Listen ("client has connected\n");
		}
	}
	return XCR_OK;
}								/* ReadListen */

/*
 * handle writeability
 */
static XBCommResult
WriteListen (XBComm * comm)
{
	return XCR_OK;
}								/* WriteListen */

/*
 * handle delete
 */
static XBCommResult
DeleteListen (XBComm * comm)
{
	XBCommListen *lComm = (XBCommListen *) comm;
	Dbg_Listen ("removing listen socket on port %u, %s\n", Net_LocalPort (comm->socket),
				(lComm->shutdown) ? "expected" : "unexpected");
	if (lComm->central) {
		/* implement this to catch errors on listen socket!
		   User_ReceiveListenClose(lComm->shutdown);
		 */
	}
	else {
		Server_ReceiveListenClose (lComm->shutdown);
	}
	CommFinish (comm);
	free (comm);
	return XCR_OK;
}								/* DeleteListen */

/*
 * create listening tcp socket
 */
XBComm *
CommCreateListen (const CFGGameHost * cfg, XBBool central)
{
	XBSocket *pSocket;
	XBCommListen *lComm;
	assert (cfg != NULL);
	/* create listen socket */
	pSocket = Net_ListenInet (cfg->port);
	if (NULL == pSocket) {
		Dbg_Listen ("failed to create socket on port %u\n", cfg->port);
		return NULL;
	}
	/* create communication data structure */
	lComm = calloc (1, sizeof (XBCommListen));
	assert (NULL != lComm);
	/* set values */
	CommInit (&lComm->comm, COMM_Listen, pSocket, ReadListen, WriteListen, DeleteListen);
	/* store type of listen socket, shutdown flag */
	lComm->central = central;
	lComm->shutdown = XBFalse;
	/* that's all */
	Dbg_Listen ("listening on port %u now (%s)\n", Net_LocalPort (pSocket),
				central ? "central" : "server");
	return &lComm->comm;
}								/* CommCreateListen */

/*
 * finish listen socket regularly
 */
void
CommFinishListen (XBComm * comm)
{
	XBCommListen *lComm = (XBCommListen *) comm;
	/* mark for shutdown */
	lComm->shutdown = XBTrue;
	/*  now delete */
	DeleteListen (comm);
}								/* CommFinishListen */

/*
 * end of file com_listen.c
 */
