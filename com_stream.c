/*
 * file com_stream.c - base struct und functions for stream connections
 *
 * $Id: com_stream.c,v 1.5 2004/11/06 01:53:47 lodott Exp $
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
#include "com_stream.h"

/************
 * handlers *
 ************/

/*
 * receiving data on stream
 */
static XBCommResult
ReadStream (XBComm *comm)
{
  XBTeleResult  result;
  XBTelegram   *tele;
  XBCommResult  cResult;
  XBCommStream *stream = (XBCommStream *) comm;
  unsigned cnt = 0;
  unsigned fd;

  assert (NULL != stream);
  fd = Socket_Fd (stream->comm.socket);
  result = Net_Receive (stream->rcvQueue, stream->comm.socket);
  if (result == XBT_R_IOError) {
    Dbg_Stream ("read-error on fd=%u, shutting down\n", fd);
    assert (stream->eventFunc != NULL);
    (void) (*stream->eventFunc) (stream, XBST_IOREAD);
    return XCR_Error;
  } else if (result == XBT_R_EndOfFile) {
    if (!stream->prepFinish) {
      Dbg_Stream ("unexpected end of file on fd=%u\n", fd);
      (void) (*stream->eventFunc) (stream, XBST_EOF);
      return XCR_Finished;
    } else {
      Dbg_Stream ("expected end of file on fd=%u\n", fd);
      /* only stream remains to be removed */
      Stream_CommFinish(stream);
      free(stream);
      return XCR_OK;
    }
  }
  assert (stream->handleFunc != NULL);
  while (NULL != (tele = Net_ReceiveTelegram (stream->rcvQueue) ) ) {
    cResult = (*stream->handleFunc) (stream, tele);
    Net_DeleteTelegram (tele);
    cnt += 1;
    if (cResult != XCR_OK) {
      Dbg_Stream("parse error on message #%u on %u, shutting down\n", cnt, fd);
      return cResult;
    }
  }
  Dbg_Stream("successfully parsed %u messages on %u\n", cnt, fd);
  return XCR_OK;
} /* ReadStream */

/*
 * write telegram to server
 */
static XBCommResult
WriteStream (XBComm *comm)
{
  XBTeleResult result;
  XBCommStream *stream = (XBCommStream *) comm;
  unsigned fd;
  assert (NULL != stream);
  fd = Socket_Fd (stream->comm.socket);
  result = Net_Send (stream->sndQueue, stream->comm.socket);
  switch (result) {
  case XBT_R_Complete:
    Socket_UnregisterWrite (CommSocket (&stream->comm));
    Dbg_Stream("sent all telegrams on fd=%u\n", fd);
    assert (stream->eventFunc != NULL);
    (void) (*stream->eventFunc) (stream, XBST_WAIT);
    if (stream->prepFinish) {
      Socket_ShutdownWrite (CommSocket (&stream->comm));
      Dbg_Stream("socket shutdown for writing\n");
    }
    return XCR_OK;
  case XBT_R_IOError:
    Dbg_Stream ("i/o-error write to fd=%u, shutting down\n", fd);
    assert (stream->eventFunc != NULL);
    (void) (*stream->eventFunc) (stream, XBST_IOWRITE);
    return XCR_Error;
    /* anything else */
  default:
    Dbg_Stream("partial send on fd=%u\n", fd);
    assert (stream->eventFunc != NULL);
    (void) (*stream->eventFunc) (stream, XBST_BUSY);
    return XCR_OK;
  }
} /* WriteStream */

/*
 * constructor for a XBCommStream
 */
void
Stream_CommInit (XBCommStream *stream, XBCommType commType, XBSocket *pSocket, StreamHandleFunc handleFunc, StreamEventFunc eventFunc, XBCommFunc deleteFunc)
{
  assert (stream != NULL);
  /* set values */
  CommInit (&stream->comm, COMM_ToServer, pSocket, ReadStream, WriteStream, deleteFunc);
  stream->handleFunc = handleFunc;
  stream->eventFunc  = eventFunc;
  stream->prepFinish = XBFalse;
  /* create tele lists */
  stream->sndQueue = Net_CreateSndQueue (commType == COMM_ToClient);
  stream->rcvQueue = Net_CreateRcvQueue (commType == COMM_ToClient);
  assert (NULL != stream->sndQueue);
  assert (NULL != stream->rcvQueue);
  Dbg_Stream("created stream on fd=%u\n", Socket_Fd (stream->comm.socket));
} /* CommCreateToServer */

/*
 * remove allocated memory
 */
void
Stream_CommFinish (XBCommStream *stream)
{
  CommFinish (&stream->comm);
  Net_DeleteSndQueue (stream->sndQueue);
  Net_DeleteRcvQueue (stream->rcvQueue);
  Dbg_Stream("removed stream on fd=%u\n", Socket_Fd (stream->comm.socket));
  assert (stream->eventFunc != NULL);
  (void) (*stream->eventFunc) (stream, XBST_CLOSE);
} /* Stream_CommFinish */

/*
 * end of file com_stream.c
 */
