/*
 * file com_to_server.c - client's communication with server
 *
 * $Id: com_to_server.h,v 1.12 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_TO_SERVER_C
#define XBLAST_COM_TO_SERVER_C

/*
 * global prototypes
 */

/* constructor */
extern XBComm *C2S_CreateComm (const CFGGameHost *);

/* get local data */
extern const char *C2S_ServerName (XBComm *);
extern const char *C2S_ClientName (XBComm *);

/* set local data */
extern void C2S_MarkEOF (XBComm * comm, XBBool flag);

/* queue data */
extern void C2S_SendDgramPort (XBComm *, unsigned short);
extern void C2S_GameDataNotAvailable (XBComm * comm);
extern XBBool C2S_SendGameConfig (XBComm * comm, CFGType cfgType, XBAtom atom);
extern void C2S_PlayerDataNotAvailable (XBComm * comm, unsigned id);
extern XBBool C2S_SendPlayerConfig (XBComm * comm, CFGType cfgType, XBAtom atom, unsigned player,
									XBBool how);
extern void C2S_SendHostState (XBComm * comm, unsigned state);
extern void C2S_SendHostStateReq (XBComm * comm, unsigned host, unsigned state);
extern void C2S_SendTeamStateReq (XBComm * comm, unsigned host, unsigned player, unsigned state);
extern void C2S_SendChat (XBComm * comm, XBChat * chat);
extern void C2S_Sync (XBComm *, XBNetworkEvent);
extern void C2S_LevelCheck (XBComm * comm, XBBool rej);
extern void C2S_SendWinner (XBComm * comm, unsigned team);

#endif
/*
 * end of file com_to_server.h
 */
