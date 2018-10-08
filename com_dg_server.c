/*
 * file com_dg_server.c - send ingame datagrams to server
 *
 * $Id: com_dg_server.c,v 1.16 2006/02/18 21:40:02 fzago Exp $
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
 * locals typedefs
 */
typedef struct
{
	XBCommDgram dgram;
} XBCommDgramServer;

/***********
 * handler *
 ***********/

/*
 * client received ping or times from server
 */
static void
ReceivePing (XBCommDgram * dComm, unsigned clientID, unsigned short pingTime)
{
	if (0 == clientID) {
		/* server wants a reply */
		Dbg_D2S ("server requests ping, queueing ping\n");
		Dgram_SendPing (dComm);
	}
	else {
		/* we have received the ping time of a client */
		if (pingTime < 0xFFFE) {
			Client_ReceivePingTime (clientID, (int)((unsigned)pingTime));
			Dbg_D2S ("server sends pingtime for client %u (%u)\n", clientID, pingTime);
		}
		else {
			Client_ReceivePingTime (clientID, -1);
		}
	}
	dComm->connected = XBTrue;
}								/* ReceivePing */

/*
 * client encountered some event while parsing
 * return XBTrue will trigger delete handler
 */
static XBBool
ReceiveInfo (XBCommDgram * dgram, XBDgramInfo info)
{
	assert (dgram != NULL);
	switch (info) {
	case XBDI_CONFAIL:
		Dbg_D2S ("failed to connect to server!\n");
		return (Client_DgramEvent (XBCC_ConnFailed));
	case XBDI_CONSUCC:
		Dbg_D2S ("udp connection established to server!\n");
		return (Client_DgramEvent (XBCC_ConnFailed));
	case XBDI_LOSS:
		Dbg_D2S ("data loss from server!\n");
		return (Client_DgramEvent (XBCC_Loss));
	case XBDI_FINISH:
		Dbg_D2S ("receive FINISH from server!\n");
		Client_ReceiveFinish ();
		return XBFalse;
	case XBDI_IGNORE:
		Dbg_D2S ("gametime %lu from server ignored\n", (unsigned long)dgram->ignore);
		return XBFalse;
	case XBDI_PARSED:
		Dbg_D2S ("Frames parsed [%lu,%lu]\n", (unsigned long)dgram->rcvfirst, (unsigned long)dgram->rcvnext - 1);
		dgram->queue = dgram->rcvfirst;
		return XBFalse;
	case XBDI_WRITEERR:
		Dbg_D2S ("write error to server!\n");
		return (Client_DgramEvent (XBCC_WriteError));
	case XBDI_CLOSE:
		Dbg_D2C ("udp to server removed\n");
		return (Client_DgramEvent (XBCC_DgramClosed));
	default:
		Dbg_D2S ("unrecognized dgram info!\n");
		return XBFalse;
	}
}								/* ReceiveInfo */

/*
 * client received player actions from server
 */
static void
ReceivePlayerAction (XBCommDgram * dComm, int gameTime, const PlayerAction * playerAction)
{
	Dbg_D2S ("actions for gt=%u received from server\n", gameTime);
	Client_ReceivePlayerAction (gameTime, playerAction);
}								/* ReceivePlayerAction */

/***************
 * constructor *
 ***************/

/*
 * create datagram connection server
 */
XBComm *
D2S_CreateComm (const char *localname, const char *hostname, unsigned short port)
{
	XBCommDgramServer *dComm;
	XBSocket *pSocket;

	/* create socket */
	pSocket = Net_BindUdp (localname, 0);
	if (NULL == pSocket) {
		Dbg_D2S ("failed to create udp to server\n");
		return NULL;
	}
	Dbg_D2S ("created udp to server\n");
	/* connect it */
	if (!Net_ConnectUdp (pSocket, hostname, port)) {
		Net_Close (pSocket);
		Dbg_D2S ("failed to connect udp to server %s:%u\n", hostname, port);
		return NULL;
	}
	Dbg_D2S ("connected to server %s:%u\n", hostname, port);
	/* create communication data structure */
	dComm = calloc (1, sizeof (*dComm));
	assert (NULL != dComm);
	/* set values */
	Dgram_CommInit (&dComm->dgram, COMM_DgServer, pSocket, ReceivePing, ReceiveInfo,
					ReceivePlayerAction);
	/* that's all */
	return &dComm->dgram.comm;
}								/* D2C_CreateComm */

/******************
 * get local data *
 ******************/

/*
 * get port for client
 */
unsigned short
D2S_Port (const XBComm * comm)
{
	return Dgram_Port ((const XBCommDgram *) comm);
}								/* D2C_Port */

/******************
 * set local data *
 ******************/

/*
 * reset datagram connection
 */
void
D2S_Reset (XBComm * comm)
{
	Dbg_D2S ("resetting frames for server\n");
	Dgram_Reset ((XBCommDgram *) comm);
}								/* D2S_Reset */

/*
 * set mask bytes
 */
void
D2S_SetMaskBytes (XBComm * comm, unsigned num)
{
	Dbg_D2S ("setting mask bytes to %u\n", num);
	Dgram_SetMaskBytes ((XBCommDgram *) comm, num);
}								/* D2S_SetMaskBytes */

/*
 * check for datagram timeout
 */
XBBool
D2S_Timeout (const XBComm * comm, const struct timeval *tv)
{
	const XBCommDgram *dgram = (const XBCommDgram *) comm;
	assert (NULL != dgram);
	assert (NULL != tv);
	return (0 != dgram->lastSnd.tv_sec && tv->tv_sec - dgram->lastSnd.tv_sec > LINK_LOST);
}								/* D2S_Timeout */

/*
 * check if server is already connected (i.e. has send a datagram)
 */
XBBool
D2S_Connected (const XBComm * comm)
{
	const XBCommDgram *dgram = (const XBCommDgram *) comm;
	assert (NULL != dgram);
	return dgram->connected;
}								/* D2S_Connected */

/**************
 * queue data *
 **************/

/*
 * send connect datagram to server
 */
void
D2S_SendConnect (XBComm * comm)
{
	if (comm != NULL) {
		Dgram_SendPing ((XBCommDgram *) comm);
	}
}								/* D2S_SendConnect */

/*
 * send player action to client
 */
void
D2S_SendPlayerAction (XBComm * comm, int gameTime, const PlayerAction * playerAction)
{
	if (comm != NULL) {
		Dbg_D2S ("queueing action for gt=%u to server\n", gameTime);
		Dgram_SendPlayerAction ((XBCommDgram *) comm, gameTime, playerAction);
	}
}								/* D2C_SendPlayerAction */

/*
 * acknowledge level finish
 */
void
D2S_SendFinish (XBComm * comm, int gameTime)
{
	if (comm != NULL) {
		Dbg_D2S ("queueing FINISH for server\n");
		Dgram_SendFinish ((XBCommDgram *) comm, gameTime);
	}
}								/* D2S_SendFinish */

/*
 * flush data - resend last send unless acknowledged by remote
 */
XBBool
D2S_Flush (XBComm * comm)
{
	XBCommDgram *dgram = (XBCommDgram *) comm;
	assert (NULL != dgram);
	/* for client, last send acknowledged if last receive larger than last sent */
	if (dgram->rcvnext > dgram->sndnext) {
		return XBFalse;
	}
	Dbg_D2S ("resending frames to server\n");
	return Dgram_Flush (dgram);
}								/* D2S_Flush */

/*
 * end of file com_dg_server.c
 */
