/*
 * file introdat.c - animation and layouts for intro and inbetween screens
 *
 * $Id: introdat.c,v 1.10 2006/06/12 10:54:23 fzago Exp $
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

#include "xblast.h"

/*
 * text for first screen
 */
const IntroTextBox introBox[] = {
	{
	 N_("On a Workstation not far away"),
	 FF_Large | FF_Black | FF_Outlined,
	 {0, 0, PIXW, BLOCK_HEIGHT},
	 },
	{
	 N_("Press Space or Return"),
	 FF_Large | FF_White | FF_Boxed,
	 {4 * BLOCK_WIDTH, PIXH + 30, 7 * BLOCK_WIDTH, 2 * BLOCK_HEIGHT / 3,},
	 },
	/* end of array */
	{
	 NULL,
	 0,
	 {0, 0, 0, 0},
	 },
};

/*
 * Coypright notice for first screen 
 */
const IntroTextBox creditsBox[] = {
	{
	 "",
	 FF_Large | FF_Boxed | FF_Black,
	 {35 * BASE_X, 38 * BASE_Y, 50 * BASE_X, 44 * BASE_Y,},
	 },
	{
	 "",
	 FF_Large | FF_Boxed | FF_White,
	 {36 * BASE_X, 39 * BASE_Y, 48 * BASE_X, 42 * BASE_Y,},
	 },
	{
	 "XBlast TNT " VERSION_STRING,
	 FF_Large | FF_White,
	 {36 * BASE_X, 39 * BASE_Y, 48 * BASE_X, 8 * BASE_Y,},
	 },
	{
	 "Copyright \251 " COPYRIGHT_YEAR " Oliver Vogel",
	 FF_Medium | FF_White,
	 {36 * BASE_X, 47 * BASE_Y, 48 * BASE_X, 6 * BASE_Y,},
	 },
	{
	 "(m.vogel@ndh.net)",
	 FF_Small | FF_White,
	 {36 * BASE_X, 53 * BASE_Y, 48 * BASE_X, 4 * BASE_Y,},
	 },
	{
	 N_("Coauthor Garth Denley"),
	 FF_Medium | FF_White,
	 {36 * BASE_X, 57 * BASE_Y, 48 * BASE_X, 6 * BASE_Y,},
	 },
	{
	 "(garthy@cs.adelaide.edu.au)",
	 FF_Small | FF_White,
	 {36 * BASE_X, 63 * BASE_Y, 48 * BASE_X, 4 * BASE_Y,},
	 },
	{
	 N_("Sound by Norbert Nicolay"),
	 FF_Medium | FF_White,
	 {36 * BASE_X, 67 * BASE_Y, 48 * BASE_X, 6 * BASE_Y,},
	 },
	{
	 "(nicolay@ikp.uni-koeln.de)",
	 FF_Small | FF_White,
	 {36 * BASE_X, 73 * BASE_Y, 48 * BASE_X, 4 * BASE_Y,},
	 },
	/* end of array */
	{
	 NULL,
	 0,
	 {0, 0, 0, 0},
	 },
};

/* 
 * level title boxes 
 */
IntroTextBox titleBox[] = {
	{
	 "",
	 FF_White | FF_Boxed,
	 {13 * BLOCK_WIDTH / 4, BLOCK_HEIGHT / 2 - 2, 17 * BLOCK_WIDTH / 2, BLOCK_HEIGHT + 4,},
	 },
	{
	 NULL,
	 FF_White | FF_Large,
	 {13 * BLOCK_WIDTH / 4, BLOCK_HEIGHT / 2 - 2, 17 * BLOCK_WIDTH / 2, 2 * BLOCK_HEIGHT / 3,},
	 },
	{
	 NULL,
	 FF_White | FF_Small,
	 {13 * BLOCK_WIDTH / 4, 7 * BLOCK_HEIGHT / 6 - 2, 17 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 3,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Large,
	 {5 * BLOCK_WIDTH / 2, 67 * BLOCK_HEIGHT / 6, 10 * BLOCK_WIDTH, 2 * BLOCK_HEIGHT / 3,}
	 },
	{
	 N_("visit http://xblast.sf.net/"),
	 FF_White | FF_Boxed | FF_Small,
	 {8 * BLOCK_WIDTH / 2, 73 * BLOCK_HEIGHT / 6, 7 * BLOCK_WIDTH, 2 * BLOCK_HEIGHT / 3,}
	 },
	/* end of array */
	{
	 NULL,
	 0,
	 {0, 0, 0, 0},
	 },
};

/* 
 * player info boxes 
 */
IntroTextBox playerInfoBox[] = {
	/* frame */
	{
	 "",
	 FF_White | FF_Boxed | FF_Transparent,
	 {3 * BLOCK_WIDTH / 8, 5 * BLOCK_HEIGHT / 2, 17 * BLOCK_WIDTH / 4, 95 * BLOCK_HEIGHT / 12,},
	 },
	/* header */
	{
	 N_("Player Info"),
	 FF_Black | FF_Boxed | FF_Medium,
	 {BLOCK_WIDTH / 2, 8 * BLOCK_HEIGHT / 3, 4 * BLOCK_WIDTH, 2 * BLOCK_HEIGHT / 3,},
	 },
	/* info 1-7 */
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 15 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 19 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 23 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 27 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 31 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 35 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {BLOCK_WIDTH / 2, 39 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	/* end of array */
	{
	 NULL, 0, {0, 0, 0, 0},
	 },
};

/*
 *  level info boxes
 */
IntroTextBox levelInfoBox[] = {
	/* frame */
	{
	 "",
	 FF_White | FF_Boxed | FF_Transparent,
	 {41 * BLOCK_WIDTH / 8, 5 * BLOCK_HEIGHT / 2, 19 * BLOCK_WIDTH / 4, 95 * BLOCK_HEIGHT / 12,},
	 },
	/* header */
	{
	 N_("Level Info"),
	 FF_Black | FF_Boxed | FF_Medium,
	 {21 * BLOCK_WIDTH / 4, 8 * BLOCK_HEIGHT / 3, 9 * BLOCK_WIDTH / 2, 2 * BLOCK_HEIGHT / 3,},
	 },
	/* info 1-7 */
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 15 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 19 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 23 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 27 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 31 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 35 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {21 * BLOCK_WIDTH / 4, 39 * BLOCK_HEIGHT / 4, 9 * BLOCK_WIDTH / 2, BLOCK_HEIGHT / 2,},
	 },
	/* end of array */
	{
	 NULL, 0, {0, 0, 0, 0},
	 },
};

/*
 *  extra info boxes
 */
IntroTextBox extraInfoBox[] = {
	/* frame */
	{
	 "",
	 FF_White | FF_Boxed | FF_Transparent,
	 {83 * BLOCK_WIDTH / 8, 5 * BLOCK_HEIGHT / 2, 17 * BLOCK_WIDTH / 4, 95 * BLOCK_HEIGHT / 12,},
	 },
	/* header */
	{
	 N_("Extra Info"),
	 FF_Black | FF_Boxed | FF_Medium,
	 {42 * BLOCK_WIDTH / 4, 8 * BLOCK_HEIGHT / 3, 4 * BLOCK_WIDTH, 2 * BLOCK_HEIGHT / 3,},
	 },
	/* info 1-6 */
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 15 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 19 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 23 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 27 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 31 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 35 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	{
	 NULL,
	 FF_White | FF_Boxed | FF_Small,
	 {42 * BLOCK_WIDTH / 4, 39 * BLOCK_HEIGHT / 4, 4 * BLOCK_WIDTH, BLOCK_HEIGHT / 2,},
	 },
	/* end of array */
	{
	 NULL, 0, {0, 0, 0, 0},
	 },
};

/*
 * data points for X-polygon
 */
const BMPoint pointx[SIZE_OF_X] = {
	{0.000000, 0.000000},
	{0.166667, 0.000000},
	{0.500000, 0.333333},
	{0.833333, 0.000000},
	{1.000000, 0.000000},
	{1.000000, 0.166667},
	{0.666667, 0.500000},
	{1.000000, 0.833333},
	{1.000000, 1.000000},
	{0.833333, 1.000000},
	{0.500000, 0.666667},
	{0.166667, 1.000000},
	{0.000000, 1.000000},
	{0.000000, 0.833333},
	{0.333333, 0.500000},
	{0.000000, 0.166667},
};

/*
 * explosion data for letter animations
 */

/* Letter A */
const int blockA[CHAR_ANIME][CHARH][CHARW] = {
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x10, 0x00, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x17, 0x18, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x17, 0x1a, 0x18},
	 {0x15, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 },
	{
	 {0x16, 0x18, 0x00},
	 {0x15, 0x00, 0x14},
	 {0x17, 0x1a, 0x1d},
	 {0x15, 0x00, 0x11},
	 {0x11, 0x00, 0x00},
	 },
	{
	 {0x16, 0x1a, 0x1c},
	 {0x15, 0x00, 0x15},
	 {0x17, 0x1a, 0x1d},
	 {0x15, 0x00, 0x15},
	 {0x11, 0x00, 0x11},
	 },
};

/* letter B */
const int blockB[CHAR_ANIME][CHARH][CHARW] = {
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x10, 0x00, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x1b, 0x18, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x1b, 0x1a, 0x18},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x17, 0x18, 0x00},
	 {0x15, 0x00, 0x14},
	 {0x1b, 0x1a, 0x19},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x17, 0x1a, 0x1c},
	 {0x15, 0x00, 0x15},
	 {0x1b, 0x1a, 0x19},
	 },
	{
	 {0x1e, 0x18, 0x00},
	 {0x15, 0x00, 0x14},
	 {0x17, 0x1a, 0x1d},
	 {0x15, 0x00, 0x15},
	 {0x1b, 0x1a, 0x19},
	 },
	{
	 {0x1e, 0x1a, 0x1c},
	 {0x15, 0x00, 0x15},
	 {0x17, 0x1a, 0x1d},
	 {0x15, 0x00, 0x15},
	 {0x1b, 0x1a, 0x19}
	 }
};

/* letter L */
const int blockL[CHAR_ANIME][CHARH][CHARW] = {
	{
	 {0x10, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x11, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x18, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x1a, 0x18},
	 },
};

/* Letter S */
const int blockS[CHAR_ANIME][CHARH][CHARW] = {
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x10, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x14, 0x00, 0x00},
	 {0x13, 0x18, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x14, 0x00, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x1a, 0x18},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x16, 0x18, 0x00},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x1a, 0x1c},
	 {0x00, 0x00, 0x11},
	 {0x00, 0x00, 0x00},
	 },
	{
	 {0x16, 0x1a, 0x18},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x1a, 0x1c},
	 {0x00, 0x00, 0x15},
	 {0x00, 0x00, 0x11},
	 },
	{
	 {0x16, 0x1a, 0x18},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x1a, 0x1c},
	 {0x00, 0x00, 0x15},
	 {0x00, 0x12, 0x19},
	 },
	{
	 {0x16, 0x1a, 0x18},
	 {0x15, 0x00, 0x00},
	 {0x13, 0x1a, 0x1c},
	 {0x00, 0x00, 0x15},
	 {0x12, 0x1a, 0x19},
	 },
};

/* Letter T */
const int blockT[CHAR_ANIME][CHARH][CHARW] = {
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x10, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x14, 0x00},
	 {0x00, 0x11, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x00, 0x00},
	 {0x00, 0x14, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x11, 0x00},
	 },
	{
	 {0x00, 0x00, 0x00},
	 {0x00, 0x14, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x11, 0x00},
	 },
	{
	 {0x00, 0x14, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x11, 0x00},
	 },
	{
	 {0x12, 0x1e, 0x18},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x15, 0x00},
	 {0x00, 0x11, 0x00},
	 },
};

/*
 * level data for generic scorebaord
 */
const XBScoreGraphics graphicsScoreBoard = {
	{"score_right_up", COLOR_BLACK, COLOR_LIGHT_STEEL_BLUE, COLOR_BLACK},
	{"score_right_down", COLOR_BLACK, COLOR_FIRE_BRICK_1, COLOR_BLACK},
	{"score_mid_up", COLOR_BLACK, COLOR_SPRING_GREEN, COLOR_LIGHT_STEEL_BLUE},
	{"score_mid_down", COLOR_BLACK, COLOR_FIRE_BRICK_1, COLOR_LIGHT_STEEL_BLUE},
	{"score_left_up", COLOR_BLACK, COLOR_SPRING_GREEN, COLOR_LIGHT_STEEL_BLUE},
	{"score_left_down", COLOR_BLACK, COLOR_FIRE_BRICK_1, COLOR_LIGHT_STEEL_BLUE},
	{"score_floor", COLOR_BLACK, COLOR_SPRING_GREEN, COLOR_BLACK},
	{"score_step", COLOR_BLACK, COLOR_FIRE_BRICK_1, COLOR_LIGHT_STEEL_BLUE},
	{"score_drop", COLOR_BLACK, COLOR_FIRE_BRICK_1, COLOR_SPRING_GREEN},
	{"score_floor", COLOR_BLACK, COLOR_BLACK, COLOR_BLACK},
	{"score_floor", COLOR_ROYAL_BLUE, COLOR_ROYAL_BLUE, COLOR_ROYAL_BLUE},
};
XBScoreMap mapScoreBoard = {
	{7, 7, 7, 6, 0, 1, 0, 1, 0, 1, 0, 1, 6,},
	{7, 7, 7, 6, 0, 1, 0, 1, 0, 1, 0, 1, 6,},
	{7, 7, 7, 6, 0, 1, 0, 1, 0, 1, 0, 1, 6,},
	{7, 7, 7, 6, 2, 3, 2, 3, 2, 3, 2, 3, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
	{7, 7, 7, 6, 4, 5, 4, 5, 4, 5, 4, 5, 6,},
};
const XBScoreMap mapWinGame = {
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
	{7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
};

/*
 * level data for load player screen
 */
const XBScoreGraphics graphicsLoadSprite = {
	{"score_floor", COLOR_WHITE, COLOR_WHITE, COLOR_WHITE},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
	{NULL, COLOR_INVALID, COLOR_INVALID, COLOR_INVALID},
};
const XBScoreMap mapLoadSprite = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};

/* 
 * audience for score board 
 */
const BMSpriteAnimation winnerAnime[NUM_WINNER_ANIME] = {
	SpriteStopDown, SpriteWinner3, SpriteWinner2, SpriteWinner,
	SpriteWinner, SpriteWinner2, SpriteWinner3, SpriteStopDown,
};
const BMSpriteAnimation looserAnime[NUM_LOOSER_ANIME] = {
	SpriteLooser, SpriteLooser, SpriteLooser1, SpriteLooser1, SpriteLooser,
	SpriteLooser, SpriteLooser, SpriteLooser2, SpriteLooser2, SpriteLooser,
};
const BMSpriteAnimation otherWinnerAnime[NUM_OTHER_WINNER_ANIME] = {
	SpriteStopDown, SpriteWinner3, SpriteWinner2, SpriteWinner,
	SpriteWinner, SpriteWinner2, SpriteWinner3, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
};
const BMSpriteAnimation otherLooserAnime[NUM_OTHER_LOOSER_ANIME] = {
	SpriteLooser, SpriteLooser, SpriteLooser1, SpriteLooser1, SpriteLooser,
	SpriteLooser, SpriteLooser, SpriteLooser2, SpriteLooser2, SpriteLooser,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
};
const BMSpriteAnimation laOlaAnime[NUM_LAOLA_ANIME] = {
	SpriteStopDown, SpriteWinner3, SpriteWinner2, SpriteWinner,
	SpriteWinner, SpriteWinner, SpriteWinner, SpriteWinner2,
	SpriteWinner3, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
	SpriteStopDown, SpriteStopDown, SpriteStopDown, SpriteStopDown,
};

/*
 * end of file introdat.h
 */
