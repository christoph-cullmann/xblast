/*
 * file mi_tag.c - Display a volatile string (const char *)
 *
 * $Id: mi_tag.c,v 1.7 2006/02/09 21:21:24 fzago Exp $
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

/*
 * local types
 */
typedef struct
{
	XBMenuItem item;
	const char **pText;
	const char *cText;
	int *pNr;
	int cNr;
	Sprite *sprite;
} XBMenuTagItem;

/* temporary buffer */
static char buf[10];

/*
 * polling a tag item
 */
static void
MenuTagPoll (XBMenuItem * ptr)
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
}								/* MenuTagPoll */

/*
 * polling an int tag item
 */
static void
MenuIntTagPoll (XBMenuItem * ptr)
{
	XBMenuTagItem *tag = (XBMenuTagItem *) ptr;
	/* sanity check */
	assert (NULL != tag);
	assert (NULL != tag->pNr);
	assert (NULL != tag->sprite);
	/* check for changed string */
	if (*tag->pNr != tag->cNr) {
		tag->cNr = *tag->pNr;
		sprintf (buf, "%i", tag->cNr);
		SetSpriteText (tag->sprite, buf);
	}
}								/* MenuIntTagPoll */

/*
 * create standard tag, framed, small
 */
XBMenuItem *
MenuCreateTag (int x, int y, int w, const char **pText)
{
	/* create item */
	XBMenuTagItem *tag = calloc (1, sizeof (XBMenuTagItem));
	assert (tag != NULL);
	MenuSetItem (&tag->item, MIT_Tag, x, y, w, CELL_H / 2, NULL, NULL, NULL, MenuTagPoll);
	/* set item specific values */
	assert (pText != NULL);
	tag->pText = pText;
	tag->cText = *pText;
	/* sprite showing text */
	tag->sprite =
		CreateTextSprite (tag->cText, (x + 1) * BASE_X, y * BASE_Y, (w - 2) * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_Small | FF_White | FF_Center, SPM_MAPPED);
	/* graphics */
	MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	return &tag->item;
}								/* MenuCreateTag */

/*
 * create integer tag, boxed, large
 */
XBMenuItem *
MenuCreateIntTag (int x, int y, int w, int *pNr)
{
	/* create item */
	XBMenuTagItem *tag = calloc (1, sizeof (XBMenuTagItem));
	assert (tag != NULL);
	MenuSetItem (&tag->item, MIT_Tag, x, y, w, CELL_H, NULL, NULL, NULL, MenuIntTagPoll);
	/* set item specific values */
	assert (pNr != NULL);
	tag->pNr = pNr;
	tag->cNr = *pNr;
	/* sprite showing text */
	sprintf (buf, "%i", *pNr);
	tag->sprite =
		CreateTextSprite (buf, (x + 1) * BASE_X, (y + 1) * BASE_Y, (w - 2) * BASE_X,
						  (CELL_H - 2) * BASE_Y, FF_Large | FF_White | FF_Center | FF_Boxed,
						  SPM_MAPPED);
	return &tag->item;
}								/* MenuCreateIntTag */

/*
 * delete a "tag"
 */
void
MenuDeleteTag (XBMenuItem * item)
{
	XBMenuTagItem *tag = (XBMenuTagItem *) item;

	assert (NULL != tag);
	assert (NULL != tag->sprite);
	DeleteSprite (tag->sprite);
}								/* MenuDeleteTag */

/*
 * end of file mi_tag.c
 */
