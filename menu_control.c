/*
 * file menu_control.h - edit control settings
 *
 * $Id: menu_control.c,v 1.7 2004/08/05 12:03:09 iskywalker Exp $
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
#include "menu_control.h"

#include "cfg_control.h"
#include "mi_tool.h"
#include "menu.h"
#include "menu_layout.h"

#include "gui.h"

/*
 * local macros
 */
#define PLAYER_KEYB_1    0
#define PLAYER_KEYB_2    1
#define PLAYER_JOY_1     2
#define PLAYER_JOY_2     3
#define PLAYER_JOY_3     4
#define PLAYER_JOY_4     5

/*
 * local variables
 */

/* player controls */
static const char *joystickString[4] = {
  "Joystick 1", 
  "Joystick 2", 
  "Joystick 3", 
  "Joystick 4", 
};

static XBBool
CreateConfigKeyboardMenu1 (void *par);
/* controls code */
static XBEventCode ctrl[NUM_LOCAL_PLAYER] = {
  XBE_KEYB_1, XBE_KEYB_2, XBE_JOYST_1, XBE_JOYST_2, XBE_JOYST_3, XBE_JOYST_4,
};

/* local copy od keybaord control */
static CFGControlKeyboard cfgKeyboard;

/*
 * store keyboard configuration
 */
static XBBool
ButtonSaveKeyboard (void *par)
{
  XBEventCode *pCtrl = par;

  assert (pCtrl != NULL);
  StoreControlKeyboard (*pCtrl, &cfgKeyboard);
  GUI_UpdateKeyTables ();
  return CreateConfigControlMenu (NULL);
} /* ButtonSaveKeyboard */

/*
 * menu to configure keyboard
 */ 
static XBBool
CreateConfigKeyboardMenu (void *par)
{
  XBEventCode *pCtrl = par;
  int row;

  assert (pCtrl != NULL);
  /* retrieve controls */
  if (! RetrieveControlKeyboard (*pCtrl, &cfgKeyboard) ) {
    memset (&cfgKeyboard, 0, sizeof (cfgKeyboard) ); 
  }
  MenuClear ();
  /* title */
  MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Configure Keyboard");
  /* Buttons */				   
  row = MENU_TOP;  	     		   
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Go up:",         &cfgKeyboard.keyUp);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Drop Bomb:",     &cfgKeyboard.keyBomb);
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Go down:",       &cfgKeyboard.keyDown);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Special extra:", &cfgKeyboard.keySpecial);
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Go left:",       &cfgKeyboard.keyLeft);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Pause game:",    &cfgKeyboard.keyPause);
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Go right:",      &cfgKeyboard.keyRight);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Abort level:",   &cfgKeyboard.keyAbort);
  /* Skywalker */
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Laola:",   &cfgKeyboard.keyLaola);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Looser:",  &cfgKeyboard.keyLooser);
  /* */
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Stop:",          &cfgKeyboard.keyStop);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Cancel abort:",  &cfgKeyboard.keyAbortCancel);
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Bot:",  &cfgKeyboard.keyBot);
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Chat Send:",  &cfgKeyboard.keyChatSend); 
  row += CELL_H;
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Chat Start:",  &cfgKeyboard.keyChatStart); 
  MenuAddKeysym (8*CELL_W, row, 5*CELL_W, "Chat Cancel:",  &cfgKeyboard.keyChatCancel);
  /* ok and cancel */
  MenuSetAbort   (MenuAddHButton ( 3 * CELL_W/2, MENU_BOTTOM, 3*CELL_W, "Cancel", CreateConfigControlMenu, NULL) );
  MenuAddHButton (11 * CELL_W/2, MENU_BOTTOM, 3*CELL_W, "Next",CreateConfigKeyboardMenu1 , par);
  MenuSetDefault (MenuAddHButton (19 * CELL_W/2, MENU_BOTTOM, 3*CELL_W, "Ok",     ButtonSaveKeyboard,      par) );
  /* --- */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateConfigKeyboardMenu */

/*
 * store keyboard configuration
 */
static XBBool
BackToCreateConfigKeyboard (void *par)
{
  XBEventCode *pCtrl = par;

  assert (pCtrl != NULL);
  StoreControlKeyboard (*pCtrl, &cfgKeyboard);
  GUI_UpdateKeyTables ();
  return CreateConfigKeyboardMenu (par);
} /* BackToCreateConfigKeyboard */
/*
 * menu to configure keyboard
 */ 
static XBBool
CreateConfigKeyboardMenu1 (void *par)
{
  XBEventCode *pCtrl = par;
  int row;

  assert (pCtrl != NULL);
  /* retrieve controls */
  StoreControlKeyboard (*pCtrl, &cfgKeyboard);
  GUI_UpdateKeyTables ();
  if (! RetrieveControlKeyboard (*pCtrl, &cfgKeyboard) ) {
    memset (&cfgKeyboard, 0, sizeof (cfgKeyboard) ); 
  }
  MenuClear ();
  /* title */
  MenuAddLabel (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Configure Keyboard");
  /* Buttons */				   
  row = MENU_TOP;
  /* ok and cancel */
  MenuAddKeysym (2*CELL_W, row, 5*CELL_W, "Chatmsg Rec.:",  &cfgKeyboard.keyChatChangeReceiver); 

  MenuSetAbort   (MenuAddHButton ( 3 * CELL_W/2, MENU_BOTTOM, 3*CELL_W, "Cancel", CreateConfigControlMenu, NULL) );
  MenuAddHButton (11 * CELL_W/2, MENU_BOTTOM, 3*CELL_W, "Last",BackToCreateConfigKeyboard , par);
  MenuSetDefault (MenuAddHButton (19 * CELL_W/2, MENU_BOTTOM, 3*CELL_W, "Ok",     ButtonSaveKeyboard,      par) );
  /* --- */
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateConfigKeyboardMenu */
/*
 * menu to select input device to configure
 */
XBBool
CreateConfigControlMenu (void *par)
{
  int i, row;
  int numJoysticks = GUI_NumJoysticks ();
  MENU_ID id;

  MenuClear ();
  /* title */
  MenuAddLabel  (TITLE_LEFT, TITLE_TOP, TITLE_WIDTH, "Configure Controls");
  /* Buttons */				   
  row = MENU_TOP;  	     		   
  MenuAddHButton (MENU_LEFT, row, MENU_WIDTH, "Right Keyboard", CreateConfigKeyboardMenu, ctrl + PLAYER_KEYB_1);
  row += CELL_H;
  MenuAddHButton (MENU_LEFT, row, MENU_WIDTH, "Left Keyboard",  CreateConfigKeyboardMenu, ctrl + PLAYER_KEYB_2);
  row += CELL_H;
  for (i = 0; i < numJoysticks; i ++) {
    id = MenuAddHButton (MENU_LEFT, row, MENU_WIDTH, joystickString[i], NULL, NULL);
    MenuSetActive (id, XBFalse);
    row += CELL_H;
  }
  MenuSetAbort (MenuAddHButton (MENU_LEFT, MENU_BOTTOM, MENU_WIDTH, "Options Menu",    CreateOptionsMenu, NULL) );
  row += CELL_H;  	
  MenuSetLinks ();
  /* that's all*/
  return XBFalse;
} /* CreateConfigControlMenu */

/*
 * end of file menu_control.c
 */
