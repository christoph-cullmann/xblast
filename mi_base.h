/*
 * file mi_base.h
 *
 * $Id: mi_base.h,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#ifndef _MI_BASE_H
#define _MI_BASE_H

#include "mi_tool.h"
#include "geom.h"

/*
 * global macros
 */
#define MIF_SELECTED    0x01
#define MIF_FOCUS       0x02
#define MIF_DEACTIVATED 0x04

/* known menu item types */
typedef enum {
  MIT_Button,
  MIT_Toggle, 
  MIT_Label,
  MIT_Combo,
  MIT_Player,
  MIT_String,
  MIT_Color,
  MIT_Keysym, 
  MIT_Cyclic,
  MIT_Integer,
  MIT_Host,
  MIT_Tag,
  MIT_Table,
  MIT_Team
} XBMenuItemType;


struct _menu_item;

/* function pointers for item specific behaviour */
typedef void (*MIC_focus)  (struct _menu_item *, XBBool);
typedef void (*MIC_select) (struct _menu_item *);
typedef void (*MIC_mouse)  (struct _menu_item *, XBEventCode);
typedef void (*MIC_poll)   (struct _menu_item *);

/* generic item data */
typedef struct _menu_item {
  XBMenuItemType     type;
  MENU_ID            id;
  struct _menu_item *next;
  struct _menu_item *prev;
  struct _menu_item *left;
  struct _menu_item *right;
  struct _menu_item *up;
  struct _menu_item *down;
  int                x;
  int                y;
  int                w;
  int                h;
  unsigned           flags;
  MIC_focus          focus;
  MIC_select         select;
  MIC_mouse          mouse;
  MIC_poll           poll;
} XBMenuItem;

/*
 * prototypes
 */
extern void MenuResetBase (void);
extern void MenuSetItem (XBMenuItem *item, XBMenuItemType type, int x, int y, int w, int h, 
			 MIC_focus focus, MIC_select select, MIC_mouse mouse, MIC_poll poll);
extern XBMenuItem *MenuFindLeftItem (const XBMenuItem *item);
extern XBMenuItem *MenuFindRightItem (const XBMenuItem *item);
extern XBMenuItem *MenuFindUpperItem (const XBMenuItem *item);
extern XBMenuItem *MenuFindLowerItem (const XBMenuItem *item);
extern XBMenuItem *MenuGetMouseItem (int x, int y);

#endif
/*
 * end of file mi_base.h
 */
