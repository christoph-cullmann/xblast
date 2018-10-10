/*
 * file chat.h - managing chat data for both client and server
 *
 * $Id: chat.h,v 1.3 2004/08/04 22:02:30 iskywalker Exp $
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
#ifndef XBLAST_CHAT_H
#define XBLAST_CHAT_H

#define CHAT_LINE_SIZE 40

#include "common.h"

/*
 * type declarations
 */
typedef struct _xb_chat XBChat;
struct _xb_chat {
  XBChat     *next;
  unsigned   fh;
  unsigned   fp;
  unsigned   th;
  unsigned   tp;
  unsigned   how; /* 0..numofplayers (private), public, team */
  char       txt[CHAT_LINE_SIZE];
  size_t     len;
  unsigned   status;
};

/*
 * global prototypes
 */

extern void Chat_Clear();
extern XBChat * Chat_Pop();
extern XBChat * Chat_Create(unsigned fh, unsigned fp, unsigned th, unsigned tp, unsigned how, const char *chat);
extern void Chat_Receive(XBChat *);

#endif
/*
 * end of file chat.h
 */
