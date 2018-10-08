/*
 * file x11_joystick.c - joystick support for linux
 *
 * $Id: sdl_joystick.c 112466 2009-07-06 08:37:37Z ingmar $
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

#include "xblast.h"

#include <SDL.h>

#include "sdl_joystick.h"

#define JOYDIR_NONE  0x00
#define JOYDIR_UP    0x01
#define JOYDIR_DOWN  0x02
#define JOYDIR_Y     0x03
#define JOYDIR_LEFT  0x04
#define JOYDIR_RIGHT 0x08
#define JOYDIR_X     0x0C

#define THRESHOLD    4096
// old value: 8192

/*
 * local types
 */
typedef struct
{
	SDL_Joystick *joy;
	XBEventCode event;
	unsigned dir;
} XBJoystick;

/*
 * local variables
 */
/*
#ifndef NO_JOYSTICK
static const char *joyDevice[NUM_JOYSTICKS] = {
  "/dev/js0",
  "/dev/js1",
  "/dev/input/js0",
  "/dev/input/js1",
  "/dev/input/js2",
  "/dev/input/js3",
};

*/
static XBJoystick joystick[NUM_JOYSTICKS] = {
	{NULL, XBE_NONE, 0},
	{NULL, XBE_NONE, 0},
	{NULL, XBE_NONE, 0},
	{NULL, XBE_NONE, 0},
	{NULL, XBE_NONE, 0},
	{NULL, XBE_NONE, 0},
};

//#endif

/*
 * global function: GUI_NumJoysticks
 * description:     Query number of connected joysticks
 * return value:    # joysticks
 */
int
GUI_NumJoysticks (void)
{
	return SDL_NumJoysticks ();
}								/* GUI_NumJoysticks */

/*
 * Initialize 
 */
XBBool
InitJoystick (void)
{
	int i;

	if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) == -1) {
		fprintf (stderr, "Error: Could not initializate joystick!\n"
				 "Reason: %s\n", SDL_GetError ());
		return XBFalse;
	}

	for (i = 0; i < SDL_NumJoysticks () && i < NUM_JOYSTICKS; i++) {
		joystick[i].joy = SDL_JoystickOpen (i);
		if (joystick[i].joy) {
			printf ("Using joysitck %d\n", i);
			i++;
		}
		else
			printf ("Couldn't open joystick %d\n", i);
	}

	return XBTrue;
}								/* InitJoystick */

/*
 * shutdown
 */
void
FinishJoystick (void)
{
	int i;
	for (i = 0; i < NUM_JOYSTICKS; i++) {
		if (joystick[i].joy != NULL) {
			printf ("Closing joystick %d\n", i);
			SDL_JoystickClose (joystick[i].joy);
		}
	}
}								/* FinishJoystick */

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
		if (value > THRESHOLD)
			newDir |= JOYDIR_RIGHT;
		else if (value < -THRESHOLD)
			newDir |= JOYDIR_LEFT;
		break;
		/* y-axis */
	case 1:
		newDir &= ~JOYDIR_Y;
		if (value > THRESHOLD)
			newDir |= JOYDIR_DOWN;
		else if (value < -THRESHOLD)
			newDir |= JOYDIR_UP;
		break;
	default:
		break;
	}
#ifdef DEBUG_JOYSTICK
	Dbg_Out ("joy move: axis=%u value=%d => %02x\n", axis, value, newDir);
#endif
	return newDir;
}								/* EvalJoystickMove */
#endif

/*
 * handle new joystick event during game
 */
void
HandleXBlastJoystick (SDL_Event * event)
{
	unsigned newDir;
	int value;

	switch (event->type) {
	case SDL_JOYAXISMOTION:
		newDir = EvalJoystickMove (joystick[event->jaxis.which].dir,
								   event->jaxis.axis, event->jaxis.value);
		switch (newDir) {
		case JOYDIR_UP:
			value = XBGK_GO_UP;
			break;
		case JOYDIR_DOWN:
			value = XBGK_GO_DOWN;
			break;
		case JOYDIR_LEFT:
			value = XBGK_GO_LEFT;
			break;
		case JOYDIR_RIGHT:
			value = XBGK_GO_RIGHT;
			break;
		case JOYDIR_NONE:
			value = XBGK_STOP_ALL;
			break;
		default:
			value = XBGK_NONE;
			break;
		}
		if (value != XBGK_NONE && newDir != joystick[event->jaxis.which].dir) {
			QueueEventValue (joystick[event->jaxis.which].event, value);
			joystick[event->jaxis.which].dir = newDir;
		}
		break;

	case SDL_JOYHATMOTION:
		if (event->jhat.value & SDL_HAT_UP) {
			QueueEventValue (joystick[event->jaxis.which].event, XBGK_GO_UP);
			joystick[event->jaxis.which].dir |= JOYDIR_UP;
		}
		if (event->jhat.value & SDL_HAT_DOWN) {
			QueueEventValue (joystick[event->jaxis.which].event, XBGK_GO_DOWN);
			joystick[event->jaxis.which].dir |= JOYDIR_DOWN;
		}
		if (event->jhat.value & SDL_HAT_RIGHT) {
			QueueEventValue (joystick[event->jaxis.which].event, XBGK_GO_RIGHT);
			joystick[event->jaxis.which].dir |= JOYDIR_RIGHT;
		}
		if (event->jhat.value & SDL_HAT_LEFT) {
			QueueEventValue (joystick[event->jaxis.which].event, XBGK_GO_LEFT);
			joystick[event->jaxis.which].dir |= JOYDIR_LEFT;
		}
		if (event->jhat.value & XBGK_STOP_ALL) {
			QueueEventValue (joystick[event->jaxis.which].event, XBGK_STOP_ALL);
			joystick[event->jaxis.which].dir = JOYDIR_NONE;
		}
		break;

	case SDL_JOYBUTTONDOWN:
		if (event->jbutton.button == 0)
			QueueEventValue (joystick[event->jbutton.which].event, XBGK_BOMB);
		if (event->jbutton.button == 1)
			QueueEventValue (joystick[event->jbutton.which].event, XBGK_SPECIAL);
		break;
	}
}								/* HandleJoystick */

/*
 * handle new joystick event in menus
 */
void
HandleMenuJoystick (SDL_Event * event)
{
	unsigned newDir;

	switch (event->type) {
	case SDL_JOYAXISMOTION:
		newDir = EvalJoystickMove (joystick[event->jaxis.which].dir,
								   event->jaxis.axis, event->jaxis.value);
		/* test changes in y dir */
		if ((newDir & JOYDIR_Y) != (joystick[event->jaxis.which].dir & JOYDIR_Y)) {
			switch (newDir & JOYDIR_Y) {
			case JOYDIR_UP:
				QueueEventValue (XBE_MENU, XBMK_UP);
				break;
			case JOYDIR_DOWN:
				QueueEventValue (XBE_MENU, XBMK_DOWN);
				break;
			default:
				break;
			}
		}
		/* test changes in x dir */
		if ((newDir & JOYDIR_X) != (joystick[event->jaxis.which].dir & JOYDIR_X)) {
			switch (newDir & JOYDIR_X) {
			case JOYDIR_LEFT:
				QueueEventValue (XBE_MENU, XBMK_LEFT);
				break;
			case JOYDIR_RIGHT:
				QueueEventValue (XBE_MENU, XBMK_RIGHT);
				break;
			default:
				break;
			}
		}
		joystick[event->jaxis.which].dir = newDir;
		break;

	case SDL_JOYHATMOTION:
		if (event->jhat.value & SDL_HAT_UP)
			QueueEventValue (XBE_MENU, XBMK_UP);
		if (event->jhat.value & SDL_HAT_DOWN)
			QueueEventValue (XBE_MENU, XBMK_DOWN);
		if (event->jhat.value & SDL_HAT_RIGHT)
			QueueEventValue (XBE_MENU, XBMK_RIGHT);
		if (event->jhat.value & SDL_HAT_LEFT)
			QueueEventValue (XBE_MENU, XBMK_LEFT);
		break;

	case SDL_JOYBUTTONUP:
		QueueEventValue (XBE_MENU, XBMK_SELECT);
		break;
	}
}								/* HandleMenuJoystick */

/*
 * end of file x11_joystick.c
 */
