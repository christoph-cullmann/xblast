/*
 * file event.h - xblast event queue
 *
 * $Id: event.h,v 1.9 2004/08/16 22:14:16 iskywalker Exp $
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
#ifndef XBLAST_EVENT_H
#define XBLAST_EVENT_H

#include "xblast.h"

/*
 * atoms for faster and secure string handling
 */
typedef int XBAtom;
/* value for invalid atom */
#define ATOM_INVALID 0

/*
 * types of events xblast knows
 */
typedef enum {
  XBE_NONE,    	    /* no event */
  XBE_TIMER,   	    /* timer event */
  XBE_POLL,         /* it was undefined... */
  XBE_SOCKET_READ,  /* socket has become readable */
  XBE_SOCKET_WRITE, /* socket has become writable */
  XBE_MENU,    	    /* menu key was pressed */
  XBE_ASCII,   	    /* ASCII character has been entered */
  XBE_CTRL,    	    /* Control character has been entered */
  XBE_KEYSYM,  	    /* Key with given name has been pressed */
  XBE_XBLAST,       /* Special game has been pressed (exit/pause) */
  XBE_KEYB_1,  	    /* XBlast game key from keyboard 1 has been pressed/released */
  XBE_KEYB_2,  	    /* XBlast game key from keyboard 2 has been pressed/released */
  XBE_JOYST_1, 	    /* XBlast game key from joystick 1 has been pressed/released */
  XBE_JOYST_2, 	    /* XBlast game key from joystick 2 has been pressed/released */
  XBE_JOYST_3, 	    /* XBlast game key from joystick 3 has been pressed/released */
  XBE_JOYST_4, 	    /* XBlast game key from joystick 4 has been pressed/released */
  XBE_MOUSE_1,      /* Mouse Button 1 has been pressed */
  XBE_MOUSE_2,      /* Mouse Button 2 has been pressed */
  XBE_MOUSE_3,      /* Mouse Button 3 has been pressed */
  XBE_RMOUSE_1,      /* Mouse Button 1 has been released */
  XBE_RMOUSE_2,      /* Mouse Button 2 has been released  */
  XBE_RMOUSE_3,      /* Mouse Button 3 has been released  */
  XBE_MOUSE_MOVE,   /* Mouse has been moved */
  XBE_SERVER,       /* server has send data for next frame */
  NUM_XBE
} XBEventCode;

/*
 * menu key events
 */
typedef enum {
  XBMK_NONE,       /* nothing */
  XBMK_PREV,       /* goto to previous field */
  XBMK_NEXT,       /* goto to next field */
  XBMK_LEFT,       /* goto left field */
  XBMK_RIGHT,      /* goto right field */
  XBMK_UP,         /* goto upper field */
  XBMK_DOWN,       /* goto lower field */
  XBMK_SELECT,     /* select current field */
  XBMK_DEFAULT,    /* select default button */
  XBMK_ABORT,      /* abort current menu */
  XBMK_STARTCHAT,  /*  start a chat, in wait client menu */
  XBMK_SENDCHAT,   /* send a chat, in wait client menu */  
  XBMK_CANCELCHAT, /* cancel a chat, in wait client menu */
  XBMK_CHANGECHATMODE, /* change chat mode */
  NUM_XBMENUKEY
} XBMenuKey;

/*
 * control key events
 */ 
typedef enum {
  XBCK_NONE,      /* nothing */
  XBCK_RETURN,    /* return key or similar */
  XBCK_ESCAPE,    /* escape key or similar */
  XBCK_BACKSPACE,  /* backspace key or similar */
  XBCK_INSERT,  /* INSERT key or similar */
  XBCK_END,  /* END key or similar */
  XBCK_DELETE,  /* DELETE key or similar */
  XBCK_HOME,  /* HOME key or similar */
} XBCtrlKey;

typedef enum {
  XBXK_NONE,
  XBXK_EXIT  /* leave game immediately */
} XBXBlastKey;

/*
 * ganme key events
 */
typedef enum {
  XBGK_NONE,        /* nothing */
  XBGK_GO_UP,       /* start moving upwards */
  XBGK_GO_LEFT,     /* start moving left */
  XBGK_GO_DOWN,     /* start moving downwards */
  XBGK_GO_RIGHT,    /* start moving right */
  XBGK_STOP_UP,     /* stop moving upwards (when not using stop key) */
  XBGK_STOP_LEFT,   /* stop moving left (when not using stop key)*/
  XBGK_STOP_DOWN,   /* stop moving downwards (when not using stop key)*/
  XBGK_STOP_RIGHT,  /* stop moving right (when not using stop key)*/
  XBGK_STOP_ALL,    /* stop moving (only when using sttop key) */
  XBGK_BOMB,        /* drop a bomb */
  XBGK_SPECIAL,     /* activate special extra */
  XBGK_PAUSE,       /* activate game pause */
  XBGK_ABORT,       /* activate abort */
  XBGK_ABORT_CANCEL, /* cancel abort */
    /* Skywalker */
  XBGK_LAOLA, /*  activate laola */
  XBGK_LOOSER, /* activate looser */
  XBGK_BOT, /* activate looser */
  XBGK_CHAT_START,  /* chat start */
  XBGK_CHAT_SEND,   /* send chat msg */
  XBGK_CHAT_CANCEL, /* cancel chat */
  XBGK_CHAT_CHANGE_RECEIVER, /* cancel chat */
  NUM_XBGK
    /* */
} XBGameKey;

/*
 * server events
 */
typedef enum {
  XBSE_FINISH = -2, /* finish level */
  XBSE_ERROR  = -1, /* error in communication */
  XBSE_NONE         /* nothing */
} XBServerEvent;

/*
 * additional data for events
 */
typedef union {
  int     value;
  void   *ptr;
  XBAtom  atom;
  struct {
    short x;
    short y;
  } pos;
} XBEventData;

/*
 * global prototypes
 */
extern XBEventCode NextEvent (XBEventData *data);
extern XBBool QueueEventVoid (XBEventCode code);
extern XBBool QueueEventValue (XBEventCode code, int value);
extern XBBool QueueEventPointer (XBEventCode code, void *ptr);
extern XBBool QueueEventPos (XBEventCode code, short x, short y);
extern XBBool QueueEventAtom (XBEventCode code, XBAtom atom);

#endif
/*
 * end of file event.h
 */
