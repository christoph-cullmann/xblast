/*
 * file com_dg_client.c - send datagrams to client
 *
 * $Id: com_dg_client.c,v 1.17 2006/02/18 21:40:02 fzago Exp $
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
 * local macros
 */
#define CLIENT_UDP_PORT(id) (16168+(id))

/*
 * local types
 */
typedef struct
{
	XBCommDgram dgram;
	unsigned id;
	long ping;
} XBCommDgramClient;

/*
 * local variables
 */
static XBCommDgramClient *commList[MAX_HOSTS] = {
	/* this entry is never used (server) */
	NULL,
	/* up to 5 clients can connect */
	NULL, NULL, NULL, NULL, NULL,
};
static int commCount = 0;

/*******************
 * local functions *
 *******************/

/*
 * calculate difference for two timevals
 */
static void
DiffTimeVal (struct timeval *diff, const struct timeval *a, const struct timeval *b)
{
	assert (NULL != diff);
	assert (NULL != a);
	assert (NULL != b);

	diff->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_usec < b->tv_usec) {
		diff->tv_usec = 1000000 + a->tv_usec - b->tv_usec;
		diff->tv_sec--;
	}
	else {
		diff->tv_usec = a->tv_usec - b->tv_usec;
	}
}								/* DiffTimeVal */

/********************
 * polling function *
 ********************/

/*
 * polling for datagram connections
 */
static void
PollDatagram (const struct timeval *tv)
{
	int i, j;
	struct timeval dtSnd;
	struct timeval dtRcv;
	XBBool initPingTime = XBFalse;
	int pingTime[MAX_HOSTS];

	assert (NULL != tv);
	for (i = 1; i < MAX_HOSTS; i++) {
		if (NULL != commList[i] && commList[i]->dgram.connected) {
			/* when was last datagram send */
			DiffTimeVal (&dtSnd, tv, &commList[i]->dgram.lastSnd);
			DiffTimeVal (&dtRcv, tv, &commList[i]->dgram.lastRcv);
			/* send a ping to client, when no datagram was send for over 500ms */
			if (dtSnd.tv_sec >= 1 || dtSnd.tv_usec > 500000) {
				if (!initPingTime) {
					pingTime[0] = -1;
					for (j = 1; j < MAX_HOSTS; j++) {
						pingTime[j] = commList[j] ? commList[j]->ping : -1;
					}
					initPingTime = XBTrue;
				}
				Dgram_SendPingData (&commList[i]->dgram, pingTime);
			}
			/* check last chance to send datagram */
			if (0 != commList[i]->dgram.lastRcv.tv_sec && dtRcv.tv_sec > LINK_LOST) {
				/* inform application */
				Dbg_D2C ("client %u timed out\n", i);
				Server_DgramEvent (i, XBSC_Timeout);
			}
		}
	}
}								/* PollDatagram */

/************
 * handlers *
 ************/

/*
 * server received ping or times from client (times are ignored)
 */
static void
ReceivePing (XBCommDgram * dgram, unsigned clientID, unsigned short pingTime)
{
	struct timeval tvPing;
	XBCommDgramClient *dComm = (XBCommDgramClient *) dgram;

	/* server reacts to empty pings only */
	if (0 == clientID) {
		assert (dComm != NULL);
		DiffTimeVal (&tvPing, &dgram->lastRcv, &dgram->lastSnd);
		dComm->ping = 1000L * tvPing.tv_sec + (tvPing.tv_usec) / 1000L;
		Dbg_D2C ("ping from client %u = %lu ms\n", dComm->id, dComm->ping);
		/* inform application */
		Server_ReceivePing (dComm->id, dComm->ping);
	}
}								/* ReceivePing */

/*
 * server encountered some event while parsing
 * return XBTrue will trigger delete handler
 */
static XBBool
ReceiveInfo (XBCommDgram * dgram, XBDgramInfo info)
{
	XBCommDgramClient *dComm = (XBCommDgramClient *) dgram;
	assert (dComm != NULL);
	switch (info) {
	case XBDI_LOSS:
		Dbg_D2C ("future frames from client %u\n", dComm->id);
		return (Server_DgramEvent (dComm->id, XBSC_GameTime));
	case XBDI_PARSED:
		Dbg_D2C ("Frames parsed [%lu,%lu]\n", (unsigned long)dgram->rcvfirst, (unsigned long)dgram->rcvnext);
		dgram->queue = dgram->rcvnext;
		return XBFalse;
	case XBDI_IGNORE:
		Dbg_D2C ("gametime %lu from client %u ignored\n", (unsigned long)dgram->ignore, dComm->id);
		return XBFalse;
	case XBDI_FINISH:
		Dbg_D2C ("Receiving FINISH from client %u\n", dComm->id);
		Server_ReceiveFinish (dComm->id);
		return XBFalse;
	case XBDI_CONSUCC:
		Dbg_D2C ("udp connection established to client %u\n", dComm->id);
		return XBFalse;
	case XBDI_CONFAIL:
		Dbg_D2C ("failed to connect to client %u\n", dComm->id);
		return (Server_DgramEvent (dComm->id, XBSC_ConnFailed));
	case XBDI_WRITEERR:
		Dbg_D2C ("write error to client %u\n", dComm->id);
		return (Server_DgramEvent (dComm->id, XBSC_WriteError));
	case XBDI_CLOSE:
		Dbg_D2C ("udp to client %u removed\n", dComm->id);
		D2C_Clear (dComm->id);
		return (Server_DgramEvent (dComm->id, XBSC_DgramClosed));
	default:
		Dbg_D2C ("ignoring unknown dgram event (%u)\n", info);
		return XBFalse;
	}
}								/* ReceiveInfo */

/*
 * server received action from client
 */
static void
ReceivePlayerAction (XBCommDgram * dgram, int gameTime, const PlayerAction * playerAction)
{
	XBCommDgramClient *dComm = (XBCommDgramClient *) dgram;
	assert (dComm != NULL);
	Dbg_D2C ("Receiving action from client %u at gt=%u\n", dComm->id, gameTime);
	Server_ReceivePlayerAction (dComm->id, gameTime, playerAction);
}								/* ReceivePlayerAction */

/***************
 * constructor *
 ***************/

/*
 * create datagram connection to a game client
 */
XBComm *
D2C_CreateComm (unsigned id, const char *localname, XBBool fixedPort)
{
	XBSocket *pSocket;
	/* sanity checks */
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] == NULL);
	/* create socket */
	pSocket = Net_BindUdp (localname, fixedPort ? CLIENT_UDP_PORT (id) : 0);
	if (NULL == pSocket) {
		Dbg_D2C ("failed to create socket to client %u\n", id);
		return NULL;
	}
	Dbg_D2C ("created socket to client %u on port %u\n", id, fixedPort ? CLIENT_UDP_PORT (id) : 0);
	/* create communication data structure */
	commList[id] = calloc (1, sizeof (*commList[id]));
	assert (NULL != commList[id]);
	/* set values */
	Dgram_CommInit (&commList[id]->dgram, COMM_DgClient, pSocket, ReceivePing, ReceiveInfo,
					ReceivePlayerAction);
	commList[id]->id = id;
	/* setup polling */
	if (0 == commCount) {
		GUI_AddPollFunction (PollDatagram);
	}
	commCount++;
	/* that's all */
	Dbg_D2C ("established handlers for client %u (poll #%u)\n", id, commCount);
	return &commList[id]->dgram.comm;
}								/* D2C_CreateComm */

/**********************
 * connect/disconnect *
 **********************/

/*
 * connect to game client port
 */
XBBool
D2C_Connect (unsigned id, const char *host, unsigned short port)
{
	/* sanity checks */
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->dgram.comm.socket != NULL);
	/* connect socket */
	if (port != 0) {
		/* connect to client address and port */
		commList[id]->dgram.connected =
			Net_ConnectUdp (commList[id]->dgram.comm.socket, host, port);
		Dbg_D2C ("connecting to client %u (%s:%u) : %s\n", id, host, port,
				 commList[id]->dgram.connected ? "ok" : "failed");
		return commList[id]->dgram.connected;
	}
	else {
		/* client uses NAT we wait for his first datagram */
		Dbg_D2C ("(NAT) setting expected host for client %u to %s\n", id, host);
		commList[id]->dgram.host = host;
		return XBTrue;
	}
}								/* D2C_Connect */

/*
 * clear connection data
 */
void
D2C_Clear (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	commList[id] = NULL;
	/* disable polling */
	commCount--;
	if (0 == commCount) {
		GUI_SubtractPollFunction (PollDatagram);
	}
	Dbg_D2C ("Cleared connection to client %u\n", id);
}								/* ClearConnection */

/*
 * disconnect given client
 */
void
D2C_Disconnect (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	/* we only need to shutdown the socket */
	Dbg_D2C ("disconnecting client %u\n", id);
	CommDelete (&commList[id]->dgram.comm);
}								/* D2C_Disconnect */

/******************
 * get local data *
 ******************/

/*
 * get port for game client
 */
unsigned short
D2C_Port (unsigned id)
{
	return Dgram_Port (&commList[id]->dgram);
}								/* D2C_Port */

/*
 * is client connected ?
 */
XBBool
D2C_Connected (unsigned id)
{
	assert (id < MAX_HOSTS);
	return (commList[id] != NULL);
}								/* D2C_Connected */

/*
 * last ping time
 */
long
D2C_LastPing (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (NULL != commList[id]);
	return commList[id]->ping;
}								/* D2C_Connected */

/******************
 * set local data *
 ******************/

/*
 * reset communication after level start
 */
void
D2C_Reset (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	Dbg_D2C ("resetting frames for client %u\n", id);
	Dgram_Reset (&commList[id]->dgram);
}								/* D2C_Reset */

/*
 * set mask bytes for all client connections
 */
void
D2C_SetMaskBytes (unsigned num)
{
	unsigned id;
	assert (num < MAX_MASK_BYTES);
	for (id = 1; id < MAX_HOSTS; id++) {
		if (D2C_Connected (id)) {
			Dgram_SetMaskBytes (&commList[id]->dgram, num);
		}
	}
	Dbg_D2C ("setting mask bytes to %u\n", num);
}								/* D2C_SetMaskBytes */

/**************
 * queue data *
 **************/

/*
 * send player action to game client
 */
void
D2C_SendPlayerAction (unsigned id, int gameTime, const PlayerAction * playerAction)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Dbg_D2C ("queueing actions for client %u at gt=%u\n", id, gameTime);
	Dgram_SendPlayerAction (&commList[id]->dgram, gameTime, playerAction);
}								/* D2C_SendPlayerAction */

/*
 * send finish level to game client
 */
void
D2C_SendFinish (unsigned id, int gameTime)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Dbg_D2C ("queueing FINISH to client %u at gt=%u\n", id, gameTime);
	Dgram_SendFinish (&commList[id]->dgram, gameTime);
}								/* D2C_SendFinish */

/*
 * flush remaining datagrams
 */
XBBool
D2C_Flush (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	/* for server, last send acknowledged if last receive equals last sent */
	if (commList[id]->dgram.rcvnext == commList[id]->dgram.sndnext) {
		return XBFalse;
	}
	Dbg_D2C ("resending frames to client %u\n", id);
	return Dgram_Flush (&commList[id]->dgram);
}								/* D2C_Flush */

/*
 * end of file com_dg_client.c
 */
