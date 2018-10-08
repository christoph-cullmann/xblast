/*
 * file mi_button.c -  buttons for menu
 *
 * $Id: mi_button.c,v 1.5 2006/02/09 21:21:24 fzago Exp $
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
/* text sprite flags */
#define FF_H_BUTTON_FOCUS    (FF_Large  | FF_Black | FF_Center   | FF_Outlined )
#define FF_H_BUTTON_NO_FOCUS (FF_Large  | FF_White | FF_Center )
#define FF_V_BUTTON_FOCUS    (FF_Medium | FF_Black | FF_Vertical | FF_Outlined)
#define FF_V_BUTTON_NO_FOCUS (FF_Medium | FF_White | FF_Vertical)

/*
 * local types
 */
typedef struct
{
	XBMenuItem item;
	const char *text;
	Sprite *sprite;
	Sprite *iconSprite;
	XBBool horizontal;
	MIC_button func;
	void *funcData;
} XBMenuButtonItem;

/*
 * local variables
 */
static MIC_button nextExec = NULL;
static void *nextExecData = NULL;

/*
 * determine current text flags
 */
static void
SetTextFlags (XBMenuButtonItem * button)
{
	assert (button != NULL);
	assert (button->sprite != NULL);
	if (button->horizontal) {
		if (button->item.flags & MIF_FOCUS) {
			SetSpriteAnime (button->sprite, FF_Large | FF_Black | FF_Center | FF_Outlined);
		}
		else if (button->item.flags & MIF_DEACTIVATED) {
			SetSpriteAnime (button->sprite, FF_Large | FF_Black | FF_Center);
		}
		else {
			SetSpriteAnime (button->sprite, FF_Large | FF_White | FF_Center);
		}
	}
	else {
		if (button->item.flags & MIF_FOCUS) {
			SetSpriteAnime (button->sprite, FF_Medium | FF_Black | FF_Vertical | FF_Outlined);
		}
		else if (button->item.flags & MIF_DEACTIVATED) {
			SetSpriteAnime (button->sprite, FF_Medium | FF_Black | FF_Vertical);
		}
		else {
			SetSpriteAnime (button->sprite, FF_Medium | FF_White | FF_Vertical);
		}
	}
}								/* ButtonTextFlags */

/*
 * button has changed activation
 */
void
MenuActivateButton (XBMenuItem * ptr, XBBool flag)
{
	SetTextFlags ((XBMenuButtonItem *) ptr);
}								/* MenuActivateButton */

/*
 * a horizontal button receives the focus 
 */
static void
MenuButtonFocus (XBMenuItem * ptr, XBBool flag)
{
	SetTextFlags ((XBMenuButtonItem *) ptr);
}								/* MenuHButtonFocus */

/*
 * a button is selected
 */
static void
MenuButtonSelect (XBMenuItem * ptr)
{
	XBMenuButtonItem *button = (XBMenuButtonItem *) ptr;

	assert (ptr != NULL);
	nextExec = button->func;
	nextExecData = button->funcData;
}								/* MenuButtonSelect */

/*
 * button was selected by mouse
 */
static void
MenuButtonMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (code == XBE_MOUSE_1) {
		MenuButtonSelect (ptr);
	}
}								/* MenuButtonMouse */

/*
 * 
 */
XBMenuItem *
MenuCreateHButton (int x, int y, int w, const char *text, MIC_button func, void *funcData)
{
	/* create item */
	XBMenuButtonItem *button = calloc (1, sizeof (XBMenuButtonItem));
	assert (button != NULL);
	MenuSetItem (&button->item, MIT_Button, x, y, w, CELL_H, MenuButtonFocus, MenuButtonSelect,
				 MenuButtonMouse, NULL);
	/* set item specific values */
	button->text = text;
	button->sprite =
		CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, (w - 2) * BASE_X,
						  (CELL_H - 2) * BASE_Y, FF_H_BUTTON_NO_FOCUS, SPM_MAPPED);
	button->func = func;
	button->funcData = funcData;
	button->horizontal = XBTrue;
	/* graphics */
	MenuAddLargeFrame ((x - CELL_W / 2) / CELL_W, (x + w + CELL_W / 2 - 1) / CELL_W, y / CELL_H);
	/* --- */
	return &button->item;
}								/* CreateMenuButton */

/*
 * 
 */
XBMenuItem *
MenuCreateVButton (int x, int y, int h, const char *text, MIC_button func, void *funcData)
{
	/* create item */
	XBMenuButtonItem *button = calloc (1, sizeof (XBMenuButtonItem));
	assert (button != NULL);
	MenuSetItem (&button->item, MIT_Button, x, y, CELL_W, h, MenuButtonFocus, MenuButtonSelect,
				 MenuButtonMouse, NULL);
	button->text = text;
	button->sprite =
		CreateTextSprite (text, (x + 1) * BASE_X, (y + 1) * BASE_Y, (CELL_W - 2) * BASE_X,
						  (h - 2) * BASE_Y, FF_V_BUTTON_NO_FOCUS, SPM_MAPPED);
	button->func = func;
	button->funcData = funcData;
	button->horizontal = XBFalse;
	/* graphics */
	MenuAddVerticalFrame (x / CELL_W, y / CELL_H, (y + h - 1) / CELL_W);
	/* --- */
	return &button->item;
}								/* CreateMenuButton */

/*
 * delete a button 
 */
void
MenuDeleteButton (XBMenuItem * item)
{
	XBMenuButtonItem *button = (XBMenuButtonItem *) item;

	assert (button->sprite != NULL);
	DeleteSprite (button->sprite);
	if (NULL != button->iconSprite) {
		DeleteSprite (button->iconSprite);
	}
}								/* DeleteButtonItem */

/*
 * button is default button
 */
void
MenuSetButtonIcon (XBMenuItem * item, IconSpriteAnimation anime)
{
	XBMenuButtonItem *button = (XBMenuButtonItem *) item;

	assert (button != NULL);
	assert (button->item.type == MIT_Button);
	/* create sprite if needed */
	assert (button->iconSprite == NULL);
	button->iconSprite =
		CreateIconSprite ((button->item.x + 1) * BASE_X, button->item.y * BASE_Y, anime,
						  SPM_MAPPED);
	assert (NULL != button->iconSprite);
}								/* MenuSetButtonIcon */

/*
 * exec function of button
 */
XBBool
MenuExecButton (void)
{
	XBBool result = XBFalse;

	if (NULL != nextExec) {
		result = (*nextExec) (nextExecData);
		nextExec = NULL;
		nextExecData = NULL;
	}
	return result;
}

/*
 * set next exec to external function 
 */
void
MenuButtonSetNextExec (MIC_button func, void *data)
{
	nextExec = func;
	nextExecData = data;
}								/* MenuButtonSetNextExec */

/*
 * end of file mi_button.c
 */
