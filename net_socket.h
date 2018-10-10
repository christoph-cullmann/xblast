/*
 * net_socket.h - low level functions to network communication
 *
 * $Id: net_socket.h,v 1.9 2005/01/16 08:48:54 iskywalker Exp $
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
#ifndef XBLAST_NET_SOCKET_H
#define XBLAST_NET_SOCKET_H
#ifdef W32
/* only WMS need it here */
#ifndef WMS
#include <winsock2.h>
#endif
#endif
#include "socket.h"

/*
 * global prototypes
 */
extern void Net_Init (void);
extern XBSocket *Net_ConnectInet (const char *hostname, unsigned short port);
extern XBSocket *Net_ListenInet (unsigned short port);
extern XBSocket *Net_Accept (const XBSocket *pSocket);
extern XBSocket *Net_BindUdp (const char *device, unsigned port);
extern XBBool Net_ConnectUdp (XBSocket *, const char *, unsigned short); 
extern void Net_Close (XBSocket *socket);
extern const char * Net_LocalName (const XBSocket *pSocket);
extern const char * Net_RemoteName (const XBSocket *pSocket);
extern unsigned Net_LocalPort (const XBSocket *pSocket);
extern unsigned Net_RemotePort (const XBSocket *pSocket);

#endif
/*
 * end of file net_socket.h
 */
