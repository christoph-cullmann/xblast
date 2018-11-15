/*
 * file x11_socket.c - true bsd sockets for xblast
 *
 * $Id: x11_socket.c,v 1.8 2004/11/04 17:04:43 lodott Exp $
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
#include "x11_socket.h"
#include "socket.h"

#include "x11_common.h"
#include "x11_joystick.h"
#include "x11_sound.h"

#include "str_util.h"
#include "com.h"

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
typedef struct _xb_socket_address {
  socklen_t len;
  struct sockaddr *addr;
} XBSocketAddress;

/* fix for sparc */
#ifdef sparc
typedef uint32_t u_int32_t;
#endif

/* socket data */
struct _xb_socket {
  int             fd;         /* file descriptor of socket */
  XBSocketAddress sock;       /* local address */
  XBSocketAddress peer;       /* remote address */
  XBBool          shutDown;   /* shutodwn flag */
};

/*
 * local variables
 */
static int    socketMax = 0;     /* current max */
static int    socketX11 = -1;    /* x11 socket */
static int    socketSnd = -1;    /* sound socket */
static fd_set socketReadSet;     /* current read set */
static fd_set socketWriteSet;    /* current write set */
static fd_set fdJoystickSet;     /* current joystick set */

/* interface list */
static XBSocketInterface *inter    = NULL;
static size_t             numInter = 0;

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
    size_t i;
    for (i = 0; i < numInter; i ++) {
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
  inter    = NULL;
  numInter = 0;
} /* DeleteInterfaces */

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
  if (-1L != (addr = inet_addr (hostName) ) ) {
    return ntohl (addr);
  }
  /* lookup hostname if not an ip string */
  if (NULL != (serverEnt = gethostbyname (hostName) ) ) {
    return ntohl (*(int32_t *) serverEnt->h_addr_list[0]);
  }
  /* lookup failed */
  return 0L;
} /* GetAddressInet */

/*
 * handler for SIGCHLD
 */
static void
HandleSigChld (int sig_num)
{
  int   stat;
  pid_t child;

  while (0 < (child = waitpid (-1, &stat, WNOHANG) ) ) {
    Dbg_Out ("child %d terminated\n", child);
  }
} /* HandleSigChld */

/**************************
 * local socket debugging *
 **************************/

#ifdef DEBUG_SOCKET
/*
 * print out an fdset
 */
static void
DebugFdSet (const char *header, fd_set *set)
{
  int i;

  Dbg_Socket ("%s:", header);
  for (i = 0; i <= socketMax; i ++) {
    if (FD_ISSET (i, set) ) {
       fprintf(stderr," %d", i);
    }
  }
  fprintf (stderr, "\n");
} /* DebugFdSet */

/*
 * print out interface config
 */
static void
DebugInterfaceData(const struct ifconf *ifConf)
{
  size_t i;
  Dbg_Socket ("Interface Config (%u bytes)", ifConf->ifc_len);
  for (i = 0; i < ifConf->ifc_len; i ++) {
    if (i % 8 == 0) {
      fprintf(stderr, "\n");
    }
    fprintf (stderr, "%02x ", (unsigned) (unsigned char) ifConf->ifc_buf[i]);
  }
  fprintf(stderr,"\n");
} /* DebugInterfaceData*/
#endif

/*******************
 * socket managing *
 *******************/

/*
 * initialisation routine
 */
XBBool
Socket_Init (void)
{
  static XBBool initDone = XBFalse;
  if (! initDone) {
    signal (SIGCHLD, HandleSigChld);
    signal (SIGPIPE, SIG_IGN);
    signal (SIGALRM, SIG_IGN);
    initDone = XBTrue;
  }
  /* clear fd_Set for sockets */
  FD_ZERO (&socketReadSet);
  FD_ZERO (&socketWriteSet);
  FD_ZERO (&fdJoystickSet);
  Dbg_Socket("initializing socket managing\n");
  return XBTrue;
} /* Socket_Init */

/*
 * cleaning up
 */
void
Socket_Finish (void)
{
} /* Socket_Finish */

/***************
 * socket data *
 ***************/

/*
 * return file descriptor for socket
 */
int
Socket_Fd (const XBSocket *pSocket)
{
  assert (NULL != pSocket);
  return pSocket->fd;
} /* Socket_Fd */

/*
 * return adress family for socket
 */
int
Socket_Family (const XBSocket *pSocket)
{
  assert (NULL != pSocket);

  return pSocket->sock.addr->sa_family;
} /* Socket_Family */

/*
 * get host name of client
 */
const char *
Socket_HostName (const XBSocket *pSocket, XBBool peer)
{
  const XBSocketAddress *sa;
  struct sockaddr_in    *inetAddr;
  assert (NULL != pSocket);
  sa = peer ? &pSocket->peer : &pSocket->sock;
  assert (NULL != sa);
  assert (NULL != sa->addr);
  inetAddr = (struct sockaddr_in *) sa->addr;
  return inet_ntoa (inetAddr->sin_addr);
} /* Socket_HostName */

/*
 * get port of host
 */
unsigned
Socket_HostPort (const XBSocket *pSocket, XBBool peer)
{
  const XBSocketAddress *sa;
  struct sockaddr_in    *inetAddr;
  assert (NULL != pSocket);
  sa = peer ? &pSocket->peer : &pSocket->sock;
  assert (NULL != sa);
  assert (NULL != sa->addr);
  inetAddr = (struct sockaddr_in *) sa->addr;
  return ntohs (inetAddr->sin_port);
} /* Socket_HostPort */

/*******************
 * socket creation *
 *******************

/*
 * create socket structure
 */
XBSocket *
Socket_Alloc (int family)
{
  int       len;
  XBSocket *pSocket;
  /* require AF_INET */
  switch (family) {
  case AF_INET:
    len = sizeof (struct sockaddr_in);
    break;
  default:
    Dbg_Socket("AF_INET family required, socket not created\n");
    return NULL;
  }
  /* alloc socket data structure */
  pSocket       = calloc (1, sizeof (XBSocket) );
  assert (NULL != pSocket);
  pSocket->fd   = -1;
  /* out address */
  pSocket->sock.len  = len;
  pSocket->sock.addr = calloc (1, len);
  assert (NULL != pSocket->sock.addr);
  pSocket->sock.addr->sa_family = family;
  /* other addresse */
  pSocket->peer.len  = len;
  pSocket->peer.addr = calloc (1, len);
  assert (NULL != pSocket->peer.addr);
  pSocket->peer.addr->sa_family = family;
  /* set shutdown flags to false */
  pSocket->shutDown = XBFalse;
  /* that's all */
  return pSocket;
} /* Socket_Alloc */

/*
 * free socket structure memory
 */
void
Socket_Free (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  if (NULL != pSocket->sock.addr) {
    free (pSocket->sock.addr);
  }
  if (NULL != pSocket->peer.addr) {
    free (pSocket->peer.addr);
  }
  free (pSocket);
} /* Socket_Free */

/*
 * set socket adress
 */
XBBool
Socket_SetAddressInet (XBSocket *pSocket, XBBool peer, const char *hostName, unsigned short port)
{
  XBSocketAddress    *sa;
  u_int32_t      addr;
  struct sockaddr_in *serverAddr;

  assert (NULL != pSocket);
  /* determine address structure to set */
  sa = peer ? &pSocket->peer : &pSocket->sock;
  /* get host name in host byte order */
  if (NULL != hostName) {
    if (0 == (addr = GetAddressInet (hostName) ) ) {
      Dbg_Socket("failed to set address %s:%u for fd=%u\n", hostName, port, pSocket->fd);
      return XBFalse;
    }
  } else {
    addr = INADDR_ANY;
  }
  /* now set the address */
  assert (NULL != sa);
  memset (sa->addr, 0, sa->len);
  serverAddr                  = (struct sockaddr_in *) sa->addr;
  serverAddr->sin_family      = AF_INET;
  serverAddr->sin_addr.s_addr = htonl (addr);
  serverAddr->sin_port        = htons (port);
  Dbg_Socket("set %s address %s:%u for fd=%u\n", peer ? "remote" : "local", hostName, port, pSocket->fd);
  return XBTrue;
} /* Socket_SetAddressInet */

/*
 * enable or disable broadcast
 */
XBBool
Socket_SetBroadcast (XBSocket *pSocket, XBBool enable)
{
  int flag = enable ? 1: 0;
  if (0 ==  setsockopt (pSocket->fd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof (flag) ) ) {
    Dbg_Socket("successfully %s broadcast flag on socket fd=%u\n", enable ? "set" : "cleared", pSocket->fd);
    return XBTrue;
  } else {
    perror("Socket_SetBroadcast");
    Dbg_Socket("failed to %s broadcast flag on socket fd=%u\n", enable ? "set" : "cleared", pSocket->fd);
    return XBFalse;
  }
} /* Socket_SetBroadcast */

/*
 * register socket for reading
 */
void
Socket_RegisterRead (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  if (pSocket->fd > socketMax) {
    socketMax = pSocket->fd;
  }
  FD_SET (pSocket->fd, &socketReadSet);
#ifdef DEBUG_SOCKET
  DebugFdSet ("read fd_set", &socketReadSet);
#endif
} /* Socket_RegisterRead */

/*
 * register socket for writing
 */
void
Socket_RegisterWrite (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  if (pSocket->fd > socketMax) {
    socketMax = pSocket->fd;
  }
  FD_SET (pSocket->fd, &socketWriteSet);
#ifdef DEBUG_SOCKET
  DebugFdSet ("write fd_set", &socketWriteSet);
#endif
} /* Socket_RegisterWrite */

/*
 * unregister socket for reading
 */
void
Socket_UnregisterRead (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  FD_CLR (pSocket->fd, &socketReadSet);
#ifdef DEBUG_SOCKET
  DebugFdSet ("read fd_set", &socketReadSet);
#endif
} /* socket_UnregisterRead */

/*
 * register socket for writing
 */
void
Socket_UnregisterWrite (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  FD_CLR (pSocket->fd, &socketWriteSet);
#ifdef DEBUG_SOCKET
  DebugFdSet ("write fd_set", &socketWriteSet);
#endif
} /* Socket_UnregisterWrite */


/*************
 * BSD calls *
 *************

/*
 * open socket
 */
XBBool
Socket_Open (XBSocket *pSocket, int type)
{
  assert (pSocket != NULL);
  /* now create a stream socket */
  if (-1 == (pSocket->fd = socket (pSocket->sock.addr->sa_family, type, 0) ) ) {
    perror("Socket_Open");
    Dbg_Socket("failed to open socket of type=%i", type);
    return XBFalse;
  }

  // try to re-use existing port
  int optval = 1;
  setsockopt(pSocket->fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  Dbg_Socket ("open socket fd=%u (type=%i)\n", pSocket->fd, type);
  return XBTrue;
} /* Socket_Open */

/*
 * close socket
 */
void
Socket_Close (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  if (pSocket->fd < 0 || pSocket->shutDown) {
    return;
  }
  if (0 != close (pSocket->fd) ) {
    perror("Socket_Close");
    Dbg_Socket ("error while closing socket fd=%u\n", pSocket->fd);
  } else {
    pSocket->shutDown = XBTrue;
    Dbg_Socket ("socket fd=%u closed\n", pSocket->fd);
  }
} /* Socket_Close */

/*
 * close write access
 */
void
Socket_ShutdownWrite (XBSocket *pSocket)
{
  assert (NULL != pSocket);
  if (pSocket->fd < 0 || pSocket->shutDown) {
    return;
  }
  if (0 != shutdown (pSocket->fd, SHUT_WR) ) {
    perror("Socket_ShutdownWrite");
    Dbg_Socket ("error while shutting down socket fd=%u: ", pSocket->fd);
  } else {
    Dbg_Socket ("socket fd=%u shutdown write\n", pSocket->fd);
    pSocket->shutDown = XBTrue;
  }
} /* Socket_ShutdownWrite */

/*
 * connect a socket
 */
XBBool
Socket_Connect (XBSocket *pSocket)
{
  assert (pSocket != NULL);
  /* try to connect, set timeout */
  alarm (CONNECT_TIMEOUT);
  if (-1 == connect (pSocket->fd, pSocket->peer.addr, pSocket->peer.len) ) {
    perror("Socket_Connect");
    alarm (0);
    Dbg_Socket("failed to connect socket fd=%u to %s:%u\n", pSocket->fd, Socket_HostName(pSocket, XBTrue), Socket_HostPort(pSocket, XBTrue));
    return XBFalse;
  }
  alarm (0);
  /* now get adress assigned by kernel. the cast to void* is needed since not all systems know socklen_t */
  if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *) &pSocket->sock.len) ) {
    perror("Socket_Connect(getsockname)");
    Dbg_Socket("failed to get local address of socket fd=%u after connecting\n", pSocket->fd);
    return XBFalse;
  }
  Dbg_Socket ("socket fd=%u connected to %s:%u\n", pSocket->fd, Socket_HostName(pSocket, XBTrue), Socket_HostPort(pSocket, XBTrue));
  return XBTrue;
} /* Socket_Connect */

/*
 * bind a socket
 */
XBBool
Socket_Bind (XBSocket *pSocket)
{
  /* bind to port */
  if (-1 == bind (pSocket->fd, pSocket->sock.addr, pSocket->sock.len) ) {
    perror("Socket_Bind");
    Dbg_Socket("failed to bind socket fd=%u\n",pSocket->fd);
    return XBFalse;
  }
  /* now get adress assigned by kernel. the cast to void* is needed since not all systems know socklen_t */
  if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *) &pSocket->sock.len) ) {
    perror("Socket_Bind(getsockname)");
    Dbg_Socket("failed to get local address of socket fd=%u after binding\n", pSocket->fd);
    return XBFalse;
  }
  /* that's all */
  Dbg_Socket ("socket fd=%u bound to %s:%u\n", pSocket->fd, Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse));
  return XBTrue;
} /* Socket_Bind */

/*
 * accept a socket
 */
XBBool
Socket_Accept (XBSocket *pSocket, const XBSocket *pListen)
{
  assert (pSocket != NULL);
  assert (pListen != NULL);
  /* try to accept, set timeout */
  alarm (ACCEPT_TIMEOUT);
  if (-1 == (pSocket->fd = accept (pListen->fd, pSocket->peer.addr, (void *) &pSocket->peer.len) ) ) {
    perror("Socket_Accept");
    alarm (0);
    Dbg_Socket("failed to accept from socket fd=%u\n", pListen->fd);
    return XBFalse;
  }
  alarm (0);
  /* now retrieve local adresse */
  if (-1 == getsockname (pSocket->fd, pSocket->sock.addr, (void *) &pSocket->sock.len) ) {
    perror("Socket_Accept(getsockname)");
    Dbg_Socket("failed to get local address from accepted socket fd=%u\n", pListen->fd);
    return XBFalse;
  }
  /* that's all */
  Dbg_Out ("accepted socket fd=%u from socket fd=%u\n", pSocket->fd, pListen->fd);
  return XBTrue;
} /* Socket_Accept */

/*
 * listen on socket
 */
XBBool
Socket_Listen (XBSocket *pSocket)
{
  assert (pSocket != NULL);
  /* now listen for client to connect */
  if (0 != listen (pSocket->fd, LISTENQ) ) {
    perror("Socket_Listen");
    Dbg_Socket("failed to listen on socket fd=%u (%s:%u)\n", pSocket->fd, Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse));
    return XBFalse;
  }
  Dbg_Socket("listening on socket fd=%u (%s:%u)\n", pSocket->fd, Socket_HostName (pSocket, XBFalse), Socket_HostPort (pSocket, XBFalse));
  return XBTrue;
} /* Socket_Listen */

/*
 * write to socket, non blocking i/o assumed
 */
int
Socket_Send (const XBSocket *pSocket, const void *buf, size_t len)
{
  int result;
  assert (NULL != pSocket);
  assert (NULL != buf);
  /* try to write */
  result = send (pSocket->fd, buf, len, MSG_DONTWAIT);
  if (result < 0) {
    perror("Socket_Send");
    if (EAGAIN == errno) {
      Dbg_Socket ("send on fd=%u would block\n", pSocket->fd);
      return XB_SOCKET_WOULD_BLOCK;
    } else {
      Dbg_Socket ("send error %u on fd=%u\n", pSocket->fd);
      return XB_SOCKET_ERROR;
    }
  }
  Dbg_Socket("sent %u bytes on fd=%u\n", result, pSocket->fd);
  return result;
} /* Socket_Send */

/*
 * write to socket, given target, non blocking i/o assumed
 */
int
Socket_SendTo (XBSocket *pSocket, const void *buf, size_t len, const char *host, unsigned short port)
{
  int result;
  assert (NULL != pSocket);
  assert (NULL != buf);
  assert (NULL != host);
  /* convert destination adress */
  if (! Socket_SetAddressInet (pSocket, XBTrue, host, port)) {
    Dbg_Socket("failed to send on fd=%u - failed to resolve %s:%u",pSocket->fd, host, port);
    return -1;
  }
  /* now try to write data */
  result = sendto (pSocket->fd, buf, len, MSG_DONTWAIT, pSocket->peer.addr, pSocket->peer.len);
  if (result < 0) {
    perror("Socket_SendTo");
    if (EAGAIN == errno) {
      Dbg_Socket ("sendto on fd=%u would block\n", pSocket->fd);
      return XB_SOCKET_WOULD_BLOCK;
    } else {
      Dbg_Socket ("sendto error on fd=%u\n", pSocket->fd);
      return XB_SOCKET_ERROR;
    }
  }
  Dbg_Socket("sent %u bytes on fd=%u to %s:%u\n", result, pSocket->fd, host, port);
  return result;
} /* Socket_SendTo */

/*
 * read from socket
 */
int
Socket_Receive (const XBSocket *pSocket, void *buf, size_t len)
{
  int result;
  assert (NULL != pSocket);
  assert (NULL != buf);
  /* try to read */
  result = recv (pSocket->fd, buf, len, 0);
  if (result < 0) {
    perror("Socket_Receive");
    if (EAGAIN == errno) {
      Dbg_Socket ("receive on fd=%u would block\n", pSocket->fd);
      return XB_SOCKET_WOULD_BLOCK;
    } else {
      Dbg_Socket ("receive on fd=%u\n", pSocket->fd);
      return XB_SOCKET_ERROR;
    }
  }
  Dbg_Socket("received %u bytes on fd=%u\n", result, pSocket->fd);
  return result;
} /* Socket_Receive */

/*
 * read from socket, get sender
 */
int
Socket_ReceiveFrom (XBSocket *pSocket, void *buf, size_t len, const char **host, unsigned short *port)
{
  ssize_t         numRead;
  assert (NULL != pSocket);
  assert (NULL != buf);
  assert (NULL != host);
  /* try to read */
  numRead = recvfrom (pSocket->fd, buf, len, 0, pSocket->peer.addr, (void *) &pSocket->peer.len);

  if (numRead >  0) {
    *host = Socket_HostName (pSocket, XBTrue);
    *port = Socket_HostPort (pSocket, XBTrue);
  } else {
    perror("Socket_ReceiveFrom");
    *host = NULL;
    *port = 0;
    if (numRead < 0) {
      if (EAGAIN == errno) {
	Dbg_Socket ("receivefrom on fd=%u would block\n", pSocket->fd);
	return XB_SOCKET_WOULD_BLOCK;
      } else {
	Dbg_Socket ("receivefrom on fd=%u\n", pSocket->fd);
	return XB_SOCKET_ERROR;
      }
    }
  }
  Dbg_Socket("received %u bytes on fd=%u from %s:%u\n", numRead, pSocket->fd, *host, *port);
  return numRead;
} /* Socket_ReceiveFrom */

/**************
 * interfaces *
 **************/

/*
 * get interface config data
 */
static const struct ifconf *
GetInterfaceConfig (int fd)
{
  size_t  len, lastLen;
  char   *buf = NULL;
  static struct ifconf       ifConf;
  /* get list of all interfaces */
  len     = 100 * sizeof (struct ifreq);
  lastLen = 0;
  for (;;) {
    /* alloc buffer to receive data */
    buf = calloc (1, len );
    assert (NULL != buf);
    /* prepare structure for query */
    ifConf.ifc_len = len;
    ifConf.ifc_buf = buf;
    /* query list of interfaces */
    if (-1 == ioctl (fd, SIOCGIFCONF, &ifConf)) {
      if (errno != EINVAL || lastLen != 0) {
	free (buf);
	return NULL;
      }
    } else if (ifConf.ifc_len == lastLen) {
      /* success */
      return &ifConf;
    } else {
      /* net new length */
      lastLen = ifConf.ifc_len;
    }
    /* next guess */
    len += 10 * sizeof (struct ifreq);
    free (buf);
  }
  return XBFalse;
} /* GetInterfaceConfig */

/*
 * check if single interface is acceptable
 */
static XBBool
GetSingleInterface (int fd, XBSocketInterface *pInter, const struct ifreq *ifReq, size_t *len)
{
  struct ifreq        ifrFlags;
  struct ifreq        ifrBroadcast;
  struct sockaddr_in *inetDevice;
  struct sockaddr_in *inetBroadcast;

  /* get address length of current entry */
  switch (ifReq->ifr_addr.sa_family) {
#ifdef IPV6
  case AF_INET6:
    *len = IFNAMSIZ + sizeof (struct sockaddr_in6);
    break;
#endif
  case AF_INET:
  default:
    *len = IFNAMSIZ + sizeof (struct sockaddr);
    break;
  }
  /* checks for protocol family */
  if (ifReq->ifr_addr.sa_family != AF_INET) {
    Dbg_Out("no INET interface, rejecting\n");
    return XBFalse;
  }
  /* get flags */
  ifrFlags = *ifReq;
  if (-1 == ioctl (fd, SIOCGIFFLAGS, &ifrFlags) ) {
    Dbg_Out("failed to get interface flags, rejecting\n");
    return XBFalse;
  }
  /* device address is simple */
  inetDevice = (struct sockaddr_in *) &ifReq->ifr_addr;
  /* try to get broadcast adress */
  inetBroadcast = NULL;
  ifrBroadcast  = *ifReq;
  if (IFF_BROADCAST & ifrFlags.ifr_flags) {
    if (-1 != (ioctl (fd, SIOCGIFBRDADDR, &ifrBroadcast) ) ) {
      inetBroadcast = (struct sockaddr_in *) &ifrBroadcast.ifr_broadaddr;
    }
  }
  /* show data so far */
  Dbg_Out("IP = %s\n", inet_ntoa(inetDevice->sin_addr));
  Dbg_Out("BC = %s\n", inetBroadcast ? inet_ntoa(inetBroadcast->sin_addr) : "n/a");
  /* check if interface is down */
  if (! (IFF_UP & ifrFlags.ifr_flags) ) {
    Dbg_Out("interface is down, rejecting\n");
    return XBFalse;
  }
  Dbg_Out("interface is up\n");
  /* store data */
  pInter->name          = DupString (ifReq->ifr_name);
  pInter->addrDevice    = DupString (inet_ntoa (inetDevice->sin_addr));
  pInter->addrBroadcast = inetBroadcast ? DupString (inet_ntoa (inetBroadcast->sin_addr)) : NULL;
  return XBTrue;
} /* GetSingleInterface */

/*
 * list available interfaces
 */
const XBSocketInterface *
Socket_GetInterfaces (size_t *pNum)
{
  int                  fd;
  size_t               maxInter;
  size_t               cur;
  char                *ptr;
  const struct ifconf *ifConf = NULL;
  size_t               len;

  assert (pNum != NULL);
  /* clean up */
  DeleteInterfaces ();
  /* open UDP socket for testing */
  fd = socket (AF_INET, SOCK_DGRAM, 0);
  if (-1 == fd) {
    Dbg_Socket("failed to get socket for interface detection\n");
    goto Error;
  }
  /* get config */
  if (NULL == (ifConf = GetInterfaceConfig (fd) ) ) {
    Dbg_Socket("failed to get interface data\n");
    goto Error;
  }
  /* alloc result buffer */
  numInter = 0;
  cur = 0;
  maxInter = ifConf->ifc_len / sizeof (struct ifreq);
  inter    = calloc (maxInter, sizeof (XBSocketInterface));
  assert (NULL != inter);
  /* now walk through buffer */
  for (ptr = ifConf->ifc_buf; ptr < ifConf->ifc_buf + ifConf->ifc_len; ptr += len) {
    Dbg_Out("### interface %u ###\n", cur);
    if (GetSingleInterface (fd, inter + numInter, (struct ifreq *) ptr, &len) ) {
      numInter ++;
    }
    cur++;
  }
  /* clean up */
  free (ifConf->ifc_buf);
  close (fd);
  /* that's all */
  *pNum = numInter;
  return inter;

 Error:
  if (NULL != ifConf && NULL != ifConf->ifc_buf) {
    free (ifConf->ifc_buf);
  }
  if (-1 != fd) {
    close (fd);
  }
  DeleteInterfaces ();
  *pNum = 0;
  return NULL;
} /* Socket_GetInterfaces */

/*******************
 * special sockets *
 *******************/

/*
 * register a socket as x11
 */
void
RegisterDisplay (int fd)
{
  if (fd > socketMax) {
    socketMax = fd;
  }
  FD_SET (fd, &socketReadSet);
  socketX11 = fd;
  Dbg_Socket("x11 socket registered as fd=%u\n", fd);
} /* RegisterDisplay */

/*
 * register a socket as joystick
 */
void
RegisterJoystick (int fd)
{
  if (fd > socketMax) {
    socketMax = fd;
  }
  FD_SET (fd, &socketReadSet);
  FD_SET (fd, &fdJoystickSet);
  Dbg_Socket("joystick socket registered as fd=%u\n", fd);
} /* RegisterJoystick */

/*
 * register a socket as sound
 */
void
RegisterSound (int fd)
{
  if (fd > socketMax) {
    socketMax = fd;
  }
  assert (-1 == socketSnd);
  FD_SET (fd, &socketReadSet);
  socketSnd = fd;
  Dbg_Socket("sound socket registered as fd=%u\n", fd);
} /* RegisterSound */

/*
 * unregister sound socket
 */
void
UnregisterSound (int fd)
{
  if (fd == socketSnd){
    FD_CLR (fd, &socketReadSet);
    socketSnd = -1;
  } else {
    return;
  }
} /* UnregisterSound */

/*****************
 * socket events *
 *****************/

/*
 * handle select
 */
XBBool
SelectSockets (XBKeyboardMode kbMode, struct timeval *timeout)
{
  XBBool xEvents = XBFalse;
  int    fd;
  fd_set rdfs;
  fd_set wrfs;
  /* now poll X11 and other sockets */
  memcpy (&rdfs, &socketReadSet,  sizeof (fd_set));
  memcpy (&wrfs, &socketWriteSet, sizeof (fd_set));
  select (socketMax + 1, &rdfs, &wrfs, NULL, timeout);
  /* check each socket */
  for (fd = 0; fd <= socketMax; fd ++) {
    /* socket is readable */
    if (FD_ISSET (fd, &rdfs)) {
      if (fd == socketX11) {
	xEvents = XBTrue;
      } else if (fd == socketSnd) {
	HandleSound (fd);
      } else if (FD_ISSET (fd, &fdJoystickSet)) {
	switch (kbMode) {
	case KB_XBLAST: HandleXBlastJoystick (fd); break;
	case KB_MENU:   HandleMenuJoystick (fd);   break;
	default:        break;
	}
      } else {
	CommReadable (fd);
      }
    }
    if (FD_ISSET (fd, &wrfs)) {
      CommWriteable (fd);
    }
  }
  return xEvents;
}

/*
 * end of file x11_socket.c
 */
