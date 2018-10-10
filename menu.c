/*
 * file menu.c - user interface for game setup
 *
 * $Id: menu.c,v 1.8 2004/11/27 02:34:52 iskywalker Exp $
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
#include "menu.h"

#include "atom.h"
#include "geom.h"
#include "intro.h"
#include "menu_control.h"
#include "menu_game.h"
#include "menu_level.h"
#include "menu_player.h"
#include "menu_extras.h"
#include "menu_layout.h"
#include "snd.h"

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
static BMSpriteAnimation player_anime[ANIME_WAVE+2*ANIME_HOLD] = {
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteWinner3,  SpriteWinner2, 
    SpriteWinner,   SpriteWinner,   SpriteWinner, 
    SpriteWinner2,  SpriteWinner3,  SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
    SpriteStopDown, SpriteStopDown, SpriteStopDown, 
};
/* combo entries for sound mode */
static XBComboEntryList soundModeList[] = {
  { "None",    XBSM_None,    NULL, ATOM_INVALID, },
  { "Beep",    XBSM_Beep,    NULL, ATOM_INVALID, },
  { "Waveout", XBSM_Waveout, NULL, ATOM_INVALID, },
  { NULL,      0,            NULL, ATOM_INVALID, },
};

static XBComboEntryList centralList[] = {
  { "xblast.debian.net (default)",  0, NULL, ATOM_INVALID, },
  { "Koen's Central",  1, NULL, ATOM_INVALID, },
  { NULL, 0, NULL, ATOM_INVALID, },
};
static XBComboEntryList centralList1[] = {
  { "xblast.debian.net",  0, NULL, ATOM_INVALID, },
  { "129.125.51.134",  1, NULL, ATOM_INVALID, },
  { NULL, 0, NULL, ATOM_INVALID, },
};
static XBBool
CreateDefaultCentralMenu (void* par);
/*
 * start local game
 */
static XBBool
ButtonStartLocal (void *par)
{
  gameType = atomLocal;
  return CreateLocalGameMenu (&gameType);
} /* ButtonStartLocal */

/*
 * start central server XBCC
 */
static XBBool
ButtonStartCentral (void *par)
{
  gameType = atomCentral;
  return CreateCentralGameMenu (&gameType);
} /* ButtonStartLocal */

/*
 * create a network game
 */
static XBBool
ButtonCreateNet (void *par)
{
  gameType = atomServer;
  return CreateStartNetGameMenu (&gameType);
} /* ButtonStartLocal */

/*
 * join a network game
 */
static XBBool
ButtonJoinNet (void *par)
{
  gameType = atomClient;
  return CreateJoinNetGameMenu (&gameType);
} /* ButtonJoinNet */

/*
 * quit xblast menu
 */
static XBBool
ButtonMainQuit (void *par)
{
  MenuClear ();
  return XBFalse;
} /* ButtonMenuQuit */

/*
 * create the main menu
 */
XBBool
CreateMainMenu (void *par)
{
  int     i;

  gameType = ATOM_INVALID;
  /* setup menu */
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "XBlast Main Menu");
  /* Buttons */
  (void) MenuAddHButton (MENU_LEFT, MENU_TOP,            MENU_WIDTH, "Start Local Game",    ButtonStartLocal,    NULL);
  (void) MenuAddHButton (MENU_LEFT, MENU_TOP + 1*CELL_H, MENU_WIDTH, "Create Network Game", ButtonCreateNet,     NULL);
  (void) MenuAddHButton (MENU_LEFT, MENU_TOP + 2*CELL_H, MENU_WIDTH, "Join Network Game",   ButtonJoinNet,       NULL);
  (void) MenuAddHButton (MENU_LEFT, MENU_TOP + 3*CELL_H, MENU_WIDTH, "Options",             CreateOptionsMenu,   NULL);
  (void) MenuAddHButton (MENU_LEFT, MENU_TOP + 4*CELL_H, MENU_WIDTH, "Extras",              CreateExtrasMenu,    NULL);
  (void) MenuAddHButton (MENU_LEFT, MENU_TOP + 6*CELL_H, MENU_WIDTH, "Start Central",       ButtonStartCentral,  NULL);
  /* exit game */

MenuAddLabel1 (10, 79, TITLE_WIDTH+60, "to talk to others go to http://IRC.xblast-center.com/");
MenuAddLabel1 (10,63 , TITLE_WIDTH+60, "visit http://xblast.sf.net/");
  MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM,  MENU_WIDTH, "Quit",                ButtonMainQuit,      NULL) );
  /* audience */
  for (i = 0; i < NUM_DEFAULT_PLAYERS; i ++) {
    audience_ptr[i] = DefaultPlayerGraphics (i % NUM_DEFAULT_PLAYERS);
    MenuAddPlayer (PLAYER_LEFT (i, NUM_DEFAULT_PLAYERS), PLAYER_TOP, PLAYER_WIDTH, i, audience_ptr +i, 
		   ANIME_LENGTH, player_anime + ANIME_HOLD - 2*i -2);
  }
  /* return and escape */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateMainMenu */

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
} /* ButtonSaveSoundSetup */

/*
 * save changed central setup
 */
static XBBool
ButtonSaveCentral (void *par)
{
  assert (NULL != par);
  /* store setup */
  centralSetup.name  = (0 != centralname[0])  ? centralname  : NULL;
  StoreCentralSetup (par);
  /* back to options */
  return CreateOptionsMenu (NULL);
} /* ButtonSaveSoundSetup */

/*
 * save changed central setup
 */
static XBBool
ButtonSaveDefaultCentral (void *par)
{
  assert (NULL != par);
  /* store setup */
  // centralSetup.name  = (0 != centralname[0])  ? centralname  : NULL;
  centralSetup.name=centralList1[defaultCentral].text;
  centralSetup.port=16160;
  StoreCentralSetup (&centralSetup);
  /* back to options */
  return CreateOptionsMenu (NULL);
} /* ButtonSaveSoundSetup */

/*
 * menu for sound setup 
 */ 
static XBBool
CreateSoundMenu (void* par)
{
  static CFGSoundSetup soundSetup;

  RetrieveSoundSetup (&soundSetup);
  /* --- */
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Sound Setup");
  /* Buttons */				   
  MenuAddComboInt  (5*CELL_W, MENU_ROW(0), 5*CELL_W, "Mode:",   3*CELL_W, (int *)&soundSetup.mode, soundModeList);
  MenuAddComboBool (5*CELL_W, MENU_ROW(1), 5*CELL_W, "Stereo:", 3*CELL_W, &soundSetup.stereo);
  /* AbsInt begin */
  MenuAddComboBool (5*CELL_W, MENU_ROW(2), 5*CELL_W, "Beep:",   3*CELL_W, &soundSetup.beep);
  /* AbsInt end */
  /* leave menu */
  /* ok and cancel */
  MenuSetAbort   (MenuAddHButton ( 5 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreateOptionsMenu, NULL) );
  MenuSetDefault (MenuAddHButton (17 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Ok",     ButtonSaveSound,   &soundSetup) );
  /* escape */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateSoundMenu */

/*
 * menu for central setup XBCC 
 */ 
static XBBool
CreateCentralSetupMenu (void* par)
{
  RetrieveCentralSetup (&centralSetup);
  /* --- */
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Central Setup");
  /* Buttons */				   
  if (NULL != centralSetup.name) {
    strncpy (centralname, centralSetup.name, MSG_LENGTH);
  }
  MenuAddString (DLG_LEFT, MSG_TOP,  DLG_WIDTH, "Hostname:",    4*CELL_W, centralname,   MSG_LENGTH);
  MenuAddInteger (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, "TCP-Port:", 4*CELL_W, &centralSetup.port, 4096, 65535);

  /* leave menu */
  /* ok and cancel */
  MenuSetAbort   (MenuAddHButton ( 3 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreateOptionsMenu, NULL) );
  MenuSetDefault (MenuAddHButton (19 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Ok",     ButtonSaveCentral, &centralSetup) );
  MenuAddHButton ( 11 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Select Central", CreateDefaultCentralMenu, NULL);
  /* escape */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateSoundMenu */

/*
 * menu for central setup XBCC 
 */ 
static XBBool
CreateDefaultCentralMenu (void* par)
{
  RetrieveCentralSetup (&centralSetup);
  /* --- */
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Central Setup");
  /* Buttons */				   
  if (NULL != centralSetup.name) {
    strncpy (centralname, centralSetup.name, MSG_LENGTH);
  }
 MenuAddCombo (DLG_LEFT,MSG_TOP,DLG_WIDTH+CELL_W,"Hostname List:" , 5*CELL_W,
		&defaultCentral,NULL,NULL,
		(void *) centralList);
  MenuAddInteger (DLG_LEFT, MENU_ROW (1), DLG_WIDTH, "TCP-Port:", 4*CELL_W, &centralSetup.port, 4096, 65535);
  // MenuAddString (DLG_LEFT, MSG_TOP,  DLG_WIDTH, "Hostname:",    4*CELL_W, centralname,   MSG_LENGTH);
 

  /* leave menu */
  /* ok and cancel */
  MenuSetAbort   (MenuAddHButton ( 3 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreateOptionsMenu, NULL) );
  MenuAddHButton ( 11 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Enter Manually",CreateCentralSetupMenu , NULL) ;
  MenuSetDefault (MenuAddHButton (19 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Ok",     ButtonSaveDefaultCentral, &centralSetup) );
  /* escape */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateSoundMenu */


/*
 * create the options menu
 */
XBBool
CreateOptionsMenu (void *par)
{
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "XBlast Options Menu");
  /* Buttons */				   
  MenuAddHButton (MENU_LEFT, MENU_ROW(0), MENU_WIDTH, "Players",  CreatePlayerOptionsMenu, NULL);
  MenuAddHButton (MENU_LEFT, MENU_ROW(1), MENU_WIDTH, "Controls", CreateConfigControlMenu, NULL);
  MenuAddHButton (MENU_LEFT, MENU_ROW(2), MENU_WIDTH, "Sound",    CreateSoundMenu,         NULL);
  MenuAddHButton (MENU_LEFT, MENU_ROW(3), MENU_WIDTH, "Central",  CreateDefaultCentralMenu,  NULL); // XBCC
  /* leave menu */
  MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, "Main Menu", CreateMainMenu, NULL) );
  /* escape */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateOptionsMenu */

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
  setAutoCentral(autoCentral);
  if(autoCentral) {
    (void) ButtonStartCentral (NULL);
  } else {
     (void) CreateMainMenu (NULL);
  }
  /* run menu */
  result = MenuEventLoop ();
  /* fade out */
  if (result) {
    DoFade (XBFM_WHITE_OUT, PIXH + SCOREH);
  } else {
    DoFade (XBFM_BLACK_OUT, PIXH + SCOREH);
  }
  /* that´s all */
  return result ?  GetHostType () : XBPH_None;
} /* DoMenu */

/*
 * end of file menu.c
 */
