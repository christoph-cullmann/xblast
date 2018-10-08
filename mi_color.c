/*
 * file mi_color.c - color editor for menus
 *
 * $Id: mi_color.c,v 1.5 2006/02/09 21:21:24 fzago Exp $
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
#define FF_COLOR_TEXT_FOCUS    (FF_Medium | FF_Black | FF_Left | FF_Outlined)
#define FF_COLOR_TEXT_NO_FOCUS (FF_Medium | FF_White | FF_Left)

/*
 * local types
 */
/* color editor */
typedef struct
{
	XBMenuItem item;
	const char *text;
	Sprite *tSprite;
	XBColor *pointer;
	XBColor current;
	XBRgbValue *pRgb;
	int index;
	Sprite *cSprite;
	Sprite *sSprite;
} XBMenuColorItem;

/*
 * local variables
 */
static int colorCount = 0;
XBMenuColorItem *itemTable[MAX_COLOR_SPRITES] = {
	NULL, NULL, NULL, NULL, NULL, NULL,
};

/*
 * a color item has received the focus
 */
static void
MenuColorFocus (XBMenuItem * ptr, XBBool flag)
{
	XBMenuColorItem *color = (XBMenuColorItem *) ptr;

	assert (color != NULL);
	assert (color->tSprite != NULL);
	SetSpriteAnime (color->tSprite, flag ? FF_COLOR_TEXT_FOCUS : FF_COLOR_TEXT_NO_FOCUS);
}								/* MenuColorFocus */

/*
 * Menu Color Item
 */
static void
MenuColorSelect (XBMenuItem * ptr)
{
	int i;
	XBMenuColorItem *color = (XBMenuColorItem *) ptr;

	assert (color != NULL);
	/* delete other color selections */
	for (i = 0; i < colorCount; i++) {
		XBMenuColorItem *item = itemTable[i];
		assert (item != NULL);
		item->item.flags &= ~MIF_SELECTED;
		SetSpriteAnime (item->sSprite, ISA_LedOff);
	}
	/* mark as selected */
	color->item.flags |= MIF_SELECTED;
	SetSpriteAnime (color->sSprite, ISA_LedOn);
	/* set rgb value */
	assert (color->pRgb != NULL);
	color->pRgb->red = GET_RED (color->current);
	color->pRgb->green = GET_GREEN (color->current);
	color->pRgb->blue = GET_BLUE (color->current);
}								/* MenuColorSelect */

/*
 * mouse click
 */
static void
MenuColorMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (XBE_MOUSE_1 == code) {
		MenuColorSelect (ptr);
	}
}								/* MenuColorMouse */

/*
 * Menu Color Item
 */
static void
MenuColorPoll (XBMenuItem * ptr)
{
	XBMenuColorItem *color = (XBMenuColorItem *) ptr;

	assert (NULL != color);
	assert (NULL != color->pRgb);
	assert (NULL != color->cSprite);
	assert (NULL != color->pointer);
	if (color->item.flags & MIF_SELECTED) {
		/* check for color changes */
		XBColor tmp = SET_COLOR (color->pRgb->red, color->pRgb->green, color->pRgb->blue);
		if (tmp != color->current) {
			/* store new value */
			color->current = tmp;
			*color->pointer = tmp;
			SetSpriteColor (color->cSprite, color->current);
		}
	}
}								/* MenuColorPoll */

/*
 *
 */
XBMenuItem *
MenuCreateColor (int x, int y, int w, const char *text, XBColor * pColor, XBRgbValue * pRgb)
{
	XBMenuColorItem *color;

	assert (pColor != NULL);
	assert (colorCount < MAX_COLOR_SPRITES);
	/* create item */
	color = calloc (1, sizeof (*color));
	assert (color != NULL);
	MenuSetItem (&color->item, MIT_Color, x, y, w, CELL_H, MenuColorFocus, MenuColorSelect,
				 MenuColorMouse, MenuColorPoll);
	/* set label */
	color->text = text;
	color->tSprite =
		CreateTextSprite (text, (x + 3) * BASE_X, (y + 1) * BASE_Y, (w - CELL_W - 4) * BASE_X,
						  (CELL_H - 2) * BASE_Y, FF_COLOR_TEXT_NO_FOCUS, SPM_MAPPED);
	/* get color */
	color->pointer = pColor;
	color->current = *pColor;
	color->pRgb = pRgb;
	color->index = colorCount++;
	color->cSprite =
		CreateIconSprite ((x + w - CELL_W) * BASE_X, y * BASE_Y, ISA_Color1 + color->index,
						  SPM_MAPPED);
	SetSpriteColor (color->cSprite, color->current);
	/* marker for selection */
	color->sSprite = CreateIconSprite ((x - 4) * BASE_X, y * BASE_Y, ISA_LedOff, SPM_MAPPED);
	/* store in table */
	itemTable[color->index] = color;
	/* graphics */
	MenuAddLargeFrame ((x - CELL_W / 2) / CELL_W, (x + w + CELL_W / 2 - 1) / CELL_W, y / CELL_H);
	/* that's all */
	return &color->item;
}								/* CreateMenuColor */

/*
 * delete color editor
 */
void
MenuDeleteColor (XBMenuItem * item)
{
	XBMenuColorItem *color = (XBMenuColorItem *) item;

	assert (color != NULL);
	assert (color->tSprite != NULL);
	assert (color->cSprite != NULL);
	assert (color->sSprite != NULL);
	DeleteSprite (color->tSprite);
	DeleteSprite (color->cSprite);
	DeleteSprite (color->sSprite);
	itemTable[color->index] = NULL;
	colorCount--;
}								/* DeleteColorItem */

/*
 * end of file mi_color.c
 */
