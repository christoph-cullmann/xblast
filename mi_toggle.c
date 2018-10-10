/*
 * file mi_toggle.c - checkbuttons for menus
 * 
 * $Id: mi_toggle.c,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#include "mi_toggle.h"

#include "mi_map.h"
#include "gui.h"

/*
 * local macors
 */
#define FF_TOGGLE_FOCUS    (FF_Small | FF_Black | FF_Left | FF_Outlined)
#define FF_TOGGLE_NO_FOCUS (FF_Small | FF_White | FF_Left)

/*
 * local types
 */
typedef struct {
  XBMenuItem  item;
  const char *text;
  Sprite     *textSprite;
  Sprite     *ledSprite;
  XBBool     *pState;
  XBBool      state;
} XBMenuToggleItem;

/*
 * a toggle item receives the focus
 */
static void
MenuToggleFocus (XBMenuItem *ptr, XBBool flag)
{
  XBMenuToggleItem *toggle = (XBMenuToggleItem *) ptr;

  assert (toggle != NULL);
  assert (toggle->textSprite != NULL);
  SetSpriteAnime (toggle->textSprite, flag ? FF_TOGGLE_FOCUS : FF_TOGGLE_NO_FOCUS);
} /* MenuToggleFocus */

/*
 * a toggle is selected
 */
static void
MenuToggleSelect (XBMenuItem *ptr)
{
  XBMenuToggleItem *toggle = (XBMenuToggleItem *) ptr;

  assert (toggle != NULL);
  if (NULL != toggle->pState) {
    *toggle->pState = ! *toggle->pState;
  }
} /* MenuToggleSelect */

/*
 * mouse click
 */ 
static void
MenuToggleMouse (XBMenuItem *ptr, XBEventCode code)
{
  if (code == XBE_MOUSE_1) {
    MenuToggleSelect (ptr);
  }
} /* MenuToggleMouse */


/*
 * polling a toggle item
 */
static void
MenuTogglePoll (XBMenuItem *ptr)
{
  XBMenuToggleItem *toggle = (XBMenuToggleItem *) ptr;

  assert (toggle != NULL);
  assert (toggle->ledSprite != NULL);
  if (*toggle->pState != toggle->state) {
    toggle->state = *toggle->pState;
    SetSpriteAnime (toggle->ledSprite, toggle->state ? ISA_LedOn : ISA_LedOff);
  }
} /* MenuTogglePoll */

/*
 * 
 */
XBMenuItem *
MenuCreateToggle (int x, int y, int w, const char *text, XBBool *pState)
{
  /* create item */
  XBMenuToggleItem *toggle = calloc (1, sizeof (*toggle));
  assert (NULL != toggle);
  MenuSetItem (&toggle->item, MIT_Toggle, x, y, w, CELL_H/2, MenuToggleFocus, MenuToggleSelect, MenuToggleMouse, MenuTogglePoll);
  /* set toggle specific data */
  toggle->text   = text;
  toggle->pState = pState;
  toggle->state  = (NULL != pState) ? *pState : XBFalse;
  /* sprite with text label */
  toggle->textSprite = CreateTextSprite (text, (x + 3) * BASE_X, y * BASE_Y, (w - 4) * BASE_X, (CELL_H/2) * BASE_Y,
					 FF_TOGGLE_NO_FOCUS, SPM_MAPPED);
  /* sprite with marker */
  toggle->ledSprite = CreateIconSprite ((x - 2) * BASE_X, (y - CELL_H/4) * BASE_Y, toggle->state ? ISA_LedOn : ISA_LedOff, 
					SPM_MAPPED);
  /* graphics */
  MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
  return &toggle->item;
} /* CreateMenuToggle */

/*
 * delete a toggle 
 */
void
MenuDeleteToggle (XBMenuItem *item)
{
  XBMenuToggleItem *toggle = (XBMenuToggleItem *) item;

  assert (toggle->textSprite != NULL);
  assert (toggle->ledSprite != NULL);
  DeleteSprite (toggle->textSprite);
  DeleteSprite (toggle->ledSprite);
} /* DeleteButtonItem */

/*
 * end of file mi_toggle.c
 */
