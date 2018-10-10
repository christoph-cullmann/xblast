/*
 * file mi_string.c - string ediotr for menus
 *
 * $Id: mi_string.c,v 1.4 2004/08/07 01:11:22 iskywalker Exp $
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
#include "mi_string.h"

#include "mi_map.h"
#include "gui.h"

/*
 * local macros
 */
#define BLINK_RATE 8

#define FF_STRING_TEXT_FOCUS      (FF_Medium | FF_Black | FF_Left | FF_Outlined)
#define FF_STRING_TEXT_NO_FOCUS   (FF_Medium | FF_White | FF_Left) 
#define FF_STRING_VALUE_SELECT    (FF_Medium | FF_Black | FF_Left | FF_Boxed)
#define FF_STRING_VALUE_NO_SELECT (FF_Medium | FF_White | FF_Left | FF_Boxed)

/*
 * local types
 */
typedef struct {
  XBMenuItem item;
  const char *text;
  char       *buffer;
  size_t      len;
  char       *work;
  size_t      pos;
  Sprite     *lSprite;
  Sprite     *eSprite;
  int         pollCount;
} XBMenuStringItem;

/*
 * a string item receives the focus
 */
static void
MenuStringFocus (XBMenuItem *ptr, XBBool flag)
{
  XBMenuStringItem *string = (XBMenuStringItem *) ptr;

  assert (string != NULL);
  assert (string->lSprite != NULL);
  SetSpriteAnime (string->lSprite, flag ? FF_STRING_TEXT_FOCUS : FF_STRING_TEXT_NO_FOCUS);
} /* MenuStringFocus */

/*
 * Menu String Item
 */
static void
MenuStringPoll (XBMenuItem *ptr)
{
  XBMenuStringItem *string = (XBMenuStringItem *) ptr;

  assert (string != NULL);
  string->pollCount ++;
  if ( (string->item.flags & MIF_SELECTED) &&
       (0 == (string->pollCount % BLINK_RATE) ) ) {
    if (0 == (string->pollCount % (2*BLINK_RATE) ) ) {
      SetSpriteAnime (string->eSprite, FF_STRING_VALUE_SELECT | FF_Cursor);
    } else {
      SetSpriteAnime (string->eSprite, FF_STRING_VALUE_SELECT);
    }
  }
} /* MenuStringPoll */

/*
 * event handling while string is selected
 */
static void
StringEventLoop (XBMenuItem *ptr)
{
  XBEventCode       event;
  XBEventData       data;
  XBMenuStringItem *string = (XBMenuStringItem *) ptr;

  assert (string != NULL);
  assert (string->eSprite != NULL);

  string->item.flags |= MIF_SELECTED;
  SetSpriteAnime (string->eSprite, FF_STRING_VALUE_SELECT);
  GUI_SetKeyboardMode (KB_ASCII);
  /* event loop */
  while (1) {
    /* update window contents */
    MenuUpdateWindow ();
    /* get event from gui */
    while (XBE_TIMER != (event = GUI_WaitEvent (&data) ) ) {
      if (event == XBE_ASCII) {
	//	Dbg_Out(" mi_string %c \n",data.value);
	/* add a character */
	if (string->pos < string->len-1) {
	  string->work[string->pos ++] = (char) data.value;
	  string->work[string->pos]    = 0;
	} 
	SetSpriteText (string->eSprite, string->work);
      } else if (event == XBE_CTRL) {
	/* control key */
	switch (data.value) {
	  /* delete last character */
	case XBCK_BACKSPACE:
	  if (string->pos > 0) {
	    string->work[-- string->pos] = 0;
	    SetSpriteText (string->eSprite, string->work);
	  }
	  break; 
	  /* accept value */
	case XBCK_RETURN: 
	  memcpy (string->buffer, string->work, string->len);
	  goto Finish;
	  /* reject value */
	case XBCK_ESCAPE:  
	  memcpy (string->work, string->buffer, string->len);
	  string->pos = strlen (string->buffer);
	  goto Finish;
	default:
	  break;
	}
      } else if (event == XBE_MOUSE_1) {
	/* accept value */
	memcpy (string->buffer, string->work, string->len);
	goto Finish;
      } else if (event == XBE_MOUSE_2) {
	/* reject value */
	memcpy (string->work, string->buffer, string->len);
	string->pos = strlen (string->buffer);
	goto Finish;
      }
    }
  }
 Finish:
  string->item.flags &= ~MIF_SELECTED;
  SetSpriteText (string->eSprite, string->work);
  SetSpriteAnime (string->eSprite, FF_STRING_VALUE_NO_SELECT);
  GUI_SetKeyboardMode (KB_MENU);
  return;
} /* StringEventLoop */

/*
 * mouse click
 */
static void
MenuStringMouse (XBMenuItem *ptr, XBEventCode code)
{
  if (code == XBE_MOUSE_1) {
    StringEventLoop (ptr);
  }
} /* MenuStringMouse */

/*
 *
 */
XBMenuItem *
MenuCreateString (int x, int y, int w, const char *text, int wEdit, char *buffer, size_t len)
{
  XBMenuStringItem *string;

  assert (w - wEdit > 0);
  assert (buffer != NULL);
  assert (len > 0);
  /* create item */
  string = calloc (1, sizeof (*string) );
  assert (NULL != string);
  MenuSetItem (&string->item, MIT_String, x, y, w, CELL_H, MenuStringFocus, StringEventLoop, MenuStringMouse, MenuStringPoll);
  /* set label */
  string->text    = text;
  string->lSprite = CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, (w - wEdit - 2) * BASE_X, (CELL_H - 2) * BASE_Y,
				      FF_STRING_TEXT_NO_FOCUS, SPM_MAPPED);
  /* create work buffer */
  string->len    = len;
  string->buffer = buffer;
  string->work   = calloc (1, len);
  assert (NULL != string->work);
  memcpy (string->work, buffer, len);
  string->pos    = strlen (buffer);
  assert (string->pos < string->len);
  /* create editor */
  string->eSprite = CreateTextSprite (string->work, (x + w - wEdit + 1) * BASE_X, (y + 2) * BASE_Y,
				      (wEdit - 2) * BASE_X, (CELL_H - 4) * BASE_Y,
				      FF_STRING_VALUE_NO_SELECT, SPM_MAPPED);
  /* graphics */
  MenuAddLargeFrame ((x - CELL_W/2) / CELL_W, (x + w + CELL_W/2 - 1) / CELL_W, y / CELL_H);
  /* that's all */
  return &string->item;
} /* CreateMenuString */

/*
 * delete a string 
 */
void
MenuDeleteString (XBMenuItem *item)
{
  XBMenuStringItem *string = (XBMenuStringItem *) item;

  assert (string->lSprite != NULL);
  assert (string->eSprite != NULL);
  assert (string->work != NULL);
  DeleteSprite (string->lSprite);
  DeleteSprite (string->eSprite);
  free (string->work);
} /* DeleteComboItem */


/*
 * end of file mi_string.c
 */
