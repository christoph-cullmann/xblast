/*
 * file com_newGame.c - client newGamees for local network game
 *
 * $Id: com_newgame.c,v 1.20 2006/02/24 21:29:16 fzago Exp $
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

#include "xblast.h"

/*
 * local types
 */
typedef struct
{
	XBCommBrowse browse;
	char *addrBroadcast;		/* ip of receiver */
	unsigned short port;		/* port of receiver */
	unsigned char serial;		/* serial */
	unsigned short gameport;	/* server port */
	int gameID;					/* game id at central, initially -1 */
	char *game;					/* game name */
	int numLives;				/* number of lives */
	int numWins;				/* numberof wins */
	int frameRate;				/* frame rate, =0 for running games */
	struct timeval tvSend;		/* time of last queueing */
	XBBool shutdown;			/* flag for shutdown when queue emptied */
	unsigned int replies;		/* counter for replies */
} XBCommNewGame;

/*
 * central sent Reply
 */
static void
HandleReply (XBCommNewGame * qComm, const XBBrowseTeleNewGameOK * tele, const char *host)
{
	long msec;
	struct timeval tv;
	/* calculate sping time */
	gettimeofday (&tv, NULL);
	msec =
		(tv.tv_sec - qComm->tvSend.tv_sec) * 1000L + (tv.tv_usec - qComm->tvSend.tv_usec) / 1000L;
	/* update received id and reply counter */
	qComm->gameID = tele->gameID;
	qComm->replies++;
	Dbg_Out ("NEWGAME: receive id=%i from %s (%ld msec, cnt=%u)\n", tele->gameID, host, msec,
			 qComm->replies);
}								/* HandleReply */

/*
 * handle received data
 */
static void
ReceiveNewGame (XBCommBrowse * bComm, const XBBrowseTele * tele, const char *host,
				unsigned short port)
{
	assert (NULL != bComm);
	assert (NULL != tele);
	assert (NULL != host);
	switch (tele->type) {
	case XBBT_NewGameOK:
		HandleReply ((XBCommNewGame *) bComm, &tele->newGameOK, host);
		break;
	default:
		Dbg_Newgame ("ignoring invalid response\n");
		break;
	}
}								/* ReceiveNewGame */

/*
 * handle event
 */
static XBBool
EventNewGame (XBCommBrowse * bComm, XBBrowseEvent ev)
{
	XBCommNewGame *qComm = (XBCommNewGame *) bComm;
	switch (ev) {
	case XBBE_Wait:
		Dbg_Newgame ("all data sent, %s\n", qComm->shutdown ? "shutting down" : "continuing");
		return qComm->shutdown;
	case XBBE_Dgram:
		Dbg_Newgame ("received invalid datagram, ignoring\n");
		break;
	case XBBE_Browse:
		Dbg_Newgame ("received invalid browse, ignoring\n");
		break;
	case XBBE_Write:
		Dbg_Newgame ("new data waits to be sent\n");
		break;
	case XBBE_Close:
		Dbg_Newgame ("browse shutdown complete\n");
		break;
	default:
		Dbg_Newgame ("unknown browse event, ignoring!\n");
	}
	return XBFalse;
}								/* EventNewGame */

/*
 * handle delete
 */
static XBCommResult
DeleteNewGame (XBComm * comm)
{
	XBCommNewGame *qComm = (XBCommNewGame *) comm;
	assert (NULL != qComm);
	Browse_Finish (&qComm->browse);
	assert (NULL != qComm->addrBroadcast);
	free (qComm->addrBroadcast);
	assert (NULL != qComm->game);
	free (qComm->game);
	free (qComm);
	Dbg_Newgame ("comm instance removed\n");
	/* tell server to null query socket */
	Server_ReceiveNewGameClose ();
	return XCR_OK;
}								/* DeleteNewGame  */

/*
 * create NewGame communication to central on given device
 */
XBComm *
NewGame_CreateComm (const char *addrDevice, unsigned short port, const char *addrBroadcast,
					const CFGGameHost * cfg, const CFGGameSetup * setup)
{
	XBSocket *pSocket;
	XBCommNewGame *cComm;
	assert (NULL != addrBroadcast);
	/* create socket and bind to given interface */
	pSocket = Net_BindUdp (addrDevice, 0);
	if (NULL == pSocket) {
		Dbg_Newgame ("failed to open udp socket on device %s\n",
					 (addrDevice == NULL) ? "(auto)" : addrDevice);
		return NULL;
	}
#ifndef sun
	/* some sun's don't like connected udp sockets and sendto */
	if (!Net_ConnectUdp (pSocket, addrBroadcast, port)) {
		Socket_Close (pSocket);
		Dbg_Newgame ("failed to connect UDP socket to central %s:%u\n", addrBroadcast, port);
		return NULL;
	}
#endif
	Dbg_Newgame ("created udp socket on %s:%u to central %s:%u\n",
				 Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse),
				 Socket_HostName (pSocket, XBTrue), Socket_HostPort (pSocket, XBTrue));
	/* create communication data structure */
	cComm = calloc (1, sizeof (*cComm));
	assert (NULL != cComm);
	/* set values */
	Browse_CommInit (&cComm->browse, COMM_NewGame, pSocket, ReceiveNewGame, EventNewGame,
					 DeleteNewGame);
	cComm->port = port;
	cComm->serial = 0;
	cComm->gameID = -1;
	cComm->addrBroadcast = DupString (addrBroadcast);
	cComm->tvSend.tv_sec = 0;
	cComm->tvSend.tv_usec = 0;
	cComm->gameport = cfg->port;
	cComm->game = DupString (cfg->game);
	cComm->numLives = setup->numLives;
	cComm->numWins = setup->numWins;
	cComm->frameRate = setup->frameRate;
	cComm->shutdown = XBFalse;
	cComm->replies = 0;
	/* that's all ? */
	return &cComm->browse.comm;
}								/* NewGame_CreateComm */

/*
 * prepend waiting/scores to game name
 */
static void
PrependData (int num, const char *score, char *name)
{
	char tempString[48];
	int len;
	/* create string to prepend */
	if (num > 0) {
		/* server is waiting */
		sprintf (tempString, "%d:", num);
	}
	else {
		/* game is busy */
		if (strlen (score) < 48) {
			strcpy (tempString, score);
		}
	}
	/* append base name, max 48 bytes */
	len = strlen (tempString);
	strncpy (tempString + len, name, 48 - len);
	/* ensure that new string is null-terminated */
	tempString[47] = (char)'\0';
	/* set as new basename */
	strncpy (name, tempString, 48);
}								/* PrependData */

/*
 * queue current game data to central
 */
void
NewGame_Send (XBComm * comm, const struct timeval *tv, int num, const char *score)
{
	XBBrowseTeleNewGame tele;
	XBCommNewGame *qComm = (XBCommNewGame *) comm;
	assert (NULL != qComm);
	assert (NULL != tv);
	/* create browse telegram */
	tele.any.type = XBBT_NewGame;
	tele.any.serial = ++qComm->serial;
	tele.gameID = qComm->gameID;
	tele.port = qComm->gameport;
	tele.version[0] = VERSION_MAJOR;
	tele.version[1] = VERSION_MINOR;
	tele.version[2] = VERSION_PATCH;
	tele.numLives = qComm->numLives;
	tele.numWins = qComm->numWins;
	tele.frameRate = (num > 0) ? qComm->frameRate : 0;
	strncpy (tele.game, qComm->game, sizeof (tele.game));
	PrependData (num, score, tele.game);
	/* queue data */
	Browse_Send (&qComm->browse, qComm->addrBroadcast, qComm->port, XBFalse, &tele.any);
	/* mark time */
	qComm->tvSend = *tv;
	Dbg_Newgame ("queued new game data #%u to central %s:%hu\n", qComm->serial,
				 qComm->addrBroadcast, qComm->port);
}								/* NewGame_Send */

/*
 * queue close request to central
 */
void
NewGame_Close (XBComm * comm, const struct timeval *tv)
{
	XBBrowseTeleNewGameOK tele;
	XBCommNewGame *qComm = (XBCommNewGame *) comm;
	assert (NULL != qComm);
	assert (NULL != tv);
	/* create browse telegram */
	tele.any.type = XBBT_NewGameOK;
	tele.any.serial = ++qComm->serial;
	tele.gameID = qComm->gameID;
	/* queue data */
	Browse_Send (&qComm->browse, qComm->addrBroadcast, qComm->port, XBFalse, &tele.any);
	/* mark time */
	qComm->tvSend = *tv;
	/* mark for shutdown */
	qComm->shutdown = XBTrue;
	Dbg_Newgame ("queued close #%u to central at %s:%hu, marking for shutdown\n", qComm->serial,
				 qComm->addrBroadcast, qComm->port);
}								/* NewGame_Poll */

/*
 * end of file com_newGame.c
 */
