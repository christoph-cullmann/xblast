/*
 * file com_stream.h - base struct und functions for stream connections
 *
 * $Id: com_stream.h,v 1.8 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_STREAM_H
#define XBLAST_COM_STREAM_H

/*
 * type definitions
 */

/* forward declaration */
typedef struct _xb_comm_stream XBCommStream;

/* stream events */
typedef enum
{
	XBST_IOREAD,
	XBST_IOWRITE,
	XBST_EOF,
	XBST_BUSY,
	XBST_WAIT,
	XBST_CLOSE
} XBStreamEvent;

/* callback for handling incoming telegrams */
typedef XBCommResult (*StreamHandleFunc) (XBCommStream *, const XBTelegram * tele);
typedef XBBool (*StreamEventFunc) (XBCommStream *, const XBStreamEvent);

/* base structure for all stream bases communications */
struct _xb_comm_stream
{
	XBComm comm;
	XBBool prepFinish;
	XBSndQueue *sndQueue;
	XBRcvQueue *rcvQueue;
	StreamHandleFunc handleFunc;
	StreamEventFunc eventFunc;
};

/*
 * global prototypes
 */
extern void Stream_CommInit (XBCommStream *, XBCommType, XBSocket *, StreamHandleFunc,
							 StreamEventFunc, XBCommFunc);
extern void Stream_CommFinish (XBCommStream * stream);

#endif
/*
 * end of file com_stream.h
 */
