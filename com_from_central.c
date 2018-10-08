/*
 * file com_from_central.c - handle communications with clients
 *
 * $Id: com_from_central.c,v 1.9 2006/02/09 21:21:23 fzago Exp $
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
	XBCommStream stream;
	unsigned serial;
	unsigned cnt;
} XBCommFromCentral;

/*
 * local variables
 */
static XBCommFromCentral *commList[MAX_HOSTS] = {
	/* this entry is never used (server) */
	NULL,
	/* up to 5 clients can connect */
	NULL, NULL, NULL, NULL, NULL,
};

/* total user count */
static unsigned usercount = 0;

/*
 * handle registration request
 */
static void
HandleRegistration (unsigned id, const char *data)
{
	int pid;
	pid = Central_ReceivePlayerConfig (id, data);
	switch (pid) {
	case XBPV_Incomplete:
		break;
	default:
		C2X_QueueUserPID (id, pid);
	}
}								/* HandleRegistration */

/*
 * handle game stat
 */
static void
HandleGameStat (unsigned id, const char *data)
{
	switch (Central_ReceiveGameStat (data)) {
	case XBGS_Error:
		Dbg_C2X ("invalid game stat\n");
		C2X_QueueDisconnect (id);
		break;
	case XBGS_Complete:
		Dbg_C2X ("game stat complete\n");
		C2X_QueueDisconnect (id);
		break;
	case XBGS_Level:
		Dbg_C2X ("level stat applied\n");
		break;
	default:
		break;
	}
}								/* HandleGameStat */

/*
 * user requests data
 */
static XBCommResult
HandleRequestData (XBCommFromCentral * fromCentral, const XBTelegram * tele)
{
	switch (Net_TeleID (tele)) {
	case XBT_ID_PlayerConfig:	/* user requests rankings */
		Dbg_C2X ("User %i requests player rankings\n", fromCentral->serial);
		C2X_QueuePlayerRankings (fromCentral->serial);
		break;
	default:
		Dbg_C2X ("User %i sent unknown request\n", fromCentral->serial);
		break;
	}
	return XCR_OK;
}								/* HandleRequestData */

/*
 * user sends data
 */
static XBCommResult
HandleSendData (XBCommFromCentral * fromCentral, const XBTelegram * tele)
{
	const char *data;
	size_t len;
	XBTeleIOB iob;
	/* get pointer to data */
	data = Net_TeleData (tele, &len);
	/* get iob */
	iob = Net_TeleIOB (tele);
	switch (Net_TeleID (tele)) {
	case XBT_ID_PlayerConfig:	/* user sends player data to register */
		/* Dbg_C2X("user %u registers player\n", fromCentral->serial); */
		HandleRegistration (fromCentral->serial, data);
		break;
	case XBT_ID_GameStat:		/* user sends level/game result */
		Dbg_C2X ("user %u sends game stat\n", fromCentral->serial);
		HandleGameStat (fromCentral->serial, data);
		break;
	default:					/* user sends unknown data */
		Dbg_C2X ("user %u sends unknown data\n", fromCentral->serial);
		return XCR_OK;
	}
	return XCR_OK;
}								/* HandleSendData */

/*
 * user sends command
 */
static XBCommResult
HandleActivate (XBCommFromCentral * fromCentral, const XBTelegram * tele)
{
	const void *data;
	size_t len;
	/*   unsigned    value; */

	data = Net_TeleData (tele, &len);
	switch (Net_TeleID (tele)) {
	case XBT_ID_RequestDisconnect:	/* user has shutdown, expects eof  */
		Dbg_C2X ("user %u requests disconnect\n", fromCentral->serial);
		return XCR_Finished;
	default:					/* user sends other command */
		Dbg_C2X ("user %u sends unknown command\n", fromCentral->serial);
		break;
	}
	return XCR_OK;
}								/* HandleActivate */

/*
 * user sends spontaneous event
 */
static XBCommResult
HandleSpontaneous (XBCommFromCentral * fromCentral, const XBTelegram * tele)
{
	switch (Net_TeleID (tele)) {
	case XBT_ID_HostDisconnected:
		Dbg_C2X ("user %u starts disconnect announcement\n", fromCentral->serial);
		Central_ReceiveDisconnect (fromCentral->serial);
		return XCR_OK;
	default:
		Dbg_C2X ("user %u sends unknown event\n", fromCentral->serial);
		break;
	}
	return XCR_OK;
}								/* HandleSpontaneous */

/*
 * handle telegrams from users
 */
static XBCommResult
HandleTelegram (XBCommStream * stream, const XBTelegram * tele)
{
	XBCommFromCentral *fromCentral = (XBCommFromCentral *) stream;
	assert (fromCentral != NULL);
	switch (Net_TeleCOT (tele)) {
	case XBT_COT_RequestData:	/* user requests data from central */
		return HandleRequestData (fromCentral, tele);
	case XBT_COT_SendData:		/* user sends data to central */
		return HandleSendData (fromCentral, tele);
	case XBT_COT_Activate:		/* server activate command on client */
		return HandleActivate (fromCentral, tele);
	case XBT_COT_Spontaneous:	/* server send spontaneous status change */
		return HandleSpontaneous (fromCentral, tele);
	default:
		Dbg_C2X ("user %u sends unknown signal, disconnect\n", fromCentral->serial);
		return XCR_Error;
	}
}								/* HandleTelegram */

/*
 * XBComm delete handler
 */
static XBCommResult
DeleteFromCentral (XBComm * comm)
{
	XBCommFromCentral *fromcentral = (XBCommFromCentral *) comm;
	assert (comm != NULL);
	assert (fromcentral == commList[fromcentral->serial]);
	/* unmark client */
	commList[fromcentral->serial] = NULL;
	/* clean comm stream, triggers close event */
	Stream_CommFinish (&fromcentral->stream);
	return XCR_OK;
}								/* DeleteFromCentral */

/*
 * XBCommStream event handler
 */
static XBBool
EventFromCentral (XBCommStream * comm, const XBStreamEvent ev)
{
	XBCommFromCentral *fromcentral = (XBCommFromCentral *) comm;
	switch (ev) {
	case XBST_IOREAD:
		Dbg_C2X ("Read error to user %u, removing\n", fromcentral->serial);
		return XBTrue;
	case XBST_IOWRITE:
		Dbg_C2X ("Write error to user %u, removing\n", fromcentral->serial);
		return XBTrue;
	case XBST_EOF:
		Dbg_C2X ("Unexpected eof to user %u, removing\n", fromcentral->serial);
		return XBTrue;
	case XBST_WAIT:
		Dbg_C2X ("send queue to user %u emptied\n", fromcentral->serial);
		return XBFalse;
	case XBST_BUSY:
		/* Dbg_C2X("data remains to be sent to user %u\n", fromcentral->serial); */
		return XBFalse;
	case XBST_CLOSE:
		/* inform parent layer of close */
		Central_ConnectionClosed (fromcentral->serial, fromcentral->cnt);
		/* now XBComm memory can be safely freed */
		free (comm);
		return XBFalse;
	default:
		Dbg_C2X ("unknown stream event to user %u\n", fromcentral->serial);
		return XBFalse;
	}
}								/* EventFromCentral */

/*
 * accept a TCP connection from a listen socket
 */
XBComm *
C2X_CreateComm (const XBSocket * socket)
{
	unsigned serial;
	XBSocket *pSocket;
	XBCommFromCentral *fromcentral;
	assert (socket != NULL);
	Dbg_C2X ("attempted connection, accepting\n");
	/* accept tcp socket first */
	pSocket = Net_Accept (socket);
	if (NULL == pSocket) {
		Dbg_C2X ("failed to accept connection, user disconnected\n");
		return NULL;
	}
	/* get free serial */
	for (serial = 1; serial < MAX_HOSTS; serial++) {
		if (NULL == commList[serial]) {
			break;
		}
	}
	/* check validity of serial */
	if (serial >= MAX_HOSTS) {
		Dbg_C2X ("no free serial, disconnecting user\n");
		Net_Close (pSocket);
		return NULL;
	}
	Dbg_C2X ("serial #%u assigned, counter %u\n", serial, usercount);
	/* create communication data structure */
	fromcentral = calloc (1, sizeof (XBCommFromCentral));
	assert (NULL != fromcentral);
	/* create XBCommStream */
	Stream_CommInit (&fromcentral->stream, COMM_FromCentral, pSocket, HandleTelegram,
					 EventFromCentral, DeleteFromCentral);
	/* set XBCommFromCentral specific data */
	fromcentral->serial = serial;
	fromcentral->cnt = usercount;
	/* add to list of current connections */
	commList[serial] = fromcentral;
	/* inform parent layer */
	Central_Accept (serial, usercount++, Net_RemoteName (pSocket), Net_RemotePort (pSocket));
	/* that's all */
	return &fromcentral->stream.comm;
}								/* C2X_CreateComm */

/************
 * get data *
 ************/

/*
 * check if client is connected
 */
XBBool
C2X_Connected (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	return (commList[id] != NULL);
}								/* C2X_Connected */

/*
 * host address of user
 */
const char *
C2X_HostName (unsigned id)
{
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	/* get name from socket */
	return Net_RemoteName (commList[id]->stream.comm.socket);
}								/* C2X_HostName */

/*
 * local address to user
 */
const char *
C2X_LocalName (unsigned id)
{
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	/* get name from socket */
	return Net_LocalName (commList[id]->stream.comm.socket);
}								/* C2X_LocalName */

/**************
 * queue data *
 **************/

/*
 * queue player rankings to user
 */
void
C2X_QueuePlayerRankings (unsigned id)
{
	XBAtom atom;
	unsigned int count;
	unsigned int pid;
	unsigned int sent = 0;
	unsigned int i;
	/* sanity checks */
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* get player count in database and check validity of atoms */
	count = GetNumPlayerConfigs (CT_Central);
	for (i = 0; i < count; i++) {
		/* get pid atom */
		atom = GetPlayerAtom (CT_Central, i);
		if (atom == ATOM_INVALID) {
			Dbg_C2X ("database player %u has no valid pid\n", i);
			continue;
		}
		/* extract position */
		pid = GUI_AtomToInt (atom);
		if (pid > 0) {
			Dbg_C2X ("Sending player config pid %u\n", pid);
			if (!SendPlayerConfig
				(CT_Central, commList[id]->stream.sndQueue, XBT_COT_DataAvailable, 0, atom,
				 XBFalse)) {
				Dbg_C2X ("Unable to queue player data pid %u\n", pid);
			}
			else {
				sent++;
			}
		}
	}
	Dbg_C2X ("%u players queued\n", sent);
	/* queue end of data */
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_DataNotAvailable, XBT_ID_PlayerConfig, 0, NULL,
										  0));
	/* mark socket for writing */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	return;
}								/* HandleRequestPlayerConfig */

/*
 * queue pid to user
 */
void
C2X_QueueUserPID (unsigned id, int PID)
{
	char tmp[16];
	/* sanity check */
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* send pid as ascii */
	sprintf (tmp, "%i", PID);
	/* queue data */
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_DataAvailable, XBT_ID_PID, 0, tmp,
										  strlen (tmp) + 1));
	Dbg_C2X ("pid = %i queued to user %u\n", PID, id);
	/* mark socket for writing */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
}								/* C2X_SendUserPID */

/*
 * send request for disconnect to given client
 */
void
C2X_QueueDisconnect (unsigned id)
{
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	/* queue disconnect announcement */
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostDisconnected, 0, NULL,
										  0));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_RequestDisconnect, 0, NULL, 0));
	Dbg_C2X ("disconnect announcement queued to user %u\n", id);
	/* mark socket for writing */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	/* remove socket when queue empty */
	commList[id]->stream.prepFinish = XBTrue;
	/* socket can be cleared already */
	commList[id] = NULL;
	Dbg_C2X ("user slot %u available again\n", id);
}								/* C2X_QueueDisconnect */

/*
 * end of file com_from_central.c
 */
