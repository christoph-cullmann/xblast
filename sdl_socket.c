/*
 * file x11_socket.c - true bsd sockets for xblast
 *
 * $Id: sdl_socket.c 112466 2009-07-06 08:37:37Z ingmar $
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

#include "socket.h"
#include "sdl_socket.h"

//#include "x11_common.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#if defined (sun)
#include <sys/sockio.h>
#endif

/*
 * local constants
 */
#define CONNECT_TIMEOUT 60
#define ACCEPT_TIMEOUT  30
#define LISTENQ          5
#ifndef SHUT_WR
#define SHUT_WR          1
#endif
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT     0
#endif

/*
 * type definitions
 */

/* socket address */
typedef struct _xb_socket_address
{
	socklen_t len;
	struct sockaddr *addr;
} XBSocketAddress;

/* fix for suns */
#ifdef sun
typedef uint32_t u_int32_t;
#endif

/* socket data */
struct _xb_socket
{
	int fd;						/* file descriptor of socket */
	XBSocketAddress sock;		/* local address */
	XBSocketAddress peer;		/* remote address */
	XBBool shutDown;			/* shutodwn flag */
};

/*
 * local variables
 */
static int socketMax = 0;		/* current max */
static fd_set socketReadSet;	/* current read set */
static fd_set socketWriteSet;	/* current write set */

/* interface list */
static XBSocketInterface *inter = NULL;
static size_t numInter = 0;

/*
 * local stuff
 */

/*
 * delete list with all interfaces
 */
static void
DeleteInterfaces (void)
{
	if (NULL != inter) {
		int i;
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
 * get inet address
 */
static u_int32_t
GetAddressInet (const char *hostName)
{
	int32_t addr;
	struct hostent *serverEnt;
	assert (hostName != NULL);
	/* check if ip string */
	if (-1L != (addr = inet_addr (hostName))) {
		return ntohl (addr);
	}
	/* lookup hostname if not an ip string */
	if (NULL != (serverEnt = gethostbyname (hostName))) {
		return ntohl (*(int32_t *) serverEnt->h_addr_list[0]);
	}
	/* lookup failed */
	return 0L;
}								/* GetAddressInet */

/*
 * handler for SIGCHLD
 */
static void
HandleSigChld (int sig_num)
{
	int stat;
	pid_t child;

	while (0 < (child = waitpid (-1, &stat, WNOHANG))) {
		Dbg_Out ("child %d terminated\n", child);
	}
}								/* HandleSigChld */

/**************************
 * local socket debugging *
 **************************/

#ifdef DEBUG_SOCKET
/*
 * print out an fdset
 */
static void
DebugFdSet (const char *header, fd_set * set)
{
	int i;

	Dbg_Socket ("%s:", header);
	for (i = 0; i <= socketMax; i++) {
		if (FD_ISSET (i, set)) {
			fprintf (stderr, " %d", i);
		}
	}
	fprintf (stderr, "\n");
}								/* DebugFdSet */

/*
 * print out interface config
 */
static void
DebugInterfaceData (const struct ifconf *ifConf)
{
	size_t i;
	Dbg_Socket ("Interface Config (%u bytes)", ifConf->ifc_len);
	for (i = 0; i < ifConf->ifc_len; i++) {
		if (i % 8 == 0) {
			fprintf (stderr, "\n");
		}
		fprintf (stderr, "%02x ", (unsigned)(unsigned char)ifConf->ifc_buf[i]);
	}
	fprintf (stderr, "\n");
}								/* DebugInterfaceData */
#endif

/*********************
 * socket management *
 *********************/

/*
 * initialisation routine
 */
XBBool
Socket_Init (void)
{
	static XBBool initDone = XBFalse;
	if (!initDone) {
		signal (SIGCHLD, HandleSigChld);
		signal (SIGPIPE, SIG_IGN);
		signal (SIGALRM, SIG_IGN);
		initDone = XBTrue;
	}
	/* clear fd_Set for sockets */
	FD_ZERO (&socketReadSet);
	FD_ZERO (&socketWriteSet);
	Dbg_Socket ("initializing socket management\n");
	return XBTrue;
}								/* Socket_Init */

/*
 * cleaning up
 */
void
Socket_Finish (void)
{
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
	sa = peer ? &pSocket->peer : &pSocket->sock;
	assert (NULL != sa);
	assert (NULL != sa->addr);
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
	sa = peer ? &pSocket->peer : &pSocket->sock;
	assert (NULL != sa);
	assert (NULL != sa->addr);
	inetAddr = (struct sockaddr_in *)sa->addr;
	return ntohs (inetAddr->sin_port);
}								/* Socket_HostPort */

/*******************
 * socket creation *
 *******************/

/*
 * create socket structure
 */
XBSocket *
Socket_Alloc (int family)
{
	int len;
	XBSocket *pSocket;
	/* require AF_INET */
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
	/* set shutdown flags to false */
	pSocket->shutDown = XBFalse;
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
	u_int32_t addr;
	struct sockaddr_in *serverAddr;

	assert (NULL != pSocket);
	/* determine address structure to set */
	sa = peer ? &pSocket->peer : &pSocket->sock;
	/* get host name in host byte order */
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
 * enable or disable broadcast
 */
XBBool
Socket_SetBroadcast (XBSocket * pSocket, XBBool enable)
{
	int flag = enable ? 1 : 0;
	if (0 == setsockopt (pSocket->fd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag))) {
		Dbg_Socket ("successfully %s broadcast flag on socket fd=%u\n", enable ? "set" : "cleared",
					pSocket->fd);
		return XBTrue;
	}
	else {
		perror ("Socket_SetBroadcast");
		Dbg_Socket ("failed to %s broadcast flag on socket fd=%u\n", enable ? "set" : "cleared",
					pSocket->fd);
		return XBFalse;
	}
}								/* Socket_SetBroadcast */

/*
 * set socket option reuse
 */
XBBool
Socket_SetReuse (XBSocket * pSocket)
{
	int so_reuseaddr = 1;
	if (-1 ==
		setsockopt (pSocket->fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof (so_reuseaddr))) {
		return XBFalse;
	}
	/* that's all */
	return XBTrue;
}								/* Socket_SetReuse */

/*
 * register socket for reading
 */
void
Socket_RegisterRead (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (pSocket->fd > socketMax) {
		socketMax = pSocket->fd;
	}
	FD_SET (pSocket->fd, &socketReadSet);
#ifdef DEBUG_SOCKET
	DebugFdSet ("read fd_set", &socketReadSet);
#endif
}								/* Socket_RegisterRead */

/*
 * register socket for writing
 */
void
Socket_RegisterWrite (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	if (pSocket->fd > socketMax) {
		socketMax = pSocket->fd;
	}
	FD_SET (pSocket->fd, &socketWriteSet);
#ifdef DEBUG_SOCKET
	DebugFdSet ("Socket_RegisterWrite write fd_set", &socketWriteSet);
#endif
}								/* Socket_RegisterWrite */

/*
 * unregister socket for reading
 */
void
Socket_UnregisterRead (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	FD_CLR (pSocket->fd, &socketReadSet);
#ifdef DEBUG_SOCKET
	DebugFdSet ("read fd_set", &socketReadSet);
#endif
}								/* socket_UnregisterRead */

/*
 * register socket for writing
 */
void
Socket_UnregisterWrite (XBSocket * pSocket)
{
	assert (NULL != pSocket);
	FD_CLR (pSocket->fd, &socketWriteSet);
#ifdef DEBUG_SOCKET
	DebugFdSet ("Socket_UnregisterWrite write fd_set", &socketWriteSet);
#endif
}								/* Socket_UnregisterWrite */

/*************
 * BSD calls *
 *************/

/*
 * open socket
 */
XBBool
Socket_Open (XBSocket * pSocket, int type)
{
	assert (pSocket != NULL);
	/* now create a stream socket */
	if (-1 == (pSocket->fd = socket (pSocket->sock.addr->sa_family, type, 0))) {
		perror ("Socket_Open");
		Dbg_Socket ("failed to open socket of type=%i", type);
		return XBFalse;
	}
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
	if (pSocket->fd < 0 || pSocket->shutDown) {
		return;
	}
	if (0 != close (pSocket->fd)) {
		perror ("Socket_Close");
		Dbg_Socket ("error while closing socket fd=%u\n", pSocket->fd);
	}
	else {
		pSocket->shutDown = XBTrue;
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
	if (pSocket->fd < 0 || pSocket->shutDown) {
		return;
	}
	if (0 != shutdown (pSocket->fd, SHUT_WR)) {
		perror ("Socket_ShutdownWrite");
		Dbg_Socket ("error while shutting down socket fd=%u: ", pSocket->fd);
	}
	else {
		Dbg_Socket ("socket fd=%u shutdown write\n", pSocket->fd);
		pSocket->shutDown = XBTrue;
	}
}								/* Socket_ShutdownWrite */

/*
 * connect a socket
 */
XBBool
Socket_Connect (XBSocket * pSocket)
{
	assert (pSocket != NULL);
	/* try to connect, set timeout */
	alarm (CONNECT_TIMEOUT);
	if (-1 == connect (pSocket->fd, pSocket->peer.addr, pSocket->peer.len)) {
		perror ("Socket_Connect");
		alarm (0);
		Dbg_Socket ("failed to connect socket fd=%u to %s:%u\n", pSocket->fd,
					Socket_HostName (pSocket, XBTrue), Socket_HostPort (pSocket, XBTrue));
		return XBFalse;
	}
	alarm (0);
	/* now get adress assigned by kernel. the cast to void* is needed since not all systems know socklen_t */
	if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *)&pSocket->sock.len)) {
		perror ("Socket_Connect(getsockname)");
		Dbg_Socket ("failed to get local address of socket fd=%u after connecting\n", pSocket->fd);
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
		perror ("Socket_Bind");
		Dbg_Socket ("failed to bind socket fd=%u\n", pSocket->fd);
		return XBFalse;
	}
	/* now get adress assigned by kernel. the cast to void* is needed since not all systems know socklen_t */
	if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *)&pSocket->sock.len)) {
		perror ("Socket_Bind(getsockname)");
		Dbg_Socket ("failed to get local address of socket fd=%u after binding\n", pSocket->fd);
		return XBFalse;
	}
	/* that's all */
	Dbg_Socket ("socket fd=%u bound to %s:%u\n", pSocket->fd, Socket_HostName (pSocket, XBFalse),
				Socket_HostPort (pSocket, XBFalse));
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
	/* try to accept, set timeout */
	alarm (ACCEPT_TIMEOUT);
	if (-1 == (pSocket->fd = accept (pListen->fd, pSocket->peer.addr, (void *)&pSocket->peer.len))) {
		perror ("Socket_Accept");
		alarm (0);
		Dbg_Socket ("failed to accept from socket fd=%u\n", pListen->fd);
		return XBFalse;
	}
	alarm (0);
	/* now retrieve local adresse */
	if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *)&pSocket->sock.len)) {
		perror ("Socket_Accept(getsockname)");
		Dbg_Socket ("failed to get local address from accepted socket fd=%u\n", pListen->fd);
		return XBFalse;
	}
	/* that's all */
	Dbg_Out ("accepted socket fd=%u from socket fd=%u\n", pSocket->fd, pListen->fd);
	return XBTrue;
}								/* Socket_Accept */

/*
 * listen on socket
 */
XBBool
Socket_Listen (XBSocket * pSocket)
{
	assert (pSocket != NULL);
	/* now listen for client to connect */
	if (0 != listen (pSocket->fd, LISTENQ)) {
		perror ("Socket_Listen");
		Dbg_Socket ("failed to listen on socket fd=%u (%s:%u)\n", pSocket->fd,
					Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse));
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
	result = send (pSocket->fd, buf, len, MSG_DONTWAIT);
	if (result < 0) {
		perror ("Socket_Send");
		if (EAGAIN == errno) {
			Dbg_Socket ("send on fd=%u would block\n", pSocket->fd);
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Socket ("send error on fd=%u\n", pSocket->fd);
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
	result = sendto (pSocket->fd, buf, len, MSG_DONTWAIT, pSocket->peer.addr, pSocket->peer.len);
	if (result < 0) {
		perror ("Socket_SendTo");
		if (EAGAIN == errno) {
			Dbg_Socket ("sendto on fd=%u would block\n", pSocket->fd);
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Socket ("sendto error on fd=%u\n", pSocket->fd);
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
		perror ("Socket_Receive");
		if (EAGAIN == errno) {
			Dbg_Socket ("receive on fd=%u would block\n", pSocket->fd);
			return XB_SOCKET_WOULD_BLOCK;
		}
		else {
			Dbg_Socket ("receive on fd=%u\n", pSocket->fd);
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
	ssize_t numRead;
	assert (NULL != pSocket);
	assert (NULL != buf);
	assert (NULL != host);
	/* try to read */
	numRead = recvfrom (pSocket->fd, buf, len, 0, pSocket->peer.addr, (void *)&pSocket->peer.len);

	if (numRead > 0) {
		*host = Socket_HostName (pSocket, XBTrue);
		*port = Socket_HostPort (pSocket, XBTrue);
	}
	else {
		perror ("Socket_ReceiveFrom");
		*host = NULL;
		*port = 0;
		if (numRead < 0) {
			if (EAGAIN == errno) {
				Dbg_Socket ("receivefrom on fd=%u would block\n", pSocket->fd);
				return XB_SOCKET_WOULD_BLOCK;
			}
			else {
				Dbg_Socket ("receivefrom on fd=%u\n", pSocket->fd);
				return XB_SOCKET_ERROR;
			}
		}
	}
	Dbg_Socket ("received %lu bytes on fd=%u from %s:%u\n", (unsigned long)numRead, pSocket->fd,
				*host, *port);
	return numRead;
}								/* Socket_ReceiveFrom */

/**************
 * interfaces *
 **************/

/*
 * Get list of all network interfaces
 */
static struct ifconf *
GetInterfaceConfig (int fd)
{
	size_t len, lastLen;
	char *buf = NULL;
	struct ifconf *ifconf;

	ifconf = malloc (sizeof (struct ifconf));
	assert (ifconf);

	lastLen = 0;

	for (len = 10 * sizeof (struct ifreq);; len += 10 * sizeof (struct ifreq)) {
		/* alloc buffer to receive data */
		buf = calloc (1, len);
		assert (NULL != buf);

		ifconf->ifc_len = len;
		ifconf->ifc_buf = buf;
		/* query list of interfaces */
		if (-1 == ioctl (fd, SIOCGIFCONF, ifconf)) {
			if (errno != EINVAL || lastLen != 0) {
				free (buf);
				free (ifconf);
				return NULL;
			}
		}
		else if (ifconf->ifc_len == lastLen) {
			/* success */
			return ifconf;
		}
		else {
			/* net new length */
			lastLen = ifconf->ifc_len;
		}
		/* next guess */
		free (buf);
	}

	free (ifconf);
	return NULL;
}								/* GetInterfaceConfig */

/*
 * check if single interface is acceptable
 */
static XBBool
GetSingleInterface (int fd, XBSocketInterface * pInter, const char *ifname)
{
	struct sockaddr_in inetDevice;
	struct sockaddr_in inetBroadcast;
	struct ifreq ifreq;
	short flags;

	/* TODO: the previous code seemed to think we could get AF_INET6
	 * addresses. In that case, ifreq might be too small to hold IPv6
	 * addresses on some platforms. */

	strcpy (ifreq.ifr_name, ifname);
	ifreq.ifr_addr.sa_family = AF_INET;
	if (ioctl (fd, SIOCGIFADDR, &ifreq) == -1) {
		Dbg_Out ("SIOCGIFADDR failed for interface %s, rejecting\n", ifname);
		return XBFalse;
	}

	inetDevice = *(struct sockaddr_in *)&ifreq.ifr_addr;

	/* get flags */
	strcpy (ifreq.ifr_name, ifname);
	ifreq.ifr_addr.sa_family = AF_INET;
	if (ioctl (fd, SIOCGIFFLAGS, &ifreq) == -1) {
		Dbg_Out ("failed to get flags for interface %s, rejecting\n", ifname);
		return XBFalse;
	}
	flags = ifreq.ifr_flags;

	/* try to get broadcast adress */
	if (IFF_BROADCAST & flags) {
		strcpy (ifreq.ifr_name, ifname);
		ifreq.ifr_addr.sa_family = AF_INET;
		if (-1 != (ioctl (fd, SIOCGIFBRDADDR, &ifreq))) {
			inetBroadcast = *(struct sockaddr_in *)&ifreq.ifr_broadaddr;
		}
		else {
			flags &= ~IFF_BROADCAST;
		}
	}

	/* show data so far */
	Dbg_Out ("IP = %s\n", inet_ntoa (inetDevice.sin_addr));
	Dbg_Out ("BC = %s\n", flags & IFF_BROADCAST ? inet_ntoa (inetBroadcast.sin_addr) : "n/a");

	/* check if interface is down */
	if (!(IFF_UP & flags)) {
		Dbg_Out ("interface is down, rejecting\n");
		return XBFalse;
	}

	Dbg_Out ("interface is up\n");

	/* store data */
	pInter->name = DupString (ifname);
	pInter->addrDevice = DupString (inet_ntoa (inetDevice.sin_addr));
	pInter->addrBroadcast =
		flags & IFF_BROADCAST ? DupString (inet_ntoa (inetBroadcast.sin_addr)) : NULL;

	return XBTrue;
}								/* GetSingleInterface */

/*
 * list available interfaces
 */
const XBSocketInterface *
Socket_GetInterfaces (size_t * pNum)
{
	int fd;
	int maxInter;
	int cur;
	struct ifconf *ifconf = NULL;

	assert (pNum != NULL);

	/* clean up */
	DeleteInterfaces ();

	/* open UDP socket for testing */
	fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		Dbg_Socket ("failed to get socket for interface detection\n");
		goto Error;
	}

	/* get config */
	ifconf = GetInterfaceConfig (fd);
	if (ifconf == NULL) {
		Dbg_Socket ("failed to get interface data\n");
		goto Error;
	}

	/* alloc result buffer */
	numInter = 0;
	maxInter = ifconf->ifc_len / sizeof (struct ifreq);
	inter = calloc (maxInter, sizeof (XBSocketInterface));
	if (inter == NULL) {
		Dbg_Socket ("failed to allocate inter buffer\n");
		goto Error;
	}

	/* now walk through buffer */
	for (cur = 0; cur < maxInter; cur++) {
		struct ifreq *ifreq = &ifconf->ifc_req[cur];

		Dbg_Out ("### interface %s ###\n", ifreq->ifr_name);
		if (GetSingleInterface (fd, &inter[numInter], ifreq->ifr_name)) {
			numInter++;
		}
	}

	/* clean up */
	free (ifconf->ifc_buf);
	free (ifconf);
	close (fd);

	/* that's all */
	*pNum = numInter;
	return inter;

  Error:
	if (ifconf) {
		if (ifconf->ifc_buf)
			free (ifconf->ifc_buf);
		free (ifconf);
	}

	if (-1 != fd) {
		close (fd);
	}

	DeleteInterfaces ();
	*pNum = 0;

	return NULL;
}								/* Socket_GetInterfaces */

/*****************
 * socket events *
 *****************/

/*
 * handle select
 */
void
SelectSockets (struct timeval *timeout)
{
	int fd;
	fd_set rdfs;
	fd_set wrfs;

	/* poll network sockets */
	rdfs = socketReadSet;
	wrfs = socketWriteSet;
	select (socketMax + 1, &rdfs, &wrfs, NULL, timeout);

	/* check each socket */
	for (fd = 0; fd <= socketMax; fd++) {
		/* socket is readable */
		if (FD_ISSET (fd, &rdfs)) {
			CommReadable (fd);
		}
		if (FD_ISSET (fd, &wrfs)) {
			CommWriteable (fd);
		}
	}
}
