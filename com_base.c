/*
 * file com_base.c - functions needed for all communications
 *
 * $Id: com_base.c,v 1.7 2006/02/24 21:29:16 fzago Exp $
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
 * local variables
 */
XBComm *list = NULL;

/*
 * display current XBComm list
 */
static void
CommDisplay (void)
{
	unsigned int cnt = 0;
	XBComm *comm;
	for (comm = list; comm != NULL; comm = comm->next) {
		fprintf (stderr, "(%u,%u) ", comm->type, Socket_Fd (comm->socket));
		cnt++;
	}
	fprintf (stderr, " - %u active instances\n", cnt);
}								/* CommDisplay */

/*
 * add a XBComm
 */
void
CommInit (XBComm * comm, XBCommType type, XBSocket * socket, XBCommFunc readFunc,
		  XBCommFunc writeFunc, XBCommFunc deleteFunc)
{
	/* sanity checks */
	assert (NULL != comm);
	assert (NULL != socket);
	/* add to list */
	Dbg_Comm ("adding XBComm type = %u, fd = %u\n", type, Socket_Fd (socket));
	comm->next = list;
	list = comm;
	comm->prev = NULL;
	if (NULL != comm->next) {
		comm->next->prev = comm;
	}
	/* set type */
	comm->type = type;
	/* set socket */
	comm->socket = socket;
	/* set handler */
	comm->readFunc = readFunc;
	comm->writeFunc = writeFunc;
	comm->deleteFunc = deleteFunc;
	/* register socket for reading */
	Socket_RegisterRead (comm->socket);
#ifdef DEBUG_COMM
	CommDisplay ();
#endif
}								/* CommInit */

/*
 * remove a XBComm from list, finish socket
 * does not free allocation for XBComm structure!!
 */
void
CommFinish (XBComm * comm)
{
	assert (comm != NULL);
	Dbg_Comm ("removing XBComm type = %u, fd = %u\n", comm->type, Socket_Fd (comm->socket));
	/* unregister socket for reading */
	Socket_UnregisterRead (comm->socket);
	Socket_UnregisterWrite (comm->socket);
	/* delete from list */
	if (comm->next != NULL) {
		comm->next->prev = comm->prev;
	}
	if (comm->prev != NULL) {
		comm->prev->next = comm->next;
	}
	if (comm == list) {
		list = list->next;
	}
	/* close and free socket */
	Net_Close (comm->socket);
	comm->socket = NULL;
#ifdef DEBUG_COMM
	CommDisplay ();
#endif
}								/* CommFinish */

/*
 * get socket data for XBComm
 */
XBSocket *
CommSocket (XBComm * comm)
{
	assert (NULL != comm);
	return comm->socket;
}								/* CommSocket */

/*
 * find XBComm associated to file descriptor
 */
XBComm *
CommFind (int fd)
{
	XBComm *comm;
	int cmp;
	for (comm = list; comm != NULL; comm = comm->next) {
		cmp = Socket_Fd (comm->socket);
		if (cmp < 0) {
			fprintf (stderr, "undefined socket! type=%u\n", comm->type);
			CommDisplay ();
		}
		if (fd == cmp) {
			return comm;
		}
	}
	return NULL;
}								/* CommFind */

/*
 * end of file com_base.c
 */
