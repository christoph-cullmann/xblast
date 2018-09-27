/*
 * cfg_control.h - keyboard and joystick configuration data
 * 
 * $Id: cfg_control.h,v 1.6 2004/08/04 08:08:39 iskywalker Exp $
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
#ifndef _CFG_CONTROL_H
#define _CFG_CONTROL_H

#include "xblast.h"
#include "event.h"

/*
 * type definitions
 */

/* ingame controls for editing*/
typedef struct {
  XBAtom keyUp;
  XBAtom keyDown;
  XBAtom keyLeft;
  XBAtom keyRight;
  XBAtom keyStop;
  XBAtom keyBomb;
  XBAtom keySpecial;
  XBAtom keyPause;
  XBAtom keyAbort;
  XBAtom keyAbortCancel;
    /* Skywalker */
  XBAtom keyLaola;
  XBAtom keyLooser;
  XBAtom keyBot;
  XBAtom keyChatStart;
  XBAtom keyChatSend;
  XBAtom keyChatCancel;
  XBAtom keyChatChangeReceiver;
    /* */
} CFGControlKeyboard;

/* key tables for GUI */
typedef struct {
  const char  *keysym;
  XBEventCode  eventCode;
  int          eventData;
} CFGKeyTable;

/*
 * global prototypes
 */
extern void LoadControlConfig (void);
extern void SaveControlConfig (void);
extern void FinishControlConfig (void);
extern void StoreControlKeyboard (XBEventCode type, const CFGControlKeyboard *ctrl);
extern XBBool RetrieveControlKeyboard (XBEventCode type, CFGControlKeyboard *ctrl);
extern const CFGKeyTable *GetGameKeyPressTable (void);
extern const CFGKeyTable *GetGameKeyReleaseTable (void);
extern const CFGKeyTable *GetMenuKeyTable (void);

#endif
/*
 * end cfg_control.h
 */
