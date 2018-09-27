/*
 * file mi_tool.h
 *
 * $Id: mi_tool.h,v 1.14 2004/10/11 14:01:55 lodott Exp $
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
#ifndef XBLAST_MI_TOOL_H
#define XBLAST_MI_TOOL_H

#include "xblast.h"
#include "cfg_demo.h"
#include "cfg_player.h"
#include "cfg_stat.h"
#include "dat_rating.h"
#include "client.h"

/*
 * macros
 */
#define CELL_W 8
#define CELL_H 8
#define CELL_MAX_X (PIXW/CELL_W)

/*
 * type defintions
 */
/* unique identifier for each created menu item */
typedef unsigned MENU_ID;
/* callback function for buttons */
typedef XBBool (*MIC_button) (void *);
typedef void   (*MIC_cyclic) (void *);
/* data input for combo items */
typedef struct {
  const char *text;
  int         value;
  void       *data;
  XBAtom      atom;
} XBComboEntryList;
/* rgb storage for color editors */
typedef struct {
  int red;
  int green;
  int blue;
} XBRgbValue;
/* host item handler */
typedef void (* XBHSFocusFunc) (unsigned);
typedef XBBool (* XBHSChangeFunc) (unsigned, XBHostState *);
typedef XBBool (* XBHSUpdateFunc) (unsigned, XBHostState *, XBHostState *, int *);
/* team item handler */
typedef void (* XBTSFocusFunc) (unsigned, unsigned);
typedef XBBool (* XBTSChangeFunc) (unsigned, unsigned, XBTeamState *);
typedef XBBool (* XBTSUpdateFunc) (unsigned, unsigned, XBTeamState *, XBTeamState *);

/*
 * global prototypes
 */
extern void MenuClear (void);
extern void MenuFocus (MENU_ID);
extern void MenuSetAbort (MENU_ID);
extern void MenuSetDefault (MENU_ID);
extern void MenuExecFunc (MIC_button func, void *data);
extern void MenuSetActive (MENU_ID, XBBool active);
void MenuDeleteItemById (MENU_ID id);
extern void MenuSetLinks (void);
extern void MenuUpdateWindow (void);
extern MENU_ID MenuGetFocus (void);
extern XBBool MenuEventLoop (void);

extern MENU_ID MenuAddHButton (int x, int y, int w, const char *text, MIC_button func, void *funcData);
extern MENU_ID MenuAddVButton (int x, int y, int h, const char *text, MIC_button func, void *funcData);
extern MENU_ID MenuAddToggle (int x, int y, int w, const char *text, XBBool *pState);
extern MENU_ID MenuAddLabel (int x, int y, int w, const char *text);
extern MENU_ID MenuAddLabel1 (int x, int y, int w, const char *text);
extern MENU_ID MenuAddLabel2 (int x, int y, int w, const char *text);
extern MENU_ID MenuAddComboInt (int x, int y, int w_text, const char *text, int w,
				int *value, XBComboEntryList *table);
extern MENU_ID MenuAddComboBool (int x, int y, int w_text, const char *text, int w, XBBool *value);
extern MENU_ID MenuAddComboData (int x, int y, int w_text, const char *text, int w,
				 void **value, XBComboEntryList *table);
extern MENU_ID MenuAddComboAtom (int x, int y, int w_text, const char *text, int w,
				 XBAtom *value, XBComboEntryList *table);
extern MENU_ID MenuAddCombo (int x, int y, int w_text, const char *text, int w,
			     int *value, void **data, XBAtom *atom, 
			     XBComboEntryList *table);
extern MENU_ID MenuAddPlayer (int x, int y, int w, int sprite, const CFGPlayerGraphics **cfg,
			      int n_anime, BMSpriteAnimation *anime);
extern MENU_ID MenuAddString (int x, int y, int w_text, const char *text, int w, char *buffer, size_t len);
extern MENU_ID MenuAddColor (int x, int y, int w, const char *text, XBColor *color, XBRgbValue *pRgb);
extern MENU_ID MenuAddKeysym (int x, int y, int w, const char *text, XBAtom *pKey);
extern MENU_ID MenuAddCyclic (MIC_cyclic func, void *par);
extern MENU_ID MenuAddInteger (int x, int y, int w_text, const char *text, int w, int *pValue, int min, int max);
extern MENU_ID MenuAddTag (int x, int y, int w, const char **pText);

/* host items for server/client wait menus */
extern MENU_ID MenuAddHost (int x, int y, int w, unsigned client, const char **pText, XBHSFocusFunc focusFunc, XBHSChangeFunc chgFunc, XBHSUpdateFunc upFunc);
extern MENU_ID MenuAddServer (int x, int y, int w, const char **pText);
extern MENU_ID MenuAddClient (int x, int y, int w, const char **pText, XBHostState *pState, const int *pPing);
extern MENU_ID MenuAddPeer   (int x, int y, int w, const char **pText, XBHostState *pState, const int *pPing);

/* team items for server/client wait menus */
extern MENU_ID MenuAddTeam (int x, int y, int w, unsigned id, unsigned player, XBTSFocusFunc focusFunc, XBTSChangeFunc chgFunc, XBTSUpdateFunc upFunc);
extern MENU_ID MenuAddServerTeam (int x, int y, int w, XBTeamState *pTeam);
extern MENU_ID MenuAddPeerTeam (int x, int y, int w, XBTeamState *pTeam);

extern MENU_ID MenuAddStatHeader (int x, int y, int w, const char *title);
extern MENU_ID MenuAddStatEntry (int x, int y, int w, const XBStatData *pStat, MIC_button func, void *funcData);
extern MENU_ID MenuAddDemoEntry (int x, int y, int w, const CFGDemoEntry *pDemo, MIC_button func, void *funcData);
extern MENU_ID MenuAddDemoHeader (int x, int y, int w);
extern MENU_ID MenuAddGameEntry (int x, int y, int w, const XBNetworkGame **, MIC_button func);
extern MENU_ID MenuAddGameHeader (int x, int y, int w);

extern MENU_ID MenuAddCentralHeader (int x, int y, int w, const char *title); // XBCC
extern MENU_ID MenuAddCentralEntry (int x, int y, int w, const XBCentralData *stat, MIC_button func, void *funcData); // XBCC

extern MENU_ID MenuAddInfoHeader (int x, int y, int w, const char *title); // XBCC
extern MENU_ID MenuAddInfoEntry (int x, int y, int w, const XBCentralInfo *stat, MIC_button func, void *funcData); // XBCC

extern void SetPressed(XBBool mode);
extern XBBool GetPressed(void);
extern void SetXBEditMapMode(XBBool mode);
extern XBBool GetXBEditMapMode(void);
extern int GetChatMode();

#endif
/*
 * end of file mi_tool.h
 */
