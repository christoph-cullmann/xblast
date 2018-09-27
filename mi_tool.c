/*
 * file mi_tool.c - toolkit for xblast menus
 *
 * $Id: mi_tool.c,v 1.21 2005/01/11 17:51:37 iskywalker Exp $
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
#include "mi_tool.h"
#include "atom.h"

#include  "server.h"
#include "map.h"
#include "mi_button.h"
#include "mi_color.h"
#include "mi_combo.h"
#include "mi_cyclic.h"
#include "mi_host.h"
#include "mi_int.h"
#include "mi_keysym.h"
#include "mi_label.h"
#include "mi_map.h"
#include "mi_player.h"
#include "mi_stat.h"
#include "mi_string.h"
#include "mi_tag.h"
#include "mi_toggle.h"
#include "menu_edit.h"
#include "menu_game.h"
#include "status.h"

/*
 * local variables
 */
static XBMenuItem *itemFirst 	   = NULL;
static XBMenuItem *itemLast  	   = NULL;
static XBMenuItem *itemFocus 	   = NULL;
static XBBool      execFlag        = XBTrue;
static XBBool      fadeFlag        = XBFalse;
static XBMenuItem *defaultItem     = NULL;
static XBMenuItem *abortItem       = NULL;
static int         chatMode        = 0;
static XBBool         mapMode         = 0;
static int         setPressed      = 0;
static XBComboEntryList yesNoTable[] = {
  { "no",  0, NULL},
  { "yes", 1, NULL},
  { NULL,  0, NULL},
};
static const char  playerName[MAX_HOSTS][NUM_LOCAL_PLAYER][256];
/*
 * delete any menu item
 */
static void
DeleteMenuItem (XBMenuItem *item)
{
  assert (item != NULL);
  switch (item->type) {
  case MIT_Button:     MenuDeleteButton     (item); break;
  case MIT_Color:      MenuDeleteColor      (item); break;
  case MIT_Combo:      MenuDeleteCombo      (item); break;
  case MIT_Cyclic:     MenuDeleteCyclic     (item); break;
  case MIT_Keysym:     MenuDeleteKeysym     (item); break;
  case MIT_Label:      MenuDeleteLabel      (item); break;
  case MIT_Player:     MenuDeletePlayer     (item); break;
  case MIT_String:     MenuDeleteString     (item); break;
  case MIT_Toggle:     MenuDeleteToggle     (item); break;
  case MIT_Integer:    MenuDeleteInteger    (item); break;
  case MIT_Tag:        MenuDeleteTag        (item); break;
  case MIT_Host:       MenuDeleteHost       (item); break;
  case MIT_Table:      MenuDeleteTable      (item); break;
  case MIT_Team:       MenuDeleteTeam       (item); break;
  default:          break;
  }
  free (item);
} /* DeleteMenuItem */



void
SetPressed(XBBool mode){
  setPressed=mode;
}

XBBool
GetPressed(void){
  return setPressed;
}


void
SetXBEditMapMode(XBBool mode){
  mapMode=mode;
}

XBBool
GetXBEditMapMode(void){
  return mapMode;
}


int
GetChatMode(){
  return chatMode;
}
/*
 * clear total menu
 */
void
MenuClear (void)
{
  XBMenuItem *item_next;
  while (itemFirst != NULL) {
    item_next = itemFirst->next;
    DeleteMenuItem (itemFirst);
    itemFirst = item_next;
  }
  itemLast        = itemFocus = NULL;
  fadeFlag    	  = XBTrue;
  defaultItem     = NULL;
  abortItem       = NULL;
  MenuResetBase ();
  MenuClearMap ();
} /* MenuClear */

/*
 * set directional links
 */
void
MenuSetLinks (void)
{
  XBMenuItem *ptr;

  for (ptr = itemFirst; ptr != NULL; ptr = ptr->next) {
    if (NULL != ptr->mouse) {
      ptr->left  = MenuFindLeftItem (ptr);
      ptr->right = MenuFindRightItem (ptr);
      ptr->up    = MenuFindUpperItem (ptr);
      ptr->down  = MenuFindLowerItem (ptr);
    }
  }
} /* MenuSetLinks */

/*
 * find item by id
 */
static XBMenuItem *
FindItem (MENU_ID id)
{
  XBMenuItem *item;

  for (item = itemFirst; item != NULL; item = item->next) {
    if (item->id == id) {
      return item;
    }
  }
  return NULL;
} /* FindItem */

/*
 * set default item
 */
void
MenuSetDefault (MENU_ID id)
{
  defaultItem = FindItem (id);
  if (NULL != defaultItem && MIT_Button == defaultItem->type) {
    MenuSetButtonIcon (defaultItem, ISA_Default);
  }
} /* MenuSetDefault */

/*
 * set abort item
 */
void
MenuSetAbort (MENU_ID id)
{
  abortItem = FindItem (id);
  if (NULL != abortItem && MIT_Button == abortItem->type) {
    MenuSetButtonIcon (abortItem, ISA_Abort);
  }
} /* MenuSetDefault */

/*
 * async execution of function
 */
void 
MenuExecFunc (MIC_button func, void *data)
{
  execFlag = XBTrue;
  MenuButtonSetNextExec (func, data);
} /* MenuExecFunc */

/*
 * activate/deactivate item
 */
void
MenuSetActive (MENU_ID id, XBBool active)
{
  XBMenuItem *item = FindItem (id);
  if (NULL != item) {
    /* set flags */
    if (active) {
      item->flags &= ~MIF_DEACTIVATED;
    } else {
      item->flags |=  MIF_DEACTIVATED;
    }
    switch (item->type) {
    case MIT_Button:     MenuActivateButton (item, active);     break;
    case MIT_Table: MenuActivateTable (item, active); break;
    default:             break;
    }
  }
} /* MenuSetActive */

/*
 * welches item hat den focus
 */
MENU_ID 
MenuGetFocus (void)
{
  return (NULL != itemFocus) ? itemFocus->id : 0;
} /* MenuGetFocus */

/*------------------------------------------------------------------------*
 *
 * add items to menu
 *
 *------------------------------------------------------------------------*/

/*
 * add an item to the menu
 */
static MENU_ID
MenuAdd (XBMenuItem *item)
{
  if (itemLast == NULL) {
    itemFirst = item;
  }
  if (itemLast != NULL) {
    itemLast->next = item;
    item->prev      = itemLast;
  } 
  itemLast = item;
  itemLast->next=NULL;
  if (itemFocus == NULL && item->focus != NULL) {
    item->flags |= MIF_FOCUS;
    itemFocus    = item;
    (*item->focus) (item, XBTrue);
  }
  return item->id;
} /* MenuAdd */

/*
 * add horizontal button to menu
 */
MENU_ID
MenuAddHButton (int x, int y, int w, const char *text, MIC_button func, void *funcData)
{
  return MenuAdd (MenuCreateHButton (x, y, w, text, func, funcData) );
} /* MenuAddHButton */

/*
 * add vertical button to menu
 */
MENU_ID
MenuAddVButton (int x, int y, int h, const char *text, MIC_button func, void *funcData)
{
  return MenuAdd (MenuCreateVButton (x, y, h, text, func, funcData) );
} /* MenuAddVButton */

/*
 * a checkbox item to menu
 */ 
MENU_ID
MenuAddToggle (int x, int y, int w, const char *text, XBBool *pState)
{
  return MenuAdd (MenuCreateToggle (x, y, w, text, pState) );
} /* MenuAddToggle */

/*
 * a label/title to menu
 */
MENU_ID
MenuAddLabel (int x, int y, int w, const char *text)
{
  return MenuAdd (MenuCreateLabel (x, y, w, text) );
} /* MenuAddLabel */


MENU_ID
MenuAddLabel1 (int x, int y, int w, const char *text)
{
  return MenuAdd (MenuCreateLabel1 (x, y, w, text) );
} /* MenuAddLabel */

MENU_ID
MenuAddLabel2 (int x, int y, int w, const char *text)
{
  return MenuAdd (MenuCreateLabel2 (x, y, w, text) );
} /* MenuAddLabel */


/*
 * a simple combobox to menu
 */
MENU_ID
MenuAddComboInt (int x, int y, int w_text, const char *text, int w, int *value, XBComboEntryList *table)
{
  return MenuAdd (MenuCreateCombo (x, y, w_text, text, w, value, NULL, NULL, table) );
} /* MenuAddComboInt */

/*
 * a simple combobox to menu
 */
MENU_ID
MenuAddComboBool (int x, int y, int w_text, const char *text, int w, XBBool *value)
{
  return MenuAdd (MenuCreateCombo (x, y, w_text, text, w, (int *) value, NULL, NULL, yesNoTable) );
} /* MenuAddComboInt */

/*
 * a simple combobox to menu
 */
MENU_ID
MenuAddComboData (int x, int y, int w_text, const char *text, int w, void **data, XBComboEntryList *table)
{
  return MenuAdd (MenuCreateCombo (x, y, w_text, text, w, NULL, data, NULL, table) );
} /* MenuAddComboData */

/*
 * a simple combobox to menu
 */
MENU_ID
MenuAddComboAtom (int x, int y, int w_text, const char *text, int w, XBAtom *atom, XBComboEntryList *table)
{
  return MenuAdd (MenuCreateCombo (x, y, w_text, text, w, NULL, NULL, atom, table) );
} /* MenuAddComboAtom */

/*
 * a simple combobox to menu
 */
MENU_ID
MenuAddCombo (int x, int y, int w_text, const char *text, int w, int *value, void **data, XBAtom *atom, XBComboEntryList *table)
{
  return MenuAdd (MenuCreateCombo (x, y, w_text, text, w, value, data, atom, table) );
} /* MenuAddCombo */

/*
 *
 */
MENU_ID
MenuAddPlayer (int x, int y, int w, int sprite, const CFGPlayerGraphics **cfg, int n_anime, BMSpriteAnimation *anime)
{
  return MenuAdd (MenuCreatePlayer (x, y, w, sprite, cfg, n_anime, anime) );
} /* MenuAddPlayer */

/*
 *
 */
MENU_ID
MenuAddString (int x, int y, int w_text, const char *text, int w, char *buffer, size_t len)
{
  return MenuAdd (MenuCreateString (x, y, w_text, text, w, buffer, len) );
} /* MenuAddString */

/*
 *
 */
MENU_ID
MenuAddColor (int x, int y, int w, const char *text, XBColor *color, XBRgbValue *pRgb)
{
  return MenuAdd (MenuCreateColor (x, y, w, text, color, pRgb) );
} /* MenuAddColor */

/*
 *
 */
MENU_ID
MenuAddKeysym (int x, int y, int w, const char *text, XBAtom *pKey)
{
  return MenuAdd (MenuCreateKeysym (x, y, w, text, pKey));
} /* MenuAddKeysym */

/*
 *
 */
MENU_ID
MenuAddCyclic (MIC_cyclic func, void *par)
{
  return MenuAdd (MenuCreateCyclic (func, par));
} /* MenuAddCyclic */

/*
 *
 */
MENU_ID
MenuAddInteger (int x, int y, int w_text, const char *text, int w, int *pValue, int min, int max)
{
  return MenuAdd (MenuCreateInteger (x, y, w_text, text, w, pValue, min, max));
} /* MenuAddInteger */

/*
 * name tag for players
 */
MENU_ID
MenuAddTag (int x, int y, int w, const char **pText)
{
  return MenuAdd (MenuCreateTag (x, y, w, pText));
} /* MenuAddTag */

/*******************************************
 * host items for server/client wait menus *
 *******************************************/

/*
 * host button (generic)
 */

MENU_ID
MenuAddHost (int x, int y, int w, unsigned client, const char **pText, XBHSFocusFunc focusFunc, XBHSChangeFunc chgFunc, XBHSUpdateFunc upFunc)
{
  return MenuAdd (MenuCreateHost (x, y, w, client, pText, focusFunc, chgFunc, upFunc) );
} /* MenuAddHost */

/*
 * host button (server)
 */
MENU_ID
MenuAddServer (int x, int y, int w, const char **pText)
{
  return MenuAdd (MenuCreateServer (x, y, w, pText) );
} /* MenuAddServer */

/*
 * host button (client)
 */
MENU_ID
MenuAddClient (int x, int y, int w, const char **pText, XBHostState *pState, const int *pPing)
{
  return MenuAdd (MenuCreateClient (x, y, w, pText, pState, pPing) );
} /* MenuAddClient */

/*
 * host button (peer)
 */
MENU_ID
MenuAddPeer (int x, int y, int w, const char **pText, XBHostState *pState, const int *pPing)
{
  return MenuAdd (MenuCreatePeer (x, y, w, pText, pState, pPing) );
} /* MenuAddPeer */

/*******************************************
 * team items for server/client wait menus *
 *******************************************/

/*
 * team button (generic)
 */
MENU_ID
MenuAddTeam (int x, int y, int w, unsigned id, unsigned player, XBTSFocusFunc focusFunc, XBTSChangeFunc chgFunc, XBTSUpdateFunc upFunc)
{
  return MenuAdd (MenuCreateTeam (x, y, w, id, player, focusFunc, chgFunc, upFunc) );
} /* MenuAddTeam */

/*
 * team button (server)
 */
MENU_ID
MenuAddServerTeam (int x, int y, int w, XBTeamState *pTeam)
{
  return MenuAdd (MenuCreateServerTeam (x, y, w, pTeam) );
} /* MenuAddServerTeam */

/*
 * team button (peeer)
 */
MENU_ID
MenuAddPeerTeam (int x, int y, int w, XBTeamState *pTeam)
{
  return MenuAdd (MenuCreatePeerTeam (x, y, w, pTeam) );
} /* MenuAddPeerTeam */



/*
 * add statitics header to menu
 */
MENU_ID
MenuAddStatHeader (int x, int y, int w, const char *title)
{
  return MenuAdd (MenuCreateStatHeader (x, y, w, title) );
} /* MenuAddHButton */

/*
 * add table entry to menu
 */
MENU_ID
MenuAddStatEntry (int x, int y, int w, const XBStatData *stat, MIC_button func, void *funcData)
{
  return MenuAdd (MenuCreateStatEntry (x, y, w, stat, func, funcData) );
} /* MenuAddHButton */

/*
 * add table entry to menu
 */
MENU_ID
MenuAddDemoEntry (int x, int y, int w, const CFGDemoEntry *demo, MIC_button func, void *funcData)
{
  return MenuAdd (MenuCreateDemoEntry (x, y, w, demo, func, funcData) );
} /* MenuAddHButton */

/*
 * add statitics header to menu
 */
MENU_ID
MenuAddDemoHeader (int x, int y, int w)
{
  return MenuAdd (MenuCreateDemoHeader (x, y, w) );
} /* MenuAddHButton */

/*
 * add statitics header to menu
 */
MENU_ID
MenuAddGameEntry (int x, int y, int w, const XBNetworkGame **game, MIC_button func)
{
  return MenuAdd (MenuCreateGameEntry (x, y, w, game, func) );
} /* MenuAddHButton */

/*
 * add statitics header to menu
 */
MENU_ID
MenuAddGameHeader (int x, int y, int w)
{
  return MenuAdd (MenuCreateGameHeader (x, y, w) );
} /* MenuAddHButton */

/*
 * XBCC add statitics header to menu
 */
MENU_ID
MenuAddCentralHeader (int x, int y, int w, const char *title)
{
  return MenuAdd (MenuCreateCentralHeader (x, y, w, title) );
} /* MenuAddHButton */

/*
 * XBCC add table entry to menu
 */
MENU_ID
MenuAddCentralEntry (int x, int y, int w, const XBCentralData *stat, MIC_button func, void *funcData)
{
  return MenuAdd (MenuCreateCentralEntry (x, y, w, stat, func, funcData) );
} /* MenuAddHButton */

/*
 * XBCC add info header to menu
 */
MENU_ID
MenuAddInfoHeader (int x, int y, int w, const char *title)
{
  return MenuAdd (MenuCreateInfoHeader (x, y, w, title) );
} /* MenuAddHButton */

/*
 * XBCC add table entry to menu
 */
MENU_ID
MenuAddInfoEntry (int x, int y, int w, const XBCentralInfo *stat, MIC_button func, void *funcData)
{
  return MenuAdd (MenuCreateInfoEntry (x, y, w, stat, func, funcData) );
} /* MenuAddHButton */



/*------------------------------------------------------------------------
 *
 * Event hanlding
 *
 *------------------------------------------------------------------------*/

/*
 * redraw routine for menu (used after timer event)
 */
void
MenuUpdateWindow (void)
{
  XBMenuItem *item;
  XBEventData eData;

  /* call poll routines for all objects */
  for (item = itemFirst; item != NULL; item = item->next) {
    if (NULL != item->poll) {
      (*item->poll) (item);
    }
  }
  /* shuffle sprites and mark them */
  ShuffleAllSprites ();
  /* set rectangles to be redrawn */
  SetRedrawRectangles ();
  /* shuffle sprites and mark them */
  MarkAllSprites ();
  /* update maze pixmap */
  UpdateMaze ();
  /* draw sprites into pixmap */
  DrawAllSprites ();
  /* fade in if neccessary */
  if (fadeFlag) {
    fadeFlag = XBFalse;
    /* inits */
    GUI_InitFade (XBFM_IN, PIXH+SCOREH);
    /* do it */
    while (GUI_DoFade ()) {
      while (XBE_TIMER != GUI_WaitEvent (&eData) ) continue;
    }
  }
  /* update window from pixmap */
  GUI_FlushPixmap (XBTrue);
  /* clear the redraw map */
  ClearRedrawMap();
} /* MenuUpdateWindow */

/*
 * move focus to next item in list
 */
static void
MoveFocus (XBMenuKey dir)
{
  if (itemFocus != NULL) {
    XBMenuItem *newFocus;
    switch (dir) {
      /* arrow keys */
    case XBMK_LEFT:  
      newFocus = itemFocus->left;  
      while (newFocus && newFocus->flags & MIF_DEACTIVATED) {
	newFocus = newFocus->left;
      }
      break;
    case XBMK_RIGHT: 
      newFocus = itemFocus->right; 
      while (newFocus && newFocus->flags & MIF_DEACTIVATED) {
	newFocus = newFocus->right;
      }
      break;
    case XBMK_UP:    
      newFocus = itemFocus->up;    
      while (newFocus && newFocus->flags & MIF_DEACTIVATED) {
	newFocus = newFocus->up;
      }
      break;
    case XBMK_DOWN:  
      newFocus = itemFocus->down;  
      while (newFocus && newFocus->flags & MIF_DEACTIVATED) {
	newFocus = newFocus->down;
      }
      break;
      /* next in list */
    case XBMK_NEXT:
      newFocus = itemFocus->next;
      while (newFocus != NULL) {
	if (newFocus->focus != NULL && 
	    ! (newFocus->flags & MIF_DEACTIVATED) ) {
	  break;
	}
	newFocus = newFocus->next;
      }
      break;
      /* previous in list */
    case XBMK_PREV:
      newFocus = itemFocus->prev;
      while (newFocus != NULL) {
	if (newFocus->focus != NULL && 
	    ! (newFocus->flags & MIF_DEACTIVATED) ) {
	  break;
	}
	newFocus = newFocus->prev;
      }
      break;
    default:         
      return;
    }
    if (NULL != newFocus) {
      itemFocus->flags &= ~MIF_FOCUS;
      newFocus->flags  |=  MIF_FOCUS;
      (*itemFocus->focus) (itemFocus, XBFalse);
      (*newFocus->focus) (newFocus, XBTrue);
      itemFocus = newFocus;
    }
  }
} /* MoveFocus */

/*
 * an menu item was selected (i.e Space was pressed)
 */
static void
SelectItem (XBMenuItem *item)
{
  if (item != NULL &&
      item->select != NULL) {
    (*item->select) (item);
  }
} /* SelectItem */

/*
 *
 */
static void
SetMousePosition (int x, int y)
{
  XBMenuItem *newItem = MenuGetMouseItem (x, y);
  /* set new focus if needed */
  if (NULL != newItem && 
      ! (newItem->flags & MIF_DEACTIVATED) &&
      newItem != itemFocus) {
    /* take back old focus */
    if (NULL != itemFocus) {
      itemFocus->flags &= ~MIF_FOCUS;
      assert (NULL != itemFocus->focus);
      (*itemFocus->focus) (itemFocus, XBFalse);
    }
    /* set new focus */
    itemFocus = newItem;
    itemFocus->flags |= MIF_FOCUS;
    assert (NULL != itemFocus->focus);
    (*itemFocus->focus) (itemFocus, XBTrue);
  }
} /* SetMousePosition */

/*
 *
 */
static void
MouseItem (XBEventCode button, int x, int y)
{
  if (itemFocus != NULL &&
      itemFocus == MenuGetMouseItem (x, y) &&
      itemFocus->mouse != NULL) {
    (*itemFocus->mouse) (itemFocus, button);
  }
} /* MouseItem */


static void
GetPlayerNames(int *players,int *localNumber){
  int player,i=0;
  CFGGamePlayers cfgGamePlayers;
  *localNumber=-1;
  Dbg_Out("Configuring names\n");
   if(GetIsServer()){  /* get local player data */
     *localNumber=0;
  if (RetrieveGamePlayers (CT_Local, atomServer, &cfgGamePlayers) ) {
    for (player = 0; player < cfgGamePlayers.num; player ++) {
     strcpy((char *) playerName[player][0] , GetPlayerName (CT_Local, cfgGamePlayers.player[player])); 
     Dbg_Out(" player Client %i %i  %i  #%s# \n",player-1,*localNumber,GetHostType (),playerName[player-1][0]);
    }
  } 
  
  }else{
     *localNumber=GetHostType ()-XBPH_Client1+1;
    for (player = 0,i=0; player < MAX_HOSTS; player ++) {
      if(ATOM_INVALID!=Network_GetPlayer(player,0)){
	if(GUI_AtomToString (Network_GetPlayer2(player,0)))
	  strcpy((char *)playerName[i++][0], GUI_AtomToString (Network_GetPlayer2(player,0)));
	Dbg_Out(" player Client %i %i  %i  #%s# \n",i,*localNumber,GetHostType (),playerName[i-1][0]);
      }
    }
  }
    *players=i;
}

/*
 * event handling for menus
 */
XBBool
MenuEventLoop (void)
{
  int result;
  XBEventCode event;
  XBEventData data;
  int recr=0;
  int chatLen=-1;
  char temp[40];
  char sendMsg[40];
  int local,localNumber=-1;

  /* load background graphics */
  MenuLoadTiles ();
  /* wait for kb event */
  GUI_SetTimer (FRAME_TIME, XBTrue);
  GUI_SetKeyboardMode (KB_MENU);
  GUI_SetMouseMode (XBTrue); 
  

//  playerAtom                    = Network_GetPlayer (clientID, player);
//  playerName[clientID][0]  = GetPlayerName (CT_Remote, playerAtom);

  
  /* event loop */
  while (1) {
 
    /* update window contents */
    MenuUpdateWindow ();
    /* exec any delayed functions */
    if (execFlag) {
      execFlag = XBFalse;
      result   = MenuExecButton ();
      /* no menu left start (or quit) game */
      if (NULL == itemFirst) {
	/* unload background graphics */
	MenuUnloadTiles ();
	return result;
      }
    }
    /* get event from gui */
    while (XBE_TIMER != (event = GUI_WaitEvent (&data) ) ) {
      //Dbg_Out("event %i value %i\n",event,data.value);
      switch (event) {
	/* Keyboard events */
      case  XBE_MENU:
	switch (data.value) {
	case XBMK_PREV:
	case XBMK_NEXT:
	case XBMK_LEFT:
	case XBMK_RIGHT:
	case XBMK_UP:   
	case XBMK_DOWN:    
	  MoveFocus (data.value);  
	  break;
	case XBMK_SELECT:  
	  SelectItem (itemFocus);          
	  break;
	case XBMK_ABORT:   
	  SelectItem (abortItem);          
	  break;
	case XBMK_DEFAULT: 
	  SelectItem (defaultItem);          
	  break;
	  case XBMK_STARTCHAT:
	    GUI_SetKeyboardMode (KB_ASCII);
	    chatMode=1;
	    chatLen=0;
	    memset(temp,0,CHAT_LEN-1);
	    temp[CHAT_LEN-1]=(char)0;
	    //	    Dbg_Out("chat %i\n",chatMode);
	    SetGet("Start Chatting",XBTrue);
	    break;
	  case XBMK_SENDCHAT:
	  if(chatMode==1){
	    GUI_SetKeyboardMode (KB_MENU);
	    chatMode=2;  
	    GetPlayerNames(&local,&localNumber);
	    if(localNumber)
	    fprintf(stderr,"Sendind msg from %i %s \n",localNumber,playerName[localNumber][0]);
	    else
	      Dbg_Out("Config error!\n");
	    sprintf(sendMsg,"%s: %s",playerName[localNumber][0],temp);
	    Client_SendChat(0,recr,0,255,
		      sendMsg);
	    Server_SendChat(0,recr,0,255,
		      sendMsg);
	    SetChat(sendMsg,XBTrue);
	    chatMode=0;
	    //	    Dbg_Out("chat %i to %s\n",chatMode);
	  }
	    break;
	  case XBMK_CANCELCHAT:
	    GUI_SetKeyboardMode (KB_MENU);
	    SetGet("Canceling Chat",XBTrue);
	   
	    chatMode=0;
	    //	    Dbg_Out("chat %i\n",chatMode);
	    break; 
	case XBMK_CHANGECHATMODE:
	  recr++;
	  SetGet("Changing to blA",XBTrue);
	  //	    Dbg_Out("chat %i\n",recr++); 
	    break; 
	  
	default:
	  break;
	}
	break;
	/* mouse button events */
      case XBE_CTRL:
	switch(data.value) {
	case XBCK_BACKSPACE:
	  temp[--chatLen]=0;
	  SetGet(temp,XBTrue);
	  break;
	case XBCK_ESCAPE :
	  SelectItem (abortItem);          
	  break;
	case XBCK_RETURN: 
	  if(chatMode==1){
	  GUI_SetKeyboardMode (KB_MENU);
	    chatMode=2; 
	    GetPlayerNames(&local,&localNumber);
	    //	    temp[chatLen]=;
	    if(localNumber)
	    fprintf(stderr,"Sendind msg from %i %s \n",localNumber,playerName[localNumber][0]);
	    else
	      Dbg_Out("Config error!\n");
	    //  fprintf(stderr,"Sendind msg from %s %i\n",playerName[localNumber][0],localNumber);
	    sprintf(sendMsg,"%s: %s",playerName[localNumber][0],temp);
	    Client_SendChat(0,0,0,255,
		      sendMsg);
	    Server_SendChat(0,0,0,255,
		      sendMsg);
	    SetChat(sendMsg,XBTrue);
	    chatMode=0;
	  }
	  break;
	case XBCK_INSERT :
	    GUI_SetKeyboardMode (KB_ASCII);
	    chatMode=1;
	    chatLen=0;
	    memset(temp,0,CHAT_LEN-1);
	    temp[CHAT_LEN-1]=(char)0;
	    //	    Dbg_Out("chat %i\n",chatMode);
	    SetGet("Start Chatting",XBTrue);
	  break;
	case XBCK_END:
	  if(chatMode==1){
  GUI_SetKeyboardMode (KB_MENU);
	    chatMode=2; 
	    GetPlayerNames(&local,&localNumber);
	    //	    temp[chatLen]=;
	    if(localNumber)
	    fprintf(stderr,"Sendind msg from %i %s \n",localNumber,playerName[localNumber][0]);
	    else
	      Dbg_Out("Config error!\n");
	    //  fprintf(stderr,"Sendind msg from %s %i\n",playerName[localNumber][0],localNumber);
	    sprintf(sendMsg,"%s: %s",playerName[localNumber][0],temp);
	    Client_SendChat(0,0,0,255,
		      sendMsg);
	    Server_SendChat(0,0,0,255,
		      sendMsg);
	    SetChat(sendMsg,XBTrue);
	    chatMode=0;
	  }
	    //	    Dbg_Out("chat %i %s\n",chatMode,temp);
	  break;
	case XBCK_DELETE:
  GUI_SetKeyboardMode (KB_MENU);
	    SetGet("Canceling Chat",XBTrue);
	    chatMode=0;
	    //	    Dbg_Out("chat %i\n",chatMode);
	  break;
	case XBCK_HOME:	 
	  recr++;
	  //	    Dbg_Out("chat %i\n",recr++);
	  SetGet("Changing to blA",XBTrue); 
	  break;
	}
	break;
      case XBE_ASCII:
	//g_Out("chat ascii %c\n",data.value);
	if(chatMode==1){
	  if(data.value==8){

	  temp[--chatLen]=0;
	      SetGet(temp,XBTrue);

	  }else{
	    if(chatLen<CHAT_LEN-strlen(playerName[localNumber][0])-2){
	      temp[chatLen++]=data.value;
	      SetGet(temp,XBTrue);
	    }
	    else{
	      chatMode=2;
	      temp[CHAT_LEN-1]=(char)0;
	    sprintf(sendMsg,"%s: %s",playerName[localNumber][0],temp);
	      SetChat(sendMsg,XBTrue);
	    Client_SendChat(0,recr,0,255,
		      sendMsg);
	    Server_SendChat(0,recr,0,255,
		      sendMsg);
	      chatLen=0;
	      memset(temp,0,CHAT_LEN-1);
	      temp[CHAT_LEN-1]=(char)0;
	      chatMode=1;
	      temp[chatLen++]=data.value;
	    }
	  }
	    
	    
	}
	break;
      case XBE_RMOUSE_2:
	SetOldBlock();
      case XBE_RMOUSE_1:
      case XBE_RMOUSE_3:
	if(mapMode){
	  setPressed=0;
	  fprintf(stderr," Setting to 0 pressed\n");
	  
	}
	break;
      
      case XBE_MOUSE_2:
	SetToBlockFree();
      case XBE_MOUSE_1:
      case XBE_MOUSE_3:
	if(mapMode){
	  setPressed=1;
	  fprintf(stderr," %i %i \n",data.pos.x, data.pos.y);
	  
	  SetEditMapBlock(data.pos.x,data.pos.y);
	}
   	SetMousePosition (data.pos.x, data.pos.y);
	MouseItem (event, data.pos.x, data.pos.y);
	break;
	/* mouse motion event */
      case XBE_MOUSE_MOVE:	
	if(mapMode){
	  if(setPressed)
	  SetEditMapBlock(data.pos.x,data.pos.y);
	}
	SetMousePosition (data.pos.x, data.pos.y);
	UpdateMaze();
	break;
      default:
	continue;
      }
      result = MenuExecButton ();
      /* no menu left start (or quit) game */
      if (NULL == itemFirst) {
	/* unload background graphics */
	MenuUnloadTiles ();
	return result;
      }
    }
  }
  /* unload background graphics */
  MenuUnloadTiles ();
  return XBTrue;
} /* MenuEventLoop */
/*
 * set default item
 */
void
MenuDeleteItemById (MENU_ID id)
{
  XBMenuItem *itemTemp; 
  XBMenuItem *item_next;
  itemTemp = FindItem (id);
  if(itemTemp==NULL)return;
  fprintf(stderr,"deletint id %i next %p\n",id,itemTemp->next);
  item_next=itemTemp->next;
  DeleteMenuItem(itemTemp);
  itemTemp=item_next;
} /* MenuSetDefault */

/*
 * end of file mi_tool.c
 */ 
