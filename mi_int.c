/*
 * file mi_int.c - Mmenu item for editing integers
 *
 * $Id: mi_int.c,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#include "mi_int.h"

#include "mi_map.h"
#include "gui.h"

/*
 * local macros
 */
#define BLINK_RATE 8

#define FF_TEXT_FOCUS      (FF_Medium | FF_Black | FF_Left | FF_Outlined)
#define FF_TEXT_NO_FOCUS   (FF_Medium | FF_White | FF_Left) 
#define FF_VALUE_SELECT    (FF_Medium | FF_Black | FF_Left | FF_Boxed)
#define FF_VALUE_NO_SELECT (FF_Medium | FF_White | FF_Left | FF_Boxed)

/*
 * local types
 */
typedef struct {
  XBMenuItem item;
  const char *text;
  int        *pValue;
  int         min;
  int         max;
  char        work[20];
  size_t      pos;
  Sprite     *lSprite;
  Sprite     *eSprite;
  int         pollCount;
} XBMenuIntegerItem;

/*
 * a string item receives the focus
 */
static void
MenuIntegerFocus (XBMenuItem *ptr, XBBool flag)
{
  XBMenuIntegerItem *string = (XBMenuIntegerItem *) ptr;

  assert (string != NULL);
  assert (string->lSprite != NULL);
  SetSpriteAnime (string->lSprite, flag ? FF_TEXT_FOCUS : FF_TEXT_NO_FOCUS);
} /* MenuIntegerFocus */

/*
 * Menu Integer Item
 */
static void
MenuIntegerPoll (XBMenuItem *ptr)
{
  XBMenuIntegerItem *string = (XBMenuIntegerItem *) ptr;

  assert (string != NULL);
  string->pollCount ++;
  if ( (string->item.flags & MIF_SELECTED) &&
       (0 == (string->pollCount % BLINK_RATE) ) ) {
    if (0 == (string->pollCount % (2*BLINK_RATE) ) ) {
      SetSpriteAnime (string->eSprite, FF_VALUE_SELECT | FF_Cursor);
    } else {
      SetSpriteAnime (string->eSprite, FF_VALUE_SELECT);
    }
  }
} /* MenuIntegerPoll */

/*
 * event handling while string is selected
 */
static void
IntegerEventLoop (XBMenuItem *ptr)
{
  XBEventCode       event;
  XBEventData       data;
  XBMenuIntegerItem *integer = (XBMenuIntegerItem *) ptr;

  assert (integer != NULL);
  assert (integer->eSprite != NULL);

  integer->item.flags |= MIF_SELECTED;
  SetSpriteAnime (integer->eSprite, FF_VALUE_SELECT);
  GUI_SetKeyboardMode (KB_ASCII);
  /* event loop */
  while (1) {
    /* update window contents */
    MenuUpdateWindow ();
    /* get event from gui */
    while (XBE_TIMER != (event = GUI_WaitEvent (&data) ) ) {
      if (event == XBE_ASCII) {
	/* add a character if it a digit */
	if (isdigit (data.value) &&
	    integer->pos < sizeof (integer->work) - 1 ) {
	  integer->work[integer->pos ++] = (char) data.value;
	  integer->work[integer->pos]    = 0;
	} 
	SetSpriteText (integer->eSprite, integer->work);
      } else if (event == XBE_CTRL) {
	/* control key */
	switch (data.value) {
	  /* delete last character */
	case XBCK_BACKSPACE:
	  if (integer->pos > 0) {
	    integer->work[-- integer->pos] = 0;
	    SetSpriteText (integer->eSprite, integer->work);
	  }
	  break; 
	  /* accept value if correct */
	case XBCK_RETURN: 
	  if (1 == sscanf (integer->work, "%d", integer->pValue) &&
	      *integer->pValue >= integer->min &&
	      *integer->pValue <= integer->max) {
	    goto Finish;
	  }
	  break;
	  /* reject value */
	case XBCK_ESCAPE:  
	  sprintf (integer->work, "%d", *integer->pValue);
	  integer->pos = strlen (integer->work);
	  goto Finish;
	default:
	  break;
	}
      }
    }
  }
 Finish:
  integer->item.flags &= ~MIF_SELECTED;
  SetSpriteText  (integer->eSprite, integer->work);
  SetSpriteAnime (integer->eSprite, FF_VALUE_NO_SELECT);
  GUI_SetKeyboardMode (KB_MENU);
  return;
} /* IntegerEventLoop */

/*
 * mouse click
 */
static void
MenuIntegerMouse (XBMenuItem *ptr, XBEventCode code)
{
  if (code == XBE_MOUSE_1) {
    IntegerEventLoop (ptr);
  }
} /* MenuIntegerMouse */

/*
 *
 */
XBMenuItem *
MenuCreateInteger (int x, int y, int w, const char *text, int wEdit, int *pValue, int min, int max)
{
  XBMenuIntegerItem *integer;

  assert (w - wEdit > 0);
  assert (pValue != NULL);
  /* create item */
  integer = calloc (1, sizeof (*integer) );
  assert (NULL != integer);
  MenuSetItem (&integer->item, MIT_Integer, x, y, w, CELL_H, MenuIntegerFocus, IntegerEventLoop, MenuIntegerMouse, MenuIntegerPoll);
  /* set label */
  integer->text    = text;
  integer->lSprite = CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, (w - wEdit - 2) * BASE_X, (CELL_H - 2) * BASE_Y,
				       FF_TEXT_NO_FOCUS, SPM_MAPPED);
  /* create work buffer */
  sprintf (integer->work, "%d", *pValue);
  integer->pValue = pValue;
  integer->min    = min;
  integer->max    = max;
  integer->pos    = strlen (integer->work);
  /* create editor */
  integer->eSprite = CreateTextSprite (integer->work, (x + w - wEdit + 1) * BASE_X, (y + 2) * BASE_Y,
				       (wEdit - 2) * BASE_X, (CELL_H - 4) * BASE_Y,
				       FF_VALUE_NO_SELECT, SPM_MAPPED);
  /* graphics */
  MenuAddLargeFrame ((x - CELL_W/2) / CELL_W, (x + w + CELL_W/2 - 1) / CELL_W, y / CELL_H);
  /* that's all */
  return &integer->item;
} /* CreateMenuInteger */

/*
 * delete a integer 
 */
void
MenuDeleteInteger (XBMenuItem *item)
{
  XBMenuIntegerItem *integer = (XBMenuIntegerItem *) item;

  assert (integer->lSprite != NULL);
  assert (integer->eSprite != NULL);
  DeleteSprite (integer->lSprite);
  DeleteSprite (integer->eSprite);
} /* DeleteComboItem */

/*
 * end of file mi_int.c
 */
