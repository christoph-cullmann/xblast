/*
 * file com_to_central.h - handle communications with clients
 *
 * $Id: com_from_central.h,v 1.7 2006/02/09 21:21:23 fzago Exp $
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
#ifndef _COM_FROM_CENTRAL_H
#define _COM_FROM_CENTRAL_H

/*
 * global prototypes
 */
extern XBComm *C2X_CreateComm (const XBSocket * socket);

/* get data */
extern XBBool C2X_Connected (unsigned id);
extern const char *C2X_HostName (unsigned id);
extern const char *C2X_LocalName (unsigned id);

/* queue data */
extern void C2X_QueuePlayerRankings (unsigned id);
extern void C2X_QueueUserPID (unsigned id, int PID);
extern void C2X_QueueDisconnect (unsigned id);

#endif
/*
 * end of file com_to_central.h
 */
