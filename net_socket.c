/*
 * net_socket.c - low level functions to network communication
 *
 * $Id: net_socket.c,v 1.10 2006/03/28 11:41:19 fzago Exp $
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

/* make sure that port range defines are in place */
#ifndef USER_MINPORT
#define USER_MINPORT 0
#endif
#ifndef USER_MAXPORT
#define USER_MAXPORT 0
#endif

/*
 * close and delete socket structure
 */
void
Net_Close (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	Socket_Close (pSocket);
	Socket_Free (pSocket);
}								/* Net_Close */

/*
 * bind a socket to a user port on given interface
 */
static XBBool
Net_BindUserPort (XBSocket * pSocket, const char *localname)
{
	unsigned short port;
	/* make sure user requests a port range */
	assert (USER_MINPORT < USER_MAXPORT);
	/* find a free port */
	for (port = USER_MINPORT; port <= USER_MAXPORT; port++) {
		if (Socket_SetAddressInet (pSocket, XBFalse, localname, port) && Socket_Bind (pSocket)) {
			return XBTrue;
		}
	}
	Dbg_Out ("NET: failed to bind to user port range\n");
	return XBFalse;
}								/* Net_BindUserPort */

/*
 * try to connect to server (via TCP/IP)
 */
XBSocket *
Net_ConnectInet (const char *hostName, unsigned short port)
{
	XBSocket *pSocket = NULL;

	/* create socket structure */
	pSocket = Socket_Alloc (AF_INET);
	/* create socket */
	if (!Socket_Open (pSocket, SOCK_STREAM)) {
		goto Error;
	}
	if (!Socket_SetAddressInet (pSocket, XBTrue, hostName, port)) {
		goto Error;
	}
	/* bind to user port range locally, if requested */
	if (USER_MINPORT < USER_MAXPORT && !Net_BindUserPort (pSocket, NULL)) {
		goto Error;
	}
	/* now try to connect */
	if (!Socket_Connect (pSocket)) {
		goto Error;
	}
	/* that's all */
	return pSocket;
	/*
	 * Error handling
	 */
  Error:
	Net_Close (pSocket);
	return NULL;
}								/* Net_ConnectInet */

/*
 * listen for client to connect
 */
XBSocket *
Net_ListenInet (unsigned short port)
{
	XBSocket *pSocket = NULL;

	/* alloc socket data structure */
	pSocket = Socket_Alloc (AF_INET);
	/* create socket */
	if (!Socket_Open (pSocket, SOCK_STREAM)) {
		goto Error;
	}
	/* set socket address structure */
	if (!Socket_SetAddressInet (pSocket, XBFalse, NULL, port)) {
		goto Error;
	}
	/* enable reuse */
	if (!Socket_SetReuse (pSocket)) {
		goto Error;
	}
	/* bind to set address */
	if (!Socket_Bind (pSocket)) {
		goto Error;
	}
	/* now activate listen socket */
	if (!Socket_Listen (pSocket)) {
		goto Error;
	}
	/* that's all */
	return pSocket;
	/*
	 * Error handling
	 */
  Error:
	Net_Close (pSocket);
	return NULL;
}								/* Net_ListenInet */

/*
 * accept client connection
 */
XBSocket *
Net_Accept (const XBSocket * pListen)
{
	XBSocket *pSocket;

	/* alloc Socket data structure */
	pSocket = Socket_Alloc (Socket_Family (pListen));
	assert (pSocket != NULL);
	/* try to accept client */
	if (!Socket_Accept (pSocket, pListen)) {
		goto Error;
	}
	/* that's all */
	return pSocket;
	/*
	 * Error handling
	 */
  Error:
	Net_Close (pSocket);
	return NULL;
}								/* Net_AcceptInet */

/*
 * create datagram socket for host
 */
XBSocket *
Net_BindUdp (const char *localname, unsigned port)
{
	XBSocket *pSocket;

	/* create socket structure */
	pSocket = Socket_Alloc (AF_INET);
	assert (pSocket != NULL);
	/* create socket */
	if (!Socket_Open (pSocket, SOCK_DGRAM)) {
		goto Error;
	}
	/* bind socket */
	if (USER_MINPORT < USER_MAXPORT && port == 0) {
		/* any port, use user port range */
		if (!Net_BindUserPort (pSocket, localname)) {
			goto Error;
		}
	}
	else {
		/* bind with given port, let system choose, if necessary */
		if (!Socket_SetAddressInet (pSocket, XBFalse, localname, port)) {
			goto Error;
		}
		if (!Socket_Bind (pSocket)) {
			goto Error;
		}
	}
	/* that's all */
	return pSocket;
	/*
	 * Error handling
	 */
  Error:
	Net_Close (pSocket);
	return NULL;
}								/* Net_BindInet */

/*
 * connect datagram socket to port
 */
XBBool
Net_ConnectUdp (XBSocket * pSocket, const char *hostName, unsigned short port)
{
	/* set it */
	if (!Socket_SetAddressInet (pSocket, XBTrue, hostName, port)) {
		return XBFalse;
	}
	/* now try to connect */
	if (!Socket_Connect (pSocket)) {
		return XBFalse;
	}
	return XBTrue;
}								/* Net_ConnectUdp */

/*
 * get local host name of socket
 */
const char *
Net_LocalName (const XBSocket * pSocket)
{
	const char *name;
	static char hostName[32];

	assert (pSocket != NULL);
	if (NULL == (name = Socket_HostName (pSocket, XBFalse))) {
		return NULL;
	}
	assert (sizeof (hostName) > strlen (name));
	strcpy (hostName, name);
	return hostName;
}								/* Net_Hostname */

/*
 * get remote host name of socket
 */
const char *
Net_RemoteName (const XBSocket * pSocket)
{
	const char *name;
	static char hostName[32];

	assert (pSocket != NULL);
	if (NULL == (name = Socket_HostName (pSocket, XBTrue))) {
		return NULL;
	}
	assert (sizeof (hostName) > strlen (name));
	strcpy (hostName, name);
	return hostName;
}								/* Net_RemoteName */

/*
 * get local port of socket
 */
unsigned
Net_LocalPort (const XBSocket * pSocket)
{
	assert (pSocket != NULL);
	return Socket_HostPort (pSocket, XBFalse);
}								/* Net_LocalPort */

/*
 * get remote port of socket
 */
unsigned
Net_RemotePort (const XBSocket * pSocket)
{
	assert (pSocket != NULL);
	return Socket_HostPort (pSocket, XBTrue);
}								/* Net_RemotePort */

/*
 * end of file net_socket.c
 */
