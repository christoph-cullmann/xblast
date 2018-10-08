/*
 * file x11_event.c - event handling 
 *
 * $Id: sdl_event.c,v 1.11 2006/02/24 21:57:55 fzago Exp $
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

#include "sdl_common.h"

/*
 * local constants
 */
#define BELL_VOLUME 80

/*
 * local types
 */
typedef enum
{
	TM_NONE,
	TM_ONCE,
	TM_PERIODIC
} TimerMode;

typedef struct
{
	SDLKey key;
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
/* Timeout */
static TimerMode timerMode = TM_NONE;
static struct timeval nextTimer;
static struct timeval timerIncr;
/* polling */
static PollFuncList *pollList = NULL;
static struct timeval nextPoll;
/* keyboard mode */
static XBKeyboardMode keyboardMode = KB_NONE;
/* keyboard lookup tables */
static int numKeyMenu = 0;
static int numKeyPress = 0;
static int numKeyChat = 0;
static int numKeyRelease = 0;
static GameKeyEvent *keyMenuTable = NULL;
static GameKeyEvent *keyPressTable = NULL;
static GameKeyEvent *keyReleaseTable = NULL;
static GameKeyEvent *keyChatTable = NULL;
static void HandleAsciiKey (SDL_KeyboardEvent * KbEvent);
static XBBool HandleChatKey (SDL_KeyboardEvent * KbEvent);

/*
 * Set keyboard mode
 */
void
GUI_SetKeyboardMode (XBKeyboardMode Mode)
{
	keyboardMode = Mode;
}								/* GUI_SetKeyboardMode */

/*
 * Set mouse mode
 */
void
GUI_SetMouseMode (XBBool enable)
{
	/* TODO?: Enable or disable mouse events. We probably don't care. */
}

/*
 * Show/Hide cursor
 */
void GUI_ShowCursor(XBBool enable)
{
	if (enable) {
		SDL_ShowCursor(SDL_ENABLE);
	} else {
		SDL_ShowCursor(SDL_DISABLE);
	}
}

/*
 * Compare to GameKeyCode
 */
static int
CompareKeyCode (const void *a, const void *b)
{
	return (*(const SDLKey *) a) - (*(const SDLKey *) b);
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
	SDLKey keySym;
	int i;

	assert (nelem != NULL);
	*nelem = 0;
	/* create list with with all keymappings (part one) */
	for (ptr = init; ptr->keysym != NULL; ptr++) {
		/* convert keysymbol name to keycode */
		keySym = StringToVirtualKey (ptr->keysym);
		if (SDLK_UNKNOWN == keySym) {
			Dbg_Out ("unknown keysymbol %s.\n", ptr->keysym);
			continue;
		}
		/* create new list element */
		elem = calloc (1, sizeof (GameKeyList));
		assert (elem != NULL);
		elem->gke.key = keySym;
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
	/* now sort it with quciksort */
	qsort (table, *nelem, sizeof (GameKeyEvent), CompareKeyCode);
	/* now check for double entries */
	for (i = 1; i < *nelem; i++) {
		if (table[i - 1].key == table[i].key) {
			//  GUI_ErrorMessage ("Multiple bindings for key \"%s\".", XKeysymToString (XKeycodeToKeysym (dpy, table[i].key, 0)));
			break;
		}
	}
	/* that's all */
	return table;
}								/* CreateGameKeyTable */

/*
 * reinitialize key tables
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
 * Init Event routine
 */
XBBool
InitEvent (void)
{
	/* setup keyboard */
	GUI_UpdateKeyTables ();
	/* register display for socket polling */
	//  RegisterDisplay (ConnectionNumber (dpy));
	/* that's all */
	return XBTrue;
}								/* InitEvent */

/*
 * finish event handling
 */
void
FinishEvent (void)
{
	/* clean up key tables */
	if (NULL != keyMenuTable) {
		free (keyMenuTable);
		keyMenuTable = NULL;
	}
	if (NULL != keyPressTable) {
		free (keyPressTable);
		keyPressTable = NULL;
	}
	if (NULL != keyReleaseTable) {
		free (keyReleaseTable);
		keyReleaseTable = NULL;
	}
	if (NULL != keyChatTable) {
		free (keyChatTable);
		keyChatTable = NULL;
	}
}								/* FinishEvent */

/*
 * Set timer for event
 */
void
GUI_SetTimer (long msec, XBBool periodic)
{
	if (0 == msec) {
		timerMode = TM_NONE;
	}
	else {
		/* get current time */
		gettimeofday (&nextTimer, NULL);
		/* set next timeout */
		timerIncr.tv_sec = msec / 1000;
		timerIncr.tv_usec = 1000 * (msec % 1000);
		/* --- */
		nextTimer.tv_sec += timerIncr.tv_sec;
		nextTimer.tv_usec += timerIncr.tv_usec;
		if (nextTimer.tv_usec > 1000000L) {
			nextTimer.tv_usec -= 1000000L;
			nextTimer.tv_sec++;
		}
		/* periodic ? */
		if (!periodic) {
			timerMode = TM_ONCE;
		}
		else {
			timerMode = TM_PERIODIC;
		}
	}
}

/*
 * 
 */
static int
CheckTimer (struct timeval *a, struct timeval *b)
{
	if (a->tv_sec < b->tv_sec) {
		return XBTrue;
	}
	if (a->tv_sec > b->tv_sec) {
		return XBFalse;
	}
	return (a->tv_usec < b->tv_usec);
}								/* CheckTimer */

/*
 * Calc difference between timvals
 */
static void
DeltaTimer (struct timeval *delta, struct timeval *a, struct timeval *b)
{
	delta->tv_sec = a->tv_sec - b->tv_sec;
	delta->tv_usec = a->tv_usec - b->tv_usec;
	if (delta->tv_usec < 0) {
		delta->tv_usec += 1000000;
		delta->tv_sec--;
	}
}								/* DeltaTimer */

/*
 * insert poll function
 */
void
GUI_AddPollFunction (XBPollFunction func)
{
	PollFuncList *ptr;
	/* set timeout */
	if (NULL == pollList) {
		gettimeofday (&nextPoll, NULL);
		nextPoll.tv_sec++;
	}
	/* create new element */
	ptr = calloc (1, sizeof (*ptr));
	assert (NULL != ptr);
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
		pollList = pollList->next;
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

static GameKeyEvent *
LookupKeyTable (const SDLKey keyCode, const int numKey, const GameKeyEvent * keyTable)
{
	return bsearch (&keyCode, keyTable, numKey, sizeof (GameKeyEvent), CompareKeyCode);
}

/*
 * Handle Keyboard-Event by looking up Menu Event
 */
static void
HandleMenuKey (SDL_KeyboardEvent * KbEvent)
{
	SDLKey keyCode;

	keyCode = KbEvent->keysym.sym;
	if (!HandleChatKey (KbEvent)) {
		GameKeyEvent *key;

		key = LookupKeyTable (keyCode, numKeyMenu, keyMenuTable);
		if (key) {
			QueueEventValue (key->code, key->value);
		}
	}
}

/*
 * Handle Keyboard-Event by looking XBlast Event
 */
static void
HandleXBlastKey (SDL_KeyboardEvent * KbEvent)
{
	GameKeyEvent *key_table;
	int num_key;
	SDLKey keyCode;
	GameKeyEvent *key;

	/* which table to loopkup */
	if (KbEvent->state == SDL_PRESSED) {
		key_table = keyPressTable;
		num_key = numKeyPress;
		if (HandleChatKey (KbEvent)) {
			return;
		}
	}
	else {
		key_table = keyReleaseTable;
		num_key = numKeyRelease;
	}
	/* search for key */
	keyCode = KbEvent->keysym.sym;
	key = LookupKeyTable (keyCode, num_key, key_table);
	if (key) {
		QueueEventValue (key->code, key->value);
	}
}								/* HandleXBlastKey */

/*
 * handle chat key event
 */
static XBBool
HandleChatKey (SDL_KeyboardEvent * KbEvent)
{
	SDLKey keyCode;
	GameKeyEvent *key;

	keyCode = KbEvent->keysym.sym;
	key = LookupKeyTable (keyCode, numKeyChat, keyChatTable);
	if (key && Chat_isListening ()) {
		if ((keyCode == SDLK_BACKSPACE || keyCode == SDLK_ESCAPE || keyCode == SDLK_RETURN)
			&& Chat_GetCurrentCode () == XBE_NONE) {
			return XBFalse;
		}

		QueueEventValue (key->code, key->value + 1000);
		return XBTrue;
	}

	/* if chat input is active, expect character event */
	if (Chat_GetCurrentCode () != XBE_NONE) {
		HandleAsciiKey (KbEvent);
		return XBTrue;
	}

	return XBFalse;
}

/*
 * Handle Keyboard-Event by looking up ascii value of key
 */
static void
HandleAsciiKey (SDL_KeyboardEvent * KbEvent)
{
	SDLKey keyCode;
	keyCode = KbEvent->keysym.sym;

	if (isprint (keyCode)) {
		/* printable character */
		QueueEventValue (XBE_ASCII, keyCode);
	}
	else {
		/* control key */
		switch (keyCode) {
		case SDLK_BACKSPACE:
			QueueEventValue (XBE_CTRL, XBCK_BACKSPACE);
			break;
		case SDLK_ESCAPE:
			QueueEventValue (XBE_CTRL, XBCK_ESCAPE);
			break;
		case SDLK_RETURN:
			QueueEventValue (XBE_CTRL, XBCK_RETURN);
			break;
		default:
			break;
		}
	}

}

/*
 * Handle Keyboard-Event by looking up keysymbol name
 */
static void
HandleKeysymKey (SDL_KeyboardEvent * KbEvent)
{
	SDLKey keyCode;
	keyCode = KbEvent->keysym.sym;
	if (keyCode == SDLK_ESCAPE) {
		QueueEventAtom (XBE_KEYSYM, ATOM_INVALID);
	}
	else {
		XBAtom atom = VirtualKeyToAtom (keyCode);
		if (ATOM_INVALID != atom) {
			QueueEventAtom (XBE_KEYSYM, atom);
		}
	}
}

/*
 * Handle SDL Events
 */
static void
HandleSDLEvent (SDL_Event * event)
{
	switch (event->type) {
	case SDL_KEYDOWN:
		switch (keyboardMode) {
		case KB_CHAT:
			(void)HandleChatKey (&event->key);
			break;
		case KB_MENU:
			HandleMenuKey (&event->key);
			break;
		case KB_XBLAST:
			HandleXBlastKey (&event->key);
			break;
		case KB_ASCII:
			HandleAsciiKey (&event->key);
			break;
		case KB_KEYSYM:
			HandleKeysymKey (&event->key);
			break;
		default:
			break;
		}
		break;

	case SDL_KEYUP:
		switch (keyboardMode) {
		case KB_XBLAST:
			HandleXBlastKey (&event->key);
			break;
		default:
			break;
		}
		break;

	case SDL_MOUSEMOTION:
		QueueEventPos (XBE_MOUSE_MOVE, event->motion.x / BASE_X, event->motion.y / BASE_Y);
		break;

	case SDL_MOUSEBUTTONUP:
		break;

	case SDL_MOUSEBUTTONDOWN:
		switch (event->button.button) {
		case SDL_BUTTON_LEFT:
			QueueEventPos (XBE_MOUSE_1, event->button.x / BASE_X, event->button.y / BASE_Y);
			break;
		case SDL_BUTTON_RIGHT:
			QueueEventPos (XBE_MOUSE_2, event->button.x / BASE_X, event->button.y / BASE_Y);
			break;
		case SDL_BUTTON_MIDDLE:
			QueueEventPos (XBE_MOUSE_3, event->button.x / BASE_X, event->button.y / BASE_Y);
			break;
		default:
			break;
		}
		break;

	case SDL_JOYAXISMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYHATMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		if (keyboardMode == KB_MENU)
			HandleMenuJoystick (event);
		else
			HandleXBlastJoystick (event);
		break;

	case SDL_QUIT:
		exit (1);

	default:
		GUI_FlushPixmap (XBFalse);
		break;
	};
}

/*
 * check application and poll timer
 */
static struct timeval *
HandleTimeout (XBBool peek)
{
	struct timeval now;
	struct timeval *timeout;
	static struct timeval dTimer;
	static struct timeval dPoll;
	static struct timeval dPeek;

	/* application timer */
	if (timerMode == TM_NONE) {
		timeout = NULL;
	}
	else {
		gettimeofday (&now, NULL);
		if (CheckTimer (&nextTimer, &now)) {
			/* timer has triggered */
			if (timerMode == TM_ONCE) {
				/* this timer will only fire once */
				timerMode = TM_NONE;
			}
			else {
				/* set new timer value */
				nextTimer.tv_sec += timerIncr.tv_sec;
				nextTimer.tv_usec += timerIncr.tv_usec;
				if (nextTimer.tv_usec > 1000000L) {
					nextTimer.tv_usec -= 1000000L;
					nextTimer.tv_sec++;
				}
			}
			/* create new timer event */
			QueueEventVoid (XBE_TIMER);
			dTimer.tv_sec = 0;
			dTimer.tv_usec = 0;
			timeout = &dTimer;
		}
		else {
			/* no timeout yet , calc time to timeout */
			DeltaTimer (&dTimer, &nextTimer, &now);
			/* link to timeout */
			timeout = &dTimer;
		}
	}
	/* polling (once per second) */
	if (pollList != NULL) {
		gettimeofday (&now, NULL);
		if (CheckTimer (&nextPoll, &now)) {
			PollFuncList *poll;
			for (poll = pollList; poll != NULL; poll = poll->next) {
				(*poll->func) (&now);
			}
			nextPoll.tv_sec = now.tv_sec + 1;
			nextPoll.tv_usec = now.tv_usec;
#ifdef DEBUG_EVENT
			Dbg_Out ("poll\n");
#endif
		}
		/* calc time for next polling */
		DeltaTimer (&dPoll, &nextPoll, &now);
		if (NULL == timeout || CheckTimer (&dPoll, timeout)) {
			timeout = &dPoll;
		}
	}
	/* only peeking */
	if (peek) {
		dPeek.tv_sec = 0;
		dPeek.tv_usec = 0;
		timeout = &dPeek;
	}
	return timeout;
}								/* CheckTimer */

/*
 * xblast main event routine
 */
XBEventCode
GUI_WaitEvent (XBEventData * data)
{
	XBEventCode ecode;
	SDL_Event event;			/* Event structure */
	//  XEvent      xev;
	struct timeval *timeout;

	/* only if there are no events in our xblast event queue */
	while (XBE_NONE == (ecode = NextEvent (data))) {
		/* wird sind schon Timeout */
		timeout = HandleTimeout (XBFalse);
		/* are their any X11-Events in the Queue */

		SelectSockets (timeout);

		/* Check for events */
		/* Loop until there are no events left on the queue */
		while (SDL_PollEvent (&event)) {
			HandleSDLEvent (&event);
		}
	}
	/* we have an event, return it */
	return ecode;
}								/* WaitEvent */

/*
 * xblast main event routine
 */
XBEventCode
GUI_PeekEvent (XBEventData * data)
{
	XBEventCode ecode;
	SDL_Event event;			/* Event structure */
	struct timeval *timeout;

	/* only if there are no events in our xblast event queue */
	if (XBE_NONE != (ecode = NextEvent (data))) {
		return ecode;
	}
	/* wird sind schon Timeout */
	timeout = HandleTimeout (XBTrue);
	/* are their any X11-Events in the Queue */

	SelectSockets (timeout);

	/* Check for events */
	/* Loop until there are no events left on the queue */
	while (SDL_PollEvent (&event)) {
		HandleSDLEvent (&event);
	}

	return NextEvent (data);
}								/* WaitEvent */

/*
 *
 */
void
GUI_Sync (void)
{
	//That was easy.
}								/* GUI_Sync */

/*
 *
 */
void
GUI_Bell (void)
{
#ifdef WIN32
//  #include "windows.h"
	MessageBeep (0);
#else
	fputc ('\a', stderr);
	fflush (stderr);
#endif
}								/* GUI_Bell */

/*
 * send event
 */
void
GUI_SendEventValue (XBEventCode code, int value)
{
	QueueEventValue (code, value);
}								/* GUI_SendEventValue */

/*
 * end of file x11_event.c
 */
