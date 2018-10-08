/*
 * file net_dgram.h - Datagrams for in game messages
 *
 * $Id: net_dgram.h,v 1.5 2006/02/09 21:21:24 fzago Exp $
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
#ifndef XBLAST_NET_DGRAM_H
#define XBLAST_NET_DGRAM_H

/*
 * global constants
 */
#define MAX_DGRAM_SIZE 255

/*
 * type definitions
 */
typedef struct _xb_datagram XBDatagram;

/*
 * global prototypes
 */
extern XBDatagram *Net_CreateDatagram (const void *data, size_t len);
extern void Net_DeleteDatagram (XBDatagram *);

extern XBBool Net_SendDatagram (const XBDatagram * dgram, const XBSocket * pSocket);
extern XBBool Net_SendDatagramTo (const XBDatagram * dgram, XBSocket * pSocket, const char *host,
								  unsigned short port, XBBool broadcast);
extern XBDatagram *Net_ReceiveDatagram (const XBSocket * pSocket);
extern XBDatagram *Net_ReceiveDatagramFrom (XBSocket * pSocket, const char **host,
											unsigned short *port);

extern const void *Net_DgramData (const XBDatagram * dgram, size_t * len);

#endif
/*
 * end of file net_dgram.h
 */
