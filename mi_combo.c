/*
 * file mi_combo.c - simple box box 
 *
 * $Id: mi_combo.c,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#include "mi_combo.h"

#include "mi_map.h"
#include "gui.h"

/*
 * local macros
 */ 
#define FF_COMBO_TEXT_FOCUS       (FF_Medium | FF_Black | FF_Left   | FF_Outlined)
#define FF_COMBO_TEXT_NO_FOCUS    (FF_Medium | FF_White | FF_Left)
#define FF_COMBO_VALUE_SELECT     (FF_Medium | FF_Black | FF_Center | FF_Boxed )
#define FF_COMBO_VALUE_NO_SELECT  (FF_Medium | FF_White | FF_Center | FF_Boxed )

/*
 * local types
 */
/* simple combo box */
typedef struct {
  XBMenuItem         item;
  const char  	    *text;
  int        	    *value;
  void   	   **data;
  XBAtom            *atom;
  XBComboEntryList  *table;
  int        	     index;
  Sprite     	    *t_sprite;
  Sprite     	    *v_sprite;
} XBMenuComboItem;

/*
 * a combo item receives the focus
 */
static void
MenuComboFocus (XBMenuItem *ptr, XBBool flag)
{
  XBMenuComboItem *combo = (XBMenuComboItem *) ptr;

  assert (combo != NULL);
  assert (combo->t_sprite != NULL);
  SetSpriteAnime (combo->t_sprite, flag ? FF_COMBO_TEXT_FOCUS : FF_COMBO_TEXT_NO_FOCUS);
} /* MenuComboFocus */

/*
 * Menu Combo Item
 */
static void
MenuComboPoll (XBMenuItem *ptr)
{
  int index = 0;
  XBMenuComboItem *combo = (XBMenuComboItem *) ptr;

  assert (combo != NULL);
  assert (combo->table != NULL);
  assert (combo->v_sprite != NULL);
  /* find current value in table */
  if (combo->value != NULL) {
    for (index = 0; combo->table[index].text != NULL; index ++) {
      if (combo->table[index].value == *combo->value) {
	break;
      }
    }
  }
  if (combo->data != NULL) {
    for (index = 0; combo->table[index].text != NULL; index ++) {
      if (combo->table[index].data == *combo->data) {
	break;
      }
    }
  }
  if (combo->atom != NULL) {
    for (index = 0; combo->table[index].text != NULL; index ++) {
      if (combo->table[index].atom == *combo->atom) {
	break;
      }
    }
  }
  /* check if index has changed */
  if (index != combo->index) {
    combo->index = index;
    /* set values according entry */
    if (combo->value != NULL) {
      *combo->value = combo->table[index].value;
    }
    if (combo->data != NULL) {
      *combo->data = combo->table[index].data;
    }
    if (combo->atom != NULL) {
      *combo->atom = combo->table[index].atom;
    }
    /* change value display */
    SetSpriteText (combo->v_sprite, combo->table[index].text);
  }
} /* MenuComboPoll */

/*
 * combo item: select new element
 */
static void
SetCombo (XBMenuComboItem *combo, XBBool next)
{
  if (next) {
    /* go forward in table */
    combo->index ++;
    if (NULL == combo->table[combo->index].text) {
      /* reched end of table */
      combo->index = 0;
    }
  } else {
    /* go backward in table */
    if (combo->index <= 0) {
      /* search for last entry */
      for (combo->index = 0; combo->table[combo->index].text != NULL; combo->index ++) continue;
    } 
    combo->index --;
  }
  /* set new text for sprite */
  SetSpriteText (combo->v_sprite, combo->table[combo->index].text);
  /* set new value */
  if (NULL != combo->value) {
    *combo->value = combo->table[combo->index].value;
  }
  if (NULL != combo->data) {
    *combo->data = combo->table[combo->index].data;
  }
  if (NULL != combo->atom) {
    *combo->atom = combo->table[combo->index].atom;
  }
} /* SetCombo */

/*
 *
 */
static void
MenuComboMouse (XBMenuItem *ptr, XBEventCode code)
{
  switch (code) {
  case XBE_MOUSE_1:
    SetCombo ((XBMenuComboItem *) ptr, XBTrue);
    break;
  case XBE_MOUSE_2:
    SetCombo ((XBMenuComboItem *) ptr, XBFalse);
    break;
  default:
    break;
  }    
} /* MenuComboMouse */

/*
 * event handling whiule combo is selected
 */
static void
ComboEventLoop (XBMenuItem *ptr)
{
  XBEventCode event;
  XBEventData data;
  XBMenuComboItem *combo = (XBMenuComboItem * )ptr;
  
  assert (combo != NULL);
  assert (combo->v_sprite != NULL);
  
  SetSpriteAnime (combo->v_sprite, FF_COMBO_VALUE_SELECT);
  /* event loop */
  while (1) {
    /* update window contents */
    MenuUpdateWindow ();
    /* get event from gui */
    while (XBE_TIMER != (event = GUI_WaitEvent (&data) ) ) {
      if (event == XBE_MENU) {
	switch (data.value) {
	case XBMK_PREV:   
	case XBMK_UP:   
	  SetCombo (combo, XBFalse); 
	  break;
	case XBMK_NEXT:   
	case XBMK_DOWN:   
	  SetCombo (combo, XBTrue); 
	  break;
	case XBMK_SELECT: 
	case XBMK_ABORT:  
	  SetSpriteAnime (combo->v_sprite, FF_COMBO_VALUE_NO_SELECT);
	  return;
	default:  
	  break;
	}
      }
    }
  }
} /* ComboEventLoop */

/*
 *
 */
XBMenuItem *
MenuCreateCombo (int x, int y, int w, const char *text, int w_val, 
		 int *value, void **data, XBAtom *atom, XBComboEntryList *table)
{
  XBMenuComboItem *combo;
  
  assert (w - w_val > 0);
  combo = calloc (1, sizeof (*combo) );
  assert (combo != NULL);
  MenuSetItem (&combo->item, MIT_Combo, x, y, w, CELL_H, MenuComboFocus, ComboEventLoop, MenuComboMouse, MenuComboPoll);
  /* set label */
  combo->text     = text;
  combo->t_sprite = CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, (w - w_val - 2) * BASE_X, (CELL_H - 2) * BASE_Y,
				      FF_COMBO_TEXT_NO_FOCUS, SPM_MAPPED);
  /* find index of value in table */
  combo->value = value;
  combo->data  = data;
  combo->atom  = atom;
  combo->table = table;
  combo->index = 0;
  /* find valid entry in table */
  if (combo->value != NULL) {
    for (combo->index = 0; table[combo->index].text != NULL; combo->index ++) {
      if (table[combo->index].value == *value) {
	break;
      }
    }
  }
  if (combo->data != NULL) {
    for (combo->index = 0; table[combo->index].text != NULL; combo->index ++) {
      if (table[combo->index].data == *data) {
	break;
      }
    }
  }
  if (combo->atom != NULL) {
    for (combo->index = 0; table[combo->index].text != NULL; combo->index ++) {
      if (table[combo->index].atom == *atom) {
	break;
      }
    }
  }
  if (NULL == table[combo->index].text) {
    combo->index = 0;
  }
  /* set values according entry */
  if (combo->value != NULL) {
    *combo->value = table[combo->index].value;
  }
  if (combo->data != NULL) {
    *combo->data = table[combo->index].data;
  }
  if (combo->atom != NULL) {
    *combo->atom = table[combo->index].atom;
  }
  /* create sprite */
  combo->v_sprite 
    = CreateTextSprite (table[combo->index].text, (x + w - w_val + 1) * BASE_X, (y + 2) * BASE_Y,
			(w_val - 2) * BASE_X, (CELL_H - 4) * BASE_Y,
			FF_COMBO_VALUE_NO_SELECT, SPM_MAPPED);
  /* graphics */
  MenuAddLargeFrame ((x - CELL_W/2) / CELL_W, (x + w + CELL_W/2 - 1) / CELL_W, y / CELL_H);
  return &combo->item;
}

/*
 * delete a combo 
 */
void
MenuDeleteCombo (XBMenuItem *item)
{
  XBMenuComboItem *combo = (XBMenuComboItem *) item;

  assert (combo != NULL);
  assert (combo->t_sprite != NULL);
  assert (combo->v_sprite != NULL);
  DeleteSprite (combo->t_sprite);
  DeleteSprite (combo->v_sprite);
} /* DeleteComboItem */


/*
 * end of file mi_bombo.c
 */
