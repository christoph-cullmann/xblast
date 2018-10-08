/*
 * file mi_label.c - menu title
 *
 * $Id: mi_label.c,v 1.8 2006/02/09 21:21:24 fzago Exp $
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
 * local macros
 */
#define FF_LABEL (FF_Large | FF_White | FF_Center | FF_Boxed )
#define FF_LABEL1 (FF_Small | FF_White | FF_Center  )
#define FF_LABEL2 (FF_Small | FF_White | FF_Center | FF_Boxed  )

/*
 *  local types
 */
typedef struct
{
	XBMenuItem item;
	const char *text;
	Sprite *sprite;
} XBMenuLabelItem;

/*
 * standard label, boxed, large
 */
XBMenuItem *
MenuCreateLabel (int x, int y, int w, const char *text)
{
	/* create item */
	XBMenuLabelItem *label = calloc (1, sizeof (XBMenuLabelItem));
	assert (label != NULL);
	MenuSetItem (&label->item, MIT_Label, x, y, w, CELL_H, NULL, NULL, NULL, NULL);
	/* set item specific data */
	label->text = text;
	label->sprite =
		CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, (w - 2) * BASE_X,
						  (CELL_H - 2) * BASE_Y, FF_LABEL, SPM_MAPPED);
	return &label->item;
}								/* MenuCreateLabel */

/*
 * variant label, framed. small
 */
XBMenuItem *
MenuCreateLabel1 (int x, int y, int w, const char *text)
{
	/* create item */
	XBMenuLabelItem *label = calloc (1, sizeof (XBMenuLabelItem));
	assert (label != NULL);
	MenuSetItem (&label->item, MIT_Label, x, y, w, CELL_H, NULL, NULL, NULL, NULL);
	/* set item specific data */
	label->text = text;
	label->sprite =
		CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y + 6, (w - 2) * BASE_X,
						  (CELL_H - 2) * BASE_Y, FF_LABEL1, SPM_MAPPED);
	/* add frame */
	MenuAddLargeFrame (x / CELL_W, (x + w - 1) / CELL_W, (y + 1) / CELL_H);
	return &label->item;
}								/* MenuCreateLabel1 */

/*
 * variant label, boxed, small
 */
XBMenuItem *
MenuCreateLabel2 (int x, int y, int w, const char *text)
{
	/* create item */
	XBMenuLabelItem *label = calloc (1, sizeof (XBMenuLabelItem));
	assert (label != NULL);
	MenuSetItem (&label->item, MIT_Label, x, y, w, CELL_H, NULL, NULL, NULL, NULL);
	/* set item specific data */
	label->text = text;
	label->sprite =
		CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y + 6, (w - 2) * BASE_X,
						  (CELL_H - 2) * BASE_Y, FF_LABEL2, SPM_MAPPED);
	return &label->item;
}								/* MenuCreateLabel2 */

/*
 * delete a label
 */
void
MenuDeleteLabel (XBMenuItem * item)
{
	XBMenuLabelItem *label = (XBMenuLabelItem *) item;
	assert (label != NULL);
	assert (label->sprite != NULL);
	DeleteSprite (label->sprite);
}								/* MenuDeleteLabel */

/*
 * end of file mi_label.c
 */
