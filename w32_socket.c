/*
 * file w32_socket.c - true bsd sockets for xblast
 *
 * $Id: w32_socket.c,v 1.13 2006/02/19 13:33:01 lodott Exp $
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
#define __USE_W32_SOCKETS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "socket.h"
#include "w32_socket.h"
#include "w32_event.h"
#include "str_util.h"
#include "com.h"
#include "gui.h"

/*
 * local constants
 */
#define LISTENQ 5
/* needed winsock version */
#define WINSOCK_VERSION (MAKEWORD (2, 0))
/* maximum number of sockets in map */
#define MAX_SOCKET 64
/* interface detection modes */
#define XBIF_MS 0x01
#define XBIF_MOD 0x02

/*
 * type definitions
 */

/* socket address */
typedef struct _xb_socket_address
{
	int len;					/* allocated length */
	struct sockaddr *addr;		/* protocol dependent data */
} XBSocketAddress;

/* socket data */
struct _xb_socket
{
	SOCKET fd;					/* file descriptor */
	XBSocketAddress sock;		/* local address */
	XBSocketAddress peer;		/* remote address */
	XBBool read;				/* read flag */
	XBBool write;				/* write flag */
	XBBool shutdown;			/* shutdown flag */
};

/* list of all sockets */
typedef struct
{
	size_t num;
	SOCKET fd[MAX_SOCKET];
	const XBSocket *socket[MAX_SOCKET];
} XBSocketMap;

/* modified sockaddr_gen, to deal with winsock bug */
typedef union sockaddr_genk
{
	struct sockaddr address;
	struct sockaddr_in addressIn;
	struct sockaddr_in6 addressIn6;
} sockaddr_genk;

/* modified INTERFACE_INFO using the modified sockaddr_gen */
typedef struct _interface_info_mod
{
	u_long iiFlags;				/* Interface flags */
	sockaddr_genk iiAddress;	/* Interface address */
	sockaddr_genk iiBroadcastAddress;	/* Broadcast address */
	sockaddr_genk iiNetmask;	/* Network mask */
} INTERFACE_INFO_MOD;

/* both INTERFACE_INFO versions */
typedef union
{
	INTERFACE_INFO ms;
	INTERFACE_INFO_MOD mod;
} XB_INTERFACE_INFO;

/*
 * local variables
 */
static XBSocketMap socketMap;	/* socket list */
static XBSocketInterface *inter = NULL;	/* interface list */
static size_t numInter = 0;		/* number of interfaces */
static short modeflags = 0;		/* interface detection mode */

/***************
 * local stuff *
 ***************/

 /**/ static void AsyncSelect (const XBSocket * pSocket);

/*
 * delete list with all interfaces
 */
static void
DeleteInterfaces (void)
{
	if (NULL != inter) {
		size_t i;
		for (i = 0; i < numInter; i++) {
			if (NULL != inter[i].name) {
				free (inter[i].name);
			}
			if (NULL != inter[i].addrDevice) {
				free (inter[i].addrDevice);
			}
			if (NULL != inter[i].addrBroadcast) {
				free (inter[i].addrBroadcast);
			}
		}
		free (inter);
	}
	inter = NULL;
	numInter = 0;
}								/* DeleteInterfaces */

/*
 * get inet address from string, host byte order
 */
static unsigned long
GetAddressInet (const char *hostName)
{
	long addr;
	struct hostent *serverEnt;
	assert (hostName != NULL);
	/* check if ip string */
	if (-1L != (addr = inet_addr (hostName))) {
		return ntohl (addr);
	}
	/* lookup hostname if not an ip string */
	if (NULL != (serverEnt = gethostbyname (hostName))) {
		return ntohl (*(long *)serverEnt->h_addr_list[0]);
	}
	/* lookup failed */
	return 0L;
}								/* GetAddressInet */

/***************************
 * general socket managing *
 ***************************/

/*
 * initialize winsock
 */
XBBool
Socket_Init (void)
{
	WSADATA wsaData;
	memset (&socketMap, 0, sizeof (XBSocketMap));
	if (0 != WSAStartup (WINSOCK_VERSION, &wsaData)) {
		GUI_ErrorMessage ("winsock startup failed - unknown version\n");
		return XBFalse;
	}
	if (wsaData.wVersion != WINSOCK_VERSION) {
		GUI_ErrorMessage ("winsock startup failed - wrong version (2.0 required)\n");
		WSACleanup ();
		return XBFalse;
	}
	return XBTrue;
}								/* Socket_Init */

/*
 * add socket to list
 */
static void
SocketMapAdd (XBSocketMap * map, const XBSocket * pSocket)
{
	assert (NULL != socket);
	assert (NULL != map);
	assert (map->num < MAX_SOCKET);
	Dbg_Socket ("add fd=%u at map position #%u\n", pSocket->fd, map->num);
	map->fd[map->num] = pSocket->fd;
	map->socket[map->num] = pSocket;
	map->num++;
}								/* SocketMapAdd */

/*
 * subtract socket from set
 */
static void
SocketMapSubtract (XBSocketMap * map, const XBSocket * pSocket)
{
	size_t i;

	assert (NULL != socket);
	assert (NULL != map);
	assert (map->num > 0);
	for (i = 0; i < map->num; i++) {
		if (map->socket[i] == pSocket) {
			Dbg_Socket ("sub fd=%u from map pos #%u (total %u)\n", pSocket->fd, i, map->num);
			map->num--;
			if (i < map->num) {
				map->fd[i] = map->fd[map->num];
				map->socket[i] = map->socket[map->num];
			}
		}
	}
}								/* SocketMapSubtract */

/*
 * find socket to fd
 */
static const XBSocket *
SocketMapFind (XBSocketMap * map, SOCKET fd)
{
	size_t i;

	assert (NULL != socket);
	assert (NULL != map);
	for (i = 0; i < map->num; i++) {
		if (map->fd[i] == fd) {
			return map->socket[i];
		}
	}
	return NULL;
}								/* SocketMapFind */

/*
 * shutdown winsock
 */
void
Socket_Finish (void)
{
	Dbg_Socket ("shutting down winsock\n");
	DeleteInterfaces ();
	WSACleanup ();
}								/* Socket_Finish */

/***************
 * socket data *
 ***************/

/*
 * return file descriptor for socket
 */
int
Socket_Fd (const XBSocket * pSocket)
{
	assert (NULL != pSocket);
	return pSocket->fd;
}								/* Socket_Fd */

/*
 * return adress family for socket
 */
int
Socket_Family (const XBSocket * pSocket)
{
	assert (NULL != pSocket);

	return pSocket->sock.addr->sa_family;
}								/* Socket_Family */

/*
 * get host name of client
 */
const char *
Socket_HostName (const XBSocket * pSocket, XBBool peer)
{
	const XBSocketAddress *sa;
	struct sockaddr_in *inetAddr;
	assert (NULL != pSocket);
	/* determine address struct */
	sa = peer ? &pSocket->peer : &pSocket->sock;
	assert (NULL != sa);
	assert (NULL != sa->addr);
	/* reconstruct address */
	inetAddr = (struct sockaddr_in *)sa->addr;
	return inet_ntoa (inetAddr->sin_addr);
}								/* Socket_HostName */

/*
 * get port of host
 */
unsigned
Socket_HostPort (const XBSocket * pSocket, XBBool peer)
{
	const XBSocketAddress *sa;
	struct sockaddr_in *inetAddr;
	assert (NULL != pSocket);
	/* determine address struct */
	sa = peer ? &pSocket->peer : &pSocket->sock;
	assert (NULL != sa);
	assert (NULL != sa->addr);
	/* reconstruct port */
	inetAddr = (struct sockaddr_in *)sa->addr;
	return ntohs (inetAddr->sin_port);
}								/* Socket_HostPort */

/*******************
 * socket creation *
 *******************

/*
 * create socket structure
 */
XBSocket *
Socket_Alloc (int family)
{
	int len;
	XBSocket *pSocket;
	/* require AF_INET family */
	switch (family) {
	case AF_INET:
		len = sizeof (struct sockaddr_in);
		break;
	default:
		Dbg_Socket ("AF_INET family required, socket not created\n");
		return NULL;
	}
	/* alloc socket data structure */
	pSocket = calloc (1, sizeof (XBSocket));
	assert (NULL != pSocket);
	pSocket->fd = -1;
	pSocket->read = XBFalse;
	pSocket->write = XBFalse;
	pSocket->shutdown = XBFalse;
	/* out address */
	pSocket->sock.len = len;
	pSocket->sock.addr = calloc (1, len);
	assert (NULL != pSocket->sock.addr);
	pSocket->sock.addr->sa_family = family;
	/* other addresse */
	pSocket->peer.len = len;
	pSocket->peer.addr = calloc (1, len);
	assert (NULL != pSocket->peer.addr);
	pSocket->peer.addr->sa_family = family;
	/* that's all */
	return pSocket;
}								/* Socket_Alloc */

/*
 * free socket structure memory
 */
void
Socket_Free (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (NULL != pSocket->sock.addr) {
		free (pSocket->sock.addr);
	}
	if (NULL != pSocket->peer.addr) {
		free (pSocket->peer.addr);
	}
	free (pSocket);
}								/* Socket_Free */

/*
 * set socket adress
 */
XBBool
Socket_SetAddressInet (XBSocket * pSocket, XBBool peer, const char *hostName, unsigned short port)
{
	XBSocketAddress *sa;
	unsigned long addr;
	struct sockaddr_in *serverAddr;
	assert (NULL != pSocket);
	/* determine address structure to set */
	sa = peer ? &pSocket->peer : &pSocket->sock;
	/* get address in host byte order */
	if (NULL != hostName) {
		if (0 == (addr = GetAddressInet (hostName))) {
			Dbg_Socket ("failed to set address %s:%u for fd=%u\n", hostName, port, pSocket->fd);
			return XBFalse;
		}
	}
	else {
		addr = INADDR_ANY;
	}
	/* now set the address */
	assert (NULL != sa);
	memset (sa->addr, 0, sa->len);
	serverAddr = (struct sockaddr_in *)sa->addr;
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_addr.s_addr = htonl (addr);
	serverAddr->sin_port = htons (port);
	Dbg_Socket ("set %s address %s:%u for fd=%u\n", peer ? "remote" : "local", hostName, port,
				pSocket->fd);
	return XBTrue;
}								/* Socket_SetAddressInet */

/*
 * set broadcast option for socket
 */
XBBool
Socket_SetBroadcast (XBSocket * pSocket, XBBool enable)
{
	BOOL flag = enable ? TRUE : FALSE;
	assert (NULL != pSocket);
	return (0 == setsockopt (pSocket->fd, SOL_SOCKET, SO_BROADCAST, (void *)&flag, sizeof (flag)));
}								/* Socket_SetBroadcast */

/*
 * set socket option reuse
 */
XBBool
Socket_SetReuse (XBSocket * pSocket)
{
#if 0
	/* TODO: implement for windows */
	int so_reuseaddr = 1;
	if (-1 ==
		setsockopt (pSocket->fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof (so_reuseaddr))) {
		return XBFalse;
	}
	/* that's all */
#endif
	return XBTrue;
}								/* Socket_SetReuse */

/*
 * register read socket for event handling
 */
void
Socket_RegisterRead (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (!pSocket->read) {
		pSocket->read = XBTrue;
		AsyncSelect (pSocket);
	}
}								/* Socket_RegisterRead */

/*
 * register read socket for event handling
 */
void
Socket_RegisterWrite (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (!pSocket->write) {
		pSocket->write = XBTrue;
		AsyncSelect (pSocket);
	}
}								/* Socket_RegisterWrite */

/*
 * register read socket for event handling
 */
void
Socket_UnregisterRead (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (pSocket->read) {
		pSocket->read = XBFalse;
		AsyncSelect (pSocket);
	}
}								/* Socket_UnregisterRead */

/*
 * register read socket for event handling
 */
void
Socket_UnregisterWrite (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (pSocket->write) {
		pSocket->write = XBFalse;
		AsyncSelect (pSocket);
	}
}								/* Socket_UnregisterWrite */

/*************
 * BSD calls *
 *************

/*
 * open socket
 */
XBBool
Socket_Open (XBSocket * pSocket, int type)
{
	assert (pSocket != NULL);
	/* request a socket from system */
	if (-1 == (pSocket->fd = socket (pSocket->sock.addr->sa_family, type, 0))) {
#ifndef WMS
		Dbg_Socket ("failed to open socket of type=%i, err=%u\n", type, WSAGetLastError ());
#endif
		pSocket->shutdown = XBTrue;
		return XBFalse;
	}
	SocketMapAdd (&socketMap, pSocket);
	Dbg_Socket ("open socket fd=%u (type=%i)\n", pSocket->fd, type);
	return XBTrue;
}								/* Socket_Open */

/*
 * close socket
 */
void
Socket_Close (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (pSocket->fd >= 0) {
		if (!pSocket->shutdown) {
			SocketMapSubtract (&socketMap, pSocket);
			pSocket->shutdown = XBTrue;
		}
		closesocket (pSocket->fd);
		Dbg_Socket ("socket fd=%u closed\n", pSocket->fd);
	}
}								/* Socket_Close */

/*
 * close write access
 */
void
Socket_ShutdownWrite (XBSocket * pSocket)
{
	assert (NULL != pSocket);

	if (pSocket->fd >= 0) {
		if (!pSocket->shutdown) {
			SocketMapSubtract (&socketMap, pSocket);
			pSocket->shutdown = XBTrue;
		}
		shutdown (pSocket->fd, SD_SEND);
		Dbg_Socket ("socket fd=%u shutdown write\n", pSocket->fd);
	}
}								/* Socket_ShutdownWrite */

/*
 * connect a socket
 */
XBBool
Socket_Connect (XBSocket * pSocket)
{
	assert (pSocket != NULL);
	/* connect */
	if (-1 == connect (pSocket->fd, pSocket->peer.addr, pSocket->peer.len)) {
#ifndef WMS
		Dbg_Socket ("failed to connect socket fd=%u to %s:%u, err=%u\n", pSocket->fd,
					Socket_HostName (pSocket, XBTrue), Socket_HostPort (pSocket, XBTrue),
					WSAGetLastError ());
#endif
		return XBFalse;
	}
	/* now get adress assigned by kernel. the cast to void* is needed since not all systems know socklen_t */
	if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *)&pSocket->sock.len)) {
#ifndef WMS
		Dbg_Socket ("failed to get local address of socket fd=%u after connecting, err=%u\n",
					pSocket->fd, WSAGetLastError ());
#endif
		return XBFalse;
	}
	Dbg_Socket ("socket fd=%u connected to %s:%u\n", pSocket->fd, Socket_HostName (pSocket, XBTrue),
				Socket_HostPort (pSocket, XBTrue));
	return XBTrue;
}								/* Socket_Connect */

/*
 * bind a socket
 */
XBBool
Socket_Bind (XBSocket * pSocket)
{
	/* bind to port */
	if (-1 == bind (pSocket->fd, pSocket->sock.addr, pSocket->sock.len)) {
#ifndef WMS
		Dbg_Socket ("failed to bind socket fd=%u, err=%u\n", pSocket->fd, WSAGetLastError ());
#endif
		return XBFalse;
	}
	/* now get adress assigned by kernel. the cast to void* is needed since not all systems know socklen_t */
	if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *)&pSocket->sock.len)) {
#ifndef WMS
		Dbg_Socket ("failed to get local address of socket fd=%u after binding, err=%u\n",
					pSocket->fd, WSAGetLastError ());
#endif
		return XBFalse;
	}
	Dbg_Socket ("socket fd=%u bound to %s:%u\n", pSocket->fd, Socket_HostName (pSocket, XBFalse),
				Socket_HostPort (pSocket, XBFalse));
	/* that's all */
	return XBTrue;
}								/* Socket_Bind */

/*
 * accept a socket
 */
XBBool
Socket_Accept (XBSocket * pSocket, const XBSocket * pListen)
{
	assert (pSocket != NULL);
	assert (pListen != NULL);
	/* try to accept */
	if (-1 == (pSocket->fd = accept (pListen->fd, pSocket->peer.addr, (void *)&pSocket->peer.len))) {
#ifndef WMS
		Dbg_Socket ("failed to accept from socket fd=%u, err=%u\n", pListen->fd,
					WSAGetLastError ());
#endif
		return XBFalse;
	}
	/* now retrieve local adress */
	if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *)&pSocket->sock.len)) {
#ifndef WMS
		Dbg_Socket ("failed to get local address from accepted socket fd=%u, err=%u\n", pListen->fd,
					WSAGetLastError ());
#endif
		return XBFalse;
	}
	/* add the new socket */
	SocketMapAdd (&socketMap, pSocket);
	Dbg_Out ("accepted socket %d from socket fd=%u\n", pSocket->fd, pListen->fd);
	/* that's all */
	return XBTrue;
}								/* Socket_Accept */

/*
 * create listen socket
 */
XBBool
Socket_Listen (XBSocket * pSocket)
{
	assert (pSocket != NULL);
	/* now listen for client to connect */
	if (0 != listen (pSocket->fd, LISTENQ)) {
#ifndef WMS
		Dbg_Socket ("failed to listen on socket fd=%u (%s:%u), err=%u\n", pSocket->fd,
					Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse),
					WSAGetLastError ());
#endif
		return XBFalse;
	}
	Dbg_Socket ("listening on socket fd=%u (%s:%u)\n", pSocket->fd,
				Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse));
	return XBTrue;
}								/* Socket_Listen */

/*
 * write to socket, non blocking i/o assumed
 */
int
Socket_Send (const XBSocket * pSocket, const void *buf, size_t len)
{
	int result;
	assert (NULL != pSocket);
	assert (NULL != buf);
	/* try to write */
	result = send (pSocket->fd, buf, len, 0);
	if (result < 0) {
		int err = WSAGetLastError ();
		if (err == WSAEWOULDBLOCK) {
			Dbg_Socket ("send on fd=%d would block\n", pSocket->fd);
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Out ("send error %d on fd=%u\n", err, pSocket->fd);
			return XB_SOCKET_ERROR;
		}
	}
	Dbg_Socket ("sent %u bytes on fd=%u\n", result, pSocket->fd);
	return result;
}								/* Socket_Send */

/*
 * write to socket, given target, non blocking i/o assumed
 */
int
Socket_SendTo (XBSocket * pSocket, const void *buf, size_t len, const char *host,
			   unsigned short port)
{
	int result;
	assert (NULL != pSocket);
	assert (NULL != buf);
	assert (NULL != host);
	/* convert destination adress */
	if (!Socket_SetAddressInet (pSocket, XBTrue, host, port)) {
		Dbg_Socket ("failed to send on fd=%u - failed to resolve %s:%u", pSocket->fd, host, port);
		return -1;
	}
	/* now try to write data */
	result = sendto (pSocket->fd, buf, len, 0, pSocket->peer.addr, pSocket->peer.len);
	if (result < 0) {
		int err = WSAGetLastError ();
		if (err == WSAEWOULDBLOCK) {
			Dbg_Socket ("sendto on fd=%u would block\n", pSocket->fd);
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Out ("sendto error on fd=%u\n", err);
			return XB_SOCKET_ERROR;
		}
	}
	Dbg_Socket ("sent %u bytes on fd=%u to %s:%u\n", result, pSocket->fd, host, port);
	return result;
}								/* Socket_SendTo */

/*
 * read from socket
 */
int
Socket_Receive (const XBSocket * pSocket, void *buf, size_t len)
{
	int result;
	assert (NULL != pSocket);
	assert (NULL != buf);
	/* try to read */
	result = recv (pSocket->fd, buf, len, 0);
	if (result < 0) {
		int err = WSAGetLastError ();
		if (err == WSAEWOULDBLOCK) {
			Dbg_Out ("receive on fd=%u would block\n", pSocket->fd);
			if (pSocket->read) {
				AsyncSelect (pSocket);
			}
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Socket ("receive error %u on fd=%u\n", err, pSocket->fd);
			return XB_SOCKET_ERROR;
		}
	}
	Dbg_Socket ("received %u bytes on fd=%u\n", result, pSocket->fd);
	return result;
}								/* Socket_Receive */

/*
 * read from socket, get sender
 */
int
Socket_ReceiveFrom (XBSocket * pSocket, void *buf, size_t len, const char **host,
					unsigned short *port)
{
	long numRead;
	assert (NULL != pSocket);
	assert (NULL != buf);
	assert (NULL != host);
	/* try to read */
	numRead = recvfrom (pSocket->fd, buf, len, 0, pSocket->peer.addr, &pSocket->peer.len);
	if (numRead >= 0) {
		*host = Socket_HostName (pSocket, XBTrue);
		*port = Socket_HostPort (pSocket, XBTrue);
	}
	else {
		int err = WSAGetLastError ();
		*host = NULL;
		*port = 0;
		if (err == WSAEWOULDBLOCK) {
			Dbg_Socket ("socket receivefrom on fd=%u would block\n", pSocket->fd);
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Socket ("receivefrom error %u on fd=%u\n", err, pSocket->fd);
			return XB_SOCKET_ERROR;
		}
	}
	Dbg_Socket ("received %u bytes on fd=%u from %s:%u\n", numRead, pSocket->fd, *host, *port);
	return numRead;
}								/* Net_ReceiveFrom */

/*****************
 * socket events *
 *****************/

/*
 * handle selection events, needed in w32_event.c
 */
void
HandleSelect (UINT wParam, LONG lParam)
{
	SOCKET fd = wParam;
	UINT event = WSAGETSELECTEVENT (lParam);
	const XBSocket *pSocket;

	switch (event) {
	case FD_READ:
	case FD_ACCEPT:
	case FD_CLOSE:
		Dbg_Socket ("socket fd=%u readable\n", fd);
		CommReadable (fd);
		break;
	case FD_WRITE:
		Dbg_Socket ("socket fd=%u writeable\n", fd);
		CommWriteable (fd);
		if (NULL != (pSocket = SocketMapFind (&socketMap, fd)) && pSocket->write) {
			AsyncSelect (pSocket);
		}
		break;
	default:
		Dbg_Socket ("unknown select event %04x\n", event);
		break;
	}
}								/* HandleSelect */

/*
 * trigger async select
 */
static void
AsyncSelect (const XBSocket * pSocket)
{
	long event = 0;
	assert (NULL != pSocket);
	Dbg_Socket ("async select on fd=%u, flags=%c%c\n", pSocket->fd, pSocket->read ? 'R' : '-',
				pSocket->write ? 'W' : '-');
	if (pSocket->read) {
		event |= (FD_READ | FD_ACCEPT | FD_CLOSE);
	}
	if (pSocket->write) {
		event |= FD_WRITE;
	}
	WSAAsyncSelect (pSocket->fd, window, MSG_XBLAST_SELECT, event);
}								/* AsyncSelect */

/**************
 * interfaces *
 **************/

/*
 * get all available interfaces
 */
const XBSocketInterface *
Socket_GetInterfaces (size_t * num)
{
	SOCKET fd = SOCKET_ERROR;
	DWORD nr, len;
	SOCKADDR_IN *pAddrInet = NULL;
	SOCKADDR_IN *pAddrNetmask = NULL;
	SOCKADDR_IN *pAddrBroadcast = NULL;
	u_long iFlags = 0;
	void *info = NULL;
	INTERFACE_INFO *ii = NULL;
	INTERFACE_INFO_MOD *iimod = NULL;
	DWORD iLen = 10;

	/* clean up */
	DeleteInterfaces ();
	/* get test udp socket for interface detection */
	if (SOCKET_ERROR == (fd = socket (AF_INET, SOCK_DGRAM, 0))) {
		Dbg_Socket ("failed to open udp socket for interface detection\n");
		goto Error;
	}
	/* allocate space for iLen interfaces */
	info = calloc (iLen, sizeof (XB_INTERFACE_INFO));
	assert (NULL != info);
	/* start scanning */
	Dbg_Socket ("guessing %u interfaces\n", iLen);
	while (SOCKET_ERROR ==
		   WSAIoctl (fd, SIO_GET_INTERFACE_LIST, NULL, 0, info, iLen * sizeof (XB_INTERFACE_INFO),
					 &len, NULL, NULL)) {
		if (WSAEFAULT != WSAGetLastError ()) {
			Dbg_Socket ("ioctl failed, err=%u\n", WSAGetLastError ());
			goto Error;
		}
		/* more space needed */
		free (info);
		iLen += 10;
		info = calloc (iLen, sizeof (XB_INTERFACE_INFO));
		assert (NULL != info);
		Dbg_Socket ("increasing guess to %u\n", iLen);
	}
	/* determine interface count */
	modeflags |= XBIF_MS & (len % sizeof (INTERFACE_INFO) == 0);
	modeflags |= XBIF_MOD & (len % sizeof (INTERFACE_INFO_MOD) == 0);
	switch (modeflags) {
	case XBIF_MS:
		len /= sizeof (INTERFACE_INFO);
		Dbg_Socket ("using MS mode!\n");
		break;
	case XBIF_MOD:
		len /= sizeof (INTERFACE_INFO_MOD);
		Dbg_Socket ("using modified MS mode!\n");
		break;
	default:
		*num = 0;
		Dbg_Socket ("no matching mode, interface detection unreliable!\n");
		goto Error;
	}
	Dbg_Socket ("interfaces detected = %u\n", len);
	/* allocate output buffer */
	inter = calloc (len, sizeof (XBSocketInterface));
	assert (NULL != inter);
	Dbg_Socket ("allocated %u interface descriptions\n", len);
	/* create interface list */
	numInter = 0;
	for (nr = 0; nr < len; nr++) {
		Dbg_Out ("### interface %u ###\n", nr);
		/* get flags and addresses */
		switch (modeflags) {
		case XBIF_MS:
			ii = (INTERFACE_INFO *) info;
			pAddrInet = (SOCKADDR_IN *) & ii[nr].iiAddress;
			pAddrNetmask = (SOCKADDR_IN *) & ii[nr].iiNetmask;
			pAddrBroadcast = (SOCKADDR_IN *) & ii[nr].iiBroadcastAddress;
			iFlags = ii[nr].iiFlags;
			break;
		case XBIF_MOD:
			iimod = (INTERFACE_INFO_MOD *) info;
			pAddrInet = (SOCKADDR_IN *) & iimod[nr].iiAddress;
			pAddrNetmask = (SOCKADDR_IN *) & iimod[nr].iiNetmask;
			pAddrBroadcast = (SOCKADDR_IN *) & iimod[nr].iiBroadcastAddress;
			iFlags = iimod[nr].iiFlags;
			break;
		default:
			break;
		}
		/* require INET family, up and broadcast */
		if (pAddrInet->sin_family == AF_INET &&
			pAddrInet->sin_addr.s_addr != INADDR_ANY && (iFlags & (IFF_UP | IFF_BROADCAST))) {
			struct in_addr bcAddr = pAddrInet->sin_addr;
			bcAddr.s_addr |= ~pAddrNetmask->sin_addr.s_addr;
			/* set interface data */
			inter[numInter].name = DupString ("net");
			inter[numInter].addrDevice = DupString (inet_ntoa (pAddrInet->sin_addr));
			inter[numInter].addrBroadcast = DupString (inet_ntoa (bcAddr));
			/* output data */
			Dbg_Out ("IP = %s\n", inter[numInter].addrDevice);
			Dbg_Out ("NET= %s\n", inet_ntoa (pAddrNetmask->sin_addr));
			Dbg_Out ("GBC= %s\n", inet_ntoa (pAddrBroadcast->sin_addr));
			Dbg_Out ("BC = %s\n", inter[numInter].addrBroadcast);
			numInter++;
		}
		else {
			/* output interface data */
			Dbg_Out ("IP = %s\n", inet_ntoa (pAddrInet->sin_addr));
			Dbg_Out ("NET= %s\n", inet_ntoa (pAddrNetmask->sin_addr));
			Dbg_Out ("GBC= %s\n", inet_ntoa (pAddrBroadcast->sin_addr));
			Dbg_Out ("skipping interface\n");
		}
	}
	/* clean up */
	free (info);
	closesocket (fd);
	/* that's all */
	*num = numInter;
	return inter;

  Error:
	if (NULL != info) {
		free (info);
	}
	if (SOCKET_ERROR != fd) {
		closesocket (fd);
	}
	return NULL;
}								/* Socket_GetInterfaces */

/*
 * return auto interface for given target and socket
 */
const char *
Socket_GetAutoInterface (XBSocket * pSocket, const char *trg)
{
	SOCKADDR_IN inaddr_trg;
	SOCKADDR_IN inaddr_if;
	DWORD ret = 0;
	const char *ifname;
	/* construct target address */
	inaddr_trg.sin_family = AF_INET;
	inaddr_trg.sin_addr.s_addr = inet_addr (trg);
	/* ioctl to get interface address */
	if (SOCKET_ERROR == WSAIoctl (pSocket->fd, SIO_ROUTING_INTERFACE_QUERY,
								  &inaddr_trg, sizeof (SOCKADDR_IN),
								  &inaddr_if, sizeof (SOCKADDR_IN), &ret, NULL, NULL)) {
		Dbg_Socket ("failed to find auto-interface, ioctl error %u\n", WSAGetLastError ());
		ifname = NULL;
	}
	else {
		ifname = inet_ntoa (inaddr_if.sin_addr);
	}
	return ifname;
}								/* Socket_GetAutoInterface */

/*
 * end of file w32_socket.c
 */
