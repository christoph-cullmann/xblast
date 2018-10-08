/*
 * file net_dgram.c - Datagrams for in game messages
 *
 * $Id: net_dgram.c,v 1.12 2006/02/09 21:21:24 fzago Exp $
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

/*
 * local structures
 */
struct _xb_datagram
{
	void *data;
	size_t len;
};

/*
 * local const
 */
#define DGRAM_START    0x68
#define DGRAM_STOP     0x16

/*
 * local variables
 */
static unsigned char buffer[MAX_DGRAM_SIZE + 3];

/*
 * create a datagram
 */
XBDatagram *
Net_CreateDatagram (const void *data, size_t len)
{
	XBDatagram *ptr;
	assert (len <= MAX_DGRAM_SIZE);
	/* alloc data */
	ptr = calloc (1, sizeof (XBDatagram));
	assert (NULL != ptr);
	/* alloc buffer */
	ptr->data = calloc (len + 1, 1);
	assert (NULL != ptr->data);
	/* set values */
	ptr->len = len;
	if (NULL != data) {
		memcpy (ptr->data, data, len);
	}
	/* that's all */
	return ptr;
}								/* Net_CreateDatagram */

/*
 * delete datagram 
 */
void
Net_DeleteDatagram (XBDatagram * ptr)
{
	assert (ptr != NULL);
	assert (ptr->data != NULL);
	free (ptr->data);
	free (ptr);
}								/* Net_DeleteDatagram */

/*
 * write datagram to local buffer
 */
static void
MakeDatagram (const XBDatagram * ptr)
{
	assert (ptr != NULL);
	/* write buffer */
	buffer[0] = DGRAM_START;
	buffer[1] = ptr->len;
	memcpy (buffer + 2, ptr->data, ptr->len);
	buffer[ptr->len + 2] = DGRAM_STOP;
	/* send it */
#ifdef DEBUG_TELE
	{
		int i;
		Dbg_Out ("snd dgram ");
		for (i = 0; i < ptr->len; i++) {
			Dbg_Out ("%02x ", (unsigned)(buffer[i + 2]));
		}
		Dbg_Out ("(%u bytes)\n", ptr->len);
	}
#endif
}								/* MakeDatagram */

/*
 * send datagram on a socket
 */
XBBool
Net_SendDatagram (const XBDatagram * ptr, const XBSocket * pSocket)
{
	int result;
	/* fill buffer */
	MakeDatagram (ptr);
#ifdef DEBUG_UDP
	/* simulate heavy data loss */
	if (rand () % 2) {
		Dbg_Dgram ("dgram discarded, simulated data loss\n");
		return XBTrue;
	}
#endif
	result = Socket_Send (pSocket, buffer, ptr->len + 3);
	switch (result) {
	case XB_SOCKET_END_OF_FILE:
	case XB_SOCKET_ERROR:
		return XBFalse;
	case XB_SOCKET_WOULD_BLOCK:
		return XBTrue;
	default:
		return (ptr->len + 3 == (size_t) result);
	}
}								/* Net_SendDatagram */

/*
 * send datagram to specified peer with given broadcast option
 */
XBBool
Net_SendDatagramTo (const XBDatagram * ptr, XBSocket * pSocket, const char *host,
					unsigned short port, XBBool broadcast)
{
	int result;
	/* fill buffer */
	MakeDatagram (ptr);
	/* enable broadcast if needed */
	if (broadcast && !Socket_SetBroadcast (pSocket, broadcast)) {
		Dbg_Out ("failed to set broadcast flag on socket!\n");
		return XBFalse;
	}
	result = Socket_SendTo (pSocket, buffer, ptr->len + 3, host, port);
	switch (result) {
	case XB_SOCKET_END_OF_FILE:
	case XB_SOCKET_ERROR:
		return XBFalse;
	case XB_SOCKET_WOULD_BLOCK:
		return XBTrue;
	default:
		return (ptr->len + 3 == (size_t) result);
	}
}								/* Net_SendDatagram */

/*
 * parse datagram in read buffer
 */
static XBDatagram *
ParseDatagram (size_t len)
{
	switch (len) {
	case XB_SOCKET_END_OF_FILE:
	case XB_SOCKET_WOULD_BLOCK:
	case XB_SOCKET_ERROR:
		return NULL;
	}
	/* length check */
	if (len < 3) {
		return NULL;
	}
	/* check start */
	if (buffer[0] != DGRAM_START) {
		return NULL;
	}
	/* check length stop */
	if (buffer[1] + 3 != (unsigned char)len) {
		return NULL;
	}
	/* check stop */
	if (buffer[len - 1] != DGRAM_STOP) {
		return NULL;
	}
#ifdef DEBUG_TELE
	{
		size_t i;
		Dbg_Out ("rcv dgram ");
		for (i = 0; i < buffer[1]; i++) {
			Dbg_Out ("%02x ", (unsigned)buffer[i + 2]);
		}
		Dbg_Out ("(%u bytes)\n", (unsigned)buffer[1]);
	}
#endif
	/* datagram is correct */
	return Net_CreateDatagram (buffer + 2, buffer[1]);
}								/* ParseDatagram */

/*
 * receive datagram
 */
XBDatagram *
Net_ReceiveDatagram (const XBSocket * pSocket)
{
#ifdef W32
	long len;
#else
	ssize_t len;
#endif

	len = Socket_Receive (pSocket, buffer, sizeof (buffer));
	return ParseDatagram (len);
}								/* Net_ReceiveDatagram */

/*
 * receive datagram
 */
XBDatagram *
Net_ReceiveDatagramFrom (XBSocket * pSocket, const char **host, unsigned short *port)
{
#ifdef W32
	long len;
#else
	ssize_t len;
#endif
	len = Socket_ReceiveFrom (pSocket, buffer, sizeof (buffer), host, port);
	return ParseDatagram (len);
}								/* Net_ReceiveDatagram */

/*
 * contents of datagram
 */
const void *
Net_DgramData (const XBDatagram * dgram, size_t * len)
{
	assert (NULL != dgram);
	assert (NULL != len);

	*len = dgram->len;
	return dgram->data;
}								/* Net_DgramData* */

/*
 * end of file net_dgram.c
 */
