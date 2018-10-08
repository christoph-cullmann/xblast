/*
 * file w32_event.h - event (message) handling for win32
 *
 * $Id: w32_event.h,v 1.3 2006/02/09 21:21:25 fzago Exp $
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
#ifndef XBLAST_W32_EVENT_H
#define XBLAST_W32_EVENT_H

#include "w32_common.h"


/*
 * global macros
 */
#define MSG_XBLAST_EVENT_VALUE    WM_USER
#define MSG_XBLAST_EVENT_POINTER (WM_USER+1)
#define MSG_XBLAST_SELECT        (WM_USER+2)
#define MSG_XBLAST_TIMER         (WM_USER+3)

/*
 * global prototypes
 */
extern long PASCAL WindowProc (HWND window, UINT message, UINT w_param, LONG l_param);
extern XBBool InitEvent (void);
extern void FinishEvent (void);

#endif
/*
 * end of file w32_event.h
 */
