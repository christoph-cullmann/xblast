/*
 * file event.h - xblast event queue
 *
 * $Id: event.c,v 1.6 2004/08/07 01:11:22 iskywalker Exp $
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
#include "event.h"

/*
 * local macros
 */
#define EVENT_QUEUE_SIZE          128

/*
 * local types
 */
typedef struct {
  XBEventCode code;
  XBEventData data;
} XBEvent;


/* 
 * local variables
 */ 
static int      eventPut = 0;
static int      eventGet = 0;
static XBEvent  eventQueue[EVENT_QUEUE_SIZE];

#ifdef DEBUG_XBEVENT
/*
 *
 */
static const char *
EventName (XBEventCode eCode)
{
  switch (eCode) {
  case XBE_NONE:  	 return "XBE_NONE";
  case XBE_POLL:  	 return "XBE_POLL";
  case XBE_TIMER: 	 return "XBE_TIMER";
  case XBE_SOCKET_READ:  return "XBE_SOCKET_READ";
  case XBE_SOCKET_WRITE: return "XBE_SOCKET_WRITE";
  case XBE_MENU:   	 return "XBE_MENU";
  case XBE_ASCII:  	 return "XBE_ASCII";
  case XBE_CTRL:   	 return "XBE_CTRL";
  case XBE_KEYSYM: 	 return "XBE_KEYSYM";
  case XBE_XBLAST: 	 return "XBE_XBLAST";
  case XBE_KEYB_1: 	 return "XBE_KEYB_1";
  case XBE_KEYB_2: 	 return "XBE_KEYB_2";
  case XBE_JOYST_1: 	 return "XBE_JOYST_1";
  case XBE_JOYST_2: 	 return "XBE_JOYST_2";
  case XBE_MOUSE_1: 	 return "XBE_MOUSE_1";
  case XBE_MOUSE_2: 	 return "XBE_MOUSE_2";
  case XBE_MOUSE_3: 	 return "XBE_MOUSE_3";
  case XBE_MOUSE_MOVE:   return "XBE_MOUSE_MOVE";
  case XBE_SERVER:       return "XBE_SERVER";
  default:               return "XBE_?";
  }
} /* EventName */
#endif

/*
 * Get next event from queue
 */
XBEventCode 
NextEvent (XBEventData *data)
{
  XBEventCode ecode;
  if (eventPut == eventGet) {
    ecode = XBE_NONE;
  } else {
    ecode = eventQueue[eventGet].code;
    *data = eventQueue[eventGet].data;
    eventGet ++;
    if (eventGet >= EVENT_QUEUE_SIZE) {
      eventGet = 0;
    }
  }
#ifdef DEBUG_XBEVENT
  if(ecode!=XBE_NONE&ecode!=XBE_TIMER)
  Dbg_Out ("next event %s\n", EventName (ecode));
#endif
  return ecode;
} /* NextEvent */

/*
 * Put event in queue
 */ 
static XBBool
QueueEvent (XBEventCode ecode, XBEventData data)
{
  int newPut;

  newPut = eventPut + 1;
  if (newPut >= EVENT_QUEUE_SIZE) {
    newPut = 0;
  }
  if (newPut == eventGet) {
    /* queue overflow abort */
    return XBFalse;
  }
  eventQueue[eventPut].code = ecode;
  eventQueue[eventPut].data = data;
  eventPut = newPut;
  /* that's all */
#ifdef DEBUG_XBEVENT
  if(ecode!=XBE_NONE&ecode!=XBE_TIMER)
  Dbg_Out ("queue event %s %c,\n", EventName (ecode),data);
#endif
  return XBTrue;
} /* QueueEvent */

/*
 * Queue Event with no argument
 */
XBBool
QueueEventVoid (XBEventCode code)
{
  XBEventData data;
  data.value = 0;
  return QueueEvent (code, data);
} /* QueueEventVoid */

/*
 * Queue Event with integer argument
 */
XBBool
QueueEventValue (XBEventCode code, int value)
{
  XBEventData data;
  data.value = value;
#ifdef DEBUG_XBEVENT
  if(code!=XBE_NONE&code!=XBE_TIMER)
  Dbg_Out ("queue event value %s %c,\n", EventName (code),data.value);
#endif
  return QueueEvent (code, data);
} /* QueueEventValue */

/*
 * Queue Event with integer argument
 */
XBBool
QueueEventPointer (XBEventCode code, void *ptr)
{
  XBEventData data;
  data.ptr = ptr;
#ifdef DEBUG_XBEVENT
  if(code!=XBE_NONE&code!=XBE_TIMER)
  Dbg_Out ("queue event pointer %s %c,\n", EventName (code),data.value);
#endif
  return QueueEvent (code, data);
} /* QueueEventPointer */

/*
 * Queue Event with integer argument
 */
XBBool
QueueEventPos (XBEventCode code, short x, short y)
{
  XBEventData data;
  data.pos.x = x;
  data.pos.y = y;
  return QueueEvent (code, data);
} /* QueueEventValue */

/*
 * Queue Event with integer argument
 */
XBBool
QueueEventAtom (XBEventCode code, XBAtom atom)
{
  XBEventData data;
  data.atom = atom;
#ifdef DEBUG_XBEVENT
  if(code!=XBE_NONE&code!=XBE_TIMER)
  Dbg_Out ("queue event atom %s %c,\n", EventName (code),data.value);
#endif
  return QueueEvent (code, data);
} /* QueueEventValue */

