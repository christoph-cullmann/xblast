/*
 * file w32_event.c - event (message) handling for windows
 *
 * $Id: w32_event.c,v 1.29 2006/02/24 21:57:55 fzago Exp $
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
#include "w32_event.h"
#include "gui.h"

#include "SDL_vkeys.h"

#include "w32_pixmap.h"
#include "w32_keysym.h"
#include "w32_joystick.h"
#include "w32_socket.h"

#include "geom.h"
#include "cfg_control.h"
#include "timeval.h"
#include "chat.h"

/*
 * local constants
 */
#define TIMER_POLL 1

/*
 * local types
 */
typedef struct
{
	UINT key;
	XBEventCode code;
	int value;
} GameKeyEvent;

typedef struct _game_key_list
{
	GameKeyEvent gke;
	struct _game_key_list *next;
} GameKeyList;

typedef struct _poll_func_list
{
	XBPollFunction func;
	struct _poll_func_list *next;
} PollFuncList;

/*
 * local variables
 */

/* MM-Timer */
static UINT idTimer = 0;
static TIMECAPS timeCaps;

/* Polling */
static UINT pollTimer = 0;
static PollFuncList *pollList = NULL;

/* keyboard mode */
static XBKeyboardMode keyboardMode = KB_NONE;

/* mouse mode */
static XBBool mouseEnable = XBTrue;

/* keyboard lookup tables */
static int numKeyMenu = 0;
static int numKeyPress = 0;
static int numKeyRelease = 0;
static int numKeyChat = 0;
static GameKeyEvent *keyMenuTable = NULL;
static GameKeyEvent *keyPressTable = NULL;
static GameKeyEvent *keyReleaseTable = NULL;
static GameKeyEvent *keyChatTable = NULL;

/* clean exit */
static XBQuitFunction quitFunc = NULL;

/*
 * compare two GameKeyCode, for sorting
 */
static int
CompareKeyCode (const void *a, const void *b)
{
	return (*(UINT *) a) - (*(UINT *) b);
}								/* CompareKeyCode */

/*
 * create keyboard lookup table from init table
 */
static GameKeyEvent *
CreateGameKeyTable (const CFGKeyTable * init, int *nelem)
{
	const CFGKeyTable *ptr;
	GameKeyList *list = NULL;
	GameKeyList *elem;
	GameKeyEvent *table = NULL;
	UINT virtualKey;
	int i;

	assert (nelem != NULL);
	*nelem = 0;
	/* create list with with all keymappings (part one) */
	for (ptr = init; ptr->keysym != NULL; ptr++) {
		/* convert keysymbol name to keycode */
		virtualKey = StringToVirtualKey (ptr->keysym);
		if (0 == virtualKey) {
			Dbg_Out ("unknown keysymbol %s.\n", ptr->keysym);
			continue;
		}
		/* create new list element */
		elem = calloc (1, sizeof (GameKeyList));
		assert (elem != NULL);
		elem->gke.key = virtualKey;
		elem->gke.code = ptr->eventCode;
		elem->gke.value = ptr->eventData;
		/* put it into list */
		elem->next = list;
		list = elem;
		/* increment counter */
		*nelem += 1;
	}
	/* create lookup table */
	table = malloc (*nelem * sizeof (GameKeyEvent));
	assert (table != NULL);
	/* store list in array and delete it */
	i = 0;
	while (list != NULL) {
		/* store next one */
		elem = list->next;
		/* copy data */
		assert (i < *nelem);
		memcpy (table + i, &list->gke, sizeof (GameKeyEvent));
		i++;
		/* next one */
		free (list);
		list = elem;
	}
	/* now sort it with quicksort */
	qsort (table, *nelem, sizeof (GameKeyEvent), CompareKeyCode);
	/* now check for double entries */
	for (i = 1; i < *nelem; i++) {
		if (table[i - 1].key == table[i].key) {
			GUI_ErrorMessage ("Multiple bindings for key \"%s\".",
							  GUI_AtomToString (VirtualKeyToAtom (table[i].key)));
			break;
		}
	}
	/* that's all */
	return table;
}								/* CreateGameKeyTable */

/*
 * create keytables
 */
void
GUI_UpdateKeyTables (void)
{
	/* game key presses */
	if (NULL != keyPressTable) {
		free (keyPressTable);
	}
	keyPressTable = CreateGameKeyTable (GetGameKeyPressTable (), &numKeyPress);
	assert (keyPressTable != NULL);
	/* game key releases */
	if (NULL != keyReleaseTable) {
		free (keyReleaseTable);
	}
	keyReleaseTable = CreateGameKeyTable (GetGameKeyReleaseTable (), &numKeyRelease);
	assert (keyReleaseTable != NULL);
	/* menu keys */
	if (NULL != keyMenuTable) {
		free (keyMenuTable);
	}
	keyMenuTable = CreateGameKeyTable (GetMenuKeyTable (), &numKeyMenu);
	assert (keyMenuTable != NULL);
	/* chat keys */
	if (NULL != keyChatTable) {
		free (keyChatTable);
	}
	keyChatTable = CreateGameKeyTable (GetChatKeyTable (), &numKeyChat);
	assert (keyChatTable != NULL);
}								/* GUI_UpdateKeyTables */

/*
 * init timer
 */
static XBBool
InitTimer (void)
{
	if (0 != timeGetDevCaps (&timeCaps, sizeof (timeCaps))) {
		return XBFalse;
	}
	if (0 != timeBeginPeriod (15)) {
		return XBFalse;
	}
	Dbg_Out ("timer from %u to %u\n", timeCaps.wPeriodMin, timeCaps.wPeriodMax);
	return XBTrue;
}								/* InitTimer */

/*
 * callback
 */
static void PASCAL _export
TimerProc (UINT idEvent, UINT dummy, DWORD userData, DWORD dummy1, DWORD dummy2)
{
	PostMessage (window, userData, 0, 0);
}								/* TimerProc */

/*
 * finish timer
 */
static void
FinishTimer ()
{
	timeEndPeriod (15);
}								/* FinishTimer */

/*
 * init event data: key tables and timer
 */
XBBool
InitEvent (void)
{
	/* initialize keyboard mapping */
	GUI_UpdateKeyTables ();
	/* init timers */
	(void)InitTimer ();
	/* that's all */
	return XBTrue;
}								/* InitEvent */

/*
 * free all key tables in use
 */
void
FinishEvent (void)
{
	if (NULL != keyMenuTable) {
		free (keyMenuTable);
		keyMenuTable = NULL;
		numKeyMenu = 0;
	}
	if (NULL != keyPressTable) {
		free (keyPressTable);
		keyPressTable = NULL;
		numKeyPress = 0;
	}
	if (NULL != keyReleaseTable) {
		free (keyReleaseTable);
		keyReleaseTable = NULL;
		numKeyRelease = 0;
	}
	if (NULL != keyChatTable) {
		free (keyChatTable);
		keyChatTable = NULL;
	}
	FinishTimer ();
}								/* FinishEvent */

/*
 * add poll function to the gui event loop
 */
void
GUI_AddPollFunction (XBPollFunction func)
{
	PollFuncList *ptr;

	if (0 == pollTimer) {
		pollTimer = SetTimer (window, TIMER_POLL, 1000, NULL);
		assert (0 != pollTimer);
	}
	assert (func != NULL);
	ptr = calloc (1, sizeof (*ptr));
	assert (ptr != NULL);
	ptr->func = func;
	ptr->next = pollList;
	pollList = ptr;
}								/* GUI_AddPollFunction */

/*
 * subtract poll function
 */
void
GUI_SubtractPollFunction (XBPollFunction func)
{
	assert (pollList != NULL);
	if (pollList->func == func) {
		PollFuncList *save = pollList;
		pollList = pollList->next;
		free (save);
		if (pollTimer && !pollList) {
			KillTimer (window, pollTimer);
			pollTimer = 0;
		}
	}
	else {
		PollFuncList *ptr;
		for (ptr = pollList; ptr->next != NULL; ptr = ptr->next) {
			if (ptr->next->func == func) {
				PollFuncList *save = ptr->next;
				ptr->next = save->next;
				free (save);
			}
		}
	}
}								/* GUI_SubtractPollFunction */

/*
 * set timer for event
 */
void
GUI_SetTimer (long msec, XBBool periodic)
{
	Dbg_Out ("Timer = %ld msec\n", msec);
	if (0 != idTimer) {
		timeKillEvent (idTimer);
		idTimer = 0;
	}
	if (0 < msec) {
		if (periodic) {
			idTimer = timeSetEvent (msec, msec / 2, TimerProc, MSG_XBLAST_TIMER, TIME_PERIODIC);
		}
		else {
			(void)timeSetEvent (msec, msec / 2, TimerProc, MSG_XBLAST_TIMER, TIME_ONESHOT);
		}
	}
}								/* GUI_SetTimer */

/*
 * set keyboard mode, determines which events are generated by keypresses
 */
void
GUI_SetKeyboardMode (XBKeyboardMode _mode)
{
	keyboardMode = _mode;
}								/* GUI_SetKeyboardMode */

/*
 * set Mouse mode, determines if mouse events are generated
 */
void
GUI_SetMouseMode (XBBool _enable)
{
	mouseEnable = _enable;
}								/* GUI_SetMouseMode */

/*
 * Show/Hide cursor
 */
void GUI_ShowCursor(XBBool enable)
{
	/* TODO: show or hide the mouse cursor */
}

/*
 * lookup a key table, return success and match
 */
static XBBool
LookupKeyTable (WPARAM wParam, int numKey, const GameKeyEvent * keyTable, GameKeyEvent * key)
{
	GameKeyEvent *tmp;
	assert (NULL != key);
	tmp = bsearch (&wParam, keyTable, numKey, sizeof (GameKeyEvent), CompareKeyCode);
	if (NULL != tmp) {
		memcpy (key, tmp, sizeof (GameKeyEvent));
		return XBTrue;
	}
	return XBFalse;
}								/* LookupKeyTable */

/*
 * try to generate chat key event
 */
static XBBool
HandleChatKey (UINT message, WPARAM wParam, LPARAM lParam)
{
	GameKeyEvent key;
	/* only try to match if chat is active at all */
	if (Chat_isListening ()) {
		if (LookupKeyTable (wParam, numKeyMenu, keyChatTable, &key)) {
			/* check input activity for escape key */
			if ((key.value == XBCE_ESCAPE || key.value == XBCE_BACK || key.value == XBCE_ENTER)
				&& Chat_GetCurrentCode () == XBE_NONE) {
				return XBFalse;
			}
			QueueEventValue (key.code, key.value + 1000);
			return XBTrue;
		}
		/* if chat input is active, expect character event */
		if (Chat_GetCurrentCode () != XBE_NONE) {
			return XBTrue;
		}
	}
	return XBFalse;
}								/* HandleChatKey */

/*
 * try to generate menu key event, but first try chat key event
 */
static void
HandleMenuKey (UINT message, WPARAM wParam, LPARAM lParam)
{
	GameKeyEvent key;
	/* first try to match chat key */
	if (!HandleChatKey (message, wParam, lParam)) {
		/* match menu key otherwise */
		if (LookupKeyTable (wParam, numKeyMenu, keyMenuTable, &key)) {
			QueueEventValue (key.code, key.value);
			return;
		}
	}
}								/* HandleMenuKey */

/*
 * try to generate game key event; for keypresses, first try chat event
 */
static void
HandleXBlastKey (UINT message, WPARAM wParam, LPARAM lParam)
{
	GameKeyEvent key;
	GameKeyEvent *table = keyReleaseTable;
	int num = numKeyRelease;
	if (message == WM_KEYDOWN) {
		/* first try matching chat keys */
		if (HandleChatKey (message, wParam, lParam)) {
			return;
		}
		table = keyPressTable;
		num = numKeyPress;
	}
	/* no chat key, match xblast key */
	if (LookupKeyTable (wParam, num, table, &key)) {
		QueueEventValue (key.code, key.value);
		return;
	}
}								/* HandleXBlastKey */

/*
 * try to generate ascii key event
 */
static void
HandleAsciiKey (UINT message, WPARAM wParam, LPARAM lParam)
{
	char key = wParam;
	static int capslock = 0;
	/* generate only if requested or chat input is active */
	if (keyboardMode != KB_ASCII && Chat_GetCurrentCode () == XBE_NONE) {
		return;
	}
	/* generate ascii event if printable */
	if (isprint (key)) {
		QueueEventValue (XBE_ASCII, key);
	}
	else {
		/* generate control event */
		switch (key) {
			/* not needed yet, for future features */
		case VK_CAPITAL:
			capslock = !capslock;
			break;
			/* standard keys */
		case VK_BACK:
			QueueEventValue (XBE_CTRL, XBCK_BACKSPACE);
			break;
		case VK_ESCAPE:
			QueueEventValue (XBE_CTRL, XBCK_ESCAPE);
			break;
		case VK_RETURN:
			QueueEventValue (XBE_CTRL, XBCK_RETURN);
			break;
		case VK_INSERT:
			if (lParam != 3473409) {
				QueueEventValue (XBE_CTRL, XBCK_INSERT);
			}
			else {
				QueueEventValue (XBE_ASCII, key);
			}
			break;
		case VK_END:
			if (lParam == 21954561) {
				QueueEventValue (XBE_CTRL, XBCK_END);
			}
			else {
				QueueEventValue (XBE_ASCII, key);
			}
			break;
		case VK_DELETE:
			if (lParam == 22216705) {
				QueueEventValue (XBE_CTRL, XBCK_DELETE);
			}
			else {
				QueueEventValue (XBE_ASCII, key);
			}
			break;
		default:
			break;
		}
	}
}								/* HandleAsciiKey */

/*
 * tyr to generate keysmbol event
 */
static XBBool
HandleKeysymKey (UINT message, WPARAM wParam, LPARAM lParam)
{
	if (VK_ESCAPE == wParam) {
		QueueEventAtom (XBE_KEYSYM, ATOM_INVALID);
		return XBTrue;
	}
	else {
		XBAtom atom = VirtualKeyToAtom (wParam);
		if (ATOM_INVALID != atom) {
			QueueEventAtom (XBE_KEYSYM, atom);
			return XBTrue;
		}
	}
	return XBFalse;
}								/* HandleKeysymKey */

/*
 * call poll functions
 */
static void
HandlePoll (void)
{
	PollFuncList *ptr;
	struct timeval tv;
	int i = 0;
	gettimeofday (&tv, NULL);
	for (i = 0, ptr = pollList; ptr != NULL && pollList != NULL; ptr = ptr->next, i++) {
		assert (NULL != ptr->func);
		(*ptr->func) (&tv);
	}
}								/* HandlePoll */

/*
 * handle query for new palette
 */
static BOOL
HandleNewPalette (void)
{
	BOOL result = FALSE;

	/* we need the device context */
	HDC hdc = GetWindowDC (window);
	if (NULL == hdc) {
		return FALSE;
	}
	/* first call , we might need to create our own palette first */
	if (NULL == palette) {
		if (RC_PALETTE & GetDeviceCaps (hdc, RASTERCAPS)) {
			Dbg_Out ("create palette\n");
			palette = CreateHalftonePalette (hdc);
		}
	}
	if (NULL != palette) {
		HPALETTE oldPalette = SelectPalette (hdc, palette, FALSE);
		Dbg_Out ("realize palette\n");
		if (RealizePalette (hdc) > 0) {
			result = TRUE;
		}
		(void)SelectPalette (hdc, oldPalette, FALSE);
	}
	ReleaseDC (window, hdc);
	if (result) {
		Dbg_Out ("update window\n");
		InvalidateRect (window, NULL, FALSE);
	}
	return result;
}								/* HandleNewPalette */

/*
 * window event routine
 */
long PASCAL _export
WindowProc (HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		/* redraw (part of) window */
	case WM_PAINT:
		PaintPixmap (window);
		return 0;
	case WM_QUERYNEWPALETTE:
		return HandleNewPalette ();
		/* timer event */
	case WM_TIMER:
		HandlePoll ();
		return 0;
		/* mm timer */
	case MSG_XBLAST_TIMER:
		QueueEventVoid (XBE_TIMER);
		return 0;
		/* network event */
	case MSG_XBLAST_SELECT:
		HandleSelect (wParam, lParam);
		return 0;
		break;
		/* handle keypress events */
	case WM_SYSKEYDOWN:
		break;
	case WM_KEYDOWN:
		switch (keyboardMode) {
		case KB_CHAT:
			(void)HandleChatKey (message, wParam, lParam);
			break;
		case KB_MENU:
			HandleMenuKey (message, wParam, lParam);
			break;
		case KB_XBLAST:
			HandleXBlastKey (message, wParam, lParam);
			break;
		case KB_KEYSYM:
			HandleKeysymKey (message, wParam, lParam);
			break;
		default:
			break;
		}
		return 0;
		/* text input */
	case WM_SYSCHAR:
		break;
	case WM_CHAR:
		HandleAsciiKey (message, wParam, lParam);
		break;
		/* handle keyrelease events */
	case WM_SYSKEYUP:
		break;
	case WM_KEYUP:
		switch (keyboardMode) {
		case KB_XBLAST:
			HandleXBlastKey (message, wParam, lParam);
			break;
		default:
			break;
		}
		return 0;
		/* joystick */
	case MM_JOY1BUTTONDOWN:
	case MM_JOY2BUTTONDOWN:
	case MM_JOY1MOVE:
	case MM_JOY2MOVE:
		switch (keyboardMode) {
		case KB_XBLAST:
			HandleXBlastJoy (message, wParam, lParam);
			break;
		case KB_MENU:
			HandleMenuJoy (message, wParam, lParam);
			break;
		default:
			break;
		}
		return 0;
		/* mouse */
	case WM_LBUTTONDOWN:
		if (mouseEnable) {
			QueueEventPos (XBE_MOUSE_1, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
	case WM_RBUTTONDOWN:
		if (mouseEnable) {
			QueueEventPos (XBE_MOUSE_2, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
	case WM_MBUTTONDOWN:
		if (mouseEnable) {
			QueueEventPos (XBE_MOUSE_3, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
	case WM_LBUTTONUP:
		if (mouseEnable) {
			QueueEventPos (XBE_RMOUSE_1, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
	case WM_RBUTTONUP:
		if (mouseEnable) {
			QueueEventPos (XBE_RMOUSE_2, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
	case WM_MBUTTONUP:
		if (mouseEnable) {
			QueueEventPos (XBE_RMOUSE_3, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (mouseEnable) {
			QueueEventPos (XBE_MOUSE_MOVE, LOWORD (lParam) / BASE_X, HIWORD (lParam) / BASE_Y);
		}
		return 0;
		/* xblast event */
	case MSG_XBLAST_EVENT_VALUE:
		QueueEventValue ((XBEventCode) wParam, (int)lParam);
		return 0;
	case MSG_XBLAST_EVENT_POINTER:
		QueueEventPointer ((XBEventCode) wParam, (void *)lParam);
		return 0;
		/* end of game */
	case WM_DESTROY:
		PostQuitMessage (0);
		return 0;
	default:
		break;
	}
	return DefWindowProc (window, message, wParam, lParam);
}								/* WindowProc */

/*
 * wait for next event in gui event queue
 */
XBEventCode
GUI_WaitEvent (XBEventData * data)
{
	XBEventCode ecode;
	MSG msg;
	/* only if there are no events in our xblast event queue */
	while (XBE_NONE == (ecode = NextEvent (data))) {
		/* yes, now get windows events */
		if (GetMessage (&msg, NULL, 0, 0)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		else {
			if (NULL != quitFunc) {
				(*quitFunc) ();
			}
			exit (msg.wParam);
		}
	}
	return ecode;
}								/* GUI_WaitEvent */

/*
 * peek at next event in queue
 */
XBEventCode
GUI_PeekEvent (XBEventData * data)
{
	XBEventCode ecode;
	MSG msg;

	/* only if there are no events in our xblast event queue */
	while (XBE_NONE == (ecode = NextEvent (data))) {
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message != WM_QUIT) {
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
			else {
				if (NULL != quitFunc) {
					(*quitFunc) ();
				}
				exit (msg.wParam);
			}
		}
		else {
			return XBE_NONE;
		}
	}
	return ecode;
}								/* GUI_PeekMessage */

/*
 * put an event with value in queue
 */
void
GUI_SendEventValue (XBEventCode code, int value)
{
	PostMessage (window, MSG_XBLAST_EVENT_VALUE, (WPARAM) code, (LPARAM) value);
}								/* GUI_SendEvent */

/*
 * put an event with pointer in queue
 */
void
GUI_SendEventPointer (XBEventCode code, void *pointer)
{
	PostMessage (window, MSG_XBLAST_EVENT_POINTER, (WPARAM) code, (LPARAM) pointer);
}								/* GUI_SendEvent */

/*
 * synchronize with display (not needed for win32)
 */
void
GUI_Sync (void)
{
}								/* GUI_Sync */

/*
 * beep
 */
void
GUI_Bell (void)
{
	MessageBeep (0xFFFFFFFF);
}								/* GUI_Bell */

/*
 * set function when window is destroyed
 */
void
GUI_OnQuit (XBQuitFunction func)
{
	quitFunc = func;
}								/* GUI_OnQuit */

/*
 * end of file w32_event.c
 */
