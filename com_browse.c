/*
 * file com_browse.c - base communication to browse for network games
 *
 * $Id: com_browse.c,v 1.10 2006/02/09 21:21:23 fzago Exp $
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
struct _xb_browse_data
{
	XBBrowseData *next;			/* browse data block */
	char *hostname;				/* where to send the datagram */
	unsigned short port;		/* which port */
	XBBool broadcast;			/* broadcast flag */
	XBDatagram *dgram;			/* datagram to send */
};

/*
 * create a browse data block to be queued
 */
static XBBrowseData *
CreateBrowseData (const char *hostname, unsigned port, XBBool broadcast,
				  const XBBrowseTeleAny * tele)
{
	XBBrowseData *ptr;
	size_t len;
	unsigned char data[MAX_DGRAM_SIZE];
	/* write telegram */
	assert (NULL != tele);
	len = BrowseTele_Write (tele, data);
	/* alloc data */
	ptr = calloc (1, sizeof (XBBrowseData));
	assert (NULL != ptr);
	/* set values */
	ptr->hostname = (hostname != NULL) ? DupString (hostname) : NULL;
	ptr->port = port;
	ptr->broadcast = broadcast;
	ptr->dgram = Net_CreateDatagram (data, len);
	/* that's all */
	return ptr;
}								/* CreateBrowseData */

/*
 * remove browse data, after being sent
 */
static void
DeleteBrowseData (XBBrowseData * ptr)
{
	assert (NULL != ptr);
	/* free the host name string */
	if (NULL != ptr->hostname) {
		free (ptr->hostname);
	}
	/* free the datagram */
	Net_DeleteDatagram (ptr->dgram);
	/* free the browse data itself */
	free (ptr);
}								/* DeleteBrowseData */

/*
 * send a single queued data block
 */
static XBCommResult
WriteBrowse (XBComm * comm)
{
	XBBrowseData *next;
	/* get communication */
	XBCommBrowse *bComm = (XBCommBrowse *) comm;
	assert (NULL != bComm);
	/* send data if any */
	if (NULL != bComm->sndFirst) {
		if (!Net_SendDatagramTo
			(bComm->sndFirst->dgram, comm->socket, bComm->sndFirst->hostname, bComm->sndFirst->port,
			 bComm->sndFirst->broadcast)) {
			Dbg_Browse ("failed to send datagram, closing the socket\n");
			return XCR_Error;
		}
		Dbg_Browse ("successfully sent datagram to %s:%u %s\n", bComm->sndFirst->hostname,
					bComm->sndFirst->port, bComm->sndFirst->broadcast ? "broadcast" : "");
		/* remove sent datagram from queue */
		next = bComm->sndFirst->next;
		DeleteBrowseData (bComm->sndFirst);
		bComm->sndFirst = next;
		if (NULL == bComm->sndFirst) {
			bComm->sndLast = NULL;
		}
	}
	/* if queue empty, unregister for writing */
	if (NULL == bComm->sndFirst) {
		Dbg_Browse ("write queue emptied!\n");
		Socket_UnregisterWrite (CommSocket (comm));
		assert (NULL != bComm->eventFunc);
		return (*bComm->eventFunc) (bComm, XBBE_Wait) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* WriteBrowse */

/*
 * browse socket has new data
 */
static XBCommResult
ReadBrowse (XBComm * comm)
{
	XBDatagram *rcv;
	const char *host;
	unsigned short port;
	const void *data;
	size_t len;
	XBBrowseTele tele;

	/* get communication */
	XBCommBrowse *bComm = (XBCommBrowse *) comm;
	assert (NULL != bComm);
	/* try to get datagram and sender */
	rcv = Net_ReceiveDatagramFrom (bComm->comm.socket, &host, &port);
	if (NULL == rcv) {
		Dbg_Browse ("failed to strip datagram layer, ignoring datagram\n");
		assert (NULL != bComm->eventFunc);
		return (*bComm->eventFunc) (bComm, XBBE_Dgram) ? XCR_Error : XCR_OK;
		return XCR_OK;
	}
	/* get actual data from datagram */
	data = Net_DgramData (rcv, &len);
	if (NULL == data) {
		Dbg_Browse ("no browse data, ignoring datagram\n");
		assert (NULL != bComm->eventFunc);
		return (*bComm->eventFunc) (bComm, XBBE_Browse) ? XCR_Error : XCR_OK;
		return XCR_OK;
	}
	/* try to strip browse layer */
	if (XBBT_None != BrowseTele_Parse (&tele, data, len)) {
		Net_DeleteDatagram (rcv);
		Dbg_Browse ("data extracted successfully, passing to parser!\n");
		assert (NULL != bComm->receiveFunc);
		(*bComm->receiveFunc) (bComm, &tele, host, port);
	}
	else {
		Net_DeleteDatagram (rcv);
		Dbg_Browse ("no browse layer or unknown browse data, ignoring datagram\n");
		assert (NULL != bComm->eventFunc);
		return (*bComm->eventFunc) (bComm, XBBE_Browse) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* ReadBrowse */

/*
 * intialize data structure
 */
XBComm *
Browse_CommInit (XBCommBrowse * bComm, XBCommType commType, XBSocket * pSocket,
				 BrowseReceiveFunc receiveFunc, BrowseEventFunc eventFunc, XBCommFunc deleteFunc)
{
	assert (NULL != bComm);
	assert (NULL != receiveFunc);
	assert (NULL != eventFunc);
	/* set values */
	CommInit (&bComm->comm, commType, pSocket, ReadBrowse, WriteBrowse, deleteFunc);
	bComm->sndFirst = NULL;
	bComm->receiveFunc = receiveFunc;
	bComm->eventFunc = eventFunc;
	Dbg_Browse ("created comm instance\n");
	/* that's all */
	return &bComm->comm;
}								/* Browse_CommInit */

/*
 * queue a browse datagram
 */
void
Browse_Send (XBCommBrowse * bComm, const char *hostname, unsigned port, XBBool broadcast,
			 const XBBrowseTeleAny * tele)
{
	XBBrowseData *ptr;
	assert (NULL != bComm);
	/* append data to queue */
	ptr = CreateBrowseData (hostname, port, broadcast, tele);
	if (NULL != bComm->sndLast) {
		bComm->sndLast->next = ptr;
	}
	else {
		bComm->sndFirst = ptr;
	}
	bComm->sndLast = ptr;
	Socket_RegisterWrite (CommSocket (&bComm->comm));
	Dbg_Browse ("queued browse signal\n");
	assert (NULL != bComm->eventFunc);
	(void)(*bComm->eventFunc) (bComm, XBBE_Write);
}								/* Browse_Send */

/*
 * remove a browse communication
 */
void
Browse_Finish (XBCommBrowse * bComm)
{
	XBBrowseData *next;
	/* clean up send queue */
	while (NULL != bComm->sndFirst) {
		next = bComm->sndFirst->next;
		DeleteBrowseData (bComm->sndFirst);
		bComm->sndFirst = next;
	}
	/* clean up base communication */
	CommFinish (&bComm->comm);
	Dbg_Browse ("finishing comm instance\n");
	/* trigger close event */
	assert (NULL != bComm->eventFunc);
	(void)(*bComm->eventFunc) (bComm, XBBE_Close);
}								/* Browse_Finish */

/*
 * end of file com_browse.c
 */
