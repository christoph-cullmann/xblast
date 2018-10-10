/*
 * file mi_keysym.c - menu item for configuring key controls
 * 
 * $Id: mi_keysym.c,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#include "mi_keysym.h"

#include "mi_map.h"
#include "gui.h"
#include "str_util.h"

/*
 * local macros
 */
#define FF_TEXT_FOCUS       (FF_Medium | FF_Black | FF_Left   | FF_Outlined)
#define FF_TEXT_NO_FOCUS    (FF_Medium | FF_White | FF_Left)
#define FF_VALUE_SELECT     (FF_Medium | FF_Black | FF_Center | FF_Boxed )
#define FF_VALUE_NO_SELECT  (FF_Medium | FF_White | FF_Center | FF_Boxed )

/*
 * local types
 */
typedef struct {
  XBMenuItem  item;
  const char *text;
  char       *keyText;
  XBAtom      key;
  XBAtom     *pKey;
  Sprite     *textSprite;
  Sprite     *keySprite;
} XBMenuKeysymItem;

/*
 * a combo item receives the focus
 */
static void
MenuKeysymFocus (XBMenuItem *ptr, XBBool flag)
{
  XBMenuKeysymItem *keysym = (XBMenuKeysymItem *) ptr;

  assert (keysym != NULL);
  assert (keysym->textSprite != NULL);
  SetSpriteAnime (keysym->textSprite, flag ? FF_TEXT_FOCUS : FF_TEXT_NO_FOCUS);
} /* MenuKeysymFocus */

/*
 * event handling whiule combo is selected
 */
static void
KeysymEventLoop (XBMenuItem *ptr)
{
  XBEventCode event;
  XBEventData data;
  XBMenuKeysymItem *keysym = (XBMenuKeysymItem * )ptr;
  
  assert (keysym != NULL);
  assert (keysym->keySprite != NULL);
  
  SetSpriteAnime (keysym->keySprite, FF_VALUE_SELECT);
  SetSpriteText (keysym->keySprite, "???");
  /* event loop */
  GUI_SetKeyboardMode (KB_KEYSYM);
  while (1) {
    /* update window contents */
    MenuUpdateWindow ();
    /* get event from gui */
    while (XBE_TIMER != (event = GUI_WaitEvent (&data) ) ) {
      if (event == XBE_KEYSYM) {
	if (ATOM_INVALID == data.atom) {
	  /* delete symbol */
	  keysym->key   = ATOM_INVALID;
	  *keysym->pKey = ATOM_INVALID;
	  if (NULL != keysym->keyText) {
	    free (keysym->keyText);
	    keysym->keyText = NULL;
	  }
	} else if (data.atom != keysym->key) {
	  /* get new keysym */
	  keysym->key   = data.atom;
	  *keysym->pKey = data.atom;
	  if (NULL != keysym->keyText) {
	    free (keysym->keyText);
	  }
	  keysym->keyText = DupString (GUI_AtomToString (keysym->key));
	}
	/* leave edit mode */
	GUI_SetKeyboardMode (KB_MENU);
	SetSpriteAnime (keysym->keySprite, FF_VALUE_NO_SELECT);
	SetSpriteText (keysym->keySprite, keysym->keyText);
	return;
      }
    }
  }
} /* ComboEventLoop */

/*
 *  handle mouse click
 */
static void
MenuKeysymMouse (XBMenuItem *ptr, XBEventCode code)
{
  if (code == XBE_MOUSE_1) {
    KeysymEventLoop (ptr);
  }
} /* MenuKeysymMouse */

/*
 * create menu item
 */
XBMenuItem *
MenuCreateKeysym (int x, int y, int w, const char *text, XBAtom *pKey)
{
  XBMenuKeysymItem *keysym;

  assert (pKey != NULL);
  /* create item */
  keysym = calloc (1, sizeof (*keysym));
  assert (keysym != NULL);
  MenuSetItem (&keysym->item, MIT_Keysym, x, y, w, CELL_H, MenuKeysymFocus, KeysymEventLoop, MenuKeysymMouse, NULL);
  /* set specific data */
  keysym->text    = text;
  keysym->pKey    = pKey;
  keysym->key     = *pKey;
  if (ATOM_INVALID != *pKey) {
    keysym->keyText = DupString (GUI_AtomToString (*pKey));
    assert (keysym->keyText != NULL);
  } else {
    keysym->keyText = NULL;
  }
  /* create sprites */
  keysym->textSprite = CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, 
					 (w - 5*CELL_W/2 - 2) * BASE_X, (CELL_H - 2) * BASE_Y,
					 FF_TEXT_NO_FOCUS, SPM_MAPPED);
  assert (keysym->textSprite);
  keysym->keySprite  = CreateTextSprite (keysym->keyText, (x + w - 5*CELL_W/2 + 1) * BASE_X, (y + 2) * BASE_Y, 
					 (5*CELL_W/2 - 2) * BASE_X, (CELL_H - 4) * BASE_Y,
					 FF_VALUE_NO_SELECT, SPM_MAPPED);
  assert (keysym->keySprite);
  /* graphics */
  MenuAddLargeFrame ((x - CELL_W/2) / CELL_W, (x + w + CELL_W/2 - 1) / CELL_W, y / CELL_H);
  /* that's all */
  return &keysym->item;
} /* MenuCreateKeysym */

/*
 * delete item
 */
void 
MenuDeleteKeysym (XBMenuItem *item)
{
  XBMenuKeysymItem *keysym = (XBMenuKeysymItem *) item;

  assert (NULL != keysym);
  assert (NULL != keysym->textSprite);
  assert (NULL != keysym->keySprite);
  DeleteSprite (keysym->textSprite);
  DeleteSprite (keysym->keySprite);
  if (NULL != keysym->keyText) {
    free (keysym->keyText);
  } 
} /* MenuDeleteKeysym */

/*
 * end of file mi_keysym.h
 */
