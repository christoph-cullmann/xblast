/*
 * file mi_map.c - background for menus
 *
 * $Id: mi_map.c,v 1.5 2004/08/21 16:04:24 iskywalker Exp $
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
#include "mi_map.h"

#include "map.h"
#include "status.h"

#include <string.h>

/*
 * local variables
 */
/* graphics */
static BMBlockTile menuBlockTile[MENU_MAX_TILE] = {
  /*  0 */ { "score_floor",      COLOR_BLACK, COLOR_SPRING_GREEN,     COLOR_BLACK },
  /*  1 */ { "score_right_down", COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_BLACK },
  /*  2 */ { "menu_left_up",     COLOR_BLACK, COLOR_LIGHT_STEEL_BLUE, COLOR_SPRING_GREEN},
  /*  3 */ { "menu_right_up",    COLOR_BLACK, COLOR_LIGHT_STEEL_BLUE, COLOR_SPRING_GREEN},
  /*  4 */ { "score_pipe_end",   COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_SPRING_GREEN}, 
  /*  5 */ { "menu_left_down",   COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_STEEL_BLUE },
  /*  6 */ { "menu_right_down",  COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_STEEL_BLUE },
  /*  7 */ { "score_right_up",   COLOR_BLACK, COLOR_LIGHT_STEEL_BLUE, COLOR_BLACK},
  /*  8 */ { "score_pipe_mid",   COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_BLACK },
  /*  9 */ { "score_pipe_begin", COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_BLACK },
  /* 10 */ { "menu_left",        COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 11 */ { "menu_center",      COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 12 */ { "menu_right",       COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 13 */ { "menu_list_center", COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 14 */ { "menu_list_join",   COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 15 */ { "menu_list_left",   COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 16 */ { "menu_list_right",  COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 17 */ { "menu_join",        COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 18 */ { "menu_top",         COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 19 */ { "menu_vertical",    COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
  /* 20 */ { "menu_bottom",      COLOR_BLACK, COLOR_FIRE_BRICK_1,     COLOR_LIGHT_GOLDENROD },
};
/* default layout for menu */
static int map[MAZE_H+1][MAZE_W];
static int defaultMap[MAZE_H][MAZE_W] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
  { 0, 0, 0, 0, 2, 7, 7, 7, 7, 7, 3, 0, 0, 0, 0, },
  { 7, 7, 7, 7, 5, 1, 1, 1, 1, 1, 6, 7, 7, 7, 7, },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
  { 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, },
  { 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, },
};

/*
 * load graphics for menu
 */
void 
MenuLoadTiles (void)
{
  int i;

  for (i = 0; i < MENU_MAX_TILE; i ++) {
    GUI_LoadBlockCch (i, menuBlockTile[i].name, menuBlockTile[i].fg,
		      menuBlockTile[i].bg, menuBlockTile[i].add);
  }
} /* MenuUnloadTiles */

/*
 * unload graphics for menu
 */
void 
MenuUnloadTiles (void)
{
  int i;

  for (i = 0; i < MENU_MAX_TILE; i ++) {
    GUI_FreeBlock (i);
  }
} /* MenuUnloadTiles */

/*
 * clear current map for menus
 */ 
void 
MenuClearMap (void)
{
  int x, y;

  /* copy default
  for(y=0;y<MAZE_H+2;y++)
  memcpy (map+y*MAZE_W+1, defaultMap+MAZE_H+1,MAZE_W );
 */
  memcpy (map, defaultMap, sizeof (defaultMap));
  
  /* draw it */
  for (x = 0; x < MAZE_W; x ++) {
    for (y = 0; y < MAZE_H; y ++) {
      SetMazeBlock (x, y, map[y][x]);
    }
  }
  ClearStatusBar (4, BTFree);
} /* MenuClearMap */

/*
 * add large frame for object
 */
void
MenuAddLargeFrame (int left, int right, int row)
{
  int x;

  /* sanity check */
  assert (left  < right);
  assert (left  < MAZE_W);
  assert (right < MAZE_W);
  assert (row   < MAZE_H+1);
  /* left side */
  if (map[row][left] == 12) {
    map[row][left] = 17;
  } else {
    map[row][left] = 10;
  }
  /* right side */
  if (map[row][right] == 10) {
    map[row][right] = 17;
  } else {
    map[row][right] = 12;
  }
  /* middle */
  for (x = left + 1; x < right; x ++) {
    map[row][x] = 11;
  }
  /* draw it */
  for (x = left; x <= right; x ++) {
    SetMazeBlock (x, row, map[row][x]);
  }
} /* MenuAddLargeFrame */

/*
 * add small frame for object
 */
void
MenuAddSmallFrame (int left, int right, int row)
{
  int x;

  /* sanity check */
  assert (left  < right);
  assert (left  < MAZE_W);
  assert (right < MAZE_W);
  assert (row   < MAZE_H);
  /* left side */
  if (map[row][left] == 16) {
    map[row][left] = 14;
  } else if (map[row][left] != 14) {
    map[row][left] = 15;
  }
  /* right side */
  if (map[row][right] == 15) {
    map[row][right] = 14;
  } else if (map[row][right] != 14) {
    map[row][right] = 16;
  }
  /* middle */
  for (x = left + 1; x < right; x ++) {
    map[row][x] = 13;
  }
  /* draw it */
  for (x = left; x <= right; x ++) {
    SetMazeBlock (x, row, map[row][x]);
  }
} /* MenuAddSmallFrame */

/*
 * vertical frame
 */
void
MenuAddVerticalFrame (int col, int top, int bottom)
{
  int y;

  /* sanity check */
  assert (top    < bottom);
  assert (top    < MAZE_H);
  assert (bottom < MAZE_H);
  assert (col    < MAZE_W);
  /* set new tiles */
  map[top][col]    = 18;
  map[bottom][col] = 20;
  for (y = top + 1; y < bottom; y ++) {
    map[y][col] = 19;
  }
  /* correct pipes */
  if (bottom + 1 < MAZE_H &&
      map[bottom + 1][col] == 8) {
    map[++ bottom][col] = 9;
  }
  /* draw it */
  for (y = top; y <= bottom; y ++) {
    SetMazeBlock (col, y, map[y][col]);
  }
} /* MenuAddVerticalFrame */


/*
 * end of file mi_map.c
 */
