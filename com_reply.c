/*
 * file com_reply.c - server answers to broadcasts by clients
 *
 * $Id: com_reply.c,v 1.8 2006/02/09 21:21:23 fzago Exp $
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
	XBCommBrowse browse;
	unsigned short port;		/* server port */
	char *game;					/* game name */
	int numLives;				/* number of lives */
	int numWins;				/* number of wins */
	int frameRate;				/* frame rate */
} XBCommReply;

/*
 * received a query from a client
 */
static void
HandleQuery (XBCommReply * rComm, const XBBrowseTeleQuery * query, const char *host,
			 unsigned short port)
{
	XBBrowseTeleReply tele;
	memset (&tele, 0, sizeof (tele));
	/* build reply */
	tele.any.type = XBBT_Reply;
	tele.any.serial = query->any.serial;
	tele.port = rComm->port;
	tele.version[0] = VERSION_MAJOR;
	tele.version[1] = VERSION_MINOR;
	tele.version[2] = VERSION_PATCH;
	tele.numLives = rComm->numLives;
	tele.numWins = rComm->numWins;
	tele.frameRate = rComm->frameRate;
	strncpy (tele.game, rComm->game, sizeof (tele.game));
	strncpy (tele.host, "", sizeof (tele.host));
	/* queue data */
	Browse_Send (&rComm->browse, host, port, XBFalse, &tele.any);
	/* --- */
	Dbg_Reply ("query from %s:%hu, queued reply\n", host, port);
}								/* HandleQuery */

/*
 * handle received data
 */
static void
ReceiveReply (XBCommBrowse * bComm, const XBBrowseTele * tele, const char *host,
			  unsigned short port)
{
	assert (NULL != bComm);
	assert (NULL != tele);
	assert (NULL != host);
	switch (tele->type) {
	case XBBT_Query:
		HandleQuery ((XBCommReply *) bComm, &tele->query, host, port);
		break;
	default:
		Dbg_Reply ("ignoring invalid query from %s:%u\n", host, port);
		break;
	}
}								/* ReceiveReply */

/*
 * handle browse events
 */
static XBBool
EventReply (XBCommBrowse * bComm, XBBrowseEvent ev)
{
	/* XBCommReply *rComm = (XBCommReply *) bComm; */
	switch (ev) {
	case XBBE_Wait:
		Dbg_Reply ("all data sent\n");
		break;
	case XBBE_Dgram:
		Dbg_Reply ("received invalid datagram, ignoring\n");
		break;
	case XBBE_Browse:
		Dbg_Reply ("received invalid browse, ignoring\n");
		break;
	case XBBE_Write:
		Dbg_Reply ("new data waits to be sent\n");
		break;
	case XBBE_Close:
		Dbg_Reply ("browse shutdown complete\n");
		break;
	default:
		Dbg_Newgame ("unknown browse event, ignoring!\n");
	}
	return XBFalse;
}								/* EventReply */

/*
 * handle delete
 */
static XBCommResult
DeleteReply (XBComm * comm)
{
	XBCommReply *rComm = (XBCommReply *) comm;
	assert (NULL != rComm);
	Browse_Finish (&rComm->browse);
	assert (NULL != rComm->game);
	free (rComm->game);
	free (rComm);
	Dbg_Reply ("comm instance removed\n");
	return XCR_OK;
}								/* DeleteReply  */

/*
 * create udp socket waiting for clients' queries
 */
XBComm *
Reply_CreateComm (unsigned short port, const CFGGameHost * cfg, const CFGGameSetup * setup)
{
	XBSocket *pSocket;
	XBCommReply *rComm;

	assert (NULL != cfg);
	assert (NULL != setup);
	/* create socket */
	pSocket = Net_BindUdp (NULL, port);
	if (NULL == pSocket) {
		Dbg_Reply ("failed to create listening udp socket!\n");
		return NULL;
	}
	Dbg_Reply ("listening udp socket created\n");
	/* create communication data structure */
	rComm = calloc (1, sizeof (*rComm));
	assert (NULL != rComm);
	/* set values */
	Browse_CommInit (&rComm->browse, COMM_Reply, pSocket, ReceiveReply, EventReply, DeleteReply);
	rComm->port = cfg->port;
	rComm->game = DupString (cfg->game);
	rComm->numLives = setup->numLives;
	rComm->numWins = setup->numWins;
	rComm->frameRate = setup->frameRate;
	/* that's all ? */
	return &rComm->browse.comm;
}								/* Reply_CreateComm */

/*
 * end of file com_reply.c
 */
