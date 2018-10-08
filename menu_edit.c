/*
 * file menu_edit.c - user interface for editing levels
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * This file (C) Lars Luthman <larsl@users.sourceforge.net>
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
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "xblast.h"

#define TEXT_ATTR_NORMAL (FF_Medium|FF_White|FF_Boxed)
#define FF_Scroll      0x40

/*
 * external variables
 */
extern DBToInt *shrinkTable;
static char fileName[STRING_LENGTH];
static char title[STRING_LENGTH];
static char author[STRING_LENGTH];
static char hint[STRING_LENGTH];
static int specialKeyDefaultComboValue;
static int specialExtraDefaultComboValue;
static int specialInitExtraDefaultComboValue;
static int specialRevExtraDefaultComboValue;
static XBBool useInitKick = XBFalse;
static XBBool useRevKick = XBFalse;
static int bombDefaultComboValue;
static int bombSpecialComboValue;
static int bombHiddenComboValue;
static int bombsNumBombs = 0;
static int bombsNumSpecials = 0;
static int bombsRange = 0;
static long block = 0;
static int loadedCreateSpecialExtrasMenu = 0;
static XBBool bombsNastyWalls;
static int bombsNastyGentle;
static int bombsNastyRange;
static XBComboEntryList *xbcel, *xbcel1;
static int shrinkComboValue;
static XBBool shrinkUseDraw;
static int scrambleDrawTime = 66;
static XBBool shrinkUseDelete;
static int scrambleDeleteTime = 33;
static int blockSelected;
static DBRoot *level;
static BMMapTile mazeSave[MAZE_H][MAZE_W];
static XBBool mazeSaveEvil[MAZE_H][MAZE_W];
static char mazeSaveChar[MAZE_H + 1][MAZE_W + 1];
static XBBool mazeScrambleDraw[MAZE_H][MAZE_W];
static XBBool mazeScrambleDelete[MAZE_H][MAZE_W];
static int numShDrBlocks = 0;
static int numShDeBlocks = 0;
static int init = 1;
static int tempBlock;
static int gameTime;
static XBDir *dirBlockName;
static XBBool useFg[BTNUM];
static XBRgbValue rgbs[BTNUM][3];
static char *blockName[BTNUM];
static XBBool recreate;
static char *graphics[] = {
	"sphere_half      #000000 #FF7F50 #4682B4",
	"sphere_half_X    #000000 #FF7F50 #4682B4",
	"sphere_dark      #000000 #4682B4 #000000",
	"sphere_light     #000000 #B0C4DE #000000",
	"sphere_light     #000000 #4682B4 #000000",
	"sphere_light_O   #000000 #4682B4 #000000",
	"bomb",
	"range",
	"trap",
	"invincible",
	"score_floor      #4169E1 #4169E1 #4169E1"
};
static MENU_ID drawButton;
static MENU_ID deleteButton;
static XBDir *ppmList;
/*static const char *buttons[13]={"bo","ra","fs","so","bb","vb","se","il",
  "sb","sd","sl","",""};*/
static XBBool SetExtras (void *par);
static XBBool SaveGraphics (void *par);
static XBBool FreeBlockMenu (void *par);
static XBBool CreateGraphicsMenu (void *par);
static XBBool BuildColorsMenu (void *par);
static XBBool SaveLevel (void *par);
static XBBool ReturnFromMap (void *par);
static XBBool SetBombs (void *par);
static XBBool CreateLevel (void *par);
static XBBool CreateLevelMainMenu (void *par);
static XBBool CreateBombsMenu (void *par);
static XBBool CreateBombsGSMenu (void *par);
static XBBool CreateSpecialExtrasMenu (void *par);
static XBBool LoadExtras (void);

/*
 * Save a level file using a new name
 */
static XBBool
CreateSaveAsMenu (void *par)
{
	XBAtom *atom = par;
	assert (atom != NULL);
	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Save Level"));
	MenuAddString (DLG_LEFT, MENU_ROW (5), DLG_WIDTH, N_("Name:"), 4 * CELL_W, fileName, STRING_LENGTH);
	MenuSetAbort (MenuAddHButton
				  (5 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Abort"), CreateEditMenu, par));
	MenuSetDefault (MenuAddHButton
					(17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Save"), SaveLevel, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}

/*
 * Save a level file
 */
static XBBool
SaveLevel (void *par)
{
	DBSection *section;
	int i;
	section = DB_GetSection (level, atomInfo);
	recreate = XBTrue;
	// if(recreate){
	ReturnFromMap (NULL);
	section = DB_CreateSection (level, atomGraphics);
	SaveGraphics (NULL);
	SetBombs (NULL);
	if (loadedCreateSpecialExtrasMenu == 0)
		LoadExtras ();
	else
		loadedCreateSpecialExtrasMenu = 0;
	SetExtras (NULL);
	section = DB_CreateSection (level, atomPlayer);
	for (i = 0; i < MAX_PLAYER; ++i)
		DB_CreateEntryString (section, atomArrayPos0[i + 1], "7 6");
	//section = DB_CreateSection(level,atomFunc);
	//DB_CreateEntryString(section,atomExtra, "invincibility");
	section = DB_CreateSection (level, atomInfo);
	DB_CreateEntryString (section, atomName, title);
	DB_CreateEntryString (section, atomAuthor, author);
	DB_CreateEntryString (section, atomHint, hint);
	DB_CreateEntryString (section, atomGameMode, "R23456STDL");

	recreate = XBFalse;
	if (fileName[0] == '\0')
		return CreateSaveAsMenu (par);
	else {
		DB_CreateEntryString (section, atomName, (char *)fileName);
		if (!DB_Store (level))
			return XBFalse;
		return CreateEditMenu (par);
		// }
	}
}								/* ButtonSave */

/*
 * Set the level info in the level database. 
 */
static XBBool
SetInfo (void *par)
{
	DBSection *infoSection;
	char temp[STRING_LENGTH];
	int i, j;

	/* copy values to database */
	strcpy (fileName, title);
	i = 0;
	j = 0;
	while (fileName[i] != 0 && i < STRING_LENGTH) {
		fprintf (stderr, " %s : %s : %i \n", fileName, temp, i);
		if (isspace ((unsigned char)fileName[i])) {
		}
		else {
			temp[j] = fileName[i];
			j++;
		}
		i++;
	}
	temp[j] = 0;
	DB_Delete (level);
	strcpy (fileName, temp);
	fprintf (stderr, "T %s : %s : %i \n", fileName, temp, i);
	level = DB_Create (DT_Level, GUI_StringToAtom (fileName));
	recreate = XBTrue;
	CreateLevel (par);
	infoSection = DB_GetSection (level, atomInfo);
	if (infoSection == NULL)
		infoSection = DB_CreateSection (level, atomInfo);
	DB_CreateEntryString (infoSection, atomName, title);
	DB_CreateEntryString (infoSection, atomAuthor, author);
	DB_CreateEntryString (infoSection, atomHint, hint);
	DB_CreateEntryString (infoSection, atomGameMode, "R23456STDL");
	recreate = XBFalse;
	/* back to edit level menu */
	return CreateLevelMainMenu (par);

}								/* ButtonSetInfo */

/*
 * Edit the level info (title, author, hint).
 */
static XBBool
CreateInfoMenu (void *par)
{
	const DBSection *infoSection;
	const char *tmp;

	/* Get info from the db. */
	infoSection = DB_GetSection (level, atomInfo);
	DB_GetEntryString (infoSection, atomName, &tmp);
	strcpy (title, tmp);
	DB_GetEntryString (infoSection, atomAuthor, &tmp);
	strcpy (author, tmp);
	DB_GetEntryString (infoSection, atomHint, &tmp);
	strcpy (hint, tmp);

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Level Info"));
	MenuAddString (DLG_LEFT, MENU_ROW (1), DLG_WIDTH + 2 * CELL_W, N_("Title:"),
				   7 * CELL_W, title, STRING_LENGTH);
	MenuAddString (DLG_LEFT, MENU_ROW (2), DLG_WIDTH + 2 * CELL_W, N_("Author:"),
				   7 * CELL_W, author, STRING_LENGTH);
	MenuAddString (DLG_LEFT, MENU_ROW (3), DLG_WIDTH + 2 * CELL_W, N_("Hint:"),
				   7 * CELL_W, hint, STRING_LENGTH);
	MenuSetAbort (MenuAddHButton (5 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Abort"), CreateLevelMainMenu, par));
	MenuSetDefault (MenuAddHButton (17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), SetInfo, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}

/*
 * Set the level info in the level database. 
 */
static XBBool
SetBombs (void *par)
{
	DBSection *section;
	char bombType[STRING_LENGTH];

	/* copy values to database */
	section = DB_GetSection (level, atomBombs);

	if (section == NULL)
		section = DB_CreateSection (level, atomBombs);
	fprintf (stderr, " %p %i %i %i \n", section, bombsNumSpecials, bombsRange, bombsNumBombs);
	DB_CreateEntryInt (section, atomSpecialBombs, bombsNumSpecials);
	/* copy values to database */
	strcpy (bombType, GetBombName ((bombDefaultComboValue)));
	DB_CreateEntryString (section, atomDefault, bombType);

	section = DB_GetSection (level, atomPlayer);

	if (section == NULL)
		section = DB_CreateSection (level, atomPlayer);
	DB_CreateEntryInt (section, atomRange, bombsRange);
	DB_CreateEntryInt (section, atomBombs, bombsNumBombs);
	/* back to edit level menu */
	if (recreate) {
		return XBTrue;
	}
	else {
		return CreateBombsMenu (NULL);
	}

}								/* ButtonSetInfo */

/*
 * Edit the general bomb settings
 */
static XBBool
CreateBombsGSMenu (void *par)
{
	const DBSection *bombSection;
	const char *currentDefaultBomb;
	const char *currentSpecialBomb;
	const char *currentHiddenBomb;
	int i;
	bombSection = DB_GetSection (level, atomBombs);
	if (!DB_GetEntryString (bombSection, atomType, &currentDefaultBomb))
		currentDefaultBomb = GetBombName (BMTnormal);
	if (currentDefaultBomb == NULL)
		currentDefaultBomb = GetBombName (BMTnormal);

	bombSection = DB_GetSection (level, atomBombs);
	if (!DB_GetEntryString (bombSection, atomSpecial, &currentSpecialBomb))
		currentSpecialBomb = GetBombName (BMTnormal);
	if (currentSpecialBomb == NULL)
		currentSpecialBomb = GetBombName (BMTnormal);

	bombSection = DB_GetSection (level, atomBombs);
	if (!DB_GetEntryString (bombSection, atomHidden, &currentHiddenBomb))
		currentHiddenBomb = GetBombName (BMTnormal);
	if (currentHiddenBomb == NULL)
		currentHiddenBomb = GetBombName (BMTnormal);

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Bomb Settings"));

	xbcel = (XBComboEntryList *) calloc (NUM_BMT + 1, sizeof (XBComboEntryList));
	memset (xbcel, 0, (NUM_BMT + 1) * sizeof (XBComboEntryList));
	for (i = 0; i < NUM_BMT; ++i) {
		xbcel[i].text = GetBombName ((BMBombType) (i));
		xbcel[i].value = (BMBombType) (i);
		if (!strcmp (xbcel[i].text, currentDefaultBomb)) {
			bombDefaultComboValue = i;

		}
		if (!strcmp (xbcel[i].text, currentSpecialBomb)) {
			bombSpecialComboValue = i;

		}

		if (!strcmp (xbcel[i].text, currentHiddenBomb)) {
			bombHiddenComboValue = i;

		}
	}
	xbcel[i].text = NULL;
	MenuAddCombo (DLG_LEFT, MENU_ROW (0), DLG_WIDTH, N_("Default Bomb Type"), 3 * CELL_W,
				  &bombDefaultComboValue, NULL, NULL, (void *)xbcel);
	MenuAddInteger (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, N_("Initial Range"),
					4 * CELL_W, &bombsRange, 0, 255);
	MenuAddInteger (DLG_LEFT, MENU_ROW (2), DLG_WIDTH, N_("Initial Bombs"),
					4 * CELL_W, &bombsNumBombs, 0, 255);
	MenuAddInteger (DLG_LEFT, MENU_ROW (3), DLG_WIDTH, N_("Special Bombs"),
					4 * CELL_W, &bombsNumSpecials, 0, 255);

	MenuAddCombo (DLG_LEFT, MENU_ROW (4), DLG_WIDTH, N_("Special Bomb Type"), 3 * CELL_W,
				  &bombSpecialComboValue, NULL, NULL, (void *)xbcel);
	MenuAddCombo (DLG_LEFT, MENU_ROW (5), DLG_WIDTH, N_("Hidden Bomb Type"), 3 * CELL_W,
				  &bombHiddenComboValue, NULL, NULL, (void *)xbcel);
	MenuSetAbort (MenuAddHButton (5 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Abort"), CreateBombsMenu, par));
	MenuSetDefault (MenuAddHButton (17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), SetBombs, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}

/*
 * Set the level info in the level database. 
 */
static XBBool
SetExtras (void *par)
{
	DBSection *section;
	char type[STRING_LENGTH];

	/* copy values to database */
	section = DB_GetSection (level, atomFunc);

	if (section == NULL)
		section = DB_CreateSection (level, atomFunc);

	/* copy values to database */
	strcpy (type, GetKeyNameInt ((specialKeyDefaultComboValue)));
	if (strcmp (type, "No Special Key") == 0) {
	}
	else {
		DB_CreateEntryString (section, atomKey, type);
	}
	fprintf (stderr, " %s key name %i specialKeyDefaultComboValue \n", type,
			 specialKeyDefaultComboValue);
	strcpy (type, GetExtraNameInt ((specialExtraDefaultComboValue)));
	fprintf (stderr, " %s extra name %i specialExtraDefaultComboValue \n", type,
			 specialExtraDefaultComboValue);
	DB_CreateEntryString (section, atomExtra, type);

	section = DB_GetSection (level, atomPlayer);

	if (section == NULL)
		section = DB_CreateSection (level, atomPlayer);
	strcpy (type, GetExtraNameInt ((specialRevExtraDefaultComboValue)));
	fprintf (stderr, " %s extra name %i specialRevExtraDefaultComboValue \n", type,
			 specialRevExtraDefaultComboValue);
	if (useRevKick) {
		strcat (type, " kick");

	}
	DB_CreateEntryString (section, atomReviveExtra, type);
	strcpy (type, GetExtraNameInt ((specialInitExtraDefaultComboValue)));
	fprintf (stderr, " %s extra name %i specialInitExtraDefaultComboValue \n", type,
			 specialInitExtraDefaultComboValue);
	if (useInitKick) {
		strcat (type, " kick");

	}
	DB_CreateEntryString (section, atomInitExtra, type);
	/* back to edit level menu */
	if (recreate) {
		return XBTrue;
	}
	else {
		return CreateLevelMainMenu (NULL);
	}

}								/* ButtonSetInfo */

static XBBool
LoadExtras (void)
{
	int i, num;
	const DBSection *specialSection;
	const char *currentDefaultSpecialInitExtra;
	const char *currentDefaultSpecialRevExtra;
	const char *currentDefaultSpecialExtra;
	const char *currentDefaultSpecialKey;
	specialSection = DB_GetSection (level, atomPlayer);
	if (!DB_GetEntryString (specialSection, atomInitExtra, &currentDefaultSpecialInitExtra))
		currentDefaultSpecialInitExtra = GetExtraNameFunc (specialExtraFunc);
	fprintf (stderr, " cur nin extra %s \n", currentDefaultSpecialInitExtra);
	if (currentDefaultSpecialInitExtra == NULL)
		currentDefaultSpecialInitExtra = GetExtraNameFunc (SpecialExtraVoid);
	fprintf (stderr, " cur ini extra %s \n", currentDefaultSpecialInitExtra);

	if (!DB_GetEntryString (specialSection, atomReviveExtra, &currentDefaultSpecialRevExtra))
		currentDefaultSpecialRevExtra = GetExtraNameFunc (specialExtraFunc);
	fprintf (stderr, " cur rev extra %s \n", currentDefaultSpecialRevExtra);
	if (currentDefaultSpecialRevExtra == NULL)
		currentDefaultSpecialRevExtra = GetExtraNameFunc (SpecialExtraVoid);
	fprintf (stderr, " cur rev extra %s \n", currentDefaultSpecialRevExtra);

	specialSection = DB_GetSection (level, atomFunc);
	if (!DB_GetEntryString (specialSection, atomKey, &currentDefaultSpecialKey))
		currentDefaultSpecialKey = GetKeyNameFunc (specialKeyFunc);
	fprintf (stderr, " cur key %s \n", currentDefaultSpecialKey);
	if (currentDefaultSpecialKey == NULL)
		currentDefaultSpecialKey = GetKeyNameFunc (SpecialKeyVoid);
	fprintf (stderr, " cur key %s \n", currentDefaultSpecialKey);

	if (!DB_GetEntryString (specialSection, atomExtra, &currentDefaultSpecialExtra))
		currentDefaultSpecialExtra = GetExtraNameFunc (specialExtraFunc);
	fprintf (stderr, " cur extra %s \n", currentDefaultSpecialExtra);
	if (currentDefaultSpecialExtra == NULL)
		currentDefaultSpecialExtra = GetExtraNameFunc (SpecialExtraVoid);
	fprintf (stderr, " cur extra %s \n", currentDefaultSpecialExtra);

	num = GetNumberOfKeys ();
	xbcel = (XBComboEntryList *) calloc (num + 1, sizeof (XBComboEntryList));
	memset (xbcel, 0, (num + 1) * sizeof (XBComboEntryList));
	for (i = 0; i < num; ++i) {
		xbcel[i].text = GetKeyNameInt ((i));
		xbcel[i].value = (int)(i);
		if (!strcmp (xbcel[i].text, currentDefaultSpecialKey)) {
			specialKeyDefaultComboValue = i;

		}

	}
	num = GetNumberOfExtras ();
	xbcel1 = (XBComboEntryList *) calloc (num + 1, sizeof (XBComboEntryList));
	memset (xbcel1, 0, (num + 1) * sizeof (XBComboEntryList));
	for (i = 0; i < num; ++i) {
		xbcel1[i].text = GetExtraNameInt ((i));
		xbcel1[i].value = (int)(i);
		if (!strcmp (xbcel1[i].text, currentDefaultSpecialExtra)) {
			specialExtraDefaultComboValue = i;

		}

		if (!strcmp (xbcel1[i].text, currentDefaultSpecialRevExtra)) {
			specialRevExtraDefaultComboValue = i;

		}
		if (!strcmp (xbcel1[i].text, currentDefaultSpecialInitExtra)) {
			specialInitExtraDefaultComboValue = i;

		}

	}
	xbcel1[i].text = NULL;

	return XBTrue;
}

/*
 * Edit the general special extra settings
 */

static XBBool
CreateSpecialExtrasMenu (void *par)
{

	loadedCreateSpecialExtrasMenu = 1;
	LoadExtras ();

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Extras Settings"));
	MenuAddCombo (DLG_LEFT, MENU_ROW (0), DLG_WIDTH, N_("Extra Type"), 3 * CELL_W,
				  &specialExtraDefaultComboValue, NULL, NULL, (void *)xbcel1);

	MenuAddCombo (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, N_("Key Type"), 3 * CELL_W,
				  &specialKeyDefaultComboValue, NULL, NULL, (void *)xbcel);

	MenuAddCombo (DLG_LEFT, MENU_ROW (2), DLG_WIDTH, N_("Init Extra"), 3 * CELL_W,
				  &specialInitExtraDefaultComboValue, NULL, NULL, (void *)xbcel1);
	MenuAddCombo (DLG_LEFT, MENU_ROW (3), DLG_WIDTH, N_("Revive Extra"), 3 * CELL_W,
				  &specialRevExtraDefaultComboValue, NULL, NULL, (void *)xbcel1);
	MenuAddComboBool (DLG_LEFT, MENU_ROW (4), DLG_WIDTH, N_("Init Kick"), 2 * CELL_W, &useInitKick);
	MenuAddComboBool (DLG_LEFT, MENU_ROW (5), DLG_WIDTH, N_("Rev Kick"), 2 * CELL_W, &useRevKick);
	MenuSetAbort (MenuAddHButton (5 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Abort"), CreateLevelMainMenu, par));
	MenuSetDefault (MenuAddHButton (17 * CELL_W / 2, MENU_BOTTOM,
									4 * CELL_W, N_("Ok"), SetExtras, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}

/*
 * Set the level info in the level database. 
 */
static XBBool
SetNasty (void *par)
{
	DBSection *section;

	/* copy values to database */
	section = DB_GetSection (level, atomBombs);
	if (section == NULL)
		section = DB_CreateSection (level, atomBombs);
	if (bombsNastyWalls) {
		DB_CreateEntryInt (section, atomNastyGentle, bombsNumSpecials);
		DB_CreateEntryInt (section, atomNastyRange, bombsRange);
	}
	//  DB_CreateEntryString(level,atomBombs,bombsNumBombs );

	/* back to edit level menu */ if (recreate) {
		return XBTrue;
	}
	else {
		return CreateBombsMenu (NULL);
	}

}								/* ButtonSetInfo */

/*
 * Edit the nasty walls settings
 */
static XBBool
CreateNastyWallsMenu (void *par)
{
	const DBSection *bombSection;

	bombSection = DB_GetSection (level, atomBombs);

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Nasty Walls"));

	MenuAddComboBool (DLG_LEFT, MENU_ROW (1), 5 * CELL_W, N_("Nasty Walls:"),
					  2 * CELL_W, &bombsNastyWalls);
	MenuAddInteger (DLG_LEFT, MENU_ROW (2), DLG_WIDTH, N_("Intensity"),
					4 * CELL_W, &bombsNastyGentle, 1, 255);
	MenuAddInteger (DLG_LEFT, MENU_ROW (3), DLG_WIDTH, N_("Range"),
					4 * CELL_W, &bombsNastyRange, 1, 255);
	MenuSetAbort (MenuAddHButton (5 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Abort"), CreateBombsMenu, par));
	MenuSetDefault (MenuAddHButton (17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), SetNasty, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

/*
 * Edit the bomb settings
 */
static XBBool
CreateBombsMenu (void *par)
{
	const DBSection *bombSection;
	bombSection = DB_GetSection (level, atomBombs);

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Bombs"));
	MenuAddHButton (9 * CELL_W / 2, MENU_TOP, 4 * CELL_W, N_("General Settings"), CreateBombsGSMenu,
					par);
	MenuAddHButton (9 * CELL_W / 2, MENU_TOP + 1 * CELL_H, 4 * CELL_W, N_("Nasty Walls"),
					CreateNastyWallsMenu, par);

	MenuSetDefault (MenuAddHButton
					(5 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Back"), CreateLevelMainMenu, par));
	//  MenuSetDefault (MenuAddHButton (17 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, N_("Save"), SaveLevel,     par) );

	// ci->push(new ButtonItem("Hidden Bombs"));
	//  ci->push(new ButtonItem("Bomb Behaviour"));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}

/*
 * Set the shrink info in the level database. 
 */
static XBBool
SetShrink (void *par)
{
	DBSection *section;
	char shrinkType[STRING_LENGTH];
	BMPosition pValue;
	int x, y, numTemp = 0;

	/* copy values to database */
	section = DB_GetSection (level, atomShrink);
	strcpy (shrinkType, GetShrinkName ((XBShrinkType) (shrinkComboValue)));

	if (section == NULL)
		section = DB_CreateSection (level, atomShrink);
	if (strcmp (shrinkType, "No Shrink") == 0) {
		// DB_CreateEntryString(section,atomType, " " );
	}
	else {
		DB_CreateEntryString (section, atomType, shrinkType);
	}
	fprintf (stderr, " use draw %i\n", numShDrBlocks);
	if (shrinkUseDraw) {
		if (numShDrBlocks > -1) {
			section = DB_GetSection (level, atomScrambleDraw);

			if (section == NULL)
				section = DB_CreateSection (level, atomScrambleDraw);
			/* time for scramble */
			section = DB_GetSection (level, atomScrambleDraw);
			numTemp = numShDrBlocks;
			for (y = 0; y < MAZE_H; y++) {
				for (x = 0; x < MAZE_W; x++) {

					if (mazeScrambleDraw[y][x]) {
						pValue.x = x;
						pValue.y = y;
						numShDrBlocks--;
						if (numShDrBlocks > -1)
							DB_CreateEntryPos (section,
											   atomArrayPos000[numShDrBlocks], &pValue);

					}
				}
			}

			DB_CreateEntryInt (section, atomNumBlocks, numTemp);
			numShDrBlocks = numTemp;
			DB_CreateEntryFloat (section, atomTime,
								 1 - (float)scrambleDrawTime / 100);
		}
		else {
			if (numShDrBlocks < 0)
				DB_GetEntryInt (section, atomNumBlocks, &numShDrBlocks);
			numTemp = numShDrBlocks;
			for (y = 0; y < MAZE_H; y++) {
				for (x = 0; x < MAZE_W; x++) {

					if (mazeScrambleDraw[y][x]) {
						pValue.x = x;
						pValue.y = y;
						numShDrBlocks--;
						if (numShDrBlocks > -1)
							DB_CreateEntryPos (section,
											   atomArrayPos000[numShDrBlocks], &pValue);

					}
				}
			}

			DB_CreateEntryInt (section, atomNumBlocks, numTemp);
			DB_CreateEntryFloat (section, atomTime,
								 1 - (float)scrambleDrawTime / 100);

		}
	}
	if (shrinkUseDelete) {
		fprintf (stderr, "1 num %i %i \n", numShDeBlocks, numTemp);
		if (numShDeBlocks > -1) {
			section = DB_GetSection (level, atomScrambleDel);

			if (section == NULL)
				section = DB_CreateSection (level, atomScrambleDel);
			/* time for scramble */
			section = DB_GetSection (level, atomScrambleDel);
			numTemp = numShDeBlocks;
			for (y = 0; y < MAZE_H; y++) {
				for (x = 0; x < MAZE_W; x++) {

					if (mazeScrambleDelete[y][x]) {
						pValue.x = x;
						pValue.y = y;
						numShDeBlocks--;
						if (numShDeBlocks > -1)
							DB_CreateEntryPos (section,
											   atomArrayPos000[numShDeBlocks], &pValue);

					}
				}
			}

			fprintf (stderr, "2 num %i %i \n", numShDeBlocks, numTemp);
			DB_CreateEntryInt (section, atomNumBlocks, numTemp);
			numShDeBlocks = numTemp;
			DB_CreateEntryFloat (section, atomTime,
								 1 - (float)scrambleDeleteTime / 100);
		}
		else {
			if (numShDeBlocks < 0)
				DB_GetEntryInt (section, atomNumBlocks, &numShDeBlocks);
			numTemp = numShDeBlocks;
			for (y = 0; y < MAZE_H; y++) {
				for (x = 0; x < MAZE_W; x++) {

					if (mazeScrambleDelete[y][x]) {
						pValue.x = x;
						pValue.y = y;
						numShDeBlocks--;
						if (numShDeBlocks > -1)
							DB_CreateEntryPos (section,
											   atomArrayPos000[numShDeBlocks], &pValue);

					}
				}
			}

			DB_CreateEntryInt (section, atomNumBlocks, numTemp);
			DB_CreateEntryFloat (section, atomTime,
								 1 - (float)scrambleDeleteTime / 100);

		}
	}
	/* back to edit level menu */
	if (recreate) {
		return XBTrue;
	}
	else {
		return CreateLevelMainMenu (par);
	}

}								/* ButtonSetShrink */

/*
 * Edit the shrink
 */
static XBBool
CreateShrinkMenu (void *par)
{
	int i;
	const DBSection *shrinkSection;
	const char *currentShrink;
	shrinkSection = DB_GetSection (level, atomShrink);
	if (shrinkSection == NULL)
		shrinkSection = DB_CreateSection (level, atomShrink);
	if (!DB_GetEntryString (shrinkSection, atomType, &currentShrink))
		currentShrink = GetShrinkName (ST_Void);
	if (currentShrink == NULL)
		currentShrink = GetShrinkName (ST_Void);

	fflush (stdout);

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Shrink"));

	xbcel = (XBComboEntryList *) calloc (NUM_ST + 1, sizeof (XBComboEntryList));
	memset (xbcel, 0, (NUM_ST + 1) * sizeof (XBComboEntryList));
	for (i = 0; i < NUM_ST; ++i) {
		xbcel[i].text = GetShrinkName ((XBShrinkType) (i));
		xbcel[i].value = (XBShrinkType) (i);
		if (!strcmp (xbcel[i].text, currentShrink))
			shrinkComboValue = i;

	}
	xbcel[i].text = NULL;
	MenuAddCombo (DLG_LEFT, MENU_ROW (0), DLG_WIDTH, N_("Shrink Type"), 3 * CELL_W,
				  &shrinkComboValue, NULL, NULL, (void *)xbcel);

	MenuAddComboBool (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, N_("Use Scramble Draw"),
					  2 * CELL_W, &shrinkUseDraw);

	MenuAddInteger (DLG_LEFT, MENU_ROW (2), DLG_WIDTH, N_("Scramble Draw Time (%)"),
					2 * CELL_W, &scrambleDrawTime, 0, GAME_TIME);
	MenuAddComboBool (DLG_LEFT, MENU_ROW (3), DLG_WIDTH, N_("Use Scramble Delete"),
					  2 * CELL_W, &shrinkUseDelete);
	MenuAddInteger (DLG_LEFT, MENU_ROW (4), DLG_WIDTH, N_("Scramble DeleteTime (%)"),
					2 * CELL_W, &scrambleDeleteTime, 0, GAME_TIME);

	MenuSetAbort (MenuAddHButton (5 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Abort"), CreateLevelMainMenu, par));
	MenuSetDefault (MenuAddHButton (17 * CELL_W / 2, MENU_BOTTOM,
									4 * CELL_W, N_("Ok"), SetShrink, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}								/* CreateShrinkMenu */

/*
 * This is called when the user presses the abort button in the map editor
 */
static XBBool
ReturnFromMap (void *par)
{
	int x, y;
	DBSection *section;
	blockSelected = 0;

	//  section = DB_GetSection(level,atomMap);

	section = DB_CreateSection (level, atomMap);
	if (!recreate) {
		for (y = 0; y < MAZE_H - 1; y++) {
			for (x = 0; x < MAZE_W; x++) {

				//mazeSave[y][x]=GetMazeBlock(x,y); 
				if (mazeSaveEvil[y][x]) {

					mazeSaveChar[y][x] = 'e';
				}
				else {
					switch (mazeSave[y][x]) {
					case BTFree:
						mazeSaveChar[y][x] = '_';
						break;
					case BTBlock:
						mazeSaveChar[y][x] = 'B';
						break;
					case BTBlockRise:
						mazeSaveChar[y][x] = 'R';
						break;
					case BTExtra:
						mazeSaveChar[y][x] = 'X';
						break;
					case BTBomb:
						mazeSaveChar[y][x] = 'b';
						break;
					case BTRange:
						mazeSaveChar[y][x] = 'r';
						break;
					case BTSick:
						mazeSaveChar[y][x] = 's';
						break;
					case BTSpecial:
						mazeSaveChar[y][x] = 'q';
						break;
					case BTVoid:
						mazeSaveChar[y][x] = 'v';
						break;
					case BTEvil:
						mazeSaveChar[y][x] = 'e';
						break;
					case BTBackground:
						mazeSaveChar[y][x] = 'V';
						break;
					case BTBurned:
						mazeSaveChar[y][x] = '_';
						break;
					case BTExtraOpen:
						mazeSaveChar[y][x] = '_';
						break;
					case BTNUM:
						mazeSaveChar[y][x] = '_';
						break;
					default:
						mazeSaveChar[y][x] = '_';
						break;

					}

				}
				fprintf (stderr, "%c", mazeSaveChar[y][x]);
			}
			fprintf (stderr, "\nfrom get\n");
			DB_CreateEntryString (section, atomArrayRow00[y], mazeSaveChar[y]);
		}
		DB_CreateEntryString (section, atomArrayRow00[MAZE_H - 1], "BBBBBBBBBBBBBBB");
		// }
	}
	if (recreate) {
		SetShrink (NULL);
		return XBTrue;
	}
	else {

		recreate = XBTrue;
		SetShrink (NULL);
		recreate = XBFalse;
		DeleteAllMapBombSprites ();
		DeleteAllBombSprites ();
		SetXBEditMapMode (XBFalse);
		SetXBMapMode (XBFalse);
		FinishLevel ();
		MenuLoadTiles ();
		DeleteAllBombSprites ();
		return CreateLevelMainMenu (par);
	}
}								/* ButtonReturnFromMap */

XBBool
SetToBlockFree (void)
{
	tempBlock = blockSelected;
	blockSelected = 0;
	return XBTrue;
}

static XBBool
SetBlockGraphics (int block)
{
	tempBlock = blockSelected;
	blockSelected = block;
	return XBTrue;
}

XBBool
SetOldBlock (void)
{
	blockSelected = tempBlock;
	return XBTrue;
}

/*
 * This is called when the user presses a tool button in the map editor
 */
static XBBool
MapToolClicked (void *par)
{
	if ((long)par == 1) {
		blockSelected = -2;
	}
	else {
		blockSelected = (long)par - 2;
	}
	printf ("Tool %ld selected block %i\n", (long)par, blockSelected);
	return XBTrue;
}								/* ButtonMapTool */

/*
 * This is called when the user presses a tool button in the map editor
 */
static void
ChangeBlock (char *name)
{

	XBColor fg, bg, add;

	Dbg_Out (" block Selected %i \n", blockSelected);
	GUI_FreeBlock (blockSelected);
	if (useFg[blockSelected]) {
		fg = SET_COLOR (rgbs[blockSelected][0].red, rgbs[blockSelected][0].green,
						rgbs[blockSelected][0].blue);
		bg = SET_COLOR (rgbs[blockSelected][1].red, rgbs[blockSelected][1].green,
						rgbs[blockSelected][1].blue);
		add =
			SET_COLOR (rgbs[blockSelected][2].red, rgbs[blockSelected][2].green,
					   rgbs[blockSelected][2].blue);
		GUI_LoadBlockCch (blockSelected, name, fg, bg, add);
	}
	else {
		GUI_LoadBlockRgb (blockSelected, name);

	}
	GUI_DrawBlock (10, 9, blockSelected);
	GUI_FlushBlocks ();
	GUI_FlushPixmap (XBFalse);

}

static XBBool
BlockGraphicToolClicked (void *par)
{
	block = block + 1;

	if (dirBlockName == NULL)
		dirBlockName = ppmList;
	dirBlockName = dirBlockName->next;
	if (dirBlockName == NULL)
		dirBlockName = ppmList;
	ChangeBlock (dirBlockName->name);
	if (blockName[blockSelected] == NULL) {
		blockName[blockSelected] = malloc (strlen (dirBlockName->name) + 1);
		strcpy (blockName[blockSelected], dirBlockName->name);
	}
	else {
		free (blockName[blockSelected]);
		blockName[blockSelected] = malloc (strlen (dirBlockName->name) + 1);
		strcpy (blockName[blockSelected], dirBlockName->name);
	}

	return XBTrue;
}								/* ButtonMapTool */

static XBBool
ApplyGraphics (void *par)
{

	if (dirBlockName != NULL) {
		ChangeBlock (dirBlockName->name);
		if (blockName[blockSelected] == NULL) {
			blockName[blockSelected] = malloc (strlen (dirBlockName->name) + 1);
			strcpy (blockName[blockSelected], dirBlockName->name);
		}
		else {
			free (blockName[blockSelected]);
			blockName[blockSelected] = malloc (strlen (dirBlockName->name) + 1);
			strcpy (blockName[blockSelected], dirBlockName->name);
		}
	}
	return XBTrue;

}

#ifdef unused
/*
 * This is called when the user presses a map square
 */
static XBBool
MapSquareClicked (void *par)
{
	printf ("Square %ld clicked\n", (long)par);
	return XBTrue;
}								/* ButtonMapSquare */
#endif

/*
static
XBBool RedrawMapMaze(void){
  int i,j;
    for (i = 0; i < MAZE_H-1; i++) {
      for (j = 0; j < MAZE_W; j++) {

	SetMazeBlock(j,i,mazeSave[i][j]);

      }
    }
  UpdateMaze ();
  return XBTrue;

  }*/
static void
RedMapEditMaze0 (void)
{

	int x, y;
	static int oldTime = 0;
	if (oldTime == 0)
		oldTime = gameTime;
	// fprintf(stderr,"updating maz time %i %i\n",gameTime,oldTime);
	if (oldTime != gameTime) {
		for (y = 1; y < MAZE_H - 1; y++) {
			for (x = 1; x < MAZE_W - 1; x++) {
				SetMazeBlock (x, y, mazeSave[y][x]);
			}
		}
		oldTime = gameTime;
	}

}
static void
RedMapEditMaze1 (void)
{

	int x, y;
	static int oldTime = 0;
	if (oldTime == 0)
		oldTime = gameTime;
	fprintf (stderr, "updating maz time %i %i\n", gameTime, oldTime);
	// if(oldTime==gameTime){
	for (y = 1; y < MAZE_H - 1; y++) {
		for (x = 1; x < MAZE_W - 1; x++) {
			//  mazeSave[y][x]=GetMazeBlock(x,y); 
			fprintf (stderr, "deletetime %i %i \n", scrambleDeleteTime, gameTime);
			if (gameTime <= scrambleDeleteTime) {

				fprintf (stderr, "block1 %i  \n", mazeScrambleDelete[y][x]);
				if (mazeScrambleDelete[y][x]) {
					SetMazeBlock (x, y, 0);
					SetMazeBlock (x, y, 1);
					GUI_DrawBlock (x, y, 0);
					GUI_DrawBlock (x, y, 1);
					fprintf (stderr, "block %i \n", GetMazeBlock (x, y));
				}
			}
			if (gameTime <= scrambleDrawTime) {
				if (mazeScrambleDraw[y][x]) {
					GUI_DrawBlock (x, y, 5);
				}
			}
		}
	}
	oldTime = gameTime;
	// }

}

/*
 *
 */
void
SetEditMapBlock (int x, int y)
{
	char timeString[3];
	int p, i;
	BMRectangle get_box = {
		69 * CELL_W, 79 * CELL_H,
		STAT_WIDTH * 13, 2 * STAT_HEIGHT / 3,
	};
	if ((y / CELL_H) < 12 && (x / CELL_W) != 0 && (x / CELL_W) != 14 && (y / CELL_H) != 0) {
		if (blockSelected == 1) {
			sprintf (timeString, "%2i", scrambleDeleteTime);
			timeString[2] = 0;
			GUI_DrawBlock ((BTNUM + 2), 13, 2);
			GUI_DrawBlock (x / CELL_W, y / CELL_H, blockSelected);
			mazeSave[y / CELL_H][x / CELL_W] = BTFree;
			for (p = 0; p < (4 * MAZE_W * scrambleDeleteTime) / 100; p++) {
				GUI_DrawTimeLed (p, 1);
			}
			for (; p < (4 * MAZE_W); p++) {
				GUI_DrawTimeLed (p, 0);
			}
			GUI_DrawTimeLed ((4 * MAZE_W) * scrambleDrawTime / 100, 3);
			GUI_DrawTimeLed ((4 * MAZE_W) * scrambleDeleteTime / 100, 4);
			GUI_DrawTextbox (timeString, FF_Medium | FF_Black, &get_box);
			gameTime = scrambleDeleteTime;
			shrinkUseDelete = XBTrue;

			if (!mazeScrambleDelete[y / CELL_H][x / CELL_W]) {
				mazeScrambleDelete[y / CELL_H][x / CELL_W] = XBTrue;
				numShDeBlocks++;
			}
			//    RedrawMapMaze();
			RedMapEditMaze0 ();
			DoShrinkMapEdit ((GAME_TIME) - (GAME_TIME * scrambleDeleteTime) / 100);
			RedMapEditMaze1 ();
			UpdateMaze ();
			MarkMazeRect (13, MAZE_H, CELL_W, CELL_H);
			GUI_FlushPixmap (XBFalse);
		}
		else if (blockSelected == 5) {
			sprintf (timeString, "%2i", scrambleDrawTime);
			timeString[2] = 0;
			GUI_DrawBlock ((BTNUM + 2), 13, 2);
			GUI_DrawBlock (x / CELL_W, y / CELL_H, blockSelected);
			mazeSave[y / CELL_H][x / CELL_W] = BTFree;
			for (p = 0; p < (4 * MAZE_W * scrambleDrawTime) / 100; p++) {
				GUI_DrawTimeLed (p, 1);
			}
			for (; p < (4 * MAZE_W); p++) {
				GUI_DrawTimeLed (p, 0);
			}
			GUI_DrawTimeLed ((4 * MAZE_W * scrambleDrawTime) / 100, 3);
			GUI_DrawTimeLed ((4 * MAZE_W * scrambleDeleteTime) / 100, 4);
			GUI_DrawTextbox (timeString, FF_Small | FF_White, &get_box);

			if (!mazeScrambleDraw[y / CELL_H][x / CELL_W]) {
				mazeScrambleDraw[y / CELL_H][x / CELL_W] = XBTrue;
				numShDrBlocks++;
			}
			shrinkUseDraw = XBTrue;
			gameTime = scrambleDrawTime;
			RedMapEditMaze0 ();
			DoShrinkMapEdit ((GAME_TIME) - (GAME_TIME * scrambleDrawTime) / 100);
			UpdateMaze ();
			RedMapEditMaze1 ();
			MarkMazeRect (13, MAZE_H, CELL_W, CELL_H);
			//GUI_FlushBlocks ();
			GUI_FlushPixmap (XBFalse);
		}
		else {
			if (blockSelected == -2) {
				mazeSaveEvil[y / CELL_H][x / CELL_W] = XBTrue;
			}
			if (mazeScrambleDraw[y / CELL_H][x / CELL_W]) {
				mazeScrambleDraw[y / CELL_H][x / CELL_W] = XBFalse;
				numShDrBlocks--;
			}
			if (mazeScrambleDelete[y / CELL_H][x / CELL_W]) {
				mazeScrambleDelete[y / CELL_H][x / CELL_W] = XBFalse;
				numShDeBlocks--;
			}
			mazeSave[y / CELL_H][x / CELL_W] = blockSelected;
			SetMazeBlock (x / CELL_W, y / CELL_H, blockSelected);
		}
	}
	else {
		if (y > 111 && y < 115) {

			for (p = 0; p < x / 2; p++) {
				GUI_DrawTimeLed (p, 1);
			}
			for (; p < (4 * MAZE_W); p++) {
				GUI_DrawTimeLed (p, 0);
			}
			gameTime = (x * 50) / (4 * MAZE_W);
			fprintf (stderr, "seeting led %i %i %i \n", y, x, gameTime);
			GUI_DrawTimeLed ((4 * MAZE_W) * scrambleDeleteTime / 100, 4);
			GUI_DrawTimeLed ((4 * MAZE_W) * scrambleDrawTime / 100, 3);
			// RedrawMapMaze0();
			RedMapEditMaze0 ();
			DoShrinkMapEdit ((GAME_TIME) - ((GAME_TIME * gameTime) / (100)));

			p = -1;
			while (p != 0) {
				p = getShrinkTimes (p);
				if (p != 0) {
					if ((p >= 0) && (p < GAME_TIME)) {
						i = 4 * MAZE_W - (p / TIME_STEP);	// inverse 
						GUI_DrawTimeLed (i, 2);
					}
				}
			}
			UpdateMaze ();
			RedMapEditMaze1 ();
			MarkMazeRect (13, MAZE_H, CELL_W, CELL_H);
			GUI_FlushPixmap (XBFalse);
		}
	}
}
static XBBool
ExitGraphicsMenu (void *par)
{
	MenuUnloadTiles ();
	MenuLoadTiles ();
	return CreateLevelMainMenu (par);
}

/*
 * create color value list
 */
static XBComboEntryList *
CreateColorValueList (void)
{
	XBComboEntryList *list;
	int i;
	char tmp[8];

	list = calloc (XBCOLOR_DEPTH + 2, sizeof (XBComboEntryList));
	assert (list != NULL);
	for (i = 0; i <= XBCOLOR_DEPTH; i++) {
		sprintf (tmp, "%3d", 255 * i / XBCOLOR_DEPTH);
		list[i].text = (char *)DupString ((char *)tmp);
		list[i].value = i;
	}
	return list;
}								/* CreateColorValueList */

#ifdef notused
static XBBool
SetFg (void *par)
{
	useFg[blockSelected] = !useFg[blockSelected];
	printf ("Isefg %i\n", useFg[blockSelected]);
	return XBTrue;
}
#endif

static XBBool
SaveGraphics (void *par)
{
	DBSection *section;
	int temp;
	char tempString[256];
	temp = blockSelected;
	section = DB_GetSection (level, atomGraphics);
	if (section == NULL)
		section = DB_CreateSection (level, atomGraphics);
	for (blockSelected = 0; blockSelected < BTNUM; blockSelected++) {
		if (blockName[blockSelected]) {
			if (useFg[blockSelected]) {
				sprintf (tempString, "%s\t #%02x%02x%02x #%02x%02x%02x #%02x%02x%02x",
						 blockName[blockSelected],
						 rgbs[blockSelected][0].red, rgbs[blockSelected][0].green,
						 rgbs[blockSelected][0].blue,
						 rgbs[blockSelected][1].red, rgbs[blockSelected][1].green,
						 rgbs[blockSelected][1].blue,
						 rgbs[blockSelected][2].red, rgbs[blockSelected][2].green,
						 rgbs[blockSelected][2].blue);
			}
			else {
				sprintf (tempString, "%s", blockName[blockSelected]);
			}
			DB_DeleteEntry (section, atomArrayBlock00[blockSelected]);
			DB_CreateEntryString (section, atomArrayBlock00[blockSelected],
								  (char *)&tempString);
		}
	} blockSelected = temp;

	if (recreate) {
		return XBTrue;
	}
	else {
		return ExitGraphicsMenu (par);
	}

}

/*
 * Edit the graphics
 */
static XBBool
FreeBlockMenu (void *par)
{
	//MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (0);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Free Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
ShawdowedBlockMenu (void *par)
{
	//MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (1);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Burned Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
SolidBlockMenu (void *par)
{
	//MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (2);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Solid Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
RisingBlockMenu (void *par)
{
	//MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (3);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Rising Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
BlastableBlockMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (4);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Blastable Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
BlastedBlockMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (5);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Void Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
ExtraBlockBombMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (6);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Bomb Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
ExtraBlockRangeMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (7);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Range Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
BlockTrapMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (8);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Trap Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
ExtraBlockSpecialMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (9);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Special Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
VoidBlockMenu (void *par)
{
	// MenuUnloadTiles ();
	MenuClear ();
	SetBlockGraphics (10);
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Void Block"));
	BuildColorsMenu (par);
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
CreateGraphicsMenu (void *par)
{
	static XBComboEntryList *colorValueList = NULL;
	//  MenuUnloadTiles ();
	// MenuLoadTiles ();
	MenuClear ();
	SetOldBlock ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Select Block"));
	if (NULL == colorValueList) {
		colorValueList = CreateColorValueList ();
	}
	MenuAddHButton (2 * CELL_W, MENU_ROW (0), 4 * CELL_W, N_("Free Block"), FreeBlockMenu, par);
	MenuAddHButton (9 * CELL_W, MENU_ROW (0), 4 * CELL_W, N_("Shadowed Block"),
					ShawdowedBlockMenu, par);
	MenuAddHButton (2 * CELL_W, MENU_ROW (1), 4 * CELL_W, N_("Solid Block"), SolidBlockMenu, par);
	MenuAddHButton (9 * CELL_W, MENU_ROW (1), 4 * CELL_W, N_("Rising Block"), RisingBlockMenu, par);
	MenuAddHButton (2 * CELL_W, MENU_ROW (2), 4 * CELL_W, N_("Blastable Block"),
					BlastableBlockMenu, par);
	MenuAddHButton (9 * CELL_W, MENU_ROW (2), 4 * CELL_W, N_("Blasted Block"), BlastedBlockMenu, par);
	MenuAddHButton (2 * CELL_W, MENU_ROW (3), 4 * CELL_W, N_("Extra Bomb"), ExtraBlockBombMenu, par);
	MenuAddHButton (9 * CELL_W, MENU_ROW (3), 4 * CELL_W, N_("Extra Range"), ExtraBlockRangeMenu, par);
	MenuAddHButton (2 * CELL_W, MENU_ROW (4), 4 * CELL_W, N_("Trap"), BlockTrapMenu, par);
	MenuAddHButton (9 * CELL_W, MENU_ROW (4), 4 * CELL_W, N_("Special Extra"),
					ExtraBlockSpecialMenu, par);
	MenuAddHButton (2 * CELL_W, MENU_ROW (5), 4 * CELL_W, N_("Void Block"), VoidBlockMenu, par);

	MenuSetAbort (MenuAddHButton
				  (3 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, N_("Abort"), ExitGraphicsMenu, par));
	MenuAddHButton (11 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, N_("Apply"), ApplyGraphics, par);
	MenuSetDefault (MenuAddHButton
					(17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Save"), SaveGraphics, par));
	MenuSetLinks ();
	/* that's all */
	return XBFalse;

}

static XBBool
BuildColorsMenu (void *par)
{
	static XBComboEntryList *colorValueList = NULL;
	if (NULL == colorValueList) {
		colorValueList = CreateColorValueList ();
	}
	MenuAddComboInt (2 * CELL_W, MENU_ROW (1), 3 * CELL_W, "FG R",
					 3 * CELL_W / 2, &rgbs[blockSelected][0].red, colorValueList);
	MenuAddComboInt (8 * CELL_W, MENU_ROW (1), 3 * CELL_W, "FG G",
					 3 * CELL_W / 2, &rgbs[blockSelected][0].green, colorValueList);
	MenuAddComboInt (2 * CELL_W, MENU_ROW (2), 3 * CELL_W, "FG B",
					 3 * CELL_W / 2, &rgbs[blockSelected][0].blue, colorValueList);
	MenuAddComboInt (8 * CELL_W, MENU_ROW (2), 3 * CELL_W, "BG R",
					 3 * CELL_W / 2, &rgbs[blockSelected][1].red, colorValueList);
	MenuAddComboInt (2 * CELL_W, MENU_ROW (3), 3 * CELL_W, "BG G",
					 3 * CELL_W / 2, &rgbs[blockSelected][1].green, colorValueList);
	MenuAddComboInt (8 * CELL_W, MENU_ROW (3), 3 * CELL_W, "BG B",
					 3 * CELL_W / 2, &rgbs[blockSelected][1].blue, colorValueList);
	MenuAddComboInt (2 * CELL_W, MENU_ROW (4), 3 * CELL_W, "Add R",
					 3 * CELL_W / 2, &rgbs[blockSelected][2].red, colorValueList);
	MenuAddComboInt (8 * CELL_W, MENU_ROW (4), 3 * CELL_W, "Add G",
					 3 * CELL_W / 2, &rgbs[blockSelected][2].green, colorValueList);
	MenuAddComboInt (2 * CELL_W, MENU_ROW (5), 3 * CELL_W, "Add B",
					 3 * CELL_W / 2, &rgbs[blockSelected][2].blue, colorValueList);
	MenuSetDefault (MenuAddHButton (2 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W,
									N_("Return"), CreateGraphicsMenu, par));
	ppmList = CreateFileList (GAME_DATADIR "/image/block/", "ppm", XBFalse);
	MenuAddHButton (3 * CELL_W, MENU_ROW (6), CELL_W * 6, N_("Change Block"),
					BlockGraphicToolClicked, (void *)block);
	MenuAddComboBool (3 * CELL_W, MENU_ROW (7), 5 * CELL_W, N_("Change Colors"),
					  2 * CELL_W, &useFg[blockSelected]);
	GUI_DrawBlock (10, 9, blockSelected);
	MenuAddHButton (11 * CELL_W / 2, MENU_BOTTOM, 3 * CELL_W, N_("Apply"), ApplyGraphics, par);

	return XBFalse;
}

/*
 * Edit the map
 */
static XBBool
CreateMapMenu (void *par)
{
	long i;
	int j, p;
	BMRectangle get_box = {
		2 * STAT_WIDTH / 12,
#ifdef SMPF
		93 * STAT_HEIGHT / 6,
#else
		87 * STAT_HEIGHT / 6,

#endif
		STAT_WIDTH * 16, 2 * STAT_HEIGHT / 3,
	};
	char *msg = "Select a block above and put on the maze!";

	// unload menu bg tiles, load map tiles
	SetXBEditMapMode (XBTrue);
	MenuUnloadTiles ();
	MenuClear ();
	SetPressed (XBFalse);
	SetXBMapMode (XBTrue);
	ConfigLevel (level);
	//  InitButtonsMap()
	// tool buttons

	ClearStatusBar (0, 0);

	DrawMaze ();
	SetMazeBlock (0, 13, (BMMapTile) 2);
	SetMazeBlock (1, 13, (BMMapTile) - 2);
	for (i = 2; i < BTNUM; ++i) {
		SetMazeBlock (i, 13, (BMMapTile) i - 2);
		MarkMazeTile (i, 13);
		GUI_DrawBlock (i, 13, i - 2);
	}
	/* for (i =BTNUM ; i <= 14; ++i)
	   SetMazeBlock(i, 13,(BMMapTile)1);
	   SetMazeBlock(0, 13,(BMMapTile)1); */
	MarkMaze (0, MAZE_H, MAZE_W, MAZE_H + 1);
	for (i = 1; i < BTNUM + 1; ++i) {
		if (i == 3) {
			deleteButton = MenuAddHButton (i * CELL_W, MENU_ROW (10), CELL_W, "de",
										   MapToolClicked, (void *)i);
		}
		else if (i == 7) {
			drawButton = MenuAddHButton (i * CELL_W, MENU_ROW (10), CELL_W, "dr",
										 MapToolClicked, (void *)i);

		}
		else {
			MenuAddHButton (i * CELL_W, MENU_ROW (10), CELL_W, "", MapToolClicked, (void *)i);

		}

	}
	MenuAddHButton ((BTNUM + 1) * CELL_W, MENU_ROW (10), CELL_W, N_("exit"), ReturnFromMap, NULL);

	DrawMaze ();
	GUI_DrawBlock (0, 13, 2);
	GUI_DrawBlock (1, 13, 2);
	GUI_DrawBlock (14, 13, 2);
	GUI_DrawBlock (13, 13, 2);
	GUI_DrawBlock (12, 13, 2);
	if (init) {
		for (i = 0; i < MAZE_H; i++) {
			for (j = 0; j < MAZE_W; j++) {
				mazeSave[i][j] = GetMazeBlock (j, i);
				mazeSaveEvil[i][j] = XBFalse;
			}
		}
		init = 0;
	}
	else {

		for (i = 0; i < MAZE_H - 1; i++) {
			for (j = 0; j < MAZE_W; j++) {
				if (mazeSaveEvil[i][j]) {
					fprintf (stderr, " evil %li %i \n", i, j);
					SetMazeBlock (j, i, (BMMapTile) - 2);
				}
			}
		}
	}
	SetMazeBlock (1, 13, (BMMapTile) - 2);
	for (i = 2; i < BTNUM + 1; ++i) {
		SetMazeBlock (i, 13, (BMMapTile) i - 2);

		GUI_DrawBlock (i, 13, (BMMapTile) i - 2);
	}
	for (i = BTNUM + 1; i <= 14; ++i)
		SetMazeBlock (i, 13, (BMMapTile) 2);
	SetMazeBlock (0, 13, (BMMapTile) 2);
	for (i = 0; i < (4 * MAZE_W); i++) {
		GUI_DrawTimeLed (i, 1);
	}

	p = -1;
	while (p != 0) {
		p = getShrinkTimes (p);
		if (p != 0) {
			if ((p >= 0) && (p < GAME_TIME)) {
				i = 4 * MAZE_W - (p / TIME_STEP);	// inverse 
				GUI_DrawTimeLed (i, 2);
			}
		}
	}
	GUI_DrawTimeLed ((4 * MAZE_W) * scrambleDeleteTime / 100, 4);
	GUI_DrawTimeLed ((4 * MAZE_W) * scrambleDrawTime / 100, 3);
	GUI_DrawTextbox (msg, TEXT_ATTR_NORMAL | FF_Scroll, &get_box);
	MarkMaze (0, MAZE_H + STAT_H - 1, 20, MAZE_H + STAT_H);
	//   for (i =0 ; i <= 14; ++i)
	//  GUI_DrawBlock (MAZE_H+2,14,(BMMapTile) 2);

	/*  for (i =0 ; i <= 14; ++i)
	   SetMazeBlock(i, 13,(BMMapTile) BTVoid); */
	/* for (i =0 ; i <= 14; ++i)
	   SetMazeBlock(i, 13,(BMMapTile) 1); */
	MenuSetLinks ();
	return XBFalse;
}

/*
 * Create the Edit Level menu
 */
static XBBool
CreateLevelMainMenu (void *par)
{

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Level"));
	MenuAddHButton (9 * CELL_W / 2, MENU_TOP, 6 * CELL_W, N_("Edit Level Info"), CreateInfoMenu, par);
	MenuAddHButton (9 * CELL_W / 2, MENU_TOP + 1 * CELL_H, 6 * CELL_W, N_("Edit Shrink"),
					CreateShrinkMenu, par);

	MenuAddHButton (9 * CELL_W / 2, MENU_TOP + 2 * CELL_H, 6 * CELL_W, N_("Edit Special Extras"),
					CreateSpecialExtrasMenu, par);

	MenuAddHButton (9 * CELL_W / 2, MENU_TOP + 3 * CELL_H, 6 * CELL_W, N_("Edit Bombs"),
					CreateBombsMenu, par);

	MenuAddHButton (9 * CELL_W / 2, MENU_TOP + 4 * CELL_H, 6 * CELL_W, N_("Edit Graphics"),
					CreateGraphicsMenu, par);

	MenuAddHButton (9 * CELL_W / 2, MENU_TOP + 5 * CELL_H, 6 * CELL_W, N_("Edit Map"), CreateMapMenu,
					par);

	MenuSetDefault (MenuAddHButton (3 * CELL_W / 2, MENU_BOTTOM,
									4 * CELL_W, "Save", SaveLevel, par));
	MenuAddHButton (11 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Save as ..."), CreateSaveAsMenu, par);

	MenuSetAbort (MenuAddHButton (19 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Cancel"), CreateEditMenu, par));

	MenuSetLinks ();
	return XBFalse;
}								/* CreateEditLevelMenu */

/*
 * Load a level file
 */
static XBBool
LoadLevel (void *par)
{
	level = DB_Create (DT_Level, GUI_StringToAtom (fileName));

	if (!DB_Load (level)) {
		DB_Delete (level);
		return XBFalse;
	}
	return CreateLevelMainMenu (par);
}								/* ButtonLoadLevel */

/*
 * Create a new level
 */
static XBBool
CreateLevel (void *par)
{
	DBSection *section;
	int i;

	if (strlen (fileName) == 0)
		strcpy (fileName, "tempLevel");
	level = DB_Create (DT_Level, GUI_StringToAtom (fileName));

	/* map */
	section = DB_CreateSection (level, atomMap);
	strcpy (mazeSaveChar[0], "BBBBBBBBBBBBBBB");
	DB_CreateEntryString (section, atomArrayRow00[0], mazeSaveChar[0]);
	for (i = 1; i < MAZE_H - 1; ++i) {
		strcpy (mazeSaveChar[i], "B_____________B");
		DB_CreateEntryString (section, atomArrayRow00[i], mazeSaveChar[i]);
	}
	strcpy (mazeSaveChar[MAZE_H - 1], "BBBBBBBBBBBBBBB");
	DB_CreateEntryString (section, atomArrayRow00[MAZE_H - 1],
						  mazeSaveChar[MAZE_H - 1]);

	/* graphics */
	section = DB_CreateSection (level, atomGraphics);
	for (i = 0; i < MAX_BLOCK; ++i)
		DB_CreateEntryString (section, atomArrayBlock00[i], graphics[i]);

	/* bombs */
	section = DB_CreateSection (level, atomBombs);

	/* player */
	section = DB_CreateSection (level, atomPlayer);
	DB_CreateEntryInt (section, atomBombs, 3);
	DB_CreateEntryInt (section, atomRange, 3);
	for (i = 0; i < MAX_PLAYER; ++i)
		DB_CreateEntryString (section, atomArrayPos0[i + 1], "7 6");

	/* func */
	section = DB_CreateSection (level, atomFunc);
	DB_CreateEntryString (section, atomExtra, "invincible");

	/* shrink */
	section = DB_CreateSection (level, atomShrink);

	/* info */
	section = DB_CreateSection (level, atomInfo);
	DB_CreateEntryString (section, atomGameMode, "R23456STDL");
	DB_CreateEntryString (section, atomHint, _("Write something clever here"));
	DB_CreateEntryString (section, atomAuthor, _("My name"));
	DB_CreateEntryString (section, atomName, _("My new level"));
	DB_Store (level);
	memset (mazeSave, 0, MAZE_H * MAZE_W);
	init = 1;
	memset (mazeScrambleDraw, 0, MAZE_H * MAZE_W);
	memset (mazeScrambleDelete, 0, MAZE_H * MAZE_W);
	if (recreate) {
		return XBTrue;
	}
	else {
		return CreateLevelMainMenu(par);
	}
}								/* ButtonCreateLevel */

/*
 * Create the Load Level menu
 */
static XBBool
CreateLoadLevelMenu (void *par)
{

	MenuClear ();

	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Load Level"));
	MenuAddString (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, N_("Level File:"),
				   4 * CELL_W, fileName, STRING_LENGTH);

	MenuSetAbort (MenuAddHButton (5 * CELL_W / 2, MENU_BOTTOM,
								  4 * CELL_W, N_("Abort"), CreateEditMenu, par));
	MenuSetDefault (MenuAddHButton (17 * CELL_W / 2, MENU_BOTTOM,
									4 * CELL_W, N_("Ok"), LoadLevel, par));

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}

XBBool
CreateEditMenu (void *par)
{
	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Edit Levels"));
	MenuAddHButton (9 * CELL_W / 2, MENU_ROW (0), 6 * CELL_W, N_("Create New Level"), CreateLevel, par);
	MenuAddHButton (9 * CELL_W / 2, MENU_ROW (1),
					6 * CELL_W, N_("Load Existing Level"), CreateLoadLevelMenu, par);
	MenuSetAbort (MenuAddHButton (9 * CELL_W / 2, MENU_BOTTOM,
								  6 * CELL_W, N_("Extras Menu"), CreateExtrasMenu, par));

	/* return and escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}
