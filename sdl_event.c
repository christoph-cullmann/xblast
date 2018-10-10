/*
 * file x11_event.c - event handling 
 *
 * $Id: sdl_event.c,v 1.2 2004/10/19 17:59:19 iskywalker Exp $
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
#include "x11_event.h"
#include "gui.h"

#include "x11_common.h"
#include "x11_socket.h"

#include "com.h"
#include "event.h"
#include "geom.h"
#include "cfg_control.h"
#include "player.h"
#include "SDL.h"
/*
 * local constants
 */
#define BELL_VOLUME 80

/*
 * local types
 */
typedef enum {
  TM_NONE,
  TM_ONCE,
  TM_PERIODIC
} TimerMode;

typedef struct {
  KeyCode     key;
  XBEventCode code;
  int         value;
} GameKeyEvent;

typedef struct _game_key_list {
  GameKeyEvent gke;
  struct _game_key_list *next;
} GameKeyList;

typedef struct _poll_func_list {
  XBPollFunction          func;
  struct _poll_func_list *next;
} PollFuncList;

/*
 * local variables
 */
/* Timeout */
static TimerMode      timerMode = TM_NONE;
static struct timeval nextTimer;
static struct timeval timerIncr;
/* polling */
static PollFuncList   *pollList = NULL;
static struct timeval  nextPoll;
/* keyboard mode */
static XBKeyboardMode keyboardMode = KB_NONE;
/* keyboard lookup tables */
static int           numKeyMenu      = 0;
static int           numKeyPress     = 0;
static int           numKeyRelease   = 0;
static GameKeyEvent *keyMenuTable    = NULL;
static GameKeyEvent *keyPressTable   = NULL;
static GameKeyEvent *keyReleaseTable = NULL;

/*
 * Set keyboard mode
 */
void
GUI_SetKeyboardMode (XBKeyboardMode Mode)
{
  keyboardMode = Mode;
} /* GUI_SetKeyboardMode */

/*
 * Set mouse moude
 */
void
GUI_SetMouseMode (XBBool enable)
{
  XSetWindowAttributes xswa;

  if (enable) {
    xswa.event_mask = EVENT_MASK_MOUSE;
  } else {
    xswa.event_mask = EVENT_MASK_NORMAL;
  }
  //  XChangeWindowAttributes (dpy, win, CWEventMask, &xswa);
} /* GUI_SetMouseMode */

/*
 * Comapre to GameKeyCode
 */
static int
CompareKeyCode (const void *a, const void *b)
{
  return (*(KeyCode *) a) - (*(KeyCode *) b);
} /* CompareKeyCode */

/*
 * create keyboard lookup table from init table
 */
static GameKeyEvent *
CreateGameKeyTable (const CFGKeyTable *init, int *nelem)
{
  const CFGKeyTable *ptr;
  GameKeyList       *list = NULL;
  GameKeyList       *elem;
  GameKeyEvent      *table = NULL;
  KeyCode            keyCode;
  KeySym             keySym;
  int                i;

  assert (nelem != NULL);
  *nelem = 0;
  /* create list with with all keymappings (part one) */
  for (ptr = init; ptr->keysym != NULL; ptr ++) {
    /* convert keysymbol name to keycode */
    keySym = XStringToKeysym (ptr->keysym);
    if (NoSymbol == keySym) {
      Dbg_Out ("unknown keysymbol %s.\n", ptr->keysym);
      continue;
    }
    keyCode = XKeysymToKeycode (dpy, keySym);
    if (0 == keyCode) {
      Dbg_Out ("unmapped keysymbol %s.\n", ptr->keysym);
      continue;
    }
    /* create new list element */
    elem = calloc (1, sizeof (GameKeyList) );
    assert (elem != NULL);
    elem->gke.key   = keyCode;
    elem->gke.code  = ptr->eventCode;
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
    i ++;
    /* next one */
    free (list);
    list = elem;
  }
  /* now sort it with quciksort */
  qsort (table, *nelem, sizeof (GameKeyEvent), CompareKeyCode);
  /* now check for double entries */
  for (i = 1; i < *nelem; i ++) {
    if (table[i-1].key == table[i].key) {
      GUI_ErrorMessage ("Multiple bindings for key \"%s\".", XKeysymToString (XKeycodeToKeysym (dpy, table[i].key, 0)));
      break;
    }
  }
  /* that's all */
  return table;
} /* CreateGameKeyTable */

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
} /* GUI_UpdateKeyTables */

/*
 * Init Event routine
 */
XBBool
InitEvent (void)
{
  /* setup keyboard */
  GUI_UpdateKeyTables ();
  /* register display for socket polling*/
  RegisterDisplay (ConnectionNumber (dpy));
  /* that's all */
  return XBTrue;
} /* InitEvent */

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
} /* FinishEvent */

/*
 * Set timer for event
 */
void
GUI_SetTimer (long msec, XBBool periodic)
{
  if (0 == msec) {
    timerMode = TM_NONE;
  } else {
    /* get current time */
    gettimeofday (&nextTimer, NULL);
    /* set next timeout */
    timerIncr.tv_sec  = msec / 1000;
    timerIncr.tv_usec = 1000 *(msec % 1000);
    /* --- */
    nextTimer.tv_sec  += timerIncr.tv_sec;
    nextTimer.tv_usec += timerIncr.tv_usec;
    if (nextTimer.tv_usec > 1000000L) {
      nextTimer.tv_usec -= 1000000L;
      nextTimer.tv_sec ++;
    }
    /* periodic ? */
    if (! periodic) {
      timerMode = TM_ONCE;
    } else {
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
    return TRUE;
  }
  if (a->tv_sec > b->tv_sec) {
    return FALSE;
  }
  return (a->tv_usec < b->tv_usec);
} /* CheckTimer */

/*
 * Calc difference between timvals
 */
static void
DeltaTimer (struct timeval *delta, struct timeval *a, struct timeval *b)
{
  delta->tv_sec  = a->tv_sec  - b->tv_sec;
  delta->tv_usec = a->tv_usec - b->tv_usec;
  if (delta->tv_usec < 0) {
    delta->tv_usec += 1000000;
    delta->tv_sec --;
  }
} /* DeltaTimer */

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
    nextPoll.tv_sec ++;
  }
  /* create new element */
  ptr = calloc (1, sizeof (*ptr));
  assert (NULL != ptr);
  ptr->func = func;
  ptr->next = pollList;
  pollList  = ptr;
} /* GUI_AddPollFunction */

/*
 * subtract poll function 
 */
void
GUI_SubtractPollFunction (XBPollFunction func)
{
  assert (pollList != NULL);
  
  if (pollList->func == func) {
    pollList = pollList->next;
  } else {
    PollFuncList *ptr;
    for (ptr = pollList; ptr->next != NULL; ptr = ptr->next) {
      if (ptr->next->func == func) {
	PollFuncList *save = ptr->next;
	ptr->next = save->next;
	free (save);
      }
    }
  }
} /* GUI_SubtractPollFunction */

/*
 * Handle X11-KEyboard-Event by looking up Menu Event
 */
static void
HandleMenuKey (XKeyEvent *xkey)
{
  GameKeyEvent *key;
  KeyCode       keyCode;
  /* search for key */
  keyCode = xkey->keycode;
  key = bsearch (&keyCode, keyMenuTable, numKeyMenu, sizeof (GameKeyEvent), CompareKeyCode);
  if (NULL != key) {
    //Dbg_Out("code %i value %c \n",key->code, key->value);
    QueueEventValue (key->code, key->value);
  }
} /* HandleMenuKey */

/*
 * Handle X11-KEyboard-Event by looking XBlast Event
 */
static void
HandleXBlastKey (XKeyEvent *xkey)
{
  GameKeyEvent *key_table;
  int           num_key;
  GameKeyEvent *key;
  KeyCode       keyCode;
  /* which table to loopkup */
  if (xkey->type == KeyPress) {
    key_table = keyPressTable;
    num_key   = numKeyPress;
  } else {
    key_table = keyReleaseTable;
    num_key   = numKeyRelease;
  }
  /* search for key */
  keyCode = xkey->keycode;
  key = bsearch (&keyCode, key_table, num_key, sizeof (GameKeyEvent), CompareKeyCode);
  if (NULL != key) {
    QueueEventValue (key->code, key->value);
  }
} /* HandleXBlastKey */

/*
 * Handle X11-KEyboard-Event by looking up ascii value of key
 */
static void
HandleAsciiKey (XKeyEvent *xkey)
{
  char buf[8];
  KeySym keySym;
  int len;
  
  len = XLookupString (xkey, buf, sizeof (buf), &keySym, NULL);
  // Dbg_Out("especial key b %i xk %c s %i l %i ks %i ip %i en %i back %i\n",buf[0],xkey->keycode,sizeof (buf),len,keySym,isprint(buf[0]),XK_End,XK_BackSpace);
  if (len == 1) {
    if (isprint (buf[0])) {
      /* printable character */
      QueueEventValue (XBE_ASCII, buf[0]);
    } else if (iscntrl (buf[0]) ) {
      /* control key */
      
      switch (keySym) {
      case  XK_BackSpace: 
	QueueEventValue (XBE_CTRL, XBCK_BACKSPACE); break;
      case XK_Escape: 
	QueueEventValue (XBE_CTRL, XBCK_ESCAPE);    break;
      case XK_Return: 
	QueueEventValue (XBE_CTRL, XBCK_RETURN);    break;
      case XK_Delete:
	QueueEventValue (XBE_CTRL, XBCK_DELETE);    break;
	
	/*    case 71: QueueEventValue (XBE_CTRL, XBCK_INSERT);    break;
      case 79: QueueEventValue (XBE_CTRL, XBCK_END);    break;
      case 127: QueueEventValue (XBE_CTRL, XBCK_DELETE);    break;
      case 129: QueueEventValue (XBE_CTRL, XBCK_HOME);    break;*/

      default: break;
      }
    }

  }
  else{
    if(keySym==XK_End){
      QueueEventValue (XBE_CTRL, XBCK_END);
    }
    if(keySym==XK_Home){
      QueueEventValue (XBE_CTRL, XBCK_HOME);
    }
    if(keySym==XK_Insert){
      QueueEventValue (XBE_CTRL, XBCK_INSERT);
    }
    if(keySym==XK_Delete){
      QueueEventValue (XBE_CTRL, XBCK_DELETE);
    }
    
  }
    
  
} /* HandleAsciiKey */

/*
 * Handle X11-KEyboard-Event by looking up keysymbol name
 */
static void
HandleKeysymKey (XKeyEvent *xkey)
{
  KeySym keySym;
  /* look up key smybol (lower case only) */
  keySym = XLookupKeysym (xkey, 0);
  if (keySym == XK_Escape) {
    QueueEventAtom (XBE_KEYSYM, ATOM_INVALID);
  } else if (keySym != NoSymbol) {
    char *keyName = XKeysymToString (keySym);
    if (NULL != keyName) {
      QueueEventAtom (XBE_KEYSYM, GUI_StringToAtom (keyName) );
    }
  }
} /* HandleKeysymKey */

/*
 * Handle X11 Events
 */
static void
HandleX11Event (XEvent *xev)
{
  BMPlayer         *ps;
  int counter=0,numofplayers;
  switch (xev->type) {
    /* window is iconfied */
  case UnmapNotify:
    iconified = TRUE;
    break;

    /* window is mapped again */
  case MapNotify:
    iconified = FALSE;
    break;

    /* part of the window was exposed */
  case Expose:
    GUI_FlushPixmap(FALSE);
    break;
    
    /* a key was pressed */
  case KeyPress:
    switch (keyboardMode) {
    case KB_MENU:
      
      if(GetChatMode()>0)
	HandleAsciiKey (&xev->xkey); 
      else
	HandleMenuKey   (&xev->xkey); 
      break;
    case KB_XBLAST:  
      numofplayers=GetNumOfPlayers();
      for (ps = player_stat,counter=0; ps < player_stat + numofplayers; ps ++,counter++) {
	  if (ps->local) {
	    break;
      	  }
	}
      //      Dbg_Out(" Chatmode %i \n",ps->chatmode);
      if(ps->chatmode==1){
	HandleAsciiKey (&xev->xkey); 
      }
      // else 
       HandleXBlastKey (&xev->xkey); break;
    case KB_ASCII:  HandleAsciiKey  (&xev->xkey); break;
    case KB_KEYSYM: HandleKeysymKey (&xev->xkey); break;
    default:        break;
    }
    break;

    /* a key was released */
  case KeyRelease:
    switch (keyboardMode) {
    case KB_XBLAST: HandleXBlastKey (&xev->xkey); break;
    default:        break;
    }
    break;

    /* a mouse button has been pressed */
  case ButtonPress:
    switch (xev->xbutton.button) {
    case Button1: QueueEventPos (XBE_MOUSE_1, xev->xbutton.x / BASE_X, xev->xbutton.y / BASE_Y); break;
    case Button2: QueueEventPos (XBE_MOUSE_3, xev->xbutton.x / BASE_X, xev->xbutton.y / BASE_Y); break;
    case Button3: QueueEventPos (XBE_MOUSE_2, xev->xbutton.x / BASE_X, xev->xbutton.y / BASE_Y); break;
    default:      break;
    }
    break;
  case ButtonRelease:
    switch (xev->xbutton.button) {
    case Button1: QueueEventPos (XBE_RMOUSE_1, xev->xbutton.x / BASE_X, xev->xbutton.y / BASE_Y); break;
    case Button2: QueueEventPos (XBE_RMOUSE_3, xev->xbutton.x / BASE_X, xev->xbutton.y / BASE_Y); break;
    case Button3: QueueEventPos (XBE_RMOUSE_2, xev->xbutton.x / BASE_X, xev->xbutton.y / BASE_Y); break;
    default:      break;
    }
    break;
    /* the mouse pointer has been moved */
  case MotionNotify:
    QueueEventPos (XBE_MOUSE_MOVE, xev->xmotion.x / BASE_X, xev->xmotion.y / BASE_Y);
    break;
  }
} /* HandleX11Event */

/*
 * check application and poll timer
 */ 
struct timeval *
HandleTimeout (XBBool peek)
{
  struct timeval         now;
  struct timeval        *timeout;
  static struct timeval  dTimer;
  static struct timeval  dPoll;
  static struct timeval  dPeek;
  
  /* application timer */
  if (timerMode == TM_NONE) {
    timeout = NULL;
  } else {
    gettimeofday (&now, NULL);
    if (CheckTimer (&nextTimer, &now)) {
      /* timer has triggered */
      if (timerMode == TM_ONCE) {
	/* this timer will only fire once */
	timerMode = TM_NONE;
      } else {
	/* set new timer value */
	nextTimer.tv_sec  += timerIncr.tv_sec;
	nextTimer.tv_usec += timerIncr.tv_usec;
	if (nextTimer.tv_usec > 1000000L) {
	  nextTimer.tv_usec -= 1000000L;
	  nextTimer.tv_sec ++;
	}
      }
      /* create new timer event */
      QueueEventVoid (XBE_TIMER);
      dTimer.tv_sec  = 0;
      dTimer.tv_usec = 0;
      timeout = &dTimer;
    } else {
      /* no timeout yet , calc time to timeout */
      DeltaTimer (&dTimer, &nextTimer, &now);
      /* link to timeout */
      timeout = &dTimer;
    }
  }
  /* polling (once per second) */
  if (pollList != NULL) {
    gettimeofday (&now, NULL);
    if (CheckTimer (&nextPoll, &now) ) {
      PollFuncList *poll;
      for (poll = pollList; poll != NULL; poll = poll->next) {
	(*poll->func) (&now);
      }
      nextPoll.tv_sec  = now.tv_sec + 1; 
      nextPoll.tv_usec = now.tv_usec; 
#ifdef DEBUG_EVENT
      Dbg_Out ("poll\n");
#endif
    }      
    /* calc time for next polling */
    DeltaTimer (&dPoll, &nextPoll, &now);
    if (NULL == timeout || 
	CheckTimer (&dPoll, timeout) ) {
      timeout = &dPoll;
    }
  }
  /* only peeking */
  if (peek) {
    dPeek.tv_sec  = 0;
    dPeek.tv_usec = 0;
    timeout = &dPeek;
  }
  return timeout;
} /* CheckTimer */

/*
 * xblast main event routine
 */
XBEventCode
GUI_WaitEvent (XBEventData *data)
{
  XBEventCode     ecode;
  int      	  i, num_queued;
  XEvent   	  xev;
  struct timeval *timeout;

  /* only if there are no events in our xblast event queue */
  // while (XBE_NONE == (ecode = NextEvent (data) ) ) {
    /* wird sind schon Timeout */
    timeout = HandleTimeout (XBFalse);
    /* are their any X11-Events in the Queue */	
    //XFlush(dpy);
    /*    num_queued = XEventsQueued (dpy, QueuedAlready);
    for (i = 0; i < num_queued; i ++) {
      XNextEvent (dpy, &xev);
      HandleX11Event (&xev);
    }
    if (SelectSockets (keyboardMode, timeout)) {
      int num_queued = XEventsQueued (dpy, QueuedAfterReading);
      for (i = 0; i < num_queued; i ++) {
	XNextEvent (dpy, &xev);
	HandleX11Event (&xev);
      }
      }*/
    //   }
  /* we have an event, return it */
  return ecode;
} /* WaitEvent */

/*
 * xblast main event routine
 */
XBEventCode
GUI_PeekEvent (XBEventData *data)
{
  XBEventCode     ecode;
  int      	  i, num_queued;
  XEvent   	  xev;
  struct timeval *timeout;
	Window u1; int u2;
	Window current_win;
	int x, y;
	unsigned int mask;

  /* only if there are no events in our xblast event queue */
  if (XBE_NONE != (ecode = NextEvent (data) ) ) {
    return ecode;
  }
  /* wird sind schon Timeout */
  timeout = HandleTimeout (XBTrue);
  /* are their any X11-Events in the Queue */
  num_queued = XEventsQueued (dpy, QueuedAlready);
  for (i = 0; i < num_queued; i ++) {
    XNextEvent (dpy, &xev);
    XQueryPointer(dpy, win, &u1, &current_win,
		  &u2, &u2, &x, &y, &mask);
    HandleX11Event (&xev);
  }
  /* if (SelectSockets (keyboardMode, timeout)) {
    int num_queued = XEventsQueued (dpy, QueuedAfterReading);
    for (i = 0; i < num_queued; i ++) {
      XNextEvent (dpy, &xev);
      HandleX11Event (&xev);
    }
    }*/
  /* we have check everything now return event if any */
  return NextEvent (data);
} /* WaitEvent */

/*
 *
 */
void 
GUI_Sync (void)
{
	SDL_Event event;
 	
	/*  if ( SDL_WaitEvent(&event) < 0 ) {
			fprintf(stderr, "SDL_PullEvent() error: %s\n",
								SDL_GetError());


								}*/
} /* GUI_Sync */

/*
 *
 */
void
GUI_Bell (void)
{
  XBell (dpy, BELL_VOLUME);
} /* GUI_Bell */

/*
 * send event
 */
void
GUI_SendEventValue (XBEventCode code, int value)
{
  QueueEventValue (code, value);
} /* GUI_SendEventValue */

/*
 * end of file x11_event.c
 */
