/*
 * file menu_game.c - menus for setting up game parameters
 *
 * $Id: menu_game.c,v 1.10 2005/01/23 14:29:12 lodott Exp $
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
#include "menu_game.h"

#include "atom.h"
#include "menu.h"
#include "menu_layout.h"
#include "menu_level.h"
#include "menu_network.h"
#include "random.h"

/*
 * local constants
 */
#define ANIME_LENGTH 4

/*
 * local variables
 */
static XBPlayerHost hostType;
static XBMenuLevelPar mlp;
/* game config */
static CFGGameSetup gameSetup  = {
  XBFalse,   /* respect the number of lives dictated by level */
  1,         /* number of lives */
  9,         /* number victories */
  30,        /* frame rate */
  XBTrue,    /* select all levels */
  XBTrue,    /* random level order */
  XBTrue,    /* random player positions */
  0,         /* level order */
  15,        /* info wait time */
  XBFalse,   /* demo on DEMOFIX*/
  XBTM_None, /* team mode */
  XBTrue,    /* XBCC not rated */
  XBFalse,   /* bot */
  2,         /* number of rec lives (valid if 1. XBTrue) */
};
static MENU_ID idButtonCentral;
static MENU_ID idButtonStart;
static MENU_ID idButtonLevels;
/* player controls */
static XBAtom _gamePlayers[NUM_LOCAL_PLAYER];
static const XBPlayerControl _gameControl[NUM_LOCAL_PLAYER] = {
  XBPC_RightKeyboard,
  XBPC_LeftKeyboard,
  XBPC_Joystick1,
  XBPC_Joystick2,
  XBPC_Joystick3,
  XBPC_Joystick4,
};
static const char *playerControl[NUM_LOCAL_PLAYER] = {
  "Right Keyboard:",
  "Left Keyboard:",
  "Joystick 1:",
  "Joystick 2:",
  "Joystick 3:",
  "Joystick 4:",
};
/* local player selection */
static XBComboEntryList *playerList = NULL;
/* number of lives */
static XBComboEntryList numLivesList[] = {
  { "1",  1, NULL, ATOM_INVALID, },
  { "2",  2, NULL, ATOM_INVALID, },
  { "3",  3, NULL, ATOM_INVALID, },
  { NULL, 0, NULL, ATOM_INVALID, },
};
static XBComboEntryList numWinsList[] = {
  { "1",  1, NULL},
  { "2",  2, NULL},
  { "3",  3, NULL},
  { "4",  4, NULL},
  { "5",  5, NULL},
  { "6",  6, NULL},
  { "7",  7, NULL},
  { "8",  8, NULL},
  { "9",  9, NULL},
  { NULL, 0, NULL},
};
static XBComboEntryList waitInfoList[] = { // LRF
  { "1",   1 , NULL, ATOM_INVALID, },
  { "2",   2 , NULL, ATOM_INVALID, },
  { "3",   3 , NULL, ATOM_INVALID, },
  { "4",   4 , NULL, ATOM_INVALID, },
  { "5",   5 , NULL, ATOM_INVALID, },
  { "7",   7 , NULL, ATOM_INVALID, },
  { "10",  10, NULL, ATOM_INVALID, },
  { "15",  15, NULL, ATOM_INVALID, },
  { "20",  20, NULL, ATOM_INVALID, },
  { "30",  30, NULL, ATOM_INVALID, },
  { NULL,   0, NULL, ATOM_INVALID, },
};
static XBComboEntryList levelOrderList[] = { // LRF
  { "Alfabet",   1 , NULL, ATOM_INVALID, },
  { "Random" ,   2 , NULL, ATOM_INVALID, },
  { "Time"   ,   3 , NULL, ATOM_INVALID, },
  { NULL     ,   0 , NULL, ATOM_INVALID, },
};
static XBComboEntryList musicList[] = { // LRF
  { "None",    0, NULL, ATOM_INVALID, },
  { "Song1",   SND_SNG1 , NULL, ATOM_INVALID, },
  { "Song2",   SND_SNG2 , NULL, ATOM_INVALID, },
  { "Song3",   SND_SNG3 , NULL, ATOM_INVALID, },
  { "Song4",   SND_SNG4 , NULL, ATOM_INVALID, },
  { "Song5",   SND_SNG5 , NULL, ATOM_INVALID, },
  { "Song6",   SND_SNG6 , NULL, ATOM_INVALID, },
  { NULL   ,   0 , NULL, ATOM_INVALID, },
};
static XBComboEntryList frameRateList[] = {
  { "10",  10, NULL, ATOM_INVALID, },
  { "12",  12, NULL, ATOM_INVALID, },
  { "14",  14, NULL, ATOM_INVALID, },
  { "16",  16, NULL, ATOM_INVALID, },
  { "18",  18, NULL, ATOM_INVALID, },
  { "20",  20, NULL, ATOM_INVALID, },
  { "22",  22, NULL, ATOM_INVALID, },
  { "24",  24, NULL, ATOM_INVALID, },
  { "26",  26, NULL, ATOM_INVALID, },
  { "28",  28, NULL, ATOM_INVALID, },
  { "30",  30, NULL, ATOM_INVALID, },
  { "32",  32, NULL, ATOM_INVALID, },
  { "34",  35, NULL, ATOM_INVALID, },
  { "36",  36, NULL, ATOM_INVALID, },
#ifdef DEBUG
  { "40",  40, NULL, ATOM_INVALID, },
  { "45",  45, NULL, ATOM_INVALID, },
  { "50",  50, NULL, ATOM_INVALID, },
  { "60",  60, NULL, ATOM_INVALID, },
  { "72",  72, NULL, ATOM_INVALID, },
  { "85",  85, NULL, ATOM_INVALID, },
  { "100",100, NULL, ATOM_INVALID, },
#endif
  { NULL,   0, NULL, ATOM_INVALID, },
};
#ifdef DEBUG_TEAM
static XBComboEntryList teamModeList[] = {
  { "None",    XBTM_None,        NULL},
  { "Team",    XBTM_Team,        NULL},
  { NULL, 0, NULL},
};
#endif
static BMSpriteAnimation playerAnime[ANIME_LENGTH] = {
    SpriteStopDown, SpriteWinner3,  SpriteWinner2, SpriteWinner,
};

XBBool autoCentral=XBFalse;

/*
 * delete player confiug array for combo box
 */
static void
DeletePlayerConfigList (XBComboEntryList * list)
{
  int i;

  assert (list != NULL);
  i = 0;
  while (list[i].text != NULL) {
    if (NULL != list[i].data) {
      free (list[i].data);
    }
    i ++;
  }
  free (list);
} /* DeletePlayerConfigList */

/*
 * Convert player config array to combo list
 */
static XBComboEntryList *
CreatePlayerConfigList (void)
{
  XBComboEntryList *list;
  int i, num;
  CFGPlayerGraphics *gfx;

  num = GetNumPlayerConfigs (CT_Local);
  /* alloc list */
  list = calloc (2 + num, sizeof (XBComboEntryList));
  assert (list != NULL);
  /* fill list */
  for (i = 0; i < num; i ++) {
    XBAtom atom  = GetPlayerAtom (CT_Local, i);
    list[i].atom = atom;
    list[i].text = GetPlayerName (CT_Local, atom);
    gfx = calloc (1, sizeof (CFGPlayerGraphics) );
    if (RetrievePlayerGraphics (CT_Local, atom, COLOR_INVALID, gfx) ) {
      list[i].data = gfx;
    } else {
      free (gfx);
    }
  }
  list[i].atom = ATOM_INVALID;
  list[i].text = "none";
  list[i].data = NULL;
  return list;
} /* CreatePlayerConfigList */

/*
 *
 */
static XBBool
ButtonMainMenu (void *par)
{
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* call main menu */
  return CreateMainMenu (par);
} /* ButtonMainMenu */

/*
 *
 */
static XBBool
ButtonStartLocalGame (void *par)
{
  /* clean up */
  MenuClear ();
  return XBTrue;
}  /* set game config */

#ifdef DEBUG_TEAM

/*
 *
 */
static void
AssignRandomTeams (XBTeamMode mode, int num, XBPlayerTeam *team)
{
  int          i, j;
  XBPlayerTeam swap;
  assert (NULL != team);
  GUI_ErrorMessage ("in assign 0");
  switch (mode) {
  case XBTM_RandomTwo:
    GUI_ErrorMessage ("in assign 1");
    for (i = 0; i < num; i ++) {
      team[i] = XBPT_None + 1 + (i % 2);
    }
    break;
  case XBTM_RandomThree:
    for (i = 0; i < num; i ++) {
      team[i] = XBPT_None + 1 + (i % 3);
    }
    break;
  default:
    return;
  }
  /* shuffle teams */
  GUI_ErrorMessage ("in assign 3");
  for (i = 0; i < num; i ++) {
    GUI_ErrorMessage ("in assign 3.5");
    j = OtherRandomNumber (i+1);
    GUI_ErrorMessage ("in assign 4");
    if (j >= i) {
      j ++;
    }
    GUI_ErrorMessage ("in assign 5");
    swap    = team[i];
    team[i] = team[j];
    team[j] = swap;
  }
  for (i = 0; i < num; i ++) {
    GUI_ErrorMessage ("Player %i is of team %i", i, team[i]);
  }
} /* AssignRandomTeams */
#endif

/*
 * store game config
 */
static void
StoreGameConfig (XBAtom atom)
{
  int i, j;
  CFGGamePlayers gamePlayers;

  /* copy players */
  for (i = 0, j = 0; i < NUM_LOCAL_PLAYER; i ++) {
    if (ATOM_INVALID != _gamePlayers[i]) {
      gamePlayers.player[j]  = _gamePlayers[i];
      gamePlayers.control[j] = _gameControl[i];
      gamePlayers.host[j]    = XBPH_Local;
      gamePlayers.team[j]    = XBPT_None;
      j ++;
    }
  }
  gamePlayers.num = j;
#ifdef DEBUG_TEAM
  gameSetup.teamMode = XBTM_None;
  AssignRandomTeams (gameSetup.teamMode, gamePlayers.num, gamePlayers.team);
#else
  gameSetup.teamMode = XBTM_None;
#endif
  /* set and store game config */
  StoreGameSetup   (CT_Local, atom, &gameSetup);
  StoreGamePlayers (CT_Local, atom, &gamePlayers);
} /* StoreGameConfig */

/*
 *
 */
static XBBool
ButtonAllLevels (void *par)
{
  XBAtom *atom = par;

  /* level selection */
  gameSetup.allLevels = XBTrue;
  /* store it */
  assert (atom != NULL);
  StoreGameConfig (*atom);
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* leave menu */
  MenuClear ();
  return XBTrue;
} /* ButtonAllLevels */

/*
 *
 */
static XBBool
ButtonSelectLevels (void *par)
{
  XBAtom *atom = par;

  /* level selection */
  gameSetup.allLevels = XBFalse;
  /* store it */
  assert (atom != NULL);
  StoreGameConfig (*atom);
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* select levels ? */
  return CreateLevelMenu (&mlp);
} /* ButtonSelectLevels */

/*
 *
 */
static XBBool
ButtonStartClient (void *par)
{
  XBAtom *atom = par;

  /* level selection */
  gameSetup.allLevels = XBTrue;
  /* set and store game config */
  assert (atom != NULL);
  StoreGameConfig (*atom);
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* next menu screen */
  return CreateClientMenu (par);
} /* ButtonSelectLevels */

/*
 * join screen for central XBCC
 */
static XBBool
ButtonSearchCentral (void *par)
{
  XBAtom *atom = par;

  /* level selection */
  gameSetup.allLevels = XBTrue;
  /* set and store game config */
  assert (atom != NULL);
  StoreGameConfig (*atom);
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* next menu screen */
  //  return CreateCentralJoinMenu (par);
  return CreateSearchCentralMenu (par);
} /* ButtonSelectLevels */

/*
 *
 */
static XBBool
ButtonSearchLan (void *par)
{
  XBAtom *atom = par;

  /* level selection */
  gameSetup.allLevels = XBTrue;
  /* set and store game config */
  assert (atom != NULL);
  StoreGameConfig (*atom);
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* next menu screen */
  return CreateSearchLanMenu (par);
} /* ButtonSelectLevels */

/*
 *
 */
static XBBool
ButtonStartServer (void *par)
{
  XBAtom *atom = par;

  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* level selection */
  gameSetup.allLevels = XBTrue;
  /* set and store game config */
  assert (atom != NULL);
  StoreGameConfig (*atom);
  /* clear player list */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
    playerList = NULL;
  }
  /* goto next screen */
  return CreateServerMenu (par);
} /* ButtonSelectLevels */

/*
 * check if enough players are selected for playing
 */
static void
PollStartGame (void *par)
{
  int     i, j, numPlayers;
  XBBool  uniq, numOK, newState;
  XBAtom *atom = par;
  static  XBBool oldState = XBTrue;

  assert (atom != NULL);
  uniq       = XBTrue;
  numPlayers = 0;
  for (i = 0; i < NUM_LOCAL_PLAYER; i ++) {
    if (ATOM_INVALID != _gamePlayers[i]) {
      for (j = i + 1; j < NUM_LOCAL_PLAYER; j ++) {
	if (_gamePlayers[i] == _gamePlayers[j]) {
	  uniq = XBFalse;
	  break;
	}
      }
      numPlayers ++;
    }
    if (! uniq) {
      break;
    }
  }
  /* check number of players .. */
  if (*atom == atomLocal) {
    /* for local games */
    switch (gameSetup.teamMode) {
    case XBTM_Hunt:        numOK = (numPlayers >= 3);                    break;
    default:               numOK = (numPlayers >= 2);                    break;
    }
  } else {
    numOK = (numPlayers >= 1);
  }
  /* check current state */
  newState = uniq && numOK;
  if (newState != oldState) {
    MenuSetActive (idButtonStart,  newState);
    MenuSetActive (idButtonLevels, newState);
    oldState = newState;
  }
} /* PollStartGame */

/*
 * create items for gamePlayers
 */
static void
CreatePlayerItems (int x)
{
  int i, numJoy;
  static const CFGPlayerGraphics *localPlayerGraphics[NUM_LOCAL_PLAYER];

  /* load player configs */
  if (NULL != playerList) {
    DeletePlayerConfigList (playerList);
  }
  playerList = CreatePlayerConfigList ();
  numJoy = GUI_NumJoysticks ();
  for (i = 0; i < 2 + numJoy; i ++) {
    MenuAddCombo (x, MENU_ROW (i), 5*CELL_W, playerControl[i], 2*CELL_W, NULL,
		  (void **) &localPlayerGraphics[i], &_gamePlayers[i], playerList);
    MenuAddPlayer (PLAYER_LEFT(i,numJoy+2), PLAYER_TOP, PLAYER_WIDTH, i, &localPlayerGraphics[i], -ANIME_LENGTH, playerAnime);
  }
} /* CreatePlayerItems */

/*
 * create items for game parameters
 */
static void
CreateSetupItems (int x, CFGGameSetup *gameSetup, XBBool networkGame)
{
  assert (gameSetup != NULL);
  MenuAddComboInt  (x, MENU_ROW (0), 5*CELL_W, "Lives:",           2*CELL_W, &gameSetup->numLives,  numLivesList);
  MenuAddComboInt  (x, MENU_ROW (1), 5*CELL_W, "Victories:",       2*CELL_W, &gameSetup->numWins,   numWinsList);
  MenuAddComboInt  (x, MENU_ROW (2), 5*CELL_W, "Frame Rate:",      2*CELL_W, &gameSetup->frameRate, frameRateList);
  //MenuAddComboBool (x, MENU_ROW (3), 5*CELL_W, "Random Levels:",   2*CELL_W, &gameSetup->randomLevels);
  //MenuAddComboBool (x, MENU_ROW (4), 5*CELL_W, "Random Position:", 2*CELL_W, &gameSetup->randomPlayers);
  MenuAddComboInt  (x, MENU_ROW (3), 5*CELL_W, "Level Order:",     2*CELL_W, &gameSetup->levelOrder, levelOrderList);
  MenuAddComboInt  (x, MENU_ROW (4), 5*CELL_W, "Info screen:",     2*CELL_W, &gameSetup->infoTime, waitInfoList);
  MenuAddComboBool (x, MENU_ROW (5), 5*CELL_W, "Record Demo:",     2*CELL_W, &gameSetup->recordDemo);
  MenuAddComboInt (x-6*CELL_W, MENU_ROW (6), 5*CELL_W, "Music:",     2*CELL_W, (int *) &gameSetup->Music, musicList);
  if(networkGame) {
    MenuAddComboBool (x, MENU_ROW (6), 5*CELL_W, "Rated Game:",      2*CELL_W, &gameSetup->rated); // XBCC

  MenuAddComboBool (x-6*CELL_W, MENU_ROW (5), 5*CELL_W, "Recom. Lives:",     2*CELL_W, &gameSetup->ifRecLives);
  }
  else{
    MenuAddComboBool (x, MENU_ROW (6), 5*CELL_W, "Bot:", 2*CELL_W, &gameSetup->bot);
  MenuAddComboBool (x, MENU_ROW (7), 5*CELL_W, "Recom. Lives:",     2*CELL_W, &gameSetup->ifRecLives);
  }
} /* CreateSetupItems */

static void
RetrieveGameConfig (XBAtom atom)
{
  size_t i;
  size_t numJoy;
  CFGGamePlayers gamePlayers;

  numJoy = GUI_NumJoysticks ();
  /* load game configs */
  (void) RetrieveGameSetup   (CT_Local, atom, &gameSetup);
  (void) RetrieveGamePlayers (CT_Local, atom, &gamePlayers);
  /* copy to edit data */
  memset (&_gamePlayers, 0, sizeof (_gamePlayers));
  for (i = 0; i < gamePlayers.num; i ++) {
    switch (gamePlayers.control[i]) {
    case XBPC_RightKeyboard:
      _gamePlayers[0] = gamePlayers.player[i];
      break;
    case XBPC_LeftKeyboard:
      _gamePlayers[1] = gamePlayers.player[i];
      break;
    case XBPC_Joystick1:
      if (numJoy >= 1) {
	_gamePlayers[2] = gamePlayers.player[i];
      }
      break;
    case XBPC_Joystick2:
      if (numJoy >= 2) {
	_gamePlayers[3] = gamePlayers.player[i];
      }
      break;
    case XBPC_Joystick3:
      if (numJoy >= 3) {
	_gamePlayers[4] = gamePlayers.player[i];
      }
      break;
    case XBPC_Joystick4:
      if (numJoy >= 4) {
	_gamePlayers[5] = gamePlayers.player[i];
      }
      break;
    default:
      break;
    }
  }
} /* RetrieveGameConfig */

/*
 * create local game
 */
XBBool
CreateLocalGameMenu (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* --- */
  hostType = XBPH_Local;
  RetrieveGameConfig (*atom);
  /* setup level selection */
  mlp.textAbort   = "Back";
  mlp.funcAbort   = CreateLocalGameMenu;
  mlp.parAbort    = par;
  mlp.textDefault = "Start";
  mlp.funcDefault = ButtonStartLocalGame;
  mlp.parDefault  = NULL;
  /* build menu */
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Setup Local Game");
  /* create player selection */
  CreatePlayerItems (2*CELL_W);
  /* --- */
  CreateSetupItems (8*CELL_W, &gameSetup, XBFalse);
  /* Buttons */
  MenuSetAbort    (MenuAddHButton ( 3 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Back",          ButtonMainMenu,     NULL) );
  idButtonLevels = MenuAddHButton (11 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Select Levels", ButtonSelectLevels, par);
  idButtonStart  = MenuAddHButton (19 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Start",         ButtonAllLevels,    par);
  MenuSetDefault (idButtonStart);
  /* polling */
  MenuAddCyclic (PollStartGame, par);
  /* --- */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateLocalGameMenu */

/*
 * create network game
 */
XBBool
CreateStartNetGameMenu (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* --- */
  hostType = XBPH_Server;
  RetrieveGameConfig (*atom);
  /* setup level selection */
  mlp.textAbort   = "Back";
  mlp.funcAbort   = CreateStartNetGameMenu;
  mlp.parAbort    = par;
  mlp.textDefault = "Cont.";
  mlp.funcDefault = CreateServerMenu;
  mlp.parDefault  = par;
  /* build menu */
  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Start Network Game");
  /* create player selection */
  CreatePlayerItems (2*CELL_W);
  /* --- */
  CreateSetupItems (8*CELL_W, &gameSetup, XBTrue);
  /* Buttons */
  MenuSetAbort    (MenuAddHButton ( 3 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Back",          ButtonMainMenu,     NULL) );
  idButtonLevels = MenuAddHButton (11 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Select Levels", ButtonSelectLevels, par);
  idButtonStart  = MenuAddHButton (19 * CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Continue",      ButtonStartServer,  par);
  MenuSetDefault (idButtonStart);
  /* polling */
  MenuAddCyclic (PollStartGame, par);
  /* --- */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateStartNetGameMenu */

/*
 * create the options menu
 */
XBBool
CreateJoinNetGameMenu (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* --- */
  hostType = XBPH_None;
  RetrieveGameConfig (*atom);
  /* build menu */
  MenuClear ();
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Join Network Game");
  /* create player selection */
  CreatePlayerItems (2*CELL_W);
  /* demo recording */
  MenuAddComboBool (8*CELL_W, MENU_ROW (0), 5*CELL_W, "Record Demo:", 2*CELL_W, &gameSetup.recordDemo);
/* bot */
  MenuAddComboBool (8*CELL_W, MENU_ROW (1), 5*CELL_W, "bot:", 2*CELL_W, &gameSetup.bot);
  MenuAddComboBool (8*CELL_W, MENU_ROW (2), 5*CELL_W, "Beep at start:", 2*CELL_W, &gameSetup.beep);
  MenuAddComboInt (8*CELL_W, MENU_ROW (3), 5*CELL_W, "Music:",     2*CELL_W, (int *) &gameSetup.Music, musicList);
  /* Buttons */
  MenuSetAbort    (MenuAddHButton ( 4 * CELL_W/2, MENU_BOTTOM, 5*CELL_W, "Back",        ButtonMainMenu,    NULL) );
  idButtonCentral= MenuAddHButton (17 * CELL_W/2, MENU_ROW(6), 5*CELL_W, "Search Central",ButtonSearchCentral, par);
  idButtonStart  = MenuAddHButton (17 * CELL_W/2, MENU_ROW(7), 5*CELL_W, "Connect ...", ButtonStartClient, par);
  idButtonLevels = MenuAddHButton (17 * CELL_W/2, MENU_BOTTOM, 5*CELL_W, "Search LAN",  ButtonSearchLan,   par);
  MenuSetDefault (idButtonLevels);
  /* polling */
  MenuAddCyclic (PollStartGame, par);
  /* --- */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateJoinNetGameMenu */

/*
 * create the central menu ( empty just redirect ) XBCC
 */
void setAutoCentral(XBBool set) {
  autoCentral=set;
  setAutoCentral2(autoCentral);
}

XBBool
CreateCentralGameMenu (void *par)
{
  return CreateCentralMenu (par);
} /* ButtonSelectLevels */

/*
 *
 */
XBPlayerHost
GetHostType (void)
{
  return hostType;
} /* GetHostType */

/*
 *
 */
void
SetHostType (XBPlayerHost _hostType)
{
  hostType = _hostType;
} /* SetHostType */

/*
 * end of file menu_game.c
 */
