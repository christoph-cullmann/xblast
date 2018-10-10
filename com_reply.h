/*
 * file com_reply.h - server answers to broadcasts by clients
 *
 * $Id: com_reply.h,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
#ifndef XBLAST_COM_REPLY_H
#define XBLAST_COM_REPLY_H

#include "com.h"
#include "cfg_game.h"

/*
 * type definitions
 */

/*
 * global prototypes
 */
extern XBComm* Reply_CreateComm (unsigned short replyPort, const CFGGameHost *cfg, const CFGGameSetup *);

#endif
/*
 * end of file com_reply.h
 */
