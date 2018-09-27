/*
 * file mi_base.c - base functions for all menu items
 * 
 * $Id: mi_base.c,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#include "mi_base.h"

/*
 * local constants
 */
#define MENU_COLS  ((PIXW+BASE_X-1)/BASE_X)
#define MENU_ROWS ((PIXH+SCOREH+BASE_Y - 1)/BASE_Y)

/*
 * local variables
 */
static MENU_ID currentID = 0;
static XBMenuItem *mouseMap[MENU_COLS][MENU_ROWS];

/*
 * reset id counter
 */
void
MenuResetBase (void)
{
  currentID = 0;
  memset (mouseMap, 0, sizeof (mouseMap));
} /* MenuResetId */

/*
 * set value for generic menu_item
 */
void
MenuSetItem (XBMenuItem *ptr, XBMenuItemType type, int x, int y, int w, int h, 
		MIC_focus focus, MIC_select select, MIC_mouse mouse, MIC_poll poll)
{
  int xPos, yPos;

  assert (ptr != NULL);
  /* fill structure */
  ptr->type   = type;
  ptr->id     = ++ currentID;
  ptr->x      = x;
  ptr->y      = y;
  ptr->w      = w;
  ptr->h      = h;
  ptr->focus  = focus;
  ptr->select = select;
  ptr->mouse  = mouse;
  ptr->poll   = poll;
  /* set mouse mapping */
  if (NULL != ptr->mouse) {
    for (xPos = x; xPos < x + w; xPos ++) {
      for (yPos = y; yPos < y + h; yPos ++) {
	assert (xPos < MENU_COLS);
	assert (yPos < MENU_ROWS);
	mouseMap[xPos][yPos] = ptr;
      }
    }
  }
} /* menu_create_item */

/*
 * find left neighbour of menu item
 */
XBMenuItem *
MenuFindLeftItem (const XBMenuItem *item)
{
  int x, y;

  assert (item != NULL);
  assert (item == mouseMap[item->x][item->y]);

  x = item->x;
  while (1) {
    x --;
    if (x < 0) {
      x = MENU_COLS - 1;
    }
    for (y = item->y; y < item->y + item->h; y ++) {
      if (NULL != mouseMap[x][y]) {
	return mouseMap[x][y];
      }
    }
  }
  return NULL;
} /* FindLeftItem */

/*
 * find left neighbour of menu item
 */
XBMenuItem *
MenuFindRightItem (const XBMenuItem *item)
{
  int x, y;

  assert (item != NULL);
  assert (item == mouseMap[item->x][item->y]);

  x = item->x + item->w;
  while (1) {
    if (x >= MENU_COLS) {
      x = 0;
    }
    for (y = item->y; y < item->y + item->h; y ++) {
      if (NULL != mouseMap[x][y]) {
	return mouseMap[x][y];
      }
    }
    x ++;
  }
  return NULL;
} /* FindRightItem */

/*
 * find left neighbour of menu item
 */
XBMenuItem *
MenuFindUpperItem (const XBMenuItem *item)
{
  int x, y;

  assert (item != NULL);
  assert (item == mouseMap[item->x][item->y]);

  y = item->y;
  while (1) {
    y --;
    if (y < 0) {
      y = MENU_ROWS - 1;
    }
    for (x = item->x; x < item->x + item->w; x ++) {
      if (NULL != mouseMap[x][y]) {
	return mouseMap[x][y];
      }
    }
  }
  return NULL;
} /* FindUpperItem */

/*
 * find left neighbour of menu item
 */
XBMenuItem *
MenuFindLowerItem (const XBMenuItem *item)
{
  int x, y;

  assert (item != NULL);
  assert (item == mouseMap[item->x][item->y]);

  y = item->y + item->h;
  while (1) {
    if (y >= MENU_ROWS) {
      y = 0;
    }
    for (x = item->x; x < item->x + item->w; x ++) {
      if (NULL != mouseMap[x][y]) {
	return mouseMap[x][y];
      }
    }
    y ++;
  }
  return NULL;
} /* FindLowerItem */

/*
 *
 */
XBMenuItem *
MenuGetMouseItem (int x, int y)
{
  /* on which item are we */
  /*
  assert (x >= 0);
  assert (x < MENU_COLS);
  assert (y >= 0);
  assert (y < MENU_ROWS);
  return mouseMap[x][y];
  */
  /* CHANGED TO MAKE X-BRESSE HAPPY */
  if((x >= 0) && (x < MENU_COLS) && (y >= 0) && (y < MENU_ROWS)) {
    return mouseMap[x][y];
  } else {
    return NULL;
  }} /* SetMousePosition */

/*
 * end of file mi_base.c
 */
