/*
 * file action.h - converting player actions to bytes and back
 *
 * $Id: action.h,v 1.9 2006/02/09 21:21:22 fzago Exp $
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
#ifndef XBLAST_ACTION_H
#define XBLAST_ACTION_H

/*
 * type definitions
 */

/* status of player abort */
typedef enum
{
	ABORT_NONE = 0,
	ABORT_TRUE,
	ABORT_CANCEL
} PlayerAbort;

/* chat defines */
#define CHAT_NONE    0
#define CHAT_START   1
#define CHAT_SEND    2
#define CHAT_CANCEL  3
#define CHAT_LEN     40

/* player action(s) at last turn */
typedef struct
{
	int player;
	BMDirection dir;
	XBBool bomb;
	XBBool special;
	XBBool pause;
	PlayerAbort abort;
	XBBool suicide;
	XBBool laola;
	XBBool looser;
	XBBool bot;
	XBBool away;
} PlayerAction;

/*
 * global prototypes
 */
extern unsigned char PlayerActionToByte (const PlayerAction *);
extern void PlayerActionFromByte (PlayerAction *, unsigned char);

#endif
/*
 * end of file action.h
 */
