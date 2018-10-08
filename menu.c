/*
 * file menu.c - user interface for game setup
 *
 * $Id: menu.c,v 1.13 2006/02/10 13:22:11 fzago Exp $
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
 * local constants
 */
#define ANIME_WAVE    9
#define ANIME_HOLD   12
#define ANIME_LENGTH (ANIME_WAVE+ANIME_HOLD)

static CFGCentralSetup centralSetup;

#define MSG_LENGTH        80
static char centralname[MSG_LENGTH];

/*
 * local variables
 */
static int defaultCentral;
static XBAtom gameType;
static const CFGPlayerGraphics *audience_ptr[MAX_PLAYER];
static BMSpriteAnimation player_anime[ANIME_WAVE + 2 * ANIME_HOLD] = {
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteWinner3, SpriteWinner2,
	SpriteWinner, SpriteWinner, SpriteWinner,
	SpriteWinner2, SpriteWinner3, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown,
};

/* combo entries for sound mode */
static XBComboEntryList soundModeList[] = {
	{N_("None"), XBSM_None, NULL, ATOM_INVALID,},
	{N_("Beep"), XBSM_Beep, NULL, ATOM_INVALID,},
	{N_("Waveout"), XBSM_Waveout, NULL, ATOM_INVALID,},
	{NULL, 0, NULL, ATOM_INVALID,},
};

/* combo entries for video mode */
static XBComboEntryList videoModeList[] = {
	{N_("No"), XBVM_Windowed, NULL, ATOM_INVALID,},
	{N_("Yes"), XBVM_Full, NULL, ATOM_INVALID,},
	{NULL, 0, NULL, ATOM_INVALID,},
};

static XBComboEntryList centralList[] = {
	{"xblast.debian.net (default)", 0, NULL, ATOM_INVALID,},
	{"Koen's Central", 1, NULL, ATOM_INVALID,},
	{NULL, 0, NULL, ATOM_INVALID,},
};
static XBComboEntryList centralList1[] = {
	{"xblast.debian.net", 0, NULL, ATOM_INVALID,},
	{"129.125.51.134", 1, NULL, ATOM_INVALID,},
	{NULL, 0, NULL, ATOM_INVALID,},
};
static XBBool CreateDefaultCentralMenu (void *par);
/*
 * start local game
 */
static XBBool
ButtonStartLocal (void *par)
{
	gameType = atomLocal;
	return CreateLocalGameMenu (&gameType);
}								/* ButtonStartLocal */

/*
 * start central server XBCC
 */
static XBBool
ButtonStartCentral (void *par)
{
	gameType = atomCentral;
	return CreateCentralGameMenu (&gameType);
}								/* ButtonStartLocal */

/*
 * create a network game
 */
static XBBool
ButtonCreateNet (void *par)
{
	gameType = atomServer;
	return CreateStartNetGameMenu (&gameType);
}								/* ButtonStartLocal */

/*
 * join a network game
 */
static XBBool
ButtonJoinNet (void *par)
{
	gameType = atomClient;
	return CreateJoinNetGameMenu (&gameType);
}								/* ButtonJoinNet */

/*
 * quit xblast menu
 */
static XBBool
ButtonMainQuit (void *par)
{
	MenuClear ();
	return XBFalse;
}								/* ButtonMenuQuit */

/*
 * create the main menu
 */
XBBool
CreateMainMenu (void *par)
{
	int i;

	gameType = ATOM_INVALID;
	/* setup menu */
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("XBlast Main Menu"));
	/* Buttons */
	(void)MenuAddHButton (MENU_LEFT, MENU_TOP, MENU_WIDTH, N_("Start Local Game"), ButtonStartLocal,
						  NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_TOP + 1 * CELL_H, MENU_WIDTH, N_("Create Network Game"),
						  ButtonCreateNet, NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_TOP + 2 * CELL_H, MENU_WIDTH, N_("Join Network Game"),
						  ButtonJoinNet, NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_TOP + 3 * CELL_H, MENU_WIDTH, N_("Options"),
						  CreateOptionsMenu, NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_TOP + 4 * CELL_H, MENU_WIDTH, N_("Extras"), CreateExtrasMenu,
						  NULL);
	(void)MenuAddHButton (MENU_LEFT, MENU_TOP + 6 * CELL_H, MENU_WIDTH, N_("Start Central"),
						  ButtonStartCentral, NULL);
	/* exit game */

	MenuAddLabel1 (10, 79, TITLE_WIDTH + 60, N_("to talk to others go to http://xblast.sf.net/irc/"));
	MenuAddLabel1 (10, 63, TITLE_WIDTH + 60, N_("visit http://xblast.sf.net/"));
	MenuSetAbort (MenuAddHButton
				  (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, N_("Quit"), ButtonMainQuit, NULL));
	/* audience */
	for (i = 0; i < NUM_DEFAULT_PLAYERS; i++) {
		audience_ptr[i] = DefaultPlayerGraphics (i % NUM_DEFAULT_PLAYERS);
		MenuAddPlayer (PLAYER_LEFT (i, NUM_DEFAULT_PLAYERS), PLAYER_TOP, PLAYER_WIDTH, i,
					   audience_ptr + i, ANIME_LENGTH, player_anime + ANIME_HOLD - 2 * i - 2);
	}
	/* return and escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateMainMenu */

/*
 * save changed sound setup
 */
static XBBool
ButtonSaveSound (void *par)
{
	assert (NULL != par);
	/* store setup */
	StoreSoundSetup (par);
	/* reactivate sound setup */
	SND_Finish ();
	RetrieveSoundSetup (par);
	SND_Init (par);
	/* back to options */
	return CreateOptionsMenu (NULL);
}								/* ButtonSaveSoundSetup */

/*
 * save changed central setup
 */
static XBBool
ButtonSaveCentral (void *par)
{
	assert (NULL != par);
	/* store setup */
	centralSetup.name = (0 != centralname[0]) ? centralname : NULL;
	StoreCentralSetup (par);
	/* back to options */
	return CreateOptionsMenu (NULL);
}								/* ButtonSaveSoundSetup */

/*
 * save changed central setup
 */
static XBBool
ButtonSaveDefaultCentral (void *par)
{
	assert (NULL != par);
	/* store setup */
	// centralSetup.name  = (0 != centralname[0])  ? centralname  : NULL;
	centralSetup.name = centralList1[defaultCentral].text;
	centralSetup.port = 16160;
	StoreCentralSetup (&centralSetup);
	/* back to options */
	return CreateOptionsMenu (NULL);
}								/* ButtonSaveSoundSetup */

/*
 * menu for sound setup 
 */
static XBBool
CreateSoundMenu (void *par)
{
	static CFGSoundSetup soundSetup;

	RetrieveSoundSetup (&soundSetup);
	/* --- */
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Sound Setup"));
	/* Buttons */
	MenuAddComboInt (5 * CELL_W, MENU_ROW (0), 5 * CELL_W, N_("Mode:"), 3 * CELL_W,
					 (int *)&soundSetup.mode, soundModeList);
	MenuAddComboBool (5 * CELL_W, MENU_ROW (1), 5 * CELL_W, N_("Stereo:"), 3 * CELL_W,
					  &soundSetup.stereo);
    /* AbsInt begin */
    MenuAddComboBool (5*CELL_W, MENU_ROW(2), 5*CELL_W, "Beep:",   3*CELL_W, &soundSetup.beep);
    /* AbsInt end */
	/* leave menu */
	/* ok and cancel */
	MenuSetAbort (MenuAddHButton
				  (5 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Cancel"), CreateOptionsMenu, NULL));
	MenuSetDefault (MenuAddHButton
					(17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), ButtonSaveSound, &soundSetup));
	/* escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateSoundMenu */

#ifdef SDL

/*
 * save changed video setup
 */
static XBBool
ButtonSaveVideo (void *par)
{
	assert (NULL != par);

	StoreVideoSetup (par);

	SetupVideo (par);

	/* back to options */
	return CreateOptionsMenu (NULL);
}

/*
 * menu for video setup
 */
static XBBool
CreateVideoMenu (void *par)
{
	static CFGVideoSetup videoSetup;

	RetrieveVideoSetup (&videoSetup);

	MenuClear ();
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Screen Setup"));
	MenuAddComboInt (5 * CELL_W, MENU_ROW (0), 5 * CELL_W, N_("Fullscreen:"), 3 * CELL_W,
					 (int *)&videoSetup.mode, videoModeList);
	MenuSetAbort (MenuAddHButton
				  (5 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Cancel"), CreateOptionsMenu, NULL));
	MenuSetDefault (MenuAddHButton
					(17 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), ButtonSaveVideo, &videoSetup));

	/* escape */
	MenuSetLinks ();

	return XBFalse;
}

#endif

/*
 * menu for central setup XBCC 
 */
static XBBool
CreateCentralSetupMenu (void *par)
{
	RetrieveCentralSetup (&centralSetup);
	/* --- */
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Central Setup"));
	/* Buttons */
	if (NULL != centralSetup.name) {
		strncpy (centralname, centralSetup.name, MSG_LENGTH);
	}
	MenuAddString (DLG_LEFT, MSG_TOP, DLG_WIDTH, N_("Hostname:"), 4 * CELL_W, centralname, MSG_LENGTH);
	MenuAddInteger (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, N_("TCP-Port:"), 4 * CELL_W, &centralSetup.port,
					4096, 65535);

	/* leave menu */
	/* ok and cancel */
	MenuSetAbort (MenuAddHButton
				  (3 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Cancel"), CreateOptionsMenu, NULL));
	MenuSetDefault (MenuAddHButton
					(19 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), ButtonSaveCentral,
					 &centralSetup));
	MenuAddHButton (11 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Select Central"),
					CreateDefaultCentralMenu, NULL);
	/* escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateSoundMenu */

/*
 * menu for central setup XBCC 
 */
static XBBool
CreateDefaultCentralMenu (void *par)
{
	RetrieveCentralSetup (&centralSetup);
	/* --- */
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("Central Setup"));
	/* Buttons */
	if (NULL != centralSetup.name) {
		strncpy (centralname, centralSetup.name, MSG_LENGTH);
	}
	MenuAddCombo (DLG_LEFT, MSG_TOP, DLG_WIDTH + CELL_W, N_("Hostname List:"), 5 * CELL_W,
				  &defaultCentral, NULL, NULL, (void *)centralList);
	MenuAddInteger (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, N_("TCP-Port:"), 4 * CELL_W, &centralSetup.port,
					4096, 65535);
	// MenuAddString (DLG_LEFT, MSG_TOP,  DLG_WIDTH, N_("Hostname:"),    4*CELL_W, centralname,   MSG_LENGTH);

	/* leave menu */
	/* ok and cancel */
	MenuSetAbort (MenuAddHButton
				  (3 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Cancel"), CreateOptionsMenu, NULL));
	MenuAddHButton (11 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Enter Manually"),
					CreateCentralSetupMenu, NULL);
	MenuSetDefault (MenuAddHButton
					(19 * CELL_W / 2, MENU_BOTTOM, 4 * CELL_W, N_("Ok"), ButtonSaveDefaultCentral,
					 &centralSetup));
	/* escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateSoundMenu */

/*
 * create the options menu
 */
XBBool
CreateOptionsMenu (void *par)
{
	MenuClear ();
	/* Title */
	MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, N_("XBlast Options Menu"));
	/* Buttons */
	MenuAddHButton (MENU_LEFT, MENU_ROW (0), MENU_WIDTH, N_("Players"), CreatePlayerOptionsMenu, NULL);
	MenuAddHButton (MENU_LEFT, MENU_ROW (1), MENU_WIDTH, N_("Controls"), CreateConfigControlMenu, NULL);
	MenuAddHButton (MENU_LEFT, MENU_ROW (2), MENU_WIDTH, N_("Sound"), CreateSoundMenu, NULL);
#ifdef SDL
	MenuAddHButton (MENU_LEFT, MENU_ROW (3), MENU_WIDTH, N_("Video"), CreateVideoMenu, NULL);
#endif
	MenuAddHButton (MENU_LEFT, MENU_ROW (4), MENU_WIDTH, N_("Central"), CreateDefaultCentralMenu, NULL);	// XBCC
	/* leave menu */
	MenuSetAbort (MenuAddHButton
				  (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, N_("Main Menu"), CreateMainMenu, NULL));
	/* escape */
	MenuSetLinks ();
	/* that's all */
	return XBFalse;
}								/* CreateOptionsMenu */

/*
 *
 */
XBPlayerHost
//DoMenu (void)
DoMenu (XBBool autoCentral)
{
	int result;

	GUI_ClearPixmap ();
	/* create main menu */
	setAutoCentral (autoCentral);
	if (autoCentral) {
		(void)ButtonStartCentral (NULL);
	}
	else {
		(void)CreateMainMenu (NULL);
	}
	/* run menu */
	result = MenuEventLoop ();
	/* fade out */
	if (result) {
		DoFade (XBFM_WHITE_OUT, PIXH + SCOREH);
	}
	else {
		DoFade (XBFM_BLACK_OUT, PIXH + SCOREH);
	}
	/* that´s all */
	return result ? GetHostType () : XBPH_None;
}								/* DoMenu */

/*
 * end of file menu.c
 */
