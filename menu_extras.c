/*
 * file menu_extras.c - user interface for extras like demo playback and statistics
 *
 * $Id: menu_extras.c,v 1.17 2006/03/28 11:41:19 fzago Exp $
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
 * local vriables
 */
static XBStatData *datStat = NULL;
static XBCentralData *datCentral = NULL;
static size_t numStat = 0;
#ifdef W32

static long firstStat = 0;
static long lastStat = 0;
static long firstDemo = 0;
static long lastDemo = 0;
#else
static ssize_t firstStat = 0;
static ssize_t lastStat = 0;
static ssize_t firstDemo = 0;
static ssize_t lastDemo = 0;

#endif
static MIC_button exitFunc = NULL;
static MIC_button selectFunc = NULL;

static CFGDemoEntry *demoList = NULL;
static size_t numDemos = 0;

static CFGPlayerGraphics gfxPlayer[4];	// XBCC
static const CFGPlayerGraphics *pGfxPlayer[4] = {
	NULL, NULL, NULL, NULL,
};

static BMSpriteAnimation allAnime[4] = {	// XBCC
	SpriteStopLeft, SpriteStopUp, SpriteStopDown, SpriteStopRight
};
static BMSpriteAnimation bigAnime[1] = { SpriteWinner };	// XBCC
static XBBool centralDat;

static int received;

/***************************
 * generic statistics menu *
 ***************************/

/* needed prototype*/
static XBBool CreateStatMenu (void *par);

/*
 * forward in selection
 */
static XBBool
ButtonForward (void *par)
{
	if (lastStat < numStat) {
		firstStat = lastStat;
	}
	return CreateStatMenu (par);
}								/* ButtonForward */

/*
 * backward in selection
 */
static XBBool
ButtonBackward (void *par)
{
	firstStat -= 2 * MAX_GSTAT_ROWS;
	if (firstStat < 0) {
		firstStat = 0;
	}
	return CreateStatMenu (par);
}								/* ButtonBackward */

/*
 * back to extras menu
 */
static XBBool
ButtonBack (void *par)
{
	MIC_button func = (MIC_button) par;

	assert (func != NULL);
	assert (datStat != NULL);
	/* --- */
	free (datStat);
	datStat = NULL;
	numStat = 0;
	firstStat = 0;
	lastStat = 0;
	exitFunc = NULL;
	selectFunc = NULL;
	/* */
	return (*func) (NULL);
}								/* ButtonBack */

/*
 * draw generic statistics menu
 */
static XBBool
CreateStatMenu (void *par)
{
	size_t i, j;

	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Player Statistics"));
	/* list buttons */
	MenuSetActive (MenuAddStatHeader (GSTAT_LEFT, GSTAT_TOP, GSTAT_WIDTH, par), XBFalse);
	for (i = 0, j = firstStat; j < numStat && i < MAX_GSTAT_ROWS; i++, j++) {
		MenuSetActive (MenuAddStatEntry
					   (GSTAT_LEFT, GSTAT_ROW (i), GSTAT_WIDTH, datStat + j, selectFunc,
						datStat + j), (selectFunc != NULL));
	}
	lastStat = j;
	/* Buttons */
	MenuSetActive (MenuAddHButton
				   (GSTAT_BACK_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, "<<<", ButtonBackward, par),
				   (firstStat > 0));
	MenuSetAbort (MenuAddHButton
				  (GSTAT_ESCAPE_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, N_("Up"), ButtonBack,
				   (void *)exitFunc));
	MenuSetActive (MenuAddHButton
				   (GSTAT_FORW_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, ">>>", ButtonForward, par),
				   (lastStat < numStat));
	/* */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateStatMenu */

/*********************
 * Player Statistics *
 *********************/

/*
 * create menu with info for selected player
 */
static XBBool
CreateSinglePlayerStatMenu (void *par)
{
	XBStatData *ptr = par;
	XBAtom player = ptr->atom;

	/* delete old list */
	assert (datStat != NULL);
	free (datStat);
	/* get play list */
	datStat = CreatePlayerSingleStat (player, &numStat);
	if (NULL == datStat) {
		return CreatePlayerStatMenu (NULL);
	}
	firstStat = 0;
	exitFunc = CreatePlayerStatMenu;
	selectFunc = NULL;
	return CreateStatMenu (N_("Level"));
}								/* CreatePlayerStatMenu */

/*
 * create player statistics menu
 */
XBBool
CreatePlayerStatMenu (void *par)
{
	/* get play list */
	datStat = CreatePlayerTotalStat (&numStat);
	if (NULL == datStat) {
		return CreateExtrasMenu (NULL);
	}
	firstStat = 0;
	exitFunc = CreateExtrasMenu;
	selectFunc = CreateSinglePlayerStatMenu;
	return CreateStatMenu (N_("Player"));
}								/* CreatePlayerStatMenu */

/********************
 * level statistics *
 ********************/

/*
 * create menu with info for a selected level
 */
static XBBool
CreateSingleLevelStatMenu (void *par)
{
	XBStatData *ptr = par;
	XBAtom level = ptr->atom;

	/* delete old list */
	assert (datStat != NULL);
	free (datStat);
	/* get play list */
	datStat = CreateLevelSingleStat (level, &numStat);
	if (NULL == datStat) {
		return CreateLevelStatMenu (NULL);
	}
	firstStat = 0;
	exitFunc = CreateLevelStatMenu;
	selectFunc = NULL;
	return CreateStatMenu (N_("Player"));
}								/* CreateLevelStatMenu */

/*
 * create level statistics menu
 */
XBBool
CreateLevelStatMenu (void *par)
{
	/* get play list */
	datStat = CreateLevelTotalStat (&numStat);
	if (NULL == datStat) {
		return CreateExtrasMenu (NULL);
	}
	firstStat = 0;
	exitFunc = CreateExtrasMenu;
	selectFunc = CreateSingleLevelStatMenu;
	return CreateStatMenu (N_("Level"));
}								/* CreateLevelStatMenu */

/**********************
 * central statistics *
 **********************/

/* needed prototypes */
static XBBool CreateCentralMenu2 (void *par);

/*
 * XBCC Forward in selection
 */
static XBBool
ButtonCentralForward (void *par)
{
	if (lastStat < numStat) {
		firstStat = lastStat;
	}
	return CreateCentralMenu2 (par);
}								/* ButtonForward */

/*
 * Forward in selection
 */
static XBBool
ButtonCentralBackward (void *par)
{
	firstStat -= MAX_GSTAT_ROWS;
	if (firstStat < 0) {
		firstStat = 0;
	}
	return CreateCentralMenu2 (par);
}								/* ButtonForward */

/*
 * back to extras menu
 */
static XBBool
ButtonCentralBack (void *par)
{
	MIC_button func = (MIC_button) par;

	assert (func != NULL);
	assert (datCentral != NULL);
	/* --- */
	free (datCentral);
	datCentral = NULL;
	numStat = 0;
	firstStat = 0;
	lastStat = 0;
	exitFunc = NULL;
	selectFunc = NULL;
	/* */
	return (*func) (NULL);
}								/* ButtonBack */

/*
 * draw menu with player info from central
 */
static XBBool
CreatePlayerInfoMenu (void *par)
{
	XBCentralData *ptr = par;
	XBAtom player = ptr->atom;
	XBCentralInfo *infoCentral = NULL;
	int i;

	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Player Info"));
	/* list buttons */
	infoCentral = CreateCentralInfo (player, *ptr);
	MenuSetActive (MenuAddInfoHeader (GSTAT_LEFT, GSTAT_TOP, GSTAT_WIDTH, NULL), XBFalse);
	for (i = 0; i < MAX_MENU_INFO; i++) {
		MenuSetActive (MenuAddInfoEntry
					   (GSTAT_LEFT, GSTAT_ROW (i), GSTAT_WIDTH, infoCentral + i, NULL,
						infoCentral + i), 0);
	}
	/* Buttons */
	RetrievePlayerGraphics (CT_Central, player, COLOR_INVALID, gfxPlayer);
	for (i = 0; i < 4; i++) {
		pGfxPlayer[i] = gfxPlayer;
		MenuAddPlayer (PLAYER_LEFT (i, 4), PLAYER_TOP, PLAYER_WIDTH, i, pGfxPlayer + i, 1,
					   allAnime + i);
	}

	MenuSetAbort (MenuAddHButton
				  (GSTAT_ESCAPE_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, N_("Back"), CreateCentralStatMenu,
				   NULL));
	return XBFalse;
}								/* CreatePlayerInfoMenu */

/*
 * draw the central statistics menu
 */
static XBBool
CreateCentralMenu2 (void *par)
{
	size_t i, j;

	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Player Statistics"));
	/* list buttons */
	MenuSetActive (MenuAddCentralHeader (GSTAT_LEFT, GSTAT_TOP, GSTAT_WIDTH, par), XBFalse);
	for (i = 0, j = firstStat; j < numStat && i < MAX_GSTAT_ROWS; i++, j++) {
		MenuSetActive (MenuAddCentralEntry
					   (GSTAT_LEFT, GSTAT_ROW (i), GSTAT_WIDTH, datCentral + j, selectFunc,
						datCentral + j), (selectFunc != NULL));
	}
	lastStat = j;
	/* Buttons */
	MenuSetActive (MenuAddHButton
				   (GSTAT_BACK_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, "<<<", ButtonCentralBackward, par),
				   (firstStat > 0));
	MenuSetAbort (MenuAddHButton
				  (GSTAT_ESCAPE_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, N_("Up"), ButtonCentralBack,
				   (void *)exitFunc));
	MenuSetActive (MenuAddHButton
				   (GSTAT_FORW_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, ">>>", ButtonCentralForward, par),
				   (lastStat < numStat));

	if (RetrievePlayerGraphics (CT_Central, datCentral->atom, COLOR_INVALID, gfxPlayer)) {
		pGfxPlayer[0] = gfxPlayer;
	}
	else {
		pGfxPlayer[0] = NULL;
	}

	MenuAddPlayer (PLAYER_LEFT (0, 1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);

	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateStatList */

/*
 * XBCC create central statistics menu
 */
XBBool
CreateCentralStatMenu (void *par)
{
	/* get play list */
	if (!centralDat) {
		datCentral = CreateCentralStat (&numStat);
		firstStat = 0;
	}
	if (NULL == datCentral) {
		return CreateExtrasMenu (NULL);
	}
	centralDat = XBTrue;
	exitFunc = CreateExtrasMenu;
	selectFunc = CreatePlayerInfoMenu;
	return CreateCentralMenu2(N_("Players"));
}								/* CreateLevelStatMenu */

/*****************
 * demo playback *
 *****************/

/* needed prototypes */
static XBBool CreateDemoMenu (void *par);
/*
 * demo button
 */
static XBBool
ButtonDemoStart (void *par)
{
	XBAtom *atom = par;

	assert (NULL != par);
	/* load demo */
	LoadDemoFromFile (*atom);
	/* start game in demo mode */
	SetHostType (XBPH_Demo);
	/* clean up */
	if (NULL != demoList) {
		free (demoList);
		demoList = NULL;
	}
	MenuClear ();
	/* let's go */
	return XBTrue;
}								/* ButtonDemoStart */

/*
 * Forward in selection
 */
static XBBool
ButtonDemoForward (void *par)
{
	if (lastDemo < numDemos) {
		firstDemo = lastDemo;
	}
	return CreateDemoMenu (par);
}								/* ButtonForward */

/*
 * Forward in selection
 */
static XBBool
ButtonDemoBackward (void *par)
{
	firstDemo -= MAX_GSTAT_ROWS;
	if (firstDemo < 0) {
		firstDemo = 0;
	}
	return CreateDemoMenu (par);
}								/* ButtonForward */

/*
 * show demo playback menu
 */
static XBBool
CreateDemoMenu (void *par)
{
	size_t i, j;

	/* create demo list if needed */
	if (NULL == demoList) {
		demoList = CreateDemoList (&numDemos);
		firstDemo = 0;
	}
	/* no demos >= back to extras */
	if (NULL == demoList) {
		return CreateExtrasMenu (NULL);
	}
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Demo Playback"));
	/* list buttons */
	MenuSetActive (MenuAddDemoHeader (GSTAT_LEFT, GSTAT_TOP, GSTAT_WIDTH), XBFalse);
	for (i = 0, j = firstDemo; j < numDemos && i < MAX_GSTAT_ROWS; i++, j++) {
		(void)MenuAddDemoEntry (GSTAT_LEFT, GSTAT_ROW (i), GSTAT_WIDTH, demoList + j,
								ButtonDemoStart, &demoList[j].atom);
	}
	lastDemo = j;
	/* Buttons */
	MenuSetActive (MenuAddHButton
				   (GSTAT_BACK_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, "<<<", ButtonDemoBackward, NULL),
				   (firstDemo > 0));
	MenuSetAbort (MenuAddHButton
				  (GSTAT_ESCAPE_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, N_("Extras"), CreateExtrasMenu,
				   NULL));
	MenuSetActive (MenuAddHButton
				   (GSTAT_FORW_LEFT, MENU_BOTTOM, GSTAT_B_WIDTH, ">>>", ButtonDemoForward, NULL),
				   (lastDemo < numDemos));
	/* */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateDemoMenu */

/*****************************
 * update central statistics *
 *****************************/

/*
 * back to extras
 */
static XBBool
ButtonAbortUpdate (void *par)
{
	if (User_Connected ()) {
		User_Disconnect ();
	}
	return CreateExtrasMenu (NULL);
}								/* ButtonAbortUpdate */

/*
 * polling for network events
 */
static void
PollNetwork (void *par)
{
	XBNetworkEvent msg;
	unsigned int id;
	unsigned int cnt = 0;
	static char buf[25];
	/* check network events */
	while (XBNW_None != (msg = Network_GetEvent (&id))) {
		switch (msg) {
		case XBNW_RightPlayerConfig:
		case XBNW_LeftPlayerConfig:
		case XBNW_Joy1PlayerConfig:
		case XBNW_Joy2PlayerConfig:
			cnt++;
			break;
		case XBNW_Disconnected:
			MenuAddLabel (5 * CELL_W / 2, MENU_ROW (5), 10 * CELL_W, N_("Update finished!"));
			SavePlayerCentral ();
			break;
		case XBNW_Error:
			sprintf (buf, "network error = %i", id);
			MenuAddLabel (5 * CELL_W / 2, MENU_ROW (5), 10 * CELL_W, buf);
			/* restoring old data */
			LoadPlayerCentral (XBFalse);
			break;
		default:
			/* anything else */
			fprintf (stderr, "unexpected network event %u, %u\n", msg, id);
			break;
		}
	}
	/* update player config count */
	if (cnt > 0) {
		Dbg_Out ("%u player configs received\n", cnt);
		received = User_Received ();
	}
}								/* PollNetwork */

/*
 * create the update menu
 */
static XBBool
CreateCentralUpdateMenu (void *par)
{
	CFGCentralSetup central;
	/* clear menu */
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Update statistics"));
	/* connect to central */
	RetrieveCentralSetup (&central);
	if (User_Connect (&central)) {
		/* request update from central */
		User_RequestUpdate ();
		/* build menu items */
		received = 0;
		MenuAddLabel (5 * CELL_W / 2, MENU_ROW (2), 10 * CELL_W, N_("Connected..."));
		MenuAddLabel (5 * CELL_W / 2, MENU_ROW (3), 10 * CELL_W, N_("Receiving players..."));
		MenuAddIntTag (12 * CELL_W / 2, MENU_ROW (4), 6 * CELL_W / 2, &received);
		MenuSetAbort (MenuAddHButton
					  (8 * CELL_W / 2, MENU_BOTTOM, 7 * CELL_W, N_("Back"), ButtonAbortUpdate, par));
		/* poll function */
		MenuAddCyclic (PollNetwork, par);
	}
	else {
		MenuAddLabel (5 * CELL_W / 2, MENU_ROW (2), 10 * CELL_W, N_("Unable to connect to central."));
		MenuSetAbort (MenuAddHButton
					  (8 * CELL_W / 2, MENU_BOTTOM, 7 * CELL_W, N_("Damn"), CreateExtrasMenu, par));
	}
	/* */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateCentralUpdateMenu */

/***************
 * extras menu *
 ***************/

/*
 * exit button
 */
static XBBool
ButtonExitExtras (void *par)
{
	FinishPlayerCentral ();
	return CreateMainMenu (NULL);
}								/* ButtonExitExtras */

/*
 * create the extras menu
 */
XBBool
CreateExtrasMenu (void *par)
{
	MenuClear ();

	// LoadDemoConfig();
	LoadPlayerCentral (XBFalse);
	centralDat = XBFalse;
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Extras Menu"));
	/* Buttons */
	(void)MenuAddHButton (MENU_LEFT, MENU_ROW (0), MENU_WIDTH, N_("Player Statistics"),
						  CreatePlayerStatMenu, NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_ROW (1), MENU_WIDTH, N_("Level Statistics"),
						  CreateLevelStatMenu, NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_ROW (2), MENU_WIDTH, N_("Demo Playback"), CreateDemoMenu,
						  NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_ROW (3), MENU_WIDTH, N_("Edit Levels"), CreateEditMenu, NULL);	// XBCC
	(void)MenuAddHButton (MENU_LEFT, MENU_ROW (5), MENU_WIDTH, N_("Central Statistics"), CreateCentralStatMenu, NULL);	// XBCC
	(void)MenuAddHButton (MENU_LEFT, MENU_ROW (6), MENU_WIDTH, N_("Update Central Statistics"), CreateCentralUpdateMenu, NULL);	// XBCC
	/* leave menu */
	// MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, N_("Main Menu"), CreateMainMenu, NULL) );
	MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, N_("Main Menu"), ButtonExitExtras, par));	// XBCC close central DB
	/* escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateExtrasMenu */

/*
 * end of file menu_extras.c
 */
