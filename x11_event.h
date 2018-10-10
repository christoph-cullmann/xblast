/*
 * file x11_event.h -
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 * July 15, 1999
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
 *
 * $Id: x11_event.h,v 1.4 2004/08/16 22:14:16 iskywalker Exp $
 */
#ifndef _X11_EVENT_H
#define _X11_EVENT_H

#include "xblast.h"

/*
 * constants
 */
#define EVENT_MASK_NORMAL (ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask)
#define EVENT_MASK_MOUSE  (EVENT_MASK_NORMAL | ButtonPressMask |ButtonReleaseMask| PointerMotionMask)

/*
 * prototypes
 */
extern XBBool InitEvent (void);
extern void FinishEvent (void);

#endif
