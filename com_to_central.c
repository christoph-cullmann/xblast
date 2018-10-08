/*
 * file com_to_central.c - handle communications with centrals
 *
 * $Id: com_to_central.c,v 1.7 2006/02/09 21:21:23 fzago Exp $
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
} XBCommToCentral;

/*
 * local variables
 */

/*
 * central sends data
 */
static XBCommResult
HandleDataAvailable (XBCommStream * toCentral, const XBTelegram * tele)
{
	const void *data;
	size_t len;
	assert (toCentral != NULL);
	/* get telegramm data */
	data = Net_TeleData (tele, &len);
	switch (Net_TeleID (tele)) {
	case XBT_ID_PlayerConfig:
		/* Dbg_X2C("receiving player config from central\n"); */
		User_ReceivePlayerConfig (data);
		break;
	case XBT_ID_PID:
		Dbg_X2C ("receiving pid from central\n");
		User_ReceivePlayerPID (data);
		break;
	default:
		Dbg_X2C ("unsupported ID received for DataAvailable\n");
		return User_EventToCentral (XBE2C_InvalidID);;
	}
	return XCR_OK;
}								/* HandleDataAvailable */

/*
 * central sends end of data
 */
static XBCommResult
HandleDataNotAvailable (XBCommStream * toCentral, const XBTelegram * tele)
{
	switch (Net_TeleID (tele)) {
	case XBT_ID_PlayerConfig:
		Dbg_X2C ("No more players, disconnecting\n");
		User_NoMorePlayers ();
		break;
	case XBT_ID_PID:
		break;
	default:
		Dbg_X2C ("unsupported ID received for DataNotAvailable\n");
		return User_EventToCentral (XBE2C_InvalidID);;
	}
	return XCR_OK;
}								/* HandleDataAvailable */

/*
 * handle telegrams from central
 */
static XBCommResult
HandleTelegram (XBCommStream * stream, const XBTelegram * tele)
{
	assert (stream != NULL);
	switch (Net_TeleCOT (tele)) {
		/* central sends requested data */
	case XBT_COT_DataAvailable:
		return HandleDataAvailable (stream, tele);
		/* central has not requested data */
	case XBT_COT_DataNotAvailable:
		return HandleDataNotAvailable (stream, tele);
	default:
		Dbg_X2C ("unsupported CoT received\n");
		return User_EventToCentral (XBE2C_InvalidCot);;
	}
}								/* HandleTelegram */

/*
 * delete handler
 */
static XBCommResult
DeleteToCentral (XBComm * comm)
{
	/* delete stream */
	Stream_CommFinish ((XBCommStream *) comm);
	/* delete structure */
	free (comm);
	Dbg_X2C ("tcp connection to central removed\n");
	return XCR_OK;
}								/* DeleteToCentral */

/*
 * handle stream events
 */
static XBBool
EventToCentral (XBCommStream * comm, const XBStreamEvent ev)
{
	switch (ev) {
	case XBST_IOREAD:
		Dbg_X2C ("read error from central\n");
		return User_EventToCentral (XBE2C_IORead);
	case XBST_IOWRITE:
		Dbg_X2C ("write error to central\n");
		return User_EventToCentral (XBE2C_IOWrite);
	case XBST_EOF:
		Dbg_X2C ("eof from central\n");
		return User_EventToCentral (XBE2C_UnexpectedEOF);
	case XBST_WAIT:
		Dbg_X2C ("all data sent to central\n");
		return User_EventToCentral (XBE2C_StreamWaiting);
	case XBST_BUSY:
		/* Dbg_X2C("data waits to be sent to central\n"); */
		return User_EventToCentral (XBE2C_StreamBusy);
	case XBST_CLOSE:
		Dbg_X2C ("connection to central has been removed\n");
		(void)User_EventToCentral (XBE2C_StreamClosed);
		return XBFalse;
	default:
		Dbg_X2C ("unknown stream event on socket to central, ignoring\n");
		return XBFalse;
	}
}								/* EventToCentral */

/*
 * create a tcp connection to central
 */
XBComm *
X2C_CreateComm (const CFGCentralSetup * cfg)
{
	XBSocket *pSocket;
	XBCommToCentral *toCentral;

	assert (cfg != NULL);
	/* create connection to server */
	pSocket = Net_ConnectInet (cfg->name, cfg->port);
	if (NULL == pSocket) {
		Dbg_X2C ("failed to create tcp socket\n");
		return NULL;
	}
	Dbg_X2C ("tcp connection to central %s:%u established\n", cfg->name, cfg->port);
	/* create communication data structure */
	toCentral = calloc (1, sizeof (*toCentral));
	assert (NULL != toCentral);
	/* set values */
	Stream_CommInit (&toCentral->stream, COMM_ToCentral, pSocket, HandleTelegram, EventToCentral,
					 DeleteToCentral);
	/* that's all */
	return &toCentral->stream.comm;
}								/* CommCreateToServer */

/**************
 * local data *
 **************/

/*
 * return remote address in dot-representation
 */
const char *
X2C_CentralName (XBComm * comm)
{
	return Net_RemoteName (comm->socket);
}								/* X2C_CentralName */

/*
 * return local address in dot-representation
 */
const char *
X2C_LocalName (XBComm * comm)
{
	return Net_LocalName (comm->socket);
}								/* X2C_LocalName */

/**************
 * queue data *
 **************/

/*
 * query player config from central
 */
void
X2C_QueryPlayerConfig (XBComm * comm)
{
	XBCommStream *stream = (XBCommStream *) comm;

	/* sanity check */
	assert (stream != NULL);
	assert (stream->sndQueue != NULL);
	/* send data */
	Socket_RegisterWrite (CommSocket (&stream->comm));
	Net_SendTelegram (stream->sndQueue,
					  Net_CreateTelegram (XBT_COT_RequestData, XBT_ID_PlayerConfig, 0, NULL, 0));
	Dbg_X2C ("queued player query to central\n");
}								/* X2C_QueryPlayerConfig */

/*
 * send game config to central
 */
void
X2C_SendPlayerConfig (XBComm * comm, XBAtom atom)
{
	XBCommStream *stream = (XBCommStream *) comm;

	/* sanity check */
	assert (stream != NULL);
	assert (stream->sndQueue != NULL);
	/* send database section */
	Socket_RegisterWrite (CommSocket (&stream->comm));
	SendPlayerConfig (CT_Local, stream->sndQueue, XBT_COT_SendData, 0, atom, XBTrue);
	Dbg_X2C ("queued player config to central\n");
}								/* X2C_SendPlayerConfig */

/*
 * send game config to central
 */
void
X2C_SendGameStat (XBComm * comm, int numPlayers, int *PID, int *Score)
{
	XBCommStream *stream = (XBCommStream *) comm;
	char tmp[256];
	int i;
	/* sanity check */
	assert (stream != NULL);
	assert (stream->sndQueue != NULL);
	/* queue data */
	memcpy (tmp, &numPlayers, 4);
	for (i = 0; i < numPlayers; i++) {
		memcpy (tmp + 4 + i * 8, PID + i, 4);
		memcpy (tmp + 8 + i * 8, Score + i, 4);
	}
	Socket_RegisterWrite (CommSocket (&stream->comm));
	Net_SendTelegram (stream->sndQueue,
					  Net_CreateTelegram (XBT_COT_SendData, XBT_ID_GameStat, 0, tmp,
										  4 + numPlayers * 8));
	Dbg_X2C ("queued game stat to central\n");
}								/* X2C_SendGameStat */

/*
 * send request for disconnect to central
 */
void
X2C_SendDisconnect (XBComm * comm)
{
	XBCommStream *stream = (XBCommStream *) comm;

	/* inform host about disconnect request */
	Socket_RegisterWrite (CommSocket (&stream->comm));
	Net_SendTelegram (stream->sndQueue,
					  Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostDisconnected, 0, NULL,
										  0));
	Net_SendTelegram (stream->sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_RequestDisconnect, 0, NULL, 0));
	/* clear the communication, prepare for automatic shutdown */
	stream->prepFinish = XBTrue;
	Dbg_X2C ("queued disconnect sequence to central\n");
}								/* X2C_SendDisconnect */

/*
 * end of file com_to_central.c
 */
