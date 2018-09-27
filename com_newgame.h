/*
 * file com_newgame.h - client newGamees for local network game
 *
 * $Id: com_newgame.h,v 1.6 2004/10/30 20:11:40 lodott Exp $
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
#ifndef XBLAST_COM_NEWGAME_H
#define XBLAST_COM_NEWGAME_H

#include "com.h"

#include "common.h"
#include "cfg_game.h"

/*
 * global prototypes
 */
extern XBComm *NewGame_CreateComm (const char *deviceAddress, unsigned short port, const char *broadcastAddress, const CFGGameHost *cfg, const CFGGameSetup *setup);
extern void    NewGame_Send (XBComm *qComm, const struct timeval *tv, int num, const char *score);
extern void    NewGame_Close (XBComm *comm, const struct timeval *tv);
#endif
/*
 * end of file com_newGame.h
 */
