/*
 * file com_to_client.c - handle communications with clients
 *
 * $Id: com_to_client.c,v 1.19 2006/02/09 21:21:23 fzago Exp $
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
	XBCommStream stream;
	unsigned serial;
} XBCommToClient;

/*
 * local variables
 */

/* streams to clients */
static XBCommToClient *commList[MAX_HOSTS] = {
	/* this entry is never used (server) */
	NULL,
	/* up to 5 clients can connect */
	NULL, NULL, NULL, NULL, NULL,
};

/************
 * handlers *
 ************/

/*
 * data received from a client
 */
static XBCommResult
HandleDataAvailable (XBCommToClient * toClient, const XBTelegram * tele)
{
	const void *data;
	size_t len;

	assert (toClient != NULL);
	/* get telegramm data */
	data = Net_TeleData (tele, &len);
	switch (Net_TeleID (tele)) {
	case XBT_ID_GameConfig:
		Dbg_S2C ("received game config line\n");
		Server_ReceiveGameConfig (toClient->serial, data);
		break;
	case XBT_ID_PlayerConfig:
		Dbg_S2C ("received player config line\n");
		Server_ReceivePlayerConfig (toClient->serial, (int)Net_TeleIOB (tele), data);
		break;
	default:
		Dbg_S2C ("received game config line\n");
		return Server_StreamEvent (toClient->serial, XBSC_IDInvalid) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* HandleDataAvailable */

/*
 * data not available on client
 */
static XBCommResult
HandleDataNotAvailable (XBCommToClient * toClient, const XBTelegram * tele)
{
	switch (Net_TeleID (tele)) {
	case XBT_ID_GameConfig:
		Dbg_S2C ("host #%u has no game config data!\n", toClient->serial);
		return Server_StreamEvent (toClient->serial, XBSC_MissingData) ? XCR_Error : XCR_OK;
	default:
		Dbg_S2C ("host #%u has no data of unrecognized type!\n", toClient->serial);
		return Server_StreamEvent (toClient->serial, XBSC_IDInvalid) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* HandleDataAvailable */

/*
 * received client request
 */
static XBCommResult
HandleActivate (XBCommToClient * toClient, const XBTelegram * tele)
{
	const char *data;
	size_t len;
	unsigned iob;
	unsigned value;

	assert (toClient != NULL);
	assert (tele != NULL);
	/* get telegramm data */
	data = Net_TeleData (tele, &len);
	iob = Net_TeleIOB (tele);
	switch (Net_TeleID (tele)) {
	case XBT_ID_DgramPort:
		/* inform application */
		if (NULL != data && 1 == sscanf (data, "%u", &value)) {
			Server_ReceiveDgramPort (toClient->serial, value);
			Dbg_S2C ("received dgram port ftrom host #%u\n", toClient->serial);
		}
		else {
			Dbg_S2C ("host #%u has no data of unrecognized type!\n", toClient->serial);
			return Server_StreamEvent (toClient->serial, XBSC_DataInvalid) ? XCR_Error : XCR_OK;
		}
		return XCR_OK;
	case XBT_ID_LevelConfig:
		Dbg_S2C ("receive level check from host #%u\n", toClient->serial);
		Server_ReceiveLevelCheck (toClient->serial, iob);
		return XCR_OK;
	case XBT_ID_WinnerTeam:
		Dbg_S2C ("receive winner team from host #%u\n", toClient->serial);
		Server_ReceiveWinnerTeam (toClient->serial, iob);
		return XCR_OK;
	case XBT_ID_HostChangeReq:
		if (len == 2) {
			Dbg_S2C ("host #%u requests host state change\n", toClient->serial);
			Server_ReceiveHostStateReq (toClient->serial, data[0], data[1]);
		}
		else {
			Dbg_S2C ("host #%u sends invalid host request data!\n", toClient->serial);
			return Server_StreamEvent (toClient->serial, XBSC_DataInvalid) ? XCR_Error : XCR_OK;
		}
		return XCR_OK;
	case XBT_ID_TeamChangeReq:
		if (len == 3) {
			Dbg_S2C ("host #%u requests team state change\n", toClient->serial);
			Server_ReceiveTeamStateReq (toClient->serial, data[0], data[1], data[2]);
		}
		else {
			Dbg_S2C ("host #%u sends invalid team request data!\n", toClient->serial);
			return Server_StreamEvent (toClient->serial, XBSC_DataInvalid) ? XCR_Error : XCR_OK;
		}
		return XCR_OK;
	default:
		Dbg_S2C ("unrecognized request from host #%u!\n", toClient->serial);
		return Server_StreamEvent (toClient->serial, XBSC_IDInvalid) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* HandleActivate */

/*
 * client sends info
 */
static XBCommResult
HandleSpontaneous (XBCommToClient * toClient, const XBTelegram * tele)
{
	const char *data;
	size_t len;
	XBTeleIOB iob;
	XBChat *chat;
	unsigned value;

	assert (toClient != NULL);
	data = Net_TeleData (tele, &len);
	iob = Net_TeleIOB (tele);
	/* get telegramm data */
	switch (Net_TeleID (tele)) {
	case XBT_ID_Sync:
		Dbg_S2C ("receiving sync from host #%u\n", toClient->serial);
		Server_ReceiveSync (toClient->serial, iob);
		return XCR_OK;
	case XBT_ID_HostChange:
		if (NULL != data && 1 == sscanf (data, "%u", &value)) {
			Dbg_S2C ("REMOVE! receiving host change from host #%u\n", toClient->serial);
			/* Server_ReceiveHostState (toClient->serial, value ); */
		}
		return XCR_OK;
	case XBT_ID_Chat:
		chat = Chat_UnpackData (data, len, iob);
		if (NULL != chat) {
			Dbg_S2C ("receiving chat from host #%u", toClient->serial);
			Server_ReceiveChat (chat);
		}
		else {
			Dbg_S2C ("invalid chat received from host #%u", toClient->serial);
			return Server_StreamEvent (toClient->serial, XBSC_IDInvalid) ? XCR_Error : XCR_OK;
		}
		return XCR_OK;
	default:
		Dbg_S2C ("invalid info received from host #%u", toClient->serial);
		return Server_StreamEvent (toClient->serial, XBSC_IDInvalid) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* HandleSpontaneous */

/*
 * handle telegrams from server
 */
static XBCommResult
HandleTelegram (XBCommStream * stream, const XBTelegram * tele)
{
	XBCommToClient *toClient = (XBCommToClient *) stream;

	assert (toClient != NULL);
	switch (Net_TeleCOT (tele)) {
		/* client sends requested data */
	case XBT_COT_DataAvailable:
		return HandleDataAvailable (toClient, tele);
		/* client has not requested data */
	case XBT_COT_DataNotAvailable:
		return HandleDataNotAvailable (toClient, tele);
		/* client command has arrived */
	case XBT_COT_Activate:
		return HandleActivate (toClient, tele);
		/* client message has arrived */
	case XBT_COT_Spontaneous:
		return HandleSpontaneous (toClient, tele);
	default:
		Dbg_S2C ("invalid cot received from host #%u", toClient->serial);
		return Server_StreamEvent (toClient->serial, XBSC_COTInvalid) ? XCR_Error : XCR_OK;
	}
	return XCR_OK;
}								/* HandleTelegram */

/*
 * delete handler: triggered by eof and parse errors
 * always called from com_base.c
 */
static XBCommResult
DeleteToClient (XBComm * comm)
{
	XBCommToClient *toClient = (XBCommToClient *) comm;
	assert (comm != NULL);
	/* unmark client */
	commList[toClient->serial] = NULL;
	/* clean up */
	Stream_CommFinish (&toClient->stream);
	free (comm);
	return XCR_OK;
}								/* DeleteToClient */

/*
 * handle stream events
 */
static XBBool
EventToClient (XBCommStream * comm, const XBStreamEvent ev)
{
	XBCommToClient *toClient = (XBCommToClient *) comm;
	XBServerConstants code;
	assert (toClient != NULL);
	switch (ev) {
	case XBST_IOREAD:
		code = XBSC_IOError;
		break;
	case XBST_IOWRITE:
		code = XBSC_IOError;
		break;
	case XBST_EOF:
		code = XBSC_UnexpectedEOF;
		break;
	case XBST_WAIT:
		code = XBSC_StreamWaiting;
		break;
	case XBST_BUSY:
		code = XBSC_StreamBusy;
		break;
	case XBST_CLOSE:
		code = XBSC_StreamClosed;
		break;
	default:
		return XBFalse;
	}
	return Server_StreamEvent (toClient->serial, code);
}								/* EventToClient */

/***************
 * constructor *
 ***************/

/*
 * create stream to client on valid socket
 */
XBComm *
S2C_CreateComm (const XBSocket * socket)
{
	unsigned serial;
	XBSocket *pSocket;
	XBCommToClient *toClient;

	assert (socket != NULL);
	Dbg_S2C ("client connected to listen socket!\n");
	/* get free serial */
	for (serial = 1; serial < MAX_HOSTS; serial++) {
		if (NULL == commList[serial]) {
			break;
		}
	}
	if (serial >= MAX_HOSTS) {
		Dbg_S2C ("failed to assign valid id, disconnecting\n");
		return NULL;
	}
	Dbg_S2C ("Assigned id #%u\n", serial);
	/* create listen socket */
	pSocket = Net_Accept (socket);
	if (NULL == pSocket) {
		Dbg_S2C ("failed to create reply socket\n");
		return NULL;
	}
	/* create communication data structure */
	toClient = calloc (1, sizeof (XBCommToClient));
	assert (NULL != toClient);
	/* set values */
	Stream_CommInit (&toClient->stream, COMM_ToClient, pSocket, HandleTelegram, EventToClient,
					 DeleteToClient);
	toClient->serial = serial;
	/* add to internal list */
	commList[serial] = toClient;
	Dbg_S2C ("stream to host #%u accepted\n", serial);
	/* inform application */
	Server_Accept (serial, Net_RemoteName (pSocket), Net_RemotePort (pSocket));
	/* that's all */
	return &toClient->stream.comm;
}								/* S2C_CreateComm */

/******************
 * get local data *
 ******************/

/*
 * check if client is connected
 */
XBBool
S2C_Connected (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	return (commList[id] != NULL);
}								/* S2C_Connected */

/*
 * output all connected clients
 */
void
S2C_ShowConnected (void)
{
	unsigned id;
	Dbg_S2C ("Connected client id's: ");
	for (id = 1; id < MAX_HOSTS; id++) {
		if (S2C_Connected (id)) {
			Dbg_Out ("%i ", id);
		}
	}
	Dbg_Out ("\n");
}								/* S2C_Connected */

/*
 * hostname of client
 */
const char *
S2C_HostName (unsigned id)
{
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	/* get name from socket */
	return Net_RemoteName (commList[id]->stream.comm.socket);
}								/* S2C_HostName */

/*
 * hostname of client
 */
const char *
S2C_LocalName (unsigned id)
{
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	/* get name from socket */
	return Net_LocalName (commList[id]->stream.comm.socket);
}								/* S2C_LocalName */

/**************
 * queue data *
 **************/

/*
 * queue game config to client
 */
void
S2C_SendGameConfig (unsigned id, unsigned hostID, XBAtom atom)
{
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* send database section */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	SendGameConfig (CT_Remote, commList[id]->stream.sndQueue, XBT_COT_SendData, (XBTeleIOB) hostID,
					atom);
	Dbg_S2C ("queued game config to host #%u\n", id);
}								/* S2C_SendGameConfig */

/*
 * queue player config to client
 */
void
S2C_SendPlayerConfig (unsigned id, unsigned hostId, int player, XBAtom atom)
{
	XBTeleIOB iob;

	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* convert id and player to iob */
	iob = ((XBTeleIOB) hostId << 4) + (XBTeleIOB) player;
	/* send database section */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	SendPlayerConfig (CT_Remote, commList[id]->stream.sndQueue, XBT_COT_SendData, iob, atom,
					  XBFalse);
	Dbg_S2C ("queued single player config to host #%u\n", id);
}								/* S2C_SendPlayerConfig */

/*
 * queue dgram port to client
 */
void
S2C_SendDgramPort (unsigned id, unsigned short port)
{
	char tmp[16];

	/* sanity check */
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* send seed as ascii */
	sprintf (tmp, "%hu", port);
	/* send data */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_DgramPort, id, tmp,
										  strlen (tmp) + 1));
	Dbg_S2C ("queued dgram port to host #%u\n", id);
}								/* S2C_SendDgramPort */

/*
 * queue game config request to client
 */
void
S2C_QueryGameConfig (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* send request */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_RequestData, XBT_ID_GameConfig, 0, NULL, 0));
	Dbg_S2C ("queued game config request to host #%u\n", id);
}								/* S2C_QueryGameConfig */

/*
 * queue player config request to client
 */
void
S2C_QueryPlayerConfig (unsigned id, int player)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	assert (player < NUM_LOCAL_PLAYER);
	/* send request */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_RequestData, XBT_ID_PlayerConfig,
										  (XBTeleIOB) player, NULL, 0));
	Dbg_S2C ("queued player config request to host #%u\n", id);
}								/* S2C_QueryPlayerConfig */

/*
 * queue host state to client
 */
void
S2C_SendHostState (unsigned id, unsigned hostID, unsigned state)
{
	char tmp[16];

	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	sprintf (tmp, "%hu", state);
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_HostChange, (XBTeleIOB) hostID,
										  tmp, strlen (tmp) + 1));
	Dbg_S2C ("queued host state to host #%u:%u to host #%u\n", hostID, state, id);
}								/* S2C_SendHostState */

/*
 * queue team state to client
 */
void
S2C_SendTeamState (unsigned id, unsigned host, unsigned player, XBTeamState team)
{
	char tmp[16];
	unsigned hpid;
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	/* version <2_10_2 cannot handle XBTS_Out, send invalid */
	if (team == XBTS_Out && !Version_AtLeast (id, &Ver_2_10_2)) {
		team = XBTS_Invalid;
	}
	sprintf (tmp, "%hu", team);
	hpid = (host * NUM_LOCAL_PLAYER) + (player & 0xFF);
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_TeamChange, (XBTeleIOB) hpid,
										  tmp, strlen (tmp) + 1));
	Dbg_S2C ("queued team state to host #%u\n", id);
}								/* S2C_SendTeamState */

/*
 * queue host state request to client
 */
void
S2C_SendHostStateReq (unsigned id, unsigned who, unsigned hostID, unsigned state)
{
	unsigned char tmp[2];
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	tmp[0] = hostID & 0xff;
	tmp[1] = state & 0xff;
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_HostChangeReq,
										  (XBTeleIOB) who & 0xFF, tmp, sizeof (tmp)));
	Dbg_S2C ("queued host state request #%u:#%u->%u to host #%u\n", who, hostID, state, id);
}								/* S2C_SendHostStateReq */

/*
 * queue team state to client
 */
void
S2C_SendTeamStateReq (unsigned id, unsigned who, unsigned hostID, unsigned player, XBTeamState team)
{
	unsigned char tmp[3];
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	/* version < 2_10_2 cannot handle XBTS_Out, send invalid */
	if (team == XBTS_Out && !Version_AtLeast (id, &Ver_2_10_2)) {
		team = XBTS_Invalid;
	}
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	tmp[0] = hostID & 0xff;
	tmp[1] = player & 0xff;
	tmp[2] = team & 0xff;
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_TeamChangeReq,
										  (XBTeleIOB) who & 0xFF, tmp, sizeof (tmp)));
	Dbg_S2C ("queued team state request to host #%u:#%u(%u)->%u to host #%u\n", who, hostID, player,
			 team, id);

}								/* S2C_SendTeamStateReq */

/*
 * send a chat line to th:tp from fh:fp
 */
void
S2C_SendChat (unsigned id, XBChat * chat)
{
	unsigned iob;
	char *data;
	size_t len;
	/* sanity check */
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (chat != NULL);
	/* pack chat data */
	len = Chat_PackData (chat, &data, &iob);
	/* prepare for writing */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_SendData, XBT_ID_Chat, iob, data, len));
	Dbg_S2C ("queued chat to host #%u\n", id);
}								/* S2C_SendChat */

/*
 * queue disconnect message to client
 */
void
S2C_HostDisconnected (unsigned id, unsigned hostID)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostDisconnected, hostID,
										  NULL, 0));
	Dbg_S2C ("queued disconnect info to host #%u\n", id);
}								/* S2C_HostDisconnected */

/*
 * queue request for disconnect to given client
 */
void
S2C_Disconnect (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	/* inform host about disconnect request */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Spontaneous, XBT_ID_HostDisconnected, 0, NULL,
										  0));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_RequestDisconnect, 0, NULL, 0));
	Dbg_S2C ("queued disconnect sequence to host #%u\n", id);
	/* free slot, prepare to finish  */
	commList[id]->stream.prepFinish = XBTrue;
	commList[id] = NULL;
}								/* S2C_Disconnect */

/*
 * queue start game signal to client
 */
void
S2C_StartGame (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_StartGame, (XBTeleIOB) id, NULL,
										  0));
	Dbg_S2C ("queued start of game to host #%u\n", id);
}								/* S2C_StartGame */

/*
 * queue sync to client
 */
void
S2C_Sync (unsigned id, XBNetworkEvent event)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_Sync, (XBTeleIOB) event, NULL,
										  0));
	Dbg_S2C ("queued sync #%u to host #%u\n", event, id);
}								/* S2C_Sync */

/*
 * queue random seed to client
 */
void
S2C_SendRandomSeed (unsigned id, unsigned seed)
{
	char tmp[16];

	/* sanity check */
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* send seed as ascii */
	sprintf (tmp, "%u", seed);
	/* send data */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_RandomSeed, 0, tmp,
										  strlen (tmp) + 1));
	Dbg_S2C ("queued random seed to host #%u\n", id);
}								/* S2C_SendRandomSeed */

/*
 * queue level data to client
 */
void
S2C_SendLevelConfig (unsigned id, const DBRoot * level)
{
	assert (id > 0);
	assert (id <= MAX_HOSTS);
	assert (commList[id] != NULL);
	assert (commList[id]->stream.sndQueue != NULL);
	/* send database section */
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	SendLevelConfig (commList[id]->stream.sndQueue, XBT_COT_SendData, level);
	Dbg_S2C ("queued level config to host #%u\n", id);
}								/* S2C_SendLevelConfig */

/*
 * queue level activation to client
 */
void
S2C_SendLevelActivate (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);
	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_LevelConfig, (XBTeleIOB) 1, NULL,
										  0));
	Dbg_S2C ("queued level activation to host #%u\n", id);
}								/* S2C_SendLevelActivate */

/*
 * queue level reset to client
 */
void
S2C_SendLevelReset (unsigned id)
{
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_LevelConfig, (XBTeleIOB) 0, NULL,
										  0));
	Dbg_S2C ("queued level reset to host #%u\n", id);
}								/* S2C_SendLevelReset */

/*
 * queue async notification to client
 */
void
S2C_SendLevelAsync (unsigned id)
{
	XBNetworkEvent ev = XBNW_SyncLevelResult;
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_Async, (XBTeleIOB) ev, NULL, 0));
	Dbg_S2C ("queued level async to host #%u\n", id);
}								/* S2C_SendLevelAsync */

/*
 * queue level sync to client
 */
void
S2C_SendLevelSync (unsigned id)
{
	XBNetworkEvent ev = XBNW_SyncLevelResult;
	assert (id > 0);
	assert (id < MAX_HOSTS);
	assert (commList[id] != NULL);

	Socket_RegisterWrite (CommSocket (&commList[id]->stream.comm));
	Net_SendTelegram (commList[id]->stream.sndQueue,
					  Net_CreateTelegram (XBT_COT_Activate, XBT_ID_Sync, (XBTeleIOB) ev, NULL, 0));
	Dbg_S2C ("queued level sync to host #%u\n", id);
}								/* S2C_SendLevelSync */

/*
 * end of file com_to_client.c
 */
