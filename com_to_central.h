/*
 * file com_to_central.h - handle communications with centrals
 *
 * $Id: com_to_central.h,v 1.4 2004/11/11 14:57:08 lodott Exp $
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
#ifndef _COM_TO_CENTRAL_H
#define _COM_TO_CENTRAL_H

#include "atom.h"
#include "com_base.h"
#include "net_socket.h"
#include "network.h"
#include "ini_file.h"
#include "cfg_xblast.h"

/*
 * event codes
 */
typedef enum {
  XBE2C_IORead,          /* read error occurred */
  XBE2C_IOWrite,         /* write error occurred */
  XBE2C_UnexpectedEOF,   /* eof received */
  XBE2C_StreamWaiting,   /* all queued data sent on stream */
  XBE2C_StreamBusy,      /* queued data on stream partially sent */
  XBE2C_StreamClosed,    /* stream has been removed */
  XBE2C_InvalidCot,      /* invalid telegram cot */
  XBE2C_InvalidID,       /* invalid telegram id */
} XBEventToCentral;

/*
 * global prototypes
 */
extern XBComm *X2C_CreateComm (const CFGCentralSetup *);

extern const char *X2C_CentralName (XBComm *);
extern const char *X2C_LocalName (XBComm *);

extern void X2C_Disconnect (XBComm *comm);
extern void X2C_SendDisconnect (XBComm *comm);

extern void X2C_QueryPlayerConfig (XBComm *comm);
extern void X2C_SendPlayerConfig (XBComm *comm, XBAtom);
extern void X2C_SendGameStat (XBComm *comm, int numPlayers, int *PID, int *Score);

#endif
/*
 * end of file com_to_central.h
 */
