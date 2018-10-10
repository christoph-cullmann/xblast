/*
 * file xblast.h - common macros, constants ansd types
 *
 * $Id: xblast.h,v 1.6 2004/11/08 19:59:08 lodott Exp $
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
 *
 */
#ifndef XBLAST_XBLAST_H
#define XBLAST_XBLAST_H

#include "common.h"

/*
 * macros
 */
#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) ( (a)>=(b) ? (a) : (b) )

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) ( (a)<=(b) ? (a) : (b) )

#ifdef ABS
#undef ABS
#endif
#define ABS(a)   ( (a)>=0 ? (a) : (-(a)) )

/* how may player are maximum */
#ifdef SMPF
#define MAX_PLAYER 16
#else
#define MAX_PLAYER 6
#endif

/* how many local players */
#define NUM_LOCAL_PLAYER 6
/* how many client can connect */
#define MAX_HOSTS  MAX_PLAYER

/* maximum allowed number of victories to win */
#define MAX_VICTORIES 9

/* dimension of of maze */
#define MAZE_W 15
#define MAZE_H 13
#define STAT_W 20
#ifdef SMPF
#define STAT_H 3
#else
#define STAT_H 2
#endif

/* number block tiles */
#define MAX_BLOCK     11
#define MENU_MAX_TILE 21
#define MAX_TILE      MAX(MAX_BLOCK,MENU_MAX_TILE)

/* number of bombs and their animations */
#define MAX_BOMBS 2
#define MAX_BOMB_ANIME 17

/* size of big letters */
#define CHARW 3
#define CHARH 5

#define MAX_EXPLOSION 16

#define TIME_STEP   48
#define DEAD_TIME    8
#define GAME_TIME   (60*TIME_STEP + DEAD_TIME)

/* Added by Fouf on 09/14/99 23:55:18 */ /* Written by Amilhastre */
#define BOMB_SEARCH_X  (2.5*BLOCK_WIDTH)
#define BOMB_SEARCH_Y  (2.5*BLOCK_HEIGHT)

/* length of a frame in ms */
#define FRAME_TIME    50

/*
 * fundamental types
 */
typedef enum {
  XBFalse = 0, XBTrue
} XBBool;

typedef struct {
  double x,y;
} BMPoint;

typedef struct {
  int x, y;
  int w, h;
} BMRectangle;

/*
 *  position vector
 */
typedef struct {
  short y,x;
} BMPosition;

/*
 *  PFV Pointer to void/int functions
 */
typedef void (*PFV)();
typedef int (*PFI)();
typedef BMRectangle *(*PFR)();

/*
 * player sprite animations phase
 */
typedef enum {
  SpriteStopDown = 0,
  SpriteWalkDown0, SpriteWalkDown1, SpriteWalkDown2, SpriteWalkDown3,
  SpriteStopUp,
  SpriteWalkUp0, SpriteWalkUp1, SpriteWalkUp2, SpriteWalkUp3,
  SpriteStopRight,
  SpriteWalkRight0, SpriteWalkRight1, SpriteWalkRight2, SpriteWalkRight3,
  SpriteStopLeft,
  SpriteWalkLeft0, SpriteWalkLeft1, SpriteWalkLeft2, SpriteWalkLeft3,
  SpriteDamagedDown, SpriteDamagedLeft, SpriteDamagedUp, SpriteDamagedRight,
  SpriteLooser, SpriteLooser1, SpriteLooser2,
  SpriteWinner, SpriteWinner2, SpriteWinner3,
  SpriteBigWinner,
  SpriteDeadDown, SpriteDeadLeft, SpriteDeadUp, SpriteDeadRight,
  SpriteMorphed,
 SpriteZombie,
  MAX_ANIME
} BMSpriteAnimation;

#define MAX_ANIME_EPM ((int) SpriteDeadDown)
#define MAX_ANIME_PPM ((int) (MAX_ANIME - MAX_ANIME_EPM))

/*
 * player directions
 */
typedef enum {
  GoStop = 0, GoUp, GoLeft, GoDown, GoRight, GoDefault, GoAll,
  MAX_DIR
} BMDirection;

/*
 * ???
 */
typedef enum {
 SBVoid = 0,
 SBTextLeft, SBTextMid, SBTextRight,
 SBDead=4,
 SBSick=4+MAX_PLAYER,
 SBPlayer=4+2*MAX_PLAYER,
 SBAbort=4+3*MAX_PLAYER,
 SBSickAbort=4+4*MAX_PLAYER,
 MAX_SCORE_TILES=4+5*MAX_PLAYER
} BMScoreTile;

#endif
/*
 * end of file xblast.h
 */
