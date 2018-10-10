/*
 * file socket.h - sockets abstraction for xblast
 *
 * $Id: socket.h,v 1.4 2004/11/04 16:41:39 lodott Exp $
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
#ifndef XBLAST_SOCKET_H
#define XBLAST_SOCKET_H

#include "xblast.h"

/*
 * global const
 */
#define XB_SOCKET_END_OF_FILE   0
#define XB_SOCKET_WOULD_BLOCK (-1)
#define XB_SOCKET_ERROR       (-2)

/*
 * type definitions
 */
typedef struct _xb_socket XBSocket;
typedef struct _xb_socket_interface {
  char *name;
  char *addrDevice;
  char *addrBroadcast;
} XBSocketInterface;

/*
 * prototypes
 */

/* managing sockets */
extern XBBool Socket_Init (void);
extern void   Socket_Finish (void);

/* data */
extern int         Socket_Fd       (const XBSocket *);
extern int         Socket_Family   (const XBSocket *);
extern const char *Socket_HostName (const XBSocket *pSocket, XBBool peer);
extern unsigned    Socket_HostPort (const XBSocket *pSocket, XBBool peer);

/* construct */
extern XBSocket *Socket_Alloc (int family);
extern void      Socket_Free  (XBSocket *);

extern XBBool      Socket_SetAddressInet (XBSocket *pSocket, XBBool peer, const char *hostName, unsigned short port);
extern XBBool      Socket_SetBroadcast   (XBSocket *pSocket, XBBool enable);

extern void Socket_RegisterWrite   (XBSocket *);
extern void Socket_RegisterRead    (XBSocket *);
extern void Socket_UnregisterWrite (XBSocket *);
extern void Socket_UnregisterRead  (XBSocket *);

/* bsd calls */
extern XBBool Socket_Open     	   (XBSocket *pSocket, int type);
extern XBBool Socket_Connect  	   (XBSocket *pSocket);
extern XBBool Socket_Bind     	   (XBSocket *pSocket);
extern XBBool Socket_Accept   	   (XBSocket *pSocket, const XBSocket *pListen);
extern XBBool Socket_Listen   	   (XBSocket *pSocket);
extern void   Socket_Close         (XBSocket *);
extern void   Socket_ShutdownWrite (XBSocket *);

extern int    Socket_Send        (const XBSocket *pSocket, const void *buf, size_t len);
extern int    Socket_Receive     (const XBSocket *pSocket, void *buf, size_t len);
extern int    Socket_SendTo      (XBSocket *pSocket, const void *buf, size_t len, const char *host, unsigned short port);
extern int    Socket_ReceiveFrom (XBSocket *pSocket, void *buf, size_t len, const char **host, unsigned short *port);

/* interfaces */
extern const XBSocketInterface *Socket_GetInterfaces (size_t *);
extern const char *Socket_GetAutoInterface (XBSocket *, const char *);

#endif
/*
 * end of file socket.h
 */
