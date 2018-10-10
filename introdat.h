/*
 * file introdat.h - animation and layouts for intro and inbetween screens
 *
 * $Id: introdat.h,v 1.3 2004/05/14 10:00:34 alfie Exp $
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
#ifndef XBLAST_INTRODAT_H
#define XBLAST_INTRODAT_H

#include "geom.h"
#include "cfg_level.h"
#include "map.h"

/*
 * type definitions
 */
typedef struct {
  const char *text;
  unsigned    flags;
  BMRectangle rect;
} IntroTextBox;

/*
 * constants
 */
#define INTRO_LENGTH           35

#define SIZE_OF_X              16
#define CHAR_ANIME              7

#define NUM_WINNER_ANIME        8 
#define NUM_LOOSER_ANIME       10
#define NUM_OTHER_WINNER_ANIME 40
#define NUM_OTHER_LOOSER_ANIME 42
#define NUM_LAOLA_ANIME        32

/*
 * global variables
 */

/* text for first screen */
extern const IntroTextBox introBox[];
/* text with copyright notice */
extern const IntroTextBox creditsBox[];
/* Big X for intro screen */
extern const BMPoint pointx[SIZE_OF_X];
/* explosion data for letter animations */
extern const int blockB[CHAR_ANIME][CHARH][CHARW];
extern const int blockL[CHAR_ANIME][CHARH][CHARW];
extern const int blockA[CHAR_ANIME][CHARH][CHARW];
extern const int blockS[CHAR_ANIME][CHARH][CHARW];
extern const int blockT[CHAR_ANIME][CHARH][CHARW];

/* title box for level intro */
extern IntroTextBox titleBox [];
extern IntroTextBox playerInfoBox[];
extern IntroTextBox levelInfoBox[];
extern IntroTextBox extraInfoBox[];

/* level data for scoreboard */
extern const XBScoreGraphics graphicsScoreBoard;
extern XBScoreMap            mapScoreBoard;
/* level data for load sprite screen */
extern const XBScoreGraphics  graphicsLoadSprite;
extern const XBScoreMap mapLoadSprite;

/* audience for score board */
extern const BMSpriteAnimation winnerAnime[NUM_WINNER_ANIME];
extern const BMSpriteAnimation looserAnime[NUM_LOOSER_ANIME];
extern const BMSpriteAnimation otherWinnerAnime[NUM_OTHER_WINNER_ANIME];
extern const BMSpriteAnimation otherLooserAnime[NUM_OTHER_LOOSER_ANIME];
extern const BMSpriteAnimation laOlaAnime[NUM_LAOLA_ANIME];

#endif
/*
 * end of file intro_dat.h
 */
