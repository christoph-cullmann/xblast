/*
 * file menu_player.c - edit player settings
 *
 * $Id: menu_player.c,v 1.6 2005/01/23 16:54:38 lodott Exp $
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
#include "menu_player.h"

#include "image.h"
#include "menu.h"
#include "menu_layout.h"
#include "mi_tool.h"
#include "str_util.h"

#include "gui.h"
#include "user.h" // XBCC

/*
 * local macros
 */
#define NUM_EDIT_PLAYERS  6
#define MSG_LENGTH        80

/*
 * local prototypes
 */
static XBBool CreateEditPlayerMenu (void *par);
static XBBool CreatePlayerColorsMenu (void *par);
static XBBool CreateNewPlayerMenu (void *par);
static XBBool CreateRenamePlayerMenu (void *par);

/*
 * local variables
 */
static int firstPlayer = 0;
static XBAtom atomPlayers[NUM_EDIT_PLAYERS];
static CFGPlayerGraphics gfxPlayer[NUM_EDIT_PLAYERS];
static const CFGPlayerGraphics *pGfxPlayer[NUM_EDIT_PLAYERS] = {
  NULL, NULL, NULL, NULL,
};
static BMSpriteAnimation playerAnime[1] = { SpriteStopDown };
static BMSpriteAnimation allAnime[4] = {
  SpriteStopLeft, SpriteStopUp, SpriteStopDown, SpriteStopRight
};
static BMSpriteAnimation bigAnime[1]    = { SpriteWinner };
static BMSpriteAnimation delAnime[1]    = { SpriteDamagedDown };
static BMSpriteAnimation renameAnime[1] = { SpriteLooser };

/* player graphics */
static XBRgbValue rgb;
/* player messages */
static CFGPlayerMessages playerMsg;
static char msgWinLevel[MSG_LENGTH];
static char msgWinGame[MSG_LENGTH];
static char msgLoseLife[MSG_LENGTH];
static char msgLoseLevel[MSG_LENGTH];
static char msgLaola[MSG_LENGTH];
static char msgLoser[MSG_LENGTH];
static char msgGloat[MSG_LENGTH];
static char msgWelcome[MSG_LENGTH];

/* misc player options */
static CFGPlayerMisc playerMisc;
static XBComboEntryList turnStepTable[] = {
  { "0",  0, NULL, ATOM_INVALID, },
  { "1",  1, NULL, ATOM_INVALID, },
  { "2",  2, NULL, ATOM_INVALID, },
  { "3",  3, NULL, ATOM_INVALID, },
  { "4",  4, NULL, ATOM_INVALID, },
  { "5",  5, NULL, ATOM_INVALID, },
  { "6",  6, NULL, ATOM_INVALID, },
  { "7",  7, NULL, ATOM_INVALID, },
  { NULL, 0, NULL, ATOM_INVALID, },
};
/* title with player name */
static char title[256];

/*
 * menu table for color boxes
 */
static XBComboEntryList *colorValueList = NULL;
static XBComboEntryList *shapeList      = NULL;

/*
 * name for renaming
 */
static char newName[16];
static char newPass[16];
static CFGPlayerID playerID;

XBBool gotPID=XBFalse; // XBCC

/*****************
 * graphics menu *
 *****************/

/*
 * create color value list
 */
static XBComboEntryList *
CreateColorValueList (void)
{
  XBComboEntryList *list;
  int i;
  char tmp[8];

  list = calloc (XBCOLOR_DEPTH+2, sizeof (XBComboEntryList));
  assert (list != NULL);
  for (i = 0; i <= XBCOLOR_DEPTH; i ++) {
    sprintf (tmp, "%3d", 255 * i / XBCOLOR_DEPTH);
    list[i].text  = DupString (tmp);
    list[i].value = i;
  }
  return list;
} /* CreateColorValueList */

/*
 * create list with shapes
 */
static XBComboEntryList *
CreateShapeList (void)
{
  const XBAtom *shape;
  XBComboEntryList *list;
  int i, num;

  shape = GetShapeList (&num);
  assert (shape != NULL);
  list = calloc (num + 1, sizeof (*list) );
  for (i = 0; i < num; i ++) {
    list[i].text = DupString (GUI_AtomToString (shape[i]));
    list[i].atom = shape[i];
  }
  /* that's all */
  return list;
} /* CreateShapeList */

/*
 * clear list with shapes
 */
static void
DeleteList (XBComboEntryList *list)
{
  int i;

  assert (NULL != list);
  for (i = 0; list[i].text != NULL; i ++) {
    free ((void *) list[i].text);
  }
  free (list);
} /* DeleteList */

/*
 * save player graphics
 */
static XBBool
ButtonSaveGraphics (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* store it */
  StorePlayerGraphics (CT_Local, *atom, gfxPlayer + 0);
  /* clean up */
  DeleteList (shapeList);
  shapeList = NULL;
  DeleteList (colorValueList);
  colorValueList = NULL;
  return CreateEditPlayerMenu (par);
} /* ButtonSaveGraphics */

/*
 * cancel button
 */
static XBBool
ButtonEditPlayerMenu (void *par)
{
  if (NULL != shapeList) {
    DeleteList (shapeList);
    shapeList = NULL;
  }
  if (NULL != colorValueList) {
    DeleteList (colorValueList);
    colorValueList = NULL;
  }
  return CreateEditPlayerMenu (par);
} /* ButtonEditPlayerMenu */

/*
 * apply colors
 */
static XBBool
ButtonApplyColors (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* store it */
  StorePlayerGraphics (CT_Local, *atom, gfxPlayer + 0);
  return CreatePlayerColorsMenu (par);
} /* ButtonApplyColors */

/*
 * parse color name
 */
static void
CheckColorName (void *par)
{
  char *colorName = par;
  XBColor color;

  assert (NULL != colorName);
  if (0 != colorName[0]) {
    color = GUI_ParseColor (colorName);
    if (COLOR_INVALID != color) {
      rgb.red   = GET_RED   (color);
      rgb.green = GET_GREEN (color);
      rgb.blue  = GET_BLUE  (color);
      colorName[0] = 0;
    }
  }
} /* CheckColorName */

/*
 * draw colors menu
 */
static XBBool
CreatePlayerColorsMenu (void *par)
{
  int          i;
  XBAtom      *atom = par;
  static char  colorName[48];

  assert (atom != NULL);
  /* get player graphics */
  RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer);
  /* create list with color values */
  if (NULL == colorValueList) {
    colorValueList = CreateColorValueList ();
  }
  assert (colorValueList != NULL);
  /* create list with shapes */
  if (NULL == shapeList) {
    shapeList = CreateShapeList ();
  }
  assert (shapeList != NULL);
  /* set color name editor to empty */
  colorName[0] = 0;
  /* build menu */
  MenuClear ();
  /* Title */
  sprintf (title, "Colors for %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* shape */
  MenuAddComboAtom (5*CELL_W, MENU_ROW (0), 5*CELL_W, "Shape:", 2*CELL_W, &gfxPlayer[0].shape, shapeList);
  /* colors windows */
  MenuAddColor (COLOR_LEFT,  MENU_ROW (1), COLOR_WIDTH, "Helmet:",       &gfxPlayer[0].helmet,    &rgb);
  MenuAddColor (COLOR_RIGHT, MENU_ROW (1), COLOR_WIDTH, "Arms & Legs:",  &gfxPlayer[0].armsLegs,  &rgb);
  MenuAddColor (COLOR_LEFT,  MENU_ROW (2), COLOR_WIDTH, "Face:",         &gfxPlayer[0].face,      &rgb);
  MenuAddColor (COLOR_RIGHT, MENU_ROW (2), COLOR_WIDTH, "Hands & Feet:", &gfxPlayer[0].handsFeet, &rgb);
  MenuAddColor (COLOR_LEFT,  MENU_ROW (3), COLOR_WIDTH, "Body:",         &gfxPlayer[0].body,      &rgb);
  MenuAddColor (COLOR_RIGHT, MENU_ROW (3), COLOR_WIDTH, "Backpack:",     &gfxPlayer[0].backpack,  &rgb);
  /* set rgb value */
  memset (&rgb, 0, sizeof (rgb));
  MenuAddComboInt ( 2*CELL_W, MENU_ROW (4), 3*CELL_W, "Red:",   3*CELL_W/2, &rgb.red,   colorValueList);
  MenuAddComboInt ( 6*CELL_W, MENU_ROW (4), 3*CELL_W, "Green:", 3*CELL_W/2, &rgb.green, colorValueList);
  MenuAddComboInt (10*CELL_W, MENU_ROW (4), 3*CELL_W, "Blue:",  3*CELL_W/2, &rgb.blue,  colorValueList);
  /* string for rgb value */
  if (COLOR_INVALID != GUI_ParseColor ("Black")) {
    MenuAddString (MSG_LEFT, MENU_ROW (5), MSG_WIDTH, "Colorname:", MSG_EDIT, colorName, sizeof (colorName));
    MenuAddCyclic (CheckColorName, colorName);
  }
  /* sprite */
  for (i = 0; i < 4; i ++) {
    pGfxPlayer[i] = gfxPlayer;
    MenuAddPlayer (PLAYER_LEFT(i,4), PLAYER_TOP, PLAYER_WIDTH, i, pGfxPlayer + i, 1, allAnime + i);
  }
  /* ok, cancel or cancel */
  MenuSetAbort   (MenuAddHButton ( 3*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", ButtonEditPlayerMenu, atom) );
  MenuSetDefault (MenuAddHButton (11*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Apply",  ButtonApplyColors,    atom) );
  (void)          MenuAddHButton (19*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonSaveGraphics,   atom);
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreatePlayerColorsMenu */

/*****************
 * messages menu *
 *****************/

/*
 * save player messages
 */
static XBBool
ButtonSaveMessages (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* get player messages */
  playerMsg.msgWinLevel  = (0 != msgWinLevel[0])  ? msgWinLevel  : NULL;
  playerMsg.msgWinGame   = (0 != msgWinGame[0])   ? msgWinGame   : NULL;
  playerMsg.msgLoseLife  = (0 != msgLoseLife[0])  ? msgLoseLife  : NULL;
  playerMsg.msgLoseLevel = (0 != msgLoseLevel[0]) ? msgLoseLevel : NULL;
  playerMsg.msgGloat     = (0 != msgGloat[0])     ? msgGloat     : NULL;
  playerMsg.msgLaola     = (0 != msgLaola[0])     ? msgLaola     : NULL;
  playerMsg.msgLoser     = (0 != msgLoser[0])     ? msgLoser     : NULL;
  playerMsg.msgWelcome   = (0 != msgWelcome[0])   ? msgWelcome   : NULL;
  /* store it */
  StorePlayerMessages (CT_Local, *atom, &playerMsg);
  return CreateEditPlayerMenu (par);
} /* ButtonSaveMessages */

/*
 * draw player messages menu
 */
static XBBool
CreatePlayerMessageMenu (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  /* get player messages */
  memset (&playerMsg, 0, sizeof (playerMsg) );
  RetrievePlayerMessages (CT_Local, *atom, &playerMsg);
  if (NULL != playerMsg.msgWinLevel) {
    strncpy (msgWinLevel, playerMsg.msgWinLevel, MSG_LENGTH);
  } else {
    msgWinLevel[0] = 0;
  }
  if (NULL != playerMsg.msgWinGame) {
    strncpy (msgWinGame, playerMsg.msgWinGame, MSG_LENGTH);
  } else {
    msgWinGame[0] = 0;
  }
  if (NULL != playerMsg.msgLoseLife) {
    strncpy (msgLoseLife, playerMsg.msgLoseLife, MSG_LENGTH);
  } else {
    msgLoseLife[0] = 0;
  }
  if (NULL != playerMsg.msgLoseLevel) {
    strncpy (msgLoseLevel, playerMsg.msgLoseLevel, MSG_LENGTH);
  } else {
    msgLoseLevel[0] = 0;
  }
  if (NULL != playerMsg.msgGloat) {
    strncpy (msgGloat, playerMsg.msgGloat, MSG_LENGTH);
  } else {
    msgGloat[0] = 0;
  }
  if (NULL != playerMsg.msgLaola) {
    strncpy (msgLaola, playerMsg.msgLaola, MSG_LENGTH);
  } else {
    msgLaola[0] = 0;
  }
  if (NULL != playerMsg.msgLoser) {
    strncpy (msgLoser, playerMsg.msgLoser, MSG_LENGTH);
  } else {
    msgLoser[0] = 0;
  }
  if (NULL != playerMsg.msgWelcome) {
    strncpy (msgWelcome, playerMsg.msgWelcome, MSG_LENGTH);
  } else {
    msgWelcome[0] = 0;
  }
  /* build menu */
  MenuClear ();
  /* Title */
  sprintf (title, "Messages for %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* message windows */
  MenuAddString (MSG_LEFT, MSG_TOP,            MSG_WIDTH, "Welcome:",    MSG_EDIT, msgWelcome,   MSG_LENGTH);
  MenuAddString (MSG_LEFT, MSG_TOP + 1*CELL_H, MSG_WIDTH, "Win Level:",  MSG_EDIT, msgWinLevel,  MSG_LENGTH);
  MenuAddString (MSG_LEFT, MSG_TOP + 2*CELL_H, MSG_WIDTH, "Win Game:",   MSG_EDIT, msgWinGame,   MSG_LENGTH);
  MenuAddString (MSG_LEFT, MSG_TOP + 3*CELL_H, MSG_WIDTH, "Lose Life:",  MSG_EDIT, msgLoseLife,  MSG_LENGTH);
  MenuAddString (MSG_LEFT, MSG_TOP + 4*CELL_H, MSG_WIDTH, "Lose Level:", MSG_EDIT, msgLoseLevel, MSG_LENGTH);
  MenuAddString (MSG_LEFT, MSG_TOP + 5*CELL_H, MSG_WIDTH, "Gloat:",      MSG_EDIT, msgGloat,     MSG_LENGTH);
  MenuAddString (MSG_LEFT, MSG_TOP + 6*CELL_H, MSG_WIDTH, "Laola:",      MSG_EDIT, msgLaola,     MSG_LENGTH);
    MenuAddString (MSG_LEFT, MSG_TOP + 7*CELL_H, MSG_WIDTH, "Loser:",      MSG_EDIT, msgLoser,     MSG_LENGTH);
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
  /* ok or cancel */
  MenuSetAbort   (MenuAddHButton ( 5*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreateEditPlayerMenu, atom) );
  MenuSetDefault (MenuAddHButton (17*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonSaveMessages,   atom) );
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreatePlayerMessageMenu */

/************************
 * control options menu *
 ************************/

/*
 * save control options
 */
static XBBool
ButtonSaveMisc (void *par)
{
  XBAtom *atom = par;
  assert (atom != NULL);
  /* store it */
  StorePlayerMisc (CT_Local, *atom, &playerMisc);
  return CreateEditPlayerMenu (par);
} /* ButtonSaveGraphics */

/*
 * draw menu for misc. player options
 */
static XBBool
CreatePlayerMiscMenu (void *par)
{
  XBAtom *atom = par;

  assert (NULL != atom);
  /* get options */
  memset (&playerMisc, 0, sizeof (playerMisc));
  RetrievePlayerMisc (CT_Local, *atom, &playerMisc);
  /* build menu */
  MenuClear ();
  /* Title */
  sprintf (title, "Other Options for %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* options */
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
  /* options */
  MenuAddComboBool (DLG_LEFT, DLG_ROW (0), DLG_WIDTH, "Use stop key:",                   2*CELL_W, &playerMisc.useStopKey);
  MenuAddComboInt  (DLG_LEFT, DLG_ROW (1), DLG_WIDTH, "Backtrack for turns (keyboard):", 2*CELL_W, &playerMisc.turnStepKeyboard,
		    turnStepTable);
  MenuAddComboInt  (DLG_LEFT, DLG_ROW (2), DLG_WIDTH, "Backtrack for turns (joystick):", 2*CELL_W, &playerMisc.turnStepJoystick,
		    turnStepTable);
  /* ok or cancel */
  MenuSetAbort   (MenuAddHButton ( 5*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreateEditPlayerMenu, atom) );
  MenuSetDefault (MenuAddHButton (17*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonSaveMisc,   atom) );
  return XBFalse;
} /* CreatePlayerMenu */

/**********************
 * delete player menu *
 **********************/

/*
 * delete a player
 */
static XBBool
ButtonDeletePlayer (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  DeletePlayerConfig (CT_Local, *atom);
  return CreatePlayerOptionsMenu (NULL);
} /* ButtonDeletePlayer */

/*
 * draw delete player menu
 */
static XBBool
CreateDeletePlayerMenu (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  MenuClear ();
  /* Title */
  sprintf (title, "Delete Player %s?", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* edit options */
  MenuSetDefault (MenuAddHButton (MENU_LEFT, MENU_TOP,    MENU_WIDTH, "Yes, delete player!", ButtonDeletePlayer,   atom) );
  /* back to previous menu */
  MenuSetAbort   (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, "No, just kidding.",   CreateEditPlayerMenu, atom) );
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, delAnime);
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateEditPlayerMenu */

/**********************
 * rename player menu *
 **********************/

/*
 * rename a player
 */
static XBBool
ButtonRenamePlayer (void *par)
{
  static XBAtom newPlayer;
  XBAtom *oldPlayer;

  /* get player to rename */
  assert (par != NULL);
  oldPlayer = par;
  /* create new player */
  newPlayer  = RenamePlayerConfig (CT_Local, *oldPlayer, newName);
  if (ATOM_INVALID != newPlayer) {
    return CreateEditPlayerMenu (&newPlayer);
  } else {
    return CreateRenamePlayerMenu (NULL);
  }
} /* ButtonNewPlayer */

/*
 * menu for renaming a player
 */
static XBBool
CreateRenamePlayerMenu (void *par)
{
  XBAtom *atom = par;

  MenuClear ();
  /* Title */
  assert (NULL != atom);
  sprintf (newName, "%s", GetPlayerName (CT_Local, *atom));
  sprintf (title, "Rename Player %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* message windows */
  MenuAddString (DLG_LEFT, DLG_TOP, DLG_WIDTH, "New Name:", 3*CELL_W, newName, sizeof (newName) );
  /* ok and cancel button */
  MenuSetAbort   (MenuAddHButton ( 7*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreatePlayerOptionsMenu, NULL) );
  MenuSetDefault (MenuAddHButton (15*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonRenamePlayer,      par) );
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, renameAnime);
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateRenamePlayerMenu */

/**************************
 * unregister player menu *
 **************************/

/*
 * abort the registration process
 */
static XBBool
ButtonAbortRegistration (void *par) {
  User_Disconnect();
  return CreatePlayerOptionsMenu (NULL);
} /* ButtonAbortRegistration */

/*
 * cyclic function checking if PID arrived
 */
static void
PollForPID (void *par)
{
  XBAtom *atom = par;
  int    PID;
  PID = User_GetPID();
  if (PID > -1000) {
    if (!gotPID) {
      switch(PID) {
      case -3:
	MenuAddLabel  (5*CELL_W/2, MENU_ROW(4), 10*CELL_W, "Error in central, press OK.");
	break;
      case -2:
	MenuAddLabel  (5*CELL_W/2, MENU_ROW(4), 10*CELL_W, "Bad user ID, press OK.");
	break;
      case -1:
	MenuAddLabel  (5*CELL_W/2, MENU_ROW(4), 10*CELL_W, "Wrong password, press OK.");
	break;
      default:
	MenuAddLabel  (5*CELL_W/2, MENU_ROW(4), 10*CELL_W, "Registered, press OK to continue.");
	break;
      }
      MenuSetDefault  (MenuAddHButton ( 8*CELL_W/2, MENU_ROW(7), 7*CELL_W, "OK", ButtonAbortRegistration, par) );
      gotPID = XBTrue;
      if(PID>0) {
	if(RetrievePlayerID(CT_Local, *atom, &playerID)) {
	  playerID.PID=PID;
	  if (NULL != playerID.pass) {
	    strncpy (newPass, playerID.pass, 16);
	  } else {
	    newPass[0] = 0;
	  }
	  playerID.pass  = (0 != newPass[0])  ? newPass  : NULL;
	  StorePlayerID(CT_Local, *atom, &playerID);
	  SavePlayerConfig();
	}
      }
      User_SendDisconnect();
    }
  }
} /* PollForPID */

/*
 * register a player
 */
static XBBool
ButtonRegisterPlayer (void *par)
{
  XBAtom *atom = par;
  CFGCentralSetup central;

  assert (atom != NULL);
  /* store it */
  playerID.pass  = (0 != newPass[0])  ? newPass  : NULL;
  StorePlayerID (CT_Local, *atom, &playerID);
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }

  MenuClear ();
  sprintf (title, "Registering %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* connect to central */
  RetrieveCentralSetup (&central);
  if (User_Connect(&central)) {
    MenuAddLabel  (5*CELL_W/2, MENU_ROW(3), 10*CELL_W, "Please wait...");
    MenuSetAbort  (MenuAddHButton ( 8*CELL_W/2, MENU_BOTTOM, 7*CELL_W, "Abort registration", ButtonAbortRegistration, par) );
    MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
    /* poll function */
    gotPID = XBFalse;
    User_SendRegisterPlayer(*atom);
    MenuAddCyclic (PollForPID, par);
  } else {
    MenuAddLabel  (5*CELL_W/2, MENU_ROW(3), 10*CELL_W, "Unable to connect to central.");
    MenuSetAbort   (MenuAddHButton ( 8*CELL_W/2, MENU_BOTTOM, 7*CELL_W, "Damn", CreatePlayerOptionsMenu, NULL) );
    MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, renameAnime);
  }
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* ButtonRegisterPlayer */

/*
 * menu for registering a player XBCC
 */
static XBBool
CreateRegisterPlayerMenu (void *par)
{
  XBAtom *atom = par;

  assert (NULL != atom);
  /* get options */
  memset (&playerID, 0, sizeof (playerID));
  RetrievePlayerID (CT_Local, *atom, &playerID);
  if (NULL != playerID.pass) {
    strncpy (newPass, playerID.pass, 16);
  } else {
    newPass[0] = 0;
  }
  /* build menu */
  MenuClear ();
  sprintf (title, "Register Player %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* message windows */
  MenuAddString (DLG_LEFT, DLG_TOP, DLG_WIDTH, "Password:", 3*CELL_W, newPass, sizeof(newPass) );
  /* ok and cancel button */
  MenuSetAbort   (MenuAddHButton ( 7*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreatePlayerOptionsMenu, NULL) );
  MenuSetDefault (MenuAddHButton (15*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonRegisterPlayer,     par) );
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateRenamePlayerMenu */

/**************************
 * unregister player menu *
 **************************/

/*
 * button to unregister a player
 */
static XBBool
ButtonUnregisterPlayer (void *par)
{
  XBAtom *atom = par;
  /*  CFGCentralSetup central; DOES NOT SEND TO CENRAL */

  assert (atom != NULL);
  /* store it */
  playerID.PID  = -1;
  playerID.pass = NULL;
  StorePlayerID (CT_Local, *atom, &playerID);
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }

  MenuClear ();
  sprintf (title, "Unregistered (locally)");
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* connect to central */
  MenuSetDefault  (MenuAddHButton ( 8*CELL_W/2, MENU_BOTTOM, 7*CELL_W, "OK", CreatePlayerOptionsMenu, NULL) );
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
  /* message windows */
  /* link items */
  MenuSetLinks ();

  /* that's all */
  return XBFalse;

  return CreateEditPlayerMenu (par);
} /* ButtonUnregisterPlayer */

/*
 * draw menu for unregistering a player
 */
static XBBool
CreateUnregisterPlayerMenu (void *par)
{
  XBAtom *atom = par;

  assert (NULL != atom);
  /* get options */
  memset (&playerID, 0, sizeof (playerID));
  RetrievePlayerID (CT_Local, *atom, &playerID);
  if (NULL != playerID.pass) {
    strncpy (newPass, playerID.pass, 16);
  } else {
    newPass[0] = 0;
  }
  /* build menu */
  MenuClear ();
  sprintf (title, "Unregister Player %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* ok and cancel button */
  MenuSetAbort   (MenuAddHButton ( 7*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreatePlayerOptionsMenu, NULL) );
  MenuSetDefault (MenuAddHButton (15*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonUnregisterPlayer,     par) );
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateRenamePlayerMenu */

/********************
 * edit player menu *
 ********************/

/*
 * draw edit player menu
 */
static XBBool
CreateEditPlayerMenu (void *par)
{
  XBAtom *atom = par;

  assert (atom != NULL);
  MenuClear ();
  /* Title */
  sprintf (title, "Edit Player %s", GetPlayerName (CT_Local, *atom));
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, title);
  /* edit options */
  MenuAddHButton (MENU_LEFT, MENU_ROW (0), MENU_WIDTH, "Edit Graphics",   CreatePlayerColorsMenu,  atom);
  MenuAddHButton (MENU_LEFT, MENU_ROW (1), MENU_WIDTH, "Edit Messages",   CreatePlayerMessageMenu, atom);
  MenuAddHButton (MENU_LEFT, MENU_ROW (2), MENU_WIDTH, "Control Options", CreatePlayerMiscMenu,    atom);
  MenuAddHButton (MENU_LEFT, MENU_ROW (3), MENU_WIDTH, "Rename Player",   CreateRenamePlayerMenu,  atom);
  MenuAddHButton (MENU_LEFT, MENU_ROW (4), MENU_WIDTH, "Delete Player",   CreateDeletePlayerMenu,  atom);
  MenuAddHButton (MENU_LEFT, MENU_ROW (5), MENU_WIDTH, "Register Player", CreateRegisterPlayerMenu,atom);
  MenuAddHButton (MENU_LEFT, MENU_ROW (6), MENU_WIDTH, "Unregister Player", CreateUnregisterPlayerMenu,atom);
  /* back to previous menu */
  MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, "Player Options Menu", CreatePlayerOptionsMenu, NULL) );
  /* sprite */
  if (RetrievePlayerGraphics (CT_Local, *atom, COLOR_INVALID, gfxPlayer) ) {
    pGfxPlayer[0] = gfxPlayer;
  } else {
    pGfxPlayer[0] = NULL;
  }
  MenuAddPlayer (PLAYER_LEFT(0,1), PLAYER_TOP, PLAYER_WIDTH, 0, &pGfxPlayer[0], 1, bigAnime);
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateEditPlayerMenu */

/***********************
 * add new player menu *
 ***********************/

/*
 * button to create a new player
 */
static XBBool
ButtonNewPlayer (void *par)
{
  static XBAtom player;

  assert (par != NULL);
  player = CreateNewPlayerConfig (CT_Local, par);
  if (ATOM_INVALID != player) {
    return CreateEditPlayerMenu (&player);
  } else {
    return CreateNewPlayerMenu (NULL);
  }
} /* ButtonNewPlayer */

/*
 * draw new player menu
 */
static XBBool
CreateNewPlayerMenu (void *par)
{
  static char name[16];

  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Create a New Player");
  /* message windows */
  MenuAddString (DLG_LEFT, DLG_TOP, DLG_WIDTH, "Player Name:", 3*CELL_W, name, sizeof (name) );
  /* ok and cancel button */
  MenuSetAbort   (MenuAddHButton ( 7*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "Cancel", CreatePlayerOptionsMenu, NULL) );
  MenuSetDefault (MenuAddHButton (15*CELL_W/2, MENU_BOTTOM, 4*CELL_W, "OK",     ButtonNewPlayer,         name) );
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateNewPlayerMenu */

/***********************
 * player options menu *
 ***********************/

/*
 * get next player list
 */
static XBBool
ButtonPlayerForward (void *par)
{
  int newFirst = firstPlayer + NUM_EDIT_PLAYERS;
  if (newFirst < GetNumPlayerConfigs (CT_Local) ) {
    firstPlayer = newFirst;
  }
  return CreatePlayerOptionsMenu (par);
} /* ButtonPlayerForward */

/*
 * get previous player list
 */
static XBBool
ButtonPlayerBackward (void *par)
{
  int newFirst = firstPlayer - NUM_EDIT_PLAYERS;
  if (newFirst > 0) {
    firstPlayer = newFirst;
  } else {
    firstPlayer = 0;
  }
  return CreatePlayerOptionsMenu (par);
} /* ButtonPlayerForward */

/*
 * draw player main menu
 */
XBBool
CreatePlayerOptionsMenu (void *par)
{
  int     row;
  int     player, numPlayers;
  int     button;
  XBBool  odd;
  MENU_ID id;

  MenuClear ();
  /* Title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Player Options Menu");
  /* Buttons */
  row = MENU_TOP;
  MenuAddHButton (MENU_LEFT, row, MENU_WIDTH, "Create New Player", CreateNewPlayerMenu, NULL);
  row += CELL_H;
  /* List players */
  numPlayers = GetNumPlayerConfigs (CT_Local);
  for (player = firstPlayer, button = 0;
       player < numPlayers && button < NUM_EDIT_PLAYERS;
       player ++, button ++) {
    atomPlayers[button] = GetPlayerAtom (CT_Local, player);
    odd = (1 == (button % 2));
    /* button */
    MenuAddHButton (MENU_LEFT + odd * MENU_WIDTH/2, row, MENU_WIDTH/2, GetPlayerName (CT_Local, atomPlayers[button]),
		    CreateEditPlayerMenu, atomPlayers + button);
    if (odd) {
      row += CELL_H;
    }
    /* sprite */
    if (RetrievePlayerGraphics (CT_Local, atomPlayers[button], COLOR_INVALID, gfxPlayer + button) ) {
      pGfxPlayer[button] = gfxPlayer + button;
    } else {
      pGfxPlayer[button] = NULL;
    }
    //    MenuAddPlayer (PLAYER_LEFT(button,MAX_PLAYER), PLAYER_TOP, PLAYER_WIDTH, button, &pGfxPlayer[button], 1, playerAnime);
    MenuAddPlayer (PLAYER_LEFT(button,NUM_EDIT_PLAYERS), PLAYER_TOP, PLAYER_WIDTH, button, &pGfxPlayer[button], 1, playerAnime);
  }
  /* forward backward buttons */
  row = MENU_TOP + (1 + NUM_EDIT_PLAYERS/2)* CELL_H;
  id = MenuAddHButton (MENU_LEFT,                row, MENU_WIDTH/2, "Back", ButtonPlayerBackward, par);
  MenuSetActive (id, (firstPlayer > 0) );
  id = MenuAddHButton (MENU_LEFT + MENU_WIDTH/2, row, MENU_WIDTH/2, "More", ButtonPlayerForward,  par);
  MenuSetActive (id, (player < numPlayers) );
  /* escape to toplevel menu */
  MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, "Options Menu", CreateOptionsMenu, NULL) );
  /* link items */
  MenuSetLinks ();
  /* that's all */
  return XBFalse;
} /* CreateMenuPlayer */

/*
 * end of file menu_player.c
 */
