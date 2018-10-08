/*
 * file w32_init.c - initialze Win32-graphics engine
 *
 * $Id: w32_init.c,v 1.4 2006/02/19 13:33:01 lodott Exp $
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
#include "gui.h"

#include "w32_config.h"
#include "w32_event.h"
#include "w32_joystick.h"
#include "w32_keysym.h"
#include "w32_pixmap.h"
#include "w32_sprite.h"
#include "w32_text.h"
#include "w32_image.h"
#include "w32_tile.h"

#include "version.h"
#include "geom.h"

/*
 * local function: InitWindow
 * description:    create the xblast main window
 * parameters:     title - title of window
 * return value:   XBTrue on success, XBFalse on error 
 */
static XBBool
InitWindow (const char *title)
{
	RECT cRect;
	RECT wRect;
	int x, y;
	int width, height;

	static WNDCLASS windowClass;

	/* register xblast class */
	windowClass.style = 0;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = instance;
	windowClass.hIcon = LoadIcon (instance, xblastClass);
	windowClass.hCursor = LoadCursor (NULL, IDC_ARROW);
	windowClass.hbrBackground = GetStockObject (BLACK_BRUSH);
	windowClass.lpszClassName = xblastClass;
	if (0 == RegisterClass (&windowClass)) {
		return XBFalse;
	}
	/* retrieve old geonetry */
	RetrieveWindowRect (&wRect);
	/* now create the window */
	window = CreateWindow (xblastClass, title,
						   WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
						   CW_USEDEFAULT, CW_USEDEFAULT,
						   wRect.right - wRect.left,
						   wRect.bottom - wRect.top, NULL, NULL, instance, NULL);
	if (window == NULL) {
		return XBFalse;
	}
	/* now show it (may be moved elsewhere) */
	ShowWindow (window, SW_SHOWDEFAULT);
	UpdateWindow (window);
	/* set window size correctly */
	if (GetClientRect (window, &cRect)) {
		x = wRect.left;
		y = wRect.top;
		width = wRect.right - wRect.left;
		height = wRect.bottom - wRect.top;
		/* adjust width and height, if client region has wrong size */
		if (PIXW != cRect.right - cRect.left) {
			width -= cRect.right - cRect.left - PIXW;
		}
		if (PIXH + SCOREH != cRect.bottom - cRect.top) {
			height -= cRect.bottom - cRect.top - PIXH - SCOREH;
		}
		Dbg_Out ("Window geom %dx%d+%d+%d\n", width, height, x, y);
		MoveWindow (window, x, y, width, height, TRUE);
	}
	/* that's all */
	Dbg_Out ("InitWindow successful\n");
	return XBTrue;
}								/* InitWindow */

/*
 * local function: FinishWindow 
 * description:    clean up window and its class
 */
static void
FinishWindow (void)
{
	RECT rect;

	/* store last position */
	if (GetWindowRect (window, &rect)) {
		StoreWindowRect (&rect);
	}
	/* delete window */
	DestroyWindow (window);
	/* unregister window class */
	UnregisterClass (xblastClass, instance);
}								/* FinishWindow */

/*
 * global function:  GUI_Init
 * description:      initializes win32-display
 * parameters:       title     - window caption
 *                   icon_name - ignored
 * return value:     0 on success, -1 on failure
 */
XBBool
GUI_Init (int argc, char *argv[])
{
	/* get program instance */
	instance = GetModuleHandleA (0);
	/* create game window */
	if (!InitWindow ("XBlast TNT " VERSION_STRING)) {
		return XBFalse;
	}
	/* init image loading */
	if (!InitImages ()) {
		return XBFalse;
	}
	/* create bitmap for double buffering */
	if (!InitPixmap ()) {
		return XBFalse;
	}
	/* setup text output and fonts */
	if (!InitText ()) {
		return XBFalse;
	}
	/* setup tile output */
	if (!InitTiles ()) {
		return XBFalse;
	}
	/* create sprites */
	if (!InitSprites ()) {
		return XBFalse;
	}
	/* setup keysmbol table */
	if (!InitKeysym ()) {
		return XBFalse;
	}
	/* Setup Event handler */
	if (!InitEvent ()) {
		return XBFalse;
	}
	/* Setup Joystick Handler */
	if (!InitJoystick ()) {
		return XBFalse;
	}
	return XBTrue;
}								/* GUI_Init */

/*
 * global function: GUI_Finish
 * description:     shutdown win32 interface
 */
void
GUI_Finish (void)
{
	/* cleanup joystick */
	FinishJoystick ();
	/* clean up event handling */
	FinishEvent ();
	/* setup keysmbol table */
	FinishKeysym ();
	/* remove all sprite bitmaps */
	FinishSprites ();
	/* romve all tiles */
	FinishTiles ();
	/* unload fonts etc */
	FinishText ();
	/* clear image conversion data structures */
	FinishImages ();
	/* remove double buffer */
	FinishPixmap ();
	/* remove window */
	FinishWindow ();
}								/* GUI_Finish */

/*
 * end of file w32_init.c
 */
