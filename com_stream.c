/*
 * file com_stream.c - base struct und functions for stream connections
 *
 * $Id: com_stream.c,v 1.12 2006/02/09 21:21:23 fzago Exp $
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
 * try to read telegrams from server
 */
static XBCommResult
ReadStream (XBComm * comm)
{
	XBTeleResult result;
	XBTelegram *tele;
	XBCommResult cResult;
	XBCommStream *stream = (XBCommStream *) comm;
	unsigned fd;
	assert (NULL != stream);
	/* get file descriptor for debug output */
	fd = Socket_Fd (stream->comm.socket);
	/* read data into queue */
	result = Net_Receive (stream->rcvQueue, stream->comm.socket);
	if (stream->prepFinish) {
		/* reading while waiting for eof */
		switch (result) {
		case XBT_R_EndOfFile:
			Dbg_Stream ("expected end of file on fd=%u\n", fd);
			break;
		case XBT_R_IOError:
			Dbg_Stream ("read error on fd=%u while waiting for eof\n", fd);
			break;
		default:
			Dbg_Stream ("successful read on fd=%u while waiting for eof\n", fd);
			break;
		}
		/* only stream remains to be removed, sends close to parent layer */
		Stream_CommFinish (stream);
		return XCR_OK;
	}
	else {
		/* active mode */
		switch (result) {
		case XBT_R_EndOfFile:
			Dbg_Stream ("unexpected end of file on fd=%u\n", fd);
			/* eof event to parent layer */
			(void)(*stream->eventFunc) (stream, XBST_EOF);
			return XCR_Finished;
		case XBT_R_IOError:
			Dbg_Stream ("read error on fd=%u\n", fd);
			/* state change to parent layer */
			assert (stream->eventFunc != NULL);
			(void)(*stream->eventFunc) (stream, XBST_IOREAD);
			return XCR_Error;
		default:
			Dbg_Stream ("successful read on fd=%u\n", fd);
		}
	}
	/* data in rcv queue, handle as much as possible */
	assert (stream->handleFunc != NULL);
	while (NULL != (tele = Net_ReceiveTelegram (stream->rcvQueue))) {
		/* handle a single message */
		cResult = (*stream->handleFunc) (stream, tele);
		/* message not needed anymore */
		Net_DeleteTelegram (tele);
		/* return if handling fails, otherwise continue */
		if (cResult != XCR_OK) {
			Dbg_Stream ("parse error on fd=%u, shutting down\n", fd);
			return cResult;
		}
	}
	return XCR_OK;
}								/* ReadStream */

/*
 * XBComm write handler for XBCommStream
 */
static XBCommResult
WriteStream (XBComm * comm)
{
	XBTeleResult result;
	XBCommStream *stream = (XBCommStream *) comm;
	unsigned fd;
	assert (NULL != stream);
	/* get file descriptor for debug output */
	fd = Socket_Fd (stream->comm.socket);
	/* send top element of send queue */
	result = Net_Send (stream->sndQueue, stream->comm.socket);
	switch (result) {
	case XBT_R_Complete:		/* queue has been emptied */
		Dbg_Stream ("sent all telegrams on fd=%u\n", fd);
		/* no more writing needed */
		Socket_UnregisterWrite (CommSocket (&stream->comm));
		/* state change to parent layer */
		assert (stream->eventFunc != NULL);
		(void)(*stream->eventFunc) (stream, XBST_WAIT);
		/* shutdown for empty queue, if asked for */
		if (stream->prepFinish) {
			Socket_ShutdownWrite (CommSocket (&stream->comm));
			Dbg_Stream ("socket shutdown for writing\n");
		}
		return XCR_OK;
	case XBT_R_IOError:		/* error while sending telegram */
		Dbg_Stream ("i/o-error write to fd=%u, shutting down\n", fd);
		/* state change to parent layer */
		assert (stream->eventFunc != NULL);
		(void)(*stream->eventFunc) (stream, XBST_IOWRITE);
		/* return error, deletes XBComm */
		return XCR_Error;
	default:					/* anything else */
		Dbg_Stream ("partial send on fd=%u\n", fd);
		/* state to parent layer */
		assert (stream->eventFunc != NULL);
		(void)(*stream->eventFunc) (stream, XBST_BUSY);
		return XCR_OK;
	}
}								/* WriteStream */

/*
 * add a XBCommStream
 */
void
Stream_CommInit (XBCommStream * stream, XBCommType commType, XBSocket * pSocket,
				 StreamHandleFunc handleFunc, StreamEventFunc eventFunc, XBCommFunc deleteFunc)
{
	assert (stream != NULL);
	/* add the underlying XBComm to internal list */
	CommInit (&stream->comm, commType, pSocket, ReadStream, WriteStream, deleteFunc);
	/* set stream specific handlers */
	stream->handleFunc = handleFunc;
	stream->eventFunc = eventFunc;
	/* flag: shutdown when send queue empty */
	stream->prepFinish = XBFalse;
	/* create queues */
	stream->sndQueue = Net_CreateSndQueue (commType == COMM_ToClient);
	stream->rcvQueue = Net_CreateRcvQueue (commType == COMM_ToClient);
	assert (NULL != stream->sndQueue);
	assert (NULL != stream->rcvQueue);
}								/* Stream_CommInit */

/*
 * remove a XBCommStream
 * does not free allocated XBComm memory!!
 */
void
Stream_CommFinish (XBCommStream * stream)
{
	/* debug output before socket is freed */
	Dbg_Stream ("removing stream on fd=%u\n", Socket_Fd (stream->comm.socket));
	/* remove XBComm from internal list, socket is freed */
	CommFinish (&stream->comm);
	/* free up queues */
	Net_DeleteSndQueue (stream->sndQueue);
	Net_DeleteRcvQueue (stream->rcvQueue);
	/* close event to parent layer */
	assert (stream->eventFunc != NULL);
	(void)(*stream->eventFunc) (stream, XBST_CLOSE);
}								/* Stream_CommFinish */

/*
 * end of file com_stream.c
 */
