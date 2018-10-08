/*
 * file com_dg_client.h - send datagrams to client
 *
 * $Id: com_dg_client.h,v 1.8 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_DG_CLIENT_H
#define XBLAST_COM_DG_CLIENT_H

/*
 * global prototypes
 */

/* constructor */
extern XBComm *D2C_CreateComm (unsigned id, const char *localname, XBBool fixedPort);

/* connect/disconnect */
extern XBBool D2C_Connect (unsigned id, const char *host, unsigned short port);
extern void D2C_Clear (unsigned id);
extern void D2C_Disconnect (unsigned id);

/* get local data */
extern unsigned short D2C_Port (unsigned id);
extern XBBool D2C_Connected (unsigned id);
extern long D2C_LastPing (unsigned id);

/* set local data */
extern void D2C_Reset (unsigned id);
extern void D2C_SetMaskBytes (unsigned bytes);

/* queue data */
extern void D2C_SendPlayerAction (unsigned id, int gameTime, const PlayerAction * data);
extern void D2C_SendFinish (unsigned id, int gameTime);
extern XBBool D2C_Flush (unsigned id);

#endif
/*
 * end of file com_dg_client.h
 */
