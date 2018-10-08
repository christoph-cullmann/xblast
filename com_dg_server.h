/*
 * file com_dg_server.h - send ingame datagrams to server
 *
 * $Id: com_dg_server.h,v 1.8 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_DG_SERVER_H
#define XBLAST_COM_DG_SERVER_H

/*
 * global prototypes
 */

/* constructor */
extern XBComm *D2S_CreateComm (const char *local, const char *host, unsigned short port);

/* get local data */
extern unsigned short D2S_Port (const XBComm *);
extern XBBool D2S_Timeout (const XBComm *, const struct timeval *tv);
extern XBBool D2S_Connected (const XBComm *);

/* set local data */
extern void D2S_Reset (XBComm * comm);
extern void D2S_SetMaskBytes (XBComm * comm, unsigned);

/* queue data */
extern void D2S_SendConnect (XBComm * comm);
extern void D2S_SendPlayerAction (XBComm * comm, int gameTime, const PlayerAction * data);
extern void D2S_SendFinish (XBComm * comm, int gameTime);
extern XBBool D2S_Flush (XBComm * comm);

#endif
/*
 * end of file com_dg_server.h
 */
