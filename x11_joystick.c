/*
 * file x11_joystick.c - joystick support for linux
 *
 * $Id: x11_joystick.c,v 1.3 2004/05/14 10:00:36 alfie Exp $
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
#if ! defined(linux) && ! defined(NO_JOYSTICK)
#define NO_JOYSTICK
#endif

#include "x11_joystick.h"
#include "gui.h"

#include "x11_socket.h"

#ifndef NO_JOYSTICK
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#endif

#define JOYDIR_NONE  0x00
#define JOYDIR_UP    0x01
#define JOYDIR_DOWN  0x02
#define JOYDIR_Y     0x03
#define JOYDIR_LEFT  0x04
#define JOYDIR_RIGHT 0x08
#define JOYDIR_X     0x0C

#define THRESHOLD    8192

/*
 * local types
 */
typedef struct {
  int         fd;
  XBEventCode event;
  unsigned    dir;
} XBJoystick;

/*
 * local variables
 */
#ifndef NO_JOYSTICK
static const char *joyDevice[NUM_JOYSTICKS] = {
  "/dev/js0",
  "/dev/js1",
  "/dev/input/js0",
  "/dev/input/js1",
  "/dev/input/js2",
  "/dev/input/js3",
};
static XBJoystick joystick[NUM_JOYSTICKS] = {
  { -1, XBE_NONE, 0 },
  { -1, XBE_NONE, 0 },
  { -1, XBE_NONE, 0 },
  { -1, XBE_NONE, 0 },
  { -1, XBE_NONE, 0 },
  { -1, XBE_NONE, 0 },
};
#endif

/*
 * global function: GUI_NumJoysticks
 * description:     Query number of connected joysticks
 * return value:    # joysticks
 */
int
GUI_NumJoysticks (void)
{
#ifndef NO_JOYSTICK
  size_t i, count;

  for (i = 0, count = 0; i < NUM_JOYSTICKS; i ++) {
    if (joystick[i].fd != -1) {
      count ++;
    }
  }
  return count;
#else
  return 0;
#endif
} /* GUI_NumJoysticks */


/*
 * Initialize 
 */
XBBool
InitJoystick (void)
{
#ifndef NO_JOYSTICK
  size_t i, j;

  for (i = 0, j = 0; i < NUM_JOYSTICKS; i ++) {
    if (-1 != (joystick[i].fd = open (joyDevice[i], O_RDONLY) ) ) {
      RegisterJoystick (joystick[i].fd);
      joystick[i].event = XBE_JOYST_1 + j;
      Dbg_Out ("joystick %s initialised\n", joyDevice[i]);
      j ++;
    } else {
      Dbg_Out ("joystick %s not initialised\n", joyDevice[i]);
    }
  }
#endif
  return XBTrue;
} /* InitJoystick */

/*
 * shutdown
 */
void
FinishJoystick (void)
{ 
#ifndef NO_JOYSTICK
  size_t i;

  for (i = 0; i < NUM_JOYSTICKS; i ++) {
    if (joystick[i].fd != -1) {
      close (joystick[i].fd);
    }
  }
#endif
} /* FinishJoystick */

#ifndef NO_JOYSTICK
/*
 * calculate new joystick direction from axis-event
 */
static unsigned
EvalJoystickMove (unsigned dir, unsigned axis, int value)
{
  unsigned newDir = dir;

  switch (axis) {
    /* x-Axis */
  case 0:
    newDir &= ~JOYDIR_X;
    if (value > THRESHOLD) {
      newDir |= JOYDIR_RIGHT;
    } else if (value < -THRESHOLD) {
      newDir |= JOYDIR_LEFT;
    } 
    break;
    /* y-axis */
  case 1:
    newDir &= ~JOYDIR_Y;
    if (value > THRESHOLD) {
      newDir |= JOYDIR_DOWN;
    } else if (value < -THRESHOLD) {
      newDir |= JOYDIR_UP;
    } 
    break;
  default:
    break;
  }
#ifdef DEBUG_JOYSTICK
  Dbg_Out ("joy move: axis=%u value=%d => %02x\n", axis, value, newDir);
#endif
  return newDir;
} /* EvalJoystickMove */
#endif

/*
 * handle new joystick event during game
 */
void
HandleXBlastJoystick (int fd)
{
#ifndef NO_JOYSTICK
  size_t          i;
  struct js_event jsEvent;
  unsigned        newDir;
  int             value;

  /* find joystick */
  for (i = 0; i < NUM_JOYSTICKS; i ++) {
    if (fd == joystick[i].fd) {
      break;
    }
  }
  if (i == NUM_JOYSTICKS) {
    return;
  }
  /* get event */
  if (sizeof (jsEvent) != read (fd, &jsEvent, sizeof (jsEvent) ) ) {
    return;
  }
  /* check type */
  switch (jsEvent.type & ~JS_EVENT_INIT) {
    /* 
     * handle button presses relases 
     */
  case JS_EVENT_BUTTON:
    if (jsEvent.value) {
      switch (jsEvent.number) {
      case 0:  QueueEventValue (joystick[i].event, XBGK_BOMB);    break;
      case 1:  QueueEventValue (joystick[i].event, XBGK_SPECIAL); break;
      default: break;
      }
    }
    break;
    /* 
     *  handle direction changes 
     */
  case JS_EVENT_AXIS:
    newDir = EvalJoystickMove (joystick[i].dir, jsEvent.number, jsEvent.value);
    switch (newDir) {
    case JOYDIR_UP:	value = XBGK_GO_UP;      break;
    case JOYDIR_DOWN:	value = XBGK_GO_DOWN;    break;
    case JOYDIR_LEFT:	value = XBGK_GO_LEFT;    break;
    case JOYDIR_RIGHT:	value = XBGK_GO_RIGHT;   break;
    case JOYDIR_NONE:   value = XBGK_STOP_ALL;   break;
    default:            value = XBGK_NONE;       break;
    }
    if (value  != XBGK_NONE &&
	newDir != joystick[i].dir) {
      QueueEventValue (joystick[i].event, value);
      joystick[i].dir = newDir;
    }
  }
#endif
} /* HandleJoystick */

/*
 * handle new joystick event in menus
 */
void
HandleMenuJoystick (int fd)
{
#ifndef NO_JOYSTICK
  size_t          i;
  struct js_event jsEvent;
  unsigned        newDir;

  /* find joystick */
  for (i = 0; i < NUM_JOYSTICKS; i ++) {
    if (fd == joystick[i].fd) {
      break;
    }
  }
  if (i == NUM_JOYSTICKS) {
    return;
  }
  /* get event */
  if (sizeof (jsEvent) != read (fd, &jsEvent, sizeof (jsEvent) ) ) {
    return;
  }
  /* check type */
  switch (jsEvent.type & ~JS_EVENT_INIT) {
    /* 
     * handle button presses relases 
     */
  case JS_EVENT_BUTTON:
    if (jsEvent.value) {
      QueueEventValue (XBE_MENU, XBMK_SELECT);
    }
    break;
    /* 
     *  handle direction changes 
     */
  case JS_EVENT_AXIS:
    newDir = EvalJoystickMove (joystick[i].dir, jsEvent.number, jsEvent.value);
    /* test changes in y dir */
    if ( (newDir & JOYDIR_Y) != (joystick[i].dir & JOYDIR_Y) ) {
      switch (newDir & JOYDIR_Y) {
      case JOYDIR_UP:   QueueEventValue (XBE_MENU, XBMK_UP);   break;
      case JOYDIR_DOWN: QueueEventValue (XBE_MENU, XBMK_DOWN); break;
      default:          break;
      }
    }
    /* test changes in x dir */
    if ( (newDir & JOYDIR_X) != (joystick[i].dir & JOYDIR_X) ) {
      switch (newDir & JOYDIR_X) {
      case JOYDIR_LEFT:  QueueEventValue (XBE_MENU, XBMK_LEFT);  break;
      case JOYDIR_RIGHT: QueueEventValue (XBE_MENU, XBMK_RIGHT); break;
      default:           break;
      }
    }
    joystick[i].dir = newDir;
  }
#endif
} /* HandleMenuJoystick */

/*
 * end of file x11_joystick.c
 */
