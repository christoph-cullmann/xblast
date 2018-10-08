/*
 * file status.c - displays status bar at window bottom
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

#include "xblast.h"

/*
 * local constants
 */
#define TEXT_ATTR_NORMAL (FF_Medium|FF_White|FF_Boxed)
#define TEXT_ATTR_NUMBER (FF_Large|FF_White|FF_Boxed)
#define TEXT_ATTR_SMALLNUMBER (FF_Medium|FF_White|FF_Boxed)	// mab
#define TEXT_ATTR_SMALLNUMBER1 (FF_Small |FF_White|FF_Boxed)
/* Added by VVL (Chat) 12/11/99 */
#define FF_Scroll      0x40

#define MAX_MSG           8
#define MESSAGE_TIME      TIME_STEP

/*
 * local variables
 */
static int numPlayer;

static int gameNext, timeLed;
static int faceUsed[MAX_PLAYER];
static int numUsed[MAX_PLAYER];

#ifdef SMPF
/* SMPF - create 2D arrays x and y pos, current values are all y = 0
   new facePos y = 1, x = 0 2 4 6 8 10 12 14 16 18 and numPos facePos.x + 1
   for this to work adjust SetScoreBlock (status.c) and 
   GUI_DrawScoreBlock(x11c_tile.c), make sure the XFillRectangle call is correct!! */

static const int facePos[MAX_PLAYER] = {
	0, 2, 4, 14, 16, 18, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18
};
static const int numPos[MAX_PLAYER] = {
	1, 3, 5, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39
};
#else
static const int facePos[MAX_PLAYER] = {
	0, 2, 4, 14, 16, 18,
};
static const int numPos[MAX_PLAYER] = {
	1, 3, 5, 15, 17, 19,
};
#endif

/* variables for message queue */
static const char *defaultMessage;
static int msgRestore;
/* Added by VVL (Chat) 12/11/99 */
static int chat_restore;
/* Added by VVL (Chat) 12/11/99 */
static int num_chat = 0;
/* Added by VVL (Chat) 12/11/99 */
static int first_chat;
/* Added by VVL (Chat) 12/11/99 */
static char chat_queue[MAX_MSG][CHAT_LEN + 128];
/* Added by VVL (Chat) 12/11/99 : Begin */
static int chat_delay[MAX_MSG + 1] = {
	-1, 200, 80, 40, 20, 15, 10, 7, 7
};

/* Added by VVL (Chat) 12/11/99 : End */
static int numMsg = 0;
static int firstMsg;
static const char *msgQueue[MAX_MSG];
static const int msgDelay[MAX_MSG + 1] = {
	-1, 50, 40, 20, 12, 7, 7, 7, 7
};

/* status board text box */
static const int textLeft = 6;
static const int textRight = 13;
static BMRectangle statusBox = {
	77 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6,
	87 * STAT_WIDTH / 12, 2 * STAT_HEIGHT / 3,
};								/* Added by VVL (Chat) 12/11/99 : Begin */
static BMRectangle get_box = {
	2 * STAT_WIDTH / 12,
#ifdef SMPF
	93 * STAT_HEIGHT / 6,
#else
	87 * STAT_HEIGHT / 6,

#endif
	116 * STAT_WIDTH / 12, 2 * STAT_HEIGHT / 3,
};
static BMRectangle chat_box = {
	121 * STAT_WIDTH / 12,
#ifdef SMPF
	93 * STAT_HEIGHT / 6,
#else
	87 * STAT_HEIGHT / 6,

#endif
	116 * STAT_WIDTH / 12, 2 * STAT_HEIGHT / 3,
};

/* Added by VVL (Chat) 12/11/99 : End */

/* status board number boxes */
#ifdef SMPF
static BMRectangle statusNumberBox[STAT_W * 2] = {
#else
static BMRectangle statusNumberBox[STAT_W] = {
#endif
	{1 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{13 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{25 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{37 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{49 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{61 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{73 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{85 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{97 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{109 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{121 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{133 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{145 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{157 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{169 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{181 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{193 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{205 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{217 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{229 * STAT_WIDTH / 12, 79 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
#ifdef SMPF
	{1 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{13 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{25 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{37 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{49 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{61 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{73 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{85 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{97 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{109 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{121 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{133 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{145 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{157 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{169 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{181 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{193 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{205 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{217 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
	{229 * STAT_WIDTH / 12, 85 * STAT_HEIGHT / 6, 8 * STAT_WIDTH / 12 - 1, 4 * STAT_HEIGHT / 6},
#endif
};

/* victory boxes, mab */

#ifdef SMPF
static BMRectangle statusNumberBox2[STAT_W * 2] = {
#else
static BMRectangle statusNumberBox2[STAT_W] = {
#endif
	{1 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{13 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{25 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{37 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{49 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{61 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{73 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{85 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{97 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{109 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{121 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{133 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{145 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{157 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{169 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{181 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{193 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{205 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{217 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
	{229 * STAT_WIDTH / 12 - 2, 79 * STAT_HEIGHT / 6 - 2, 4 * STAT_WIDTH / 16 - 1,
	 2 * STAT_HEIGHT / 6},
#ifdef SMPF
	{1 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{13 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{25 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{37 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{49 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{61 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{73 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{85 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{97 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{109 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{121 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{133 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{145 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{157 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{169 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{181 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{193 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{205 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{217 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
	{229 * STAT_WIDTH / 12 - 2, 85 * STAT_HEIGHT / 6, 4 * STAT_WIDTH / 16 - 1, 2 * STAT_HEIGHT / 6},
#endif
};

/*
 *
 */
void
ClearStatusBar (int tile1, int tile2)
{
	int x;

	for (x = 0; x < MAZE_W; x++) {
		GUI_DrawBlock (x, MAZE_H, tile1);
		GUI_DrawBlock (x, MAZE_H + 1, tile2);
		GUI_DrawBlock (x, MAZE_H + 2, tile2);
#ifdef SMPF
		GUI_DrawBlock (x, MAZE_H + 3, tile2);
#endif
	}
	MarkMazeRect (0, PIXH, PIXW, SCOREH);
}								/* ClearStatusBar */

/*
 *
 */
//#ifdef SMPF
static void
SetScoreBlock (int x, int y, int block)
{
	GUI_DrawScoreBlock (x, y, block);
	MarkMazeTile (x, MAZE_H + y);
}								/* SetScoreBlock */

/*#else
static void
SetScoreBlock (int x, int block)
{
  GUI_DrawScoreBlock (x, block);
  MarkMazeTile (x, MAZE_H);
				    } *//* SetScoreBlock */
//#endif
/*
 *
 */
static void
SetStatusText (const char *msg)
{
	GUI_DrawSimpleTextbox (msg, TEXT_ATTR_NORMAL, &statusBox);
	MarkMaze (textLeft, MAZE_H, textRight, MAZE_H);
}								/* SetStatusText */

/* Added by VVL (Chat) 12/11/99 : Begin */
 /**/
/* local function set chat text */
static void
SetChatText (char *msg)
{
	GUI_DrawTextbox (msg, TEXT_ATTR_NORMAL | FF_Scroll, &chat_box);
	MarkMaze (10, MAZE_H + STAT_H - 1, 20, MAZE_H + STAT_H);
}

/* Added by VVL (Chat) 12/11/99 : End */

/*
 *
 */
static void
#ifdef SMPF
SetStatusNumber (int pos, int y, int value)
#else
SetStatusNumber (int pos, int value)
#endif
{
	char numString[2];

	numString[0] = '0' + value;
	numString[1] = '\0';
	GUI_DrawSimpleTextbox (numString, TEXT_ATTR_NUMBER, statusNumberBox + pos);
#ifdef SMPF
	//  fprintf(stderr," y1 %i pos %i pos  20 %i value %i\n",y,pos,pos%20,value);
	MarkMazeTile (pos % 20, MAZE_H + y);	// SMPF - very carefull here!!!!!
#else
	MarkMazeTile (pos, MAZE_H);
#endif
}								/* SetStatusNumber */

/*
 *
 */
static void
#ifdef SMPF
SetStatusNumber2 (int pos, int y, int value)
#else
SetStatusNumber2 (int pos, int value)
#endif
{
	char numString[2];

	numString[0] = '0' + value;
	numString[1] = '\0';
	GUI_DrawSimpleTextbox (numString, TEXT_ATTR_SMALLNUMBER1, statusNumberBox2 + (pos));
#ifdef SMPF
	//  fprintf(stderr," y2 %i pos %i pos  20 %i value %i\n",y,pos,pos%20,value);
	MarkMazeTile (pos % 20, MAZE_H + y);	// SMPF - very carefull here!!!!!
#else
	MarkMazeTile (pos, MAZE_H);
#endif
}								/* SetStatusNumber2, mab */

/*
 *
 */
static void
SetTimeLed (int x, int val)
{
	GUI_DrawTimeLed (x, val);
#ifdef SMPF
	MarkMazeTile (x / 3, MAZE_H + 2);
#else
	MarkMazeTile (x / 3, MAZE_H + 1);
#endif
}								/* SetTimeLed */

/*
 *
 */
void
InitStatusBar (int numPlayers)
{
	int p;

	/* set player and display number */
	numPlayer = numPlayers;

	/* clear all */
	for (p = 0; p < STAT_W; p++) {
#ifdef SMPF
		SetScoreBlock (p, 0, SBVoid);	// SMPF - 2D shit here see above..
		SetScoreBlock (p, 1, SBVoid);	// SMPF - 2D shit here see above..
		SetScoreBlock (p, 2, SBVoid);	// SMPF - 2D shit here see above..
		SetScoreBlock (p, 3, SBVoid);	// SMPF - 2D shit here see above..
#else
		SetScoreBlock (p, 0, SBVoid);
		SetScoreBlock (p, 1, SBVoid);
		SetScoreBlock (p, 2, SBVoid);
#endif
	}

	/* player boxes */
	for (p = 0; p < numPlayer; p++) {
		faceUsed[p] = SBPlayer + p;
		numUsed[p] = 0;
#if 0
		SetScoreBlock (facePos[p], p);
#endif
#ifdef SMPF
		SetScoreBlock (facePos[p], (p < 6 ? 0 : 1), faceUsed[p]);	// SMPF - 2D shit here see above..
		SetScoreBlock (facePos[p] + 1, (p < 6 ? 0 : 1), SBTextRight);
		SetStatusNumber (numPos[p], (p < 6 ? 0 : 1), numUsed[p]);
#else
		SetScoreBlock (facePos[p], 0, faceUsed[p]);
		SetScoreBlock (numPos[p], 0, SBTextRight);
		SetStatusNumber (numPos[p], numUsed[p]);
#endif
	}
	/* text box */
#ifdef SMPF
	SetScoreBlock (textLeft, 0, SBTextLeft);
	for (p = textLeft + 1; p < textRight; p++) {
		SetScoreBlock (p, 0, SBTextMid);
	}
	SetScoreBlock (textRight, 0, SBTextRight);
#else
	SetScoreBlock (textLeft, 0, SBTextLeft);
	for (p = textLeft + 1; p < textRight; p++) {
		SetScoreBlock (p, 0, SBTextMid);
	}
	SetScoreBlock (textRight, 0, SBTextRight);
#endif
	/* set queue to default message */
	defaultMessage = "";
	numMsg = 0;
	SetStatusText (defaultMessage);

	/* set leds */
	for (p = 0; p < (4 * MAZE_W); p++) {
		SetTimeLed (p, 1);
	}
	/* set timers for leds */
	gameNext = TIME_STEP;
	timeLed = MAZE_W * 4;
}								/* InitStatusBar */

/* 
 *
 */
void
ResetStatusBar (const BMPlayer * ps, const char *msg, XBBool flag)
{
	int p, i, j;

	/* player boxes */
	for (p = 0; p < numPlayer; p++) {
		faceUsed[p] = SBPlayer + p;
		numUsed[p] = flag ? ps[p].lives : ps[p].victories;
#ifdef SMPF
		SetScoreBlock (facePos[p], (p < 6 ? 0 : 1), faceUsed[p]);
		SetStatusNumber (numPos[p], (p < 6 ? 0 : 1), numUsed[p]);
		SetStatusNumber2 (numPos[p], (p < 6 ? 0 : 1), ps[p].victories);
#else
		SetScoreBlock (facePos[p], 0, faceUsed[p]);
		SetStatusNumber (numPos[p], numUsed[p]);
		SetStatusNumber2 (numPos[p], ps[p].victories);
#endif
	}
	/* default message */
	defaultMessage = msg;
	numMsg = 0;
	SetStatusText (msg);
	/* set leds */

	for (p = 0; p < 4 * MAZE_W; p++) {
		SetTimeLed (p, 1);
	}

	if (flag) {					// XBCC draw shrink in other color (for loosers with no memory) not on scorebord
		p = -1;
		while (p != 0) {
			p = getShrinkTimes (p);
			if (p != 0) {
				if ((p >= 0) && (p < GAME_TIME)) {
					i = 4 * MAZE_W - (p / TIME_STEP);	// inverse 
					SetTimeLed (i, 2);
				}
			}
		}
		p = -1;
		while (p != 0) {
			p = getScrambleTimes (p);
			if (p < 0) {
				j = 4;			// DEL
				p = -p;
			}
			else {
				j = 3;			// DRAW
			}
			if (p != 0) {
				if ((p >= 0) && (p < GAME_TIME)) {
					i = 4 * MAZE_W - (p / TIME_STEP);	// inverse 
					SetTimeLed (i, j);
				}
			}
		}
	}
	/* set timers for leds */
	gameNext = TIME_STEP;
	timeLed = MAZE_W * 4;
	/* mark all filled to be redrawed */
#ifdef SMPF
	MarkMaze (0, MAZE_H, MAZE_W, MAZE_H + 2);	// SMPF
#else
	MarkMaze (0, MAZE_H, MAZE_W, MAZE_H + 1);
#endif
}								/* ResetStatusBar */

/*
 *
 */
void
UpdateStatusBar (const BMPlayer * ps, int game_time)
{
	int p;
	int newFace;

	for (p = 0; p < numPlayer; p++) {
		/* determine current face for player */
		if (ps[p].dying || ps[p].lives == 0) {
			newFace = SBDead + p;
		}
		else if (ps[p].illness != ps[p].health) {
			if (ps[p].abort) {
				newFace = SBSickAbort + p;
			}
			else {
				newFace = SBSick + p;
			}
		}
		else {
			if (ps[p].abort) {
				newFace = SBAbort + p;
			}
			else {
				newFace = SBPlayer + p;
			}

		}
		if (faceUsed[p] != newFace) {
			faceUsed[p] = newFace;
#ifdef SMPF
			SetScoreBlock (facePos[p], (p < 6 ? 0 : 1), faceUsed[p]);
#else
			SetScoreBlock (facePos[p], 0, faceUsed[p]);
#endif
		}
		/* check player lives */
		if (numUsed[p] != ps[p].lives) {
			numUsed[p] = ps[p].lives;
#ifdef SMPF
			SetStatusNumber (numPos[p], (p < 6 ? 0 : 1), numUsed[p]);
			SetStatusNumber2 (numPos[p], (p < 6 ? 0 : 1), ps[p].victories);
#else
			SetStatusNumber (numPos[p], numUsed[p]);
			SetStatusNumber2 (numPos[p], ps[p].victories);
#endif
		}
	}
	/* check leds */
	if (game_time > gameNext) {
		gameNext += TIME_STEP;
		timeLed--;
		SetTimeLed (timeLed, 0);
	}
	/* check message */
	/* if any message is in the queue */
	if (numMsg) {
		msgRestore++;
		/* check if message is outdated */
		if (msgRestore >= msgDelay[numMsg]) {
			/* return to default message if last queued one */
			if (--numMsg) {
				/* set next message */
				firstMsg = (firstMsg + 1) % MAX_MSG;
				SetStatusText (msgQueue[firstMsg]);
				msgRestore = 0;
			}
			else {
				/* return to default message if last queued one */
				SetStatusText (defaultMessage);
			}
		}
	}
/* Added by VVL (Chat) 12/11/99 */
	/* check chat */
	/* if any chat message is in the queue */
	if (num_chat) {
		chat_restore++;
		/* check if message is outdated */
		if (chat_restore >= chat_delay[num_chat]) {
			/* return to default message if last queued one */
			if (--num_chat) {
				/* set next message */
				first_chat = (first_chat + 1) % MAX_MSG;
				SetChatText (chat_queue[first_chat]);
				chat_restore = 0;
			}
			else {
				/* return to default message if last queued one */
				SetChatText ("");
			}
		}
	}
/* Added by VVL (Chat) 12/11/99 : End */
}								/* UpdateStatusBar */

/*
 * 
 */
void
SetMessage (const char *msg, XBBool perm)
{
	int m;

	if (NULL != msg) {
		if (perm) {
			SetStatusText (msg);
			numMsg = 0;
		}
		else {
			/* if no other message exists set to start of queue */
			if (0 == numMsg) {
				m = firstMsg = 0;
				msgRestore = 0;
				SetStatusText (msg);
			}
			else {
				m = (firstMsg + numMsg) % MAX_MSG;
			}
			msgQueue[m] = msg;
			numMsg++;
			/* check for queue overflow */
			if (numMsg > MAX_MSG) {
				/* remove first message */
				numMsg--;
				firstMsg = (firstMsg + 1) % MAX_MSG;
			}
		}
	}
}								/* SetMessage */

/* Added by VVL (Chat) 12/11/99 : Begin */
 /**/
/* public function set_get
 */
	void
SetGet (const char *msg, int perm)
{

	GUI_DrawTextbox (msg, TEXT_ATTR_NORMAL | FF_Scroll, &get_box);
	MarkMaze (0, MAZE_H + STAT_H - 1, 20, MAZE_H + STAT_H);
}

/* Added by VVL (Chat) 12/11/99 : End */

/* Added by VVL (Chat) 12/11/99 : Begin */
 /**/
/* public function set_chat
*/
	void
SetChat (char *msg, int perm)
{
	int m;

	if (NULL != msg) {
		if (perm) {
			SetChatText (msg);
			num_chat = 0;
		}
		else {
			/* if no other message exists set to start of queue */
			if (0 == num_chat) {
				m = first_chat = 0;
				chat_restore = 0;
				SetChatText (msg);
			}
			else {
				m = (first_chat + num_chat) % MAX_MSG;
			}
			strncpy (chat_queue[m], msg, CHAT_LEN + 128);
			num_chat++;
			/* check for queue overflow */
			if (num_chat > MAX_MSG) {
				/* remove first message */
				num_chat--;
				first_chat = (first_chat + 1) % MAX_MSG;
			}
		}
	}
}

/* Added by VVL (Chat) 12/11/99 : End */
/*
 * 
 */
void
ResetMessage (void)
{
	if (numMsg > 0) {
		SetStatusText (msgQueue[firstMsg]);
	}
	else {
		SetStatusText (defaultMessage);
	}
}								/* ResetMessage */

/*
 * end of file status.c
 */
