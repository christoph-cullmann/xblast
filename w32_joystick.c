/*
 * file w32_joystick.c - joystick support
 *
 * $Id: w32_joystick.c,v 1.6 2006/02/19 13:33:01 lodott Exp $
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
#include "w32_joystick.h"
#include "gui.h"

//#include "w32_mm.h"

/*
 * local macros
 */
#define MAX_JOYSTICK 2

/* flags for joystick X and Y positions */
#define JOYDIR_NONE  0x00
#define JOYDIR_UP    0x01
#define JOYDIR_DOWN  0x02
#define JOYDIR_Y     0x03
#define JOYDIR_LEFT  0x04
#define JOYDIR_RIGHT 0x08
#define JOYDIR_X     0x0C

/*
 * local types
 */

/* joystick data */
typedef struct
{
	XBEventCode event;			/* event code to send */
	unsigned dir;				/* current direction */
	unsigned xLow;				/* calibration data */
	unsigned xHigh;				/* calibration data */
	unsigned yLow;				/* calibration data */
	unsigned yHigh;				/* calibration data */
} XBJoystick;

/* 
 * local variables
 */

/* available joystick IDs */
static const UINT joyDevice[MAX_JOYSTICK] = {
	JOYSTICKID1,
	JOYSTICKID2,
};

/* joystick parameters */
static XBJoystick joystick[MAX_JOYSTICK];

/*
 *  how many joystick do we support
 */
int
GUI_NumJoysticks (void)
{
	int i, num;

	for (i = 0, num = 0; i < MAX_JOYSTICK; i++) {
		if (XBE_NONE != joystick[i].event) {
			num++;
		}
	}
	return num;
}								/* GUI_NumJoysticks */

/*
 * init joystick support
 */
XBBool
InitJoystick (void)
{
	int i, j;
	UINT joyThres;
	static JOYCAPS joyCaps;

	memset (joystick, 0, sizeof (joystick));
	for (i = 0, j = 0; i < joyGetNumDevs () && i < MAX_JOYSTICK && j < MAX_JOYSTICK; i++) {
		/* get device */
		if (JOYERR_NOERROR != joyGetDevCaps (joyDevice[i], &joyCaps, sizeof (joyCaps))) {
			continue;
		}
		/* set threshold */
		joyThres = ABS ((int)joyCaps.wYmin - (int)joyCaps.wYmax) / 8;
		if (JOYERR_NOERROR != joySetThreshold (joyDevice[i], joyThres)) {
			continue;
		}
		if (JOYERR_NOERROR != joySetCapture (window, joyDevice[i], 0, TRUE)) {
			continue;
		}
		joystick[i].event = XBE_JOYST_1 + j;
		joystick[i].dir = JOYDIR_NONE;
		joystick[i].xLow = 3 * (joyCaps.wXmin / 4) + 1 * (joyCaps.wXmax / 4);
		joystick[i].xHigh = 1 * (joyCaps.wXmin / 4) + 3 * (joyCaps.wXmax / 4);
		joystick[i].yLow = 3 * (joyCaps.wYmin / 4) + 1 * (joyCaps.wYmax / 4);
		joystick[i].yHigh = 1 * (joyCaps.wYmin / 4) + 3 * (joyCaps.wYmax / 4);
		j++;
		Dbg_Out ("Joystick (%d) %s connected as XBE_JOYST_%d. x=(%u,%u) y=(%u,%u)\n",
				 i + 1, joyCaps.szPname, j, joyCaps.wXmin, joyCaps.wXmax, joyCaps.wYmin,
				 joyCaps.wYmax);
	}
	return XBTrue;
}								/* InitJoystick */

/*
 * finish joystick support
 */
void
FinishJoystick (void)
{
	size_t i;

	for (i = 0; i < MAX_JOYSTICK; i++) {
		if (XBE_NONE != joystick[i].event) {
			joyReleaseCapture (joyDevice[i]);
		}
	}
}								/* FinishJoystick */

/*
 * calculate new joystick position for window message
 */
static unsigned
EvalJoystickMove (LONG lParam, const XBJoystick * pJoy)
{
	UINT xPos, yPos;
	unsigned newDir = 0;

	/* sanity check */
	assert (NULL != pJoy);

	/* get x-position */
	xPos = LOWORD (lParam);
	if (pJoy->xHigh < pJoy->xLow) {
		if (xPos < pJoy->xHigh) {
			newDir |= JOYDIR_LEFT;
		}
		else if (xPos > pJoy->xLow) {
			newDir |= JOYDIR_RIGHT;
		}
	}
	else {
		if (xPos > pJoy->xHigh) {
			newDir |= JOYDIR_RIGHT;
		}
		else if (xPos < pJoy->xLow) {
			newDir |= JOYDIR_LEFT;
		}
	}
	/* get y-position */
	yPos = HIWORD (lParam);
	if (pJoy->yHigh < pJoy->yLow) {
		if (yPos < pJoy->yHigh) {
			newDir |= JOYDIR_UP;
		}
		else if (yPos > pJoy->yLow) {
			newDir |= JOYDIR_DOWN;
		}
	}
	else {
		if (yPos > pJoy->yHigh) {
			newDir |= JOYDIR_DOWN;
		}
		else if (yPos < pJoy->yLow) {
			newDir |= JOYDIR_UP;
		}
	}
	return newDir;
}								/* EvalJoyMove */

/*
 * handle joysticks messages in menu mode
 */
void
HandleMenuJoy (UINT message, UINT wParam, LONG lParam)
{
	XBJoystick *pJoy;
	unsigned newDir;

	switch (message) {
		/* button presses */
	case MM_JOY1BUTTONDOWN:
	case MM_JOY2BUTTONDOWN:
		QueueEventValue (XBE_MENU, XBMK_SELECT);
		break;
		/* joystick movement */
	case MM_JOY1MOVE:
	case MM_JOY2MOVE:
		if (message == MM_JOY1MOVE) {
			pJoy = joystick + 0;
		}
		else {
			pJoy = joystick + 1;
		}
		newDir = EvalJoystickMove (lParam, pJoy);
		if (newDir != pJoy->dir) {
			/* test changes in y dir */
			if ((newDir & JOYDIR_Y) != (pJoy->dir & JOYDIR_Y)) {
				switch (newDir & JOYDIR_Y) {
				case JOYDIR_UP:
					QueueEventValue (XBE_MENU, XBMK_UP);
					break;
				case JOYDIR_DOWN:
					QueueEventValue (XBE_MENU, XBMK_DOWN);
					break;
				}
			}
			if ((newDir & JOYDIR_X) != (pJoy->dir & JOYDIR_X)) {
				switch (newDir & JOYDIR_X) {
				case JOYDIR_LEFT:
					QueueEventValue (XBE_MENU, XBMK_LEFT);
					break;
				case JOYDIR_RIGHT:
					QueueEventValue (XBE_MENU, XBMK_RIGHT);
					break;
				}
			}
			pJoy->dir = newDir;
		}
	}
}								/* HandleMenuJoy */

/*
 * handle joysticks messages in game mode
 */
void
HandleXBlastJoy (UINT message, UINT wParam, LONG lParam)
{
	XBJoystick *pJoy;
	unsigned newDir;
	int value;

	switch (message) {
		/* button presses */
	case MM_JOY1BUTTONDOWN:
	case MM_JOY2BUTTONDOWN:
		if (message == MM_JOY1BUTTONDOWN) {
			pJoy = joystick + 0;
		}
		else {
			pJoy = joystick + 1;
		}
		if (pJoy->event != XBE_NONE) {
			if (JOY_BUTTON1CHG & wParam) {
				QueueEventValue (pJoy->event, XBGK_BOMB);
			}
			if (JOY_BUTTON2CHG & wParam) {
				QueueEventValue (pJoy->event, XBGK_SPECIAL);
			}
		}
		break;
		/* moving */
	case MM_JOY1MOVE:
	case MM_JOY2MOVE:
		if (message == MM_JOY1MOVE) {
			pJoy = joystick + 0;
		}
		else {
			pJoy = joystick + 1;
		}
		if (pJoy->event != XBE_NONE) {
			newDir = EvalJoystickMove (lParam, pJoy);
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
			if (value != XBGK_NONE && newDir != pJoy->dir) {
				QueueEventValue (pJoy->event, value);
				pJoy->dir = newDir;
			}
		}
		break;
	}
}								/* HandleXBlastJoy */

/*
 * end fo file w32_joystick.c
 */
