/*
 * file menu_level.c - level selection menu
 *
 * $Id: menu_level.c,v 1.9 2006/03/28 11:41:19 fzago Exp $
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
	XBAtom atom;
	const char *name;
	XBBool newSelect;
	XBBool oldSelect;
} XBLevelSelection;

/*
 * local variables
 */
static int numLevels = 0;
static XBLevelSelection *selectList = NULL;
static int levelFirst = 0;
static int levelLast = 0;

/*
 * compare to level selection elments by name
 */
static int
CompareLevelSelections (const void *a, const void *b)
{
	return strcmp (((const XBLevelSelection *) a)->name, ((const XBLevelSelection *) b)->name);
}								/* CompareLevelSelections */

/*
 * create list with all levels configs
 */
static XBLevelSelection *
CreateLevelSelection (int *pNumLevels)
{
	XBLevelSelection *list;
	int i;
	XBAtom atom;
	assert (pNumLevels != NULL);
	/* how many levels do we have */
	*pNumLevels = GetNumLevels ();
	/* alloc data field */
	list = calloc (*pNumLevels, sizeof (XBLevelSelection));
	assert (list != NULL);
	/* store levels */
	for (i = 0; i < *pNumLevels; i++) {
		/* get level data */
		atom = GetLevelAtom (i);
		assert (ATOM_INVALID != atom);
		/* set selection data */
		list[i].atom = atom;
		list[i].name = GetLevelNameByAtom (atom);
		list[i].newSelect = list[i].oldSelect = GetLevelSelected (atom);
	}
	/* sort list */
	qsort (list, *pNumLevels, sizeof (XBLevelSelection), CompareLevelSelections);
	/* that's all */
	return list;
}								/* CreateLevelList */

/*
 * select all levels
 */
static XBBool
ButtonLevelSelectAll (void *par)
{
	int i;
	for (i = 0; i < numLevels; i++) {
		selectList[i].newSelect = XBTrue;
	}
	return XBFalse;
}								/* ButtonLevelSelectAll */

/*
 * deselect all levels
 */
static XBBool
ButtonLevelSelectNone (void *par)
{
	int i;
	for (i = 0; i < numLevels; i++) {
		selectList[i].newSelect = XBFalse;
	}
	return XBFalse;
}								/* ButtonLevelSelectNone */

/*
 * Forward in selection
 */
static XBBool
ButtonLevelForward (void *par)
{
	if (levelLast < numLevels) {
		levelFirst = levelLast;
	}
	return CreateLevelMenu (par);
}								/* ButtonLevelForward */

/*
 * Continue with game setup
 */
static XBBool
ButtonContinue (void *par)
{
	int i;
	XBMenuLevelPar *mlp = par;
	assert (mlp != NULL);
	/* store level selection */
	for (i = 0; i < numLevels; i++) {
		if (selectList[i].newSelect != selectList[i].oldSelect) {
			StoreLevelSelected (selectList[i].atom, selectList[i].newSelect);
			selectList[i].oldSelect = selectList[i].newSelect;	/* KOEN 26-06-2002 */
		}
	}
	/* call start game funtion */
	return (*mlp->funcDefault) (mlp->parDefault);
}								/* ButtonContinue */

/*
 * Forward in selection
 */
static XBBool
ButtonLevelBackward (void *par)
{
	levelFirst -= LEVEL_ROWS * LEVEL_COLS;
	if (levelFirst < 0) {
		levelFirst = 0;
	}
	return CreateLevelMenu (par);
}								/* ButtonLevelForward */

/*
 * create level selection menu
 */
XBBool
CreateLevelMenu (void *par)
{
	int i, c, r;
	XBMenuLevelPar *mlp = par;
	/* get parameter */
	assert (mlp != NULL);
	/* init level selection */
	if (NULL == selectList) {
		selectList = CreateLevelSelection (&numLevels);
		assert (selectList != NULL);
		levelFirst = 0;
	}
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Level Selection"));
	/* levels */
	c = 0;
	r = 0;
	for (i = levelFirst; i < numLevels; i++) {
		MenuAddToggle ((c * 6 + 3) * CELL_W / 2, MENU_TOP + r * CELL_H / 2, 13 * CELL_W / 4,
					   selectList[i].name, &selectList[i].newSelect);
		r++;
		if (r >= LEVEL_ROWS) {
			r = 0;
			c++;
			if (c >= LEVEL_COLS) {
				break;
			}
		}
	}
	levelLast = i + 1;
	/* backwards and forwards */
	MenuSetActive (MenuAddVButton
				   (0, 3 * CELL_H, LEVEL_ROWS * CELL_H / 2, N_("Backward"), ButtonLevelBackward, mlp),
				   (levelFirst > 0));
	MenuSetActive (MenuAddVButton
				   (14 * CELL_W, 3 * CELL_H, LEVEL_ROWS * CELL_H / 2, N_("Forward"), ButtonLevelForward,
					mlp), (levelLast < numLevels));
	/* selection buttons */
	MenuAddHButton (9 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, N_("All"), ButtonLevelSelectAll, mlp);
	MenuAddHButton (15 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, N_("None"), ButtonLevelSelectNone, mlp);
	/* cancel & ok */
	MenuSetAbort (MenuAddHButton
				  (3 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, mlp->textAbort, mlp->funcAbort,
				   mlp->parAbort));
	MenuSetDefault (MenuAddHButton
					(21 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, mlp->textDefault, ButtonContinue,
					 mlp));
	/* --- */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateLevelMenu */

/*
 * finish list with all levels configs
 */
void
FinishLevelSelection (void)
{
	if (NULL != selectList) {
		free (selectList);
		selectList = NULL;
	}
}								/* FinishLevelSelection */

/*
 * end of file menu_level.h
 */
