/*
 * file mi_tag.c - Display a volatile string (const char *)
 *
 * $Id: mi_tag.c,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#include "mi_tag.h"

#include "mi_map.h"

#include "gui.h"
#include "sprite.h"

/*
 * local types
 */
typedef struct {
  XBMenuItem   item;
  const char **pText;
  const char  *cText;
  Sprite      *sprite;
} XBMenuTagItem;

/*
 * polling a tag item
 */
static void
MenuTagPoll (XBMenuItem *ptr)
{
  XBMenuTagItem *tag = (XBMenuTagItem *) ptr;
  /* sanity check */
  assert (NULL != tag);
  assert (NULL != tag->pText);
  assert (NULL != tag->sprite);
  /* check for changed string */
  if (*tag->pText != tag->cText) {
    tag->cText = *tag->pText;
    SetSpriteText (tag->sprite, tag->cText);
  }
} /* MenuTagPoll */

/*
 * create "tag"
 */
XBMenuItem *
MenuCreateTag (int x, int y, int w, const char **pText)
{
  /* create item */
  XBMenuTagItem *tag = calloc (1, sizeof (XBMenuTagItem) );
  assert (tag != NULL);
  MenuSetItem (&tag->item, MIT_Tag, x, y, w, CELL_H/2, NULL, NULL, NULL, MenuTagPoll);
  /* set item specific values */
  assert (pText != NULL);
  tag->pText  = pText;
  tag->cText  = *pText;
  /* sprite showing text*/
  tag->sprite = CreateTextSprite (tag->cText, (x + 1) * BASE_X, y * BASE_Y, (w - 2) * BASE_X, (CELL_H/2) * BASE_Y,
				  FF_Small | FF_White | FF_Center, SPM_MAPPED);
  /* graphics */
  MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
  return &tag->item;

} /* MenuCreateTag */

/*
 * delete a "tag"
 */
void
MenuDeleteTag (XBMenuItem *item)
{
  XBMenuTagItem *tag = (XBMenuTagItem *) item;

  assert (NULL != tag);
  assert (NULL != tag->sprite);
  DeleteSprite (tag->sprite);
} /* MenuDeleteTag */

/*
 * end of file mi_tag.c
 */
