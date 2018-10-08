/*
 * file com_base.h
 *
 * $Id: com_base.h,v 1.5 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_BASE_H
#define XBLAST_COM_BASE_H

/*
 * type definitions
 */

/* types of communication */
typedef enum
{
	XCS_Init,
	XCS_Connected,
	XCS_Finished,
	XCS_Start
} XBCommStatus;

/* types of communications */
typedef enum
{
	COMM_ToServer,				/* connection to server */
	COMM_ToClient,				/* connection to client */
	COMM_DgServer,				/* datagram link to server */
	COMM_DgClient,				/* datagram link to client */
	COMM_Listen,				/* listen for clients to connect */
	COMM_Query,					/* query for network games */
	COMM_Reply,					/* reply with network game */
	COMM_NewGame,				/* XBCC */
	COMM_NewGameOK,				/* XBCC */
	COMM_ToCentral,				/* XBCC connection to central */
	COMM_FromCentral			/* XBCC connection from central */
} XBCommType;

/* process functions */
typedef XBCommResult (*XBCommFunc) (XBComm *);

/* generic data of communication */
struct _xb_comm
{
	XBCommType type;
	XBComm *next;
	XBComm *prev;
	XBSocket *socket;
	XBCommFunc readFunc;
	XBCommFunc writeFunc;
	XBCommFunc deleteFunc;
};

/*
 * global prototypes
 */
extern void CommInit (XBComm * comm, XBCommType type, XBSocket * socket, XBCommFunc readFunc,
					  XBCommFunc writeFunc, XBCommFunc deleteFunc);
extern void CommFinish (XBComm * comm);
extern XBSocket *CommSocket (XBComm * comm);
extern XBComm *CommFind (int fd);

#endif
/*
 * end of file com_base.h
 */
