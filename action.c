/*
 * file action.c - converting player actions to bytes and back
 *
 * $Id: action.c,v 1.6 2005/01/15 17:41:35 iskywalker Exp $
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
#include "action.h"

/*
 *  local macros
 */

/* direction action masks */
#define PA_DIR_MASK     ((unsigned char) 0x07)
#define PA_GO_DEFAULT   ((unsigned char) 0x00)
#define PA_GO_STOP      ((unsigned char) 0x01)
#define PA_GO_UP        ((unsigned char) 0x02)
#define PA_GO_LEFT      ((unsigned char) 0x03)
#define PA_GO_DOWN      ((unsigned char) 0x04)
#define PA_GO_RIGHT     ((unsigned char) 0x05)

/* other action masks */
#define PA_BOMB         ((unsigned char) 0x08)
#define PA_SPECIAL      ((unsigned char) 0x10)
#define PA_PAUSE        ((unsigned char) 0x20)

/* abort action masks */
#define PA_ABORT_MASK   ((unsigned char) 0xC0)
#define PA_ABORT_NONE   ((unsigned char) 0x00)
#define PA_ABORT_TRUE   ((unsigned char) 0x40)
#define PA_ABORT_CANCEL ((unsigned char) 0x80)

/* laola/loser */
#define PA_LAOLA PA_PAUSE
#define PA_LOSER (PA_ABORT_CANCEL | PA_ABORT_TRUE)

/* suicide */
#define PA_GO_ALL PA_DIR_MASK

/*
 * global function: PlayerActionToByte
 * description:     convert player action to byte
 * return value:    byte wih player action
 * parameters:      playerAction - action struct to convert
 */
unsigned char
PlayerActionToByte (const PlayerAction *playerAction)
{
  unsigned char result;
  assert (playerAction != NULL);
  /* direction 0-2 */
  if (playerAction->suicide) {
    result = PA_GO_ALL;
  } else {
    switch (playerAction->dir) {
    case GoStop: result = PA_GO_STOP;    break;
    case GoUp:   result = PA_GO_UP;      break;
    case GoLeft: result = PA_GO_LEFT;    break;
    case GoDown: result = PA_GO_DOWN;    break;
    case GoRight:result = PA_GO_RIGHT;   break;
    default:     result = PA_GO_DEFAULT; break;
    }
  }
  /* drop bomb 3 */
  if (playerAction->bomb) {
    result |= PA_BOMB;
  }
  /* special extra 4 */
  if (playerAction->special) {
    result |= PA_SPECIAL;
  }
  /* laola uses PAUSE mask */
  if (playerAction->laola) {
    result|=PA_LAOLA;
  }
  /* loser */
  if (playerAction->looser) {
    result |= PA_LOSER;
  } else {
    switch (playerAction->abort) {
    case ABORT_CANCEL: result |= PA_ABORT_CANCEL; break;
    case ABORT_TRUE:   result |= PA_ABORT_TRUE;   break;
    default:           result |= PA_ABORT_NONE;   break;
    }
  }
  /* that's all */
  return result;
} /* PlayerActionToByte */

/*
 * global function: PlayerActionFromByte
 * description:     convert byte to player action
 * return value:    none
 * parameters:      playerAction - player action to set
 *                  value        - byte to convert
 */
void
PlayerActionFromByte (PlayerAction *playerAction, unsigned char value)
{
  assert (playerAction != NULL);
  /* direction */
  switch (value & PA_DIR_MASK) {
  case PA_GO_STOP:  playerAction->dir = GoStop;     break;
  case PA_GO_UP:    playerAction->dir = GoUp;       break;
  case PA_GO_LEFT:  playerAction->dir = GoLeft;     break;
  case PA_GO_DOWN:  playerAction->dir = GoDown;     break;
  case PA_GO_RIGHT: playerAction->dir = GoRight;    break;
  case PA_GO_ALL:   playerAction->suicide = XBTrue; break;
  default:          playerAction->dir = GoDefault;  break;
  }
  /* drop bomb */
  playerAction->bomb    = (value & PA_BOMB)    ? XBTrue : XBFalse;
  /* special extras */
  playerAction->special = (value & PA_SPECIAL) ? XBTrue : XBFalse;
  /* pause keys */
  playerAction->laola    = (value & PA_LAOLA)   ? XBTrue : XBFalse;
  /* abort 6-7 */
  if (value & PA_ABORT_TRUE && 
      value & PA_ABORT_CANCEL ) {
    playerAction->looser = XBTrue;
  } else {
    playerAction->looser = XBFalse;
    switch (value & PA_ABORT_MASK) {
    case PA_ABORT_CANCEL: playerAction->abort = ABORT_CANCEL; break;
    case PA_ABORT_TRUE:   playerAction->abort = ABORT_TRUE;   break;
    default:              playerAction->abort = ABORT_NONE;   break;
    }
  }
} /* PlayerActionFromByte */

/*
 * end of file action.c
 */
