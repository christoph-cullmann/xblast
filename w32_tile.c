/*
 * file w32_tile.c - loading and drawing maptiles
 *
 * $Id: w32_tile.c,v 1.8 2006/02/19 13:33:01 lodott Exp $
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
#include "w32_tile.h"
#include "gui.h"

#include "w32_image.h"
#include "w32_sprite.h"
#include "w32_pixmap.h"

#include "geom.h"
#include "image.h"

/*
 * local variables
 */

#define MAX_LEDS 5				// 2+3 XBCC

static HDC hdcDst = NULL;
static HDC hdcSrc = NULL;

static HBITMAP pixBlock[MAX_TILE];
static HBITMAP pixLeds[MAX_LEDS];
static HBITMAP pixScore[MAX_SCORE_TILES];
static HBITMAP pixExplBlock[MAX_EXPLOSION];

static RECT voidList[MAZE_W * (MAZE_H + 2)];
static RECT blockList[MAX_TILE][MAZE_W * (MAZE_H + 2)];
static RECT explList[MAX_EXPLOSION][MAZE_W * (MAZE_H + 2)];

static RECT *voidLast = voidList;
static RECT *blockLast[MAX_TILE];
static RECT *explLast[MAX_EXPLOSION];
/*
 *
 */
XBBool
InitTiles (void)
{
	HDC hdc;
	int i;

	/* create device contextes for drawing block to pixmap */
	hdc = GetDC (window);
	if (NULL == hdc) {
		return XBFalse;
	}
	hdcDst = CreateCompatibleDC (hdc);
	if (NULL == hdcDst) {
		return XBFalse;
	}
	hdcSrc = CreateCompatibleDC (hdc);
	if (NULL == hdcSrc) {
		return XBFalse;
	}
	/* init led tiles  */
	for (i = 0; i < 2; i++) {
		pixLeds[i] =
			ReadCchPixmap (imgPathScore, imgFileScoreLed[i], COLOR_BLACK, COLOR_LIGHT_GOLDENROD,
						   COLOR_SPRING_GREEN);
	}
	// XBCC
	pixLeds[2] = ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, COLOR_LIGHT_GOLDENROD, COLOR_RED);	// SHRINK Led
	pixLeds[3] = ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, COLOR_LIGHT_GOLDENROD, COLOR_ORANGE);	// SCRAMBLE DRAW Led
	pixLeds[4] = ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, COLOR_LIGHT_GOLDENROD, COLOR_BLUE);	// SCRAMBLE DEL Led  
	// -XBCC
	/* init other score tiles */
	for (i = 0; i < SBDead; i++) {
		pixScore[i] =
			ReadCchPixmap (imgPathScore, imgFileScoreTile[i], COLOR_BLACK, COLOR_LIGHT_GOLDENROD,
						   COLOR_SADDLE_BROWN);
	}
	/* init drawing lists */
	for (i = 0; i < MAX_TILE; i++) {
		blockLast[i] = blockList[i];
	}
	for (i = 0; i < MAX_EXPLOSION; i++) {
		explLast[i] = explList[i];
	}
	/* just clear this ones */
	memset (pixBlock, 0, sizeof (pixBlock));
	memset (pixExplBlock, 0, sizeof (pixExplBlock));
	return XBTrue;
}								/* InitTiles */

/*
 *
 */
void
FinishTiles (void)
{
	int i;

	/* delete block tiles if loaded */
	for (i = 0; i < MAX_TILE; i++) {
		if (NULL != pixBlock[i]) {
			DeleteObject (pixBlock[i]);
		}
	}
	/* score tiles */
	for (i = 0; i < 2; i++) {
		if (NULL != pixLeds[i]) {
			DeleteObject (pixLeds[i]);
		}
	}
	for (i = 0; i < MAX_SCORE_TILES; i++) {
		if (NULL != pixScore[i]) {
			DeleteObject (pixScore[i]);
		}
	}
	/* explosion blocks */
	for (i = 0; i < MAX_EXPLOSION; i++) {
		if (NULL != pixExplBlock[i]) {
			DeleteObject (pixExplBlock[i]);
		}
	}
	/* remove devicerc contexts */
	if (NULL != hdcDst) {
		DeleteObject (hdcDst);
	}
	if (NULL != hdcSrc) {
		DeleteObject (hdcSrc);
	}
}								/* FinishTiles */

/*
 *
 */
void
GUI_DrawBlock (int x, int y, int block)
{
	assert (x >= 0);
	assert (x < MAZE_W);
	assert (y >= 0);
#ifdef SMPF
	assert (y < MAZE_H + 4);
#else
	assert (y < MAZE_H + 3);
#endif
	assert (block >= -1);
	assert (block < MAX_TILE);

	if (block >= 0) {
		blockLast[block]->left = x * BLOCK_WIDTH;
		blockLast[block]->top = y * BLOCK_HEIGHT;
		blockLast[block]->right = (x + 1) * BLOCK_WIDTH;
		blockLast[block]->bottom = (y + 1) * BLOCK_HEIGHT;

		blockLast[block]++;
	}
	else {
		voidLast->left = x * BLOCK_WIDTH;
		voidLast->top = y * BLOCK_HEIGHT;
		voidLast->right = (x + 1) * BLOCK_WIDTH;
		voidLast->bottom = (y + 1) * BLOCK_HEIGHT;

		voidLast++;
	}
}								/* GUI_DrawBlock */

/*
 *
 */
void
GUI_DrawExplosion (int x, int y, int block)
{
	assert (block >= 0);
	assert (block < MAX_EXPLOSION);

	explLast[block]->left = x * BLOCK_WIDTH;
	explLast[block]->top = y * BLOCK_HEIGHT;
	explLast[block]->right = (x + 1) * BLOCK_WIDTH;
	explLast[block]->bottom = (y + 1) * BLOCK_HEIGHT;

	explLast[block]++;
}								/* GUI_DrawExplosion */

/*
 *
 */
void
GUI_LoadBlockRgb (int id, const char *name)
{
	assert (id >= 0);
	assert (id < MAX_TILE);
	assert (NULL == pixBlock[id]);

	if (NULL != pixBlock[id]) {
		DeleteObject (pixBlock[id]);
	}
	pixBlock[id] = ReadRgbPixmap (imgPathBlock, name);

	if (pixBlock[id] == NULL) {
		fprintf (stderr, " Faile to load rgb Block %s.ppm \n", name);
		pixBlock[id] = ReadRgbPixmap (imgPathBlock, "unknown-file");
	}
}								/* GUI_LoadBlockPpm */

/*
 *
 */
void
GUI_LoadBlockCch (int id, const char *name, XBColor fg, XBColor bg, XBColor add)
{
	assert (id >= 0);
	assert (id < MAX_TILE);
	assert (NULL == pixBlock[id]);

	if (NULL != pixBlock[id]) {
		DeleteObject (pixBlock[id]);
	}
	pixBlock[id] = ReadCchPixmap (imgPathBlock, name, fg, bg, add);
	if (pixBlock[id] == NULL) {
		fprintf (stderr, " Faile to load rgb Block %s.ppm \n", name);
		pixBlock[id] = ReadRgbPixmap (imgPathBlock, "unknown-file");
	}
}								/* GUI_LoadBlock */

/*
 *
 */
static void
MultiBlt (HDC hdcDst, HDC hdcSrc, RECT * rect, int n_rect, DWORD rop)
{
	int i;

	for (i = 0; i < n_rect; i++) {
		BitBlt (hdcDst, rect[i].left, rect[i].top, rect[i].right - rect[i].left,
				rect[i].bottom - rect[i].top, hdcSrc, 0, 0, rop);
	}
}								/* MultiBlt */

/*
 *
 */
void
GUI_FlushBlocks (void)
{
	int i;
	HGDIOBJ oldSrc;
	HGDIOBJ oldDst;

	oldDst = SelectObject (hdcDst, pix);
	/* normal blocks */
	for (i = 0; i < MAX_TILE; i++) {
		if (pixBlock[i] != NULL && blockLast[i] != blockList[i]) {
			oldSrc = SelectObject (hdcSrc, pixBlock[i]);
			MultiBlt (hdcDst, hdcSrc, blockList[i], blockLast[i] - blockList[i], SRCCOPY);
			SelectObject (hdcSrc, oldSrc);
		}
		blockLast[i] = blockList[i];
	}
	/* void blocks */
	ClearRectangles (hdcDst, hdcSrc, voidList, voidLast - voidList);
	voidLast = voidList;
	/* explosion blocks */
	for (i = 0; i < MAX_EXPLOSION; i++) {
		if (pixExplBlock[i] != NULL) {
			if (explLast[i] != explList[i]) {
				oldSrc = SelectObject (hdcSrc, pixExplBlock[i]);
				MultiBlt (hdcDst, hdcSrc, explList[i], explLast[i] - explList[i], SRCCOPY);
				SelectObject (hdcSrc, oldSrc);
				explLast[i] = explList[i];
			}
		}
	}
	/* that's all */
	(void)SelectObject (hdcDst, oldDst);
}								/* GUI_FlushBlocks */

void
GUI_FreeBlock (int block)
{
	assert (block >= 0);
	assert (block < MAX_TILE);
	/* delete bitmap */
	if (NULL != pixBlock[block]) {
		DeleteObject (pixBlock[block]);
		pixBlock[block] = NULL;
	}
}								/* GUI_FreeBlock */

/*
 *
 */
void
GUI_InitExplosionBlocks (void)
{
	HGDIOBJ oldSrc;
	HGDIOBJ oldDst;
	int i;

	for (i = 0; i < MAX_EXPLOSION; i++) {
		oldSrc = SelectObject (hdcSrc, pixBlock[0]);
		pixExplBlock[i] = CreateCompatibleBitmap (hdcSrc, BLOCK_WIDTH, BLOCK_HEIGHT);
		assert (pixExplBlock[i] != NULL);
		oldDst = SelectObject (hdcDst, pixExplBlock[i]);
		/* draw floor tile into it */
		BitBlt (hdcDst, 0, 0, BLOCK_WIDTH, BLOCK_HEIGHT, hdcSrc, 0, 0, SRCCOPY);
		SelectObject (hdcSrc, oldSrc);
		SelectObject (hdcDst, oldDst);
		/* now copy explosion into it */
		CopyExplosion (pixExplBlock[i], i);
	}
}								/* GUI_InitExplosionBlocks */

/*
 *
 */
void
GUI_FreeExplosionBlocks (void)
{
	int i;

	for (i = 0; i < MAX_EXPLOSION; i++) {
		if (NULL != pixExplBlock[i]) {
			DeleteObject (pixExplBlock[i]);
			pixExplBlock[i] = NULL;
		}
	}
}								/* GUI_FreeExplosionBlocks */

/*
 *
 */
void
GUI_LoadPlayerScoreTiles (int player, const CFGPlayerGraphics * config)
{
	XBColor scoreColors[NUM_PLAYER_COLORS];

	assert (player < MAX_PLAYER);
	assert (config != NULL);
	/* copy colors */
	scoreColors[0] = config->helmet;
	scoreColors[1] = config->face;
	scoreColors[2] = config->handsFeet;
	scoreColors[3] = config->armsLegs;
	scoreColors[4] = COLOR_LIGHT_GOLDENROD;
	scoreColors[5] = COLOR_SADDLE_BROWN;
	scoreColors[6] = COLOR_WHITE;
	/* load pixmap */
	pixScore[SBDead + player] =
		ReadEpmPixmap (imgPathScore, imgFileScorePlayer[0], NUM_PLAYER_COLORS, scoreColors);
	pixScore[SBSick + player] =
		ReadEpmPixmap (imgPathScore, imgFileScorePlayer[1], NUM_PLAYER_COLORS, scoreColors);
	pixScore[SBPlayer + player] =
		ReadEpmPixmap (imgPathScore, imgFileScorePlayer[2], NUM_PLAYER_COLORS, scoreColors);
	pixScore[SBAbort + player] =
		ReadEpmPixmap (imgPathScore, imgFileScorePlayer[3], NUM_PLAYER_COLORS, scoreColors);
	pixScore[SBSickAbort + player] =
		ReadEpmPixmap (imgPathScore, imgFileScorePlayer[4], NUM_PLAYER_COLORS, scoreColors);

}								/* GUI_LoadPlayerScoreTile */

/*
 *
 */
void
//#ifdef SMPF
GUI_DrawScoreBlock (int x, int y, int block)	// SMPF
	 /*#else
	    GUI_DrawScoreBlock (int x, int block)
	    #endif */
{
	HGDIOBJ oldSrc;
	HGDIOBJ oldDst;

	assert (block < MAX_SCORE_TILES);
	assert (pixScore[block] != NULL);

	oldSrc = SelectObject (hdcSrc, pixScore[block]);
	oldDst = SelectObject (hdcDst, pix);
	//#ifdef SMPF
	BitBlt (hdcDst, x * STAT_WIDTH, MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT, STAT_WIDTH, STAT_HEIGHT, hdcSrc, 0, 0, SRCCOPY);	// SMPF
	/*#else
	   BitBlt (hdcDst, x*STAT_WIDTH, MAZE_H*BLOCK_HEIGHT, STAT_WIDTH, STAT_HEIGHT, hdcSrc, 0, 0, SRCCOPY);
	   #endif */
	SelectObject (hdcDst, oldDst);
	SelectObject (hdcSrc, oldSrc);
}								/* GUI_DrawScoreBlock */

/* 
 *
 */
void
GUI_DrawTimeLed (int x, int block)
{
	HGDIOBJ oldSrc;
	HGDIOBJ oldDst;

	assert (block < MAX_LEDS);
	assert (pixLeds[block] != NULL);

	oldSrc = SelectObject (hdcSrc, pixLeds[block]);
	oldDst = SelectObject (hdcDst, pix);
#ifdef SMPF
	BitBlt (hdcDst, x * LED_WIDTH, MAZE_H * BLOCK_HEIGHT + STAT_HEIGHT * 2, LED_WIDTH, LED_HEIGHT,
			hdcSrc, 0, 0, SRCCOPY);
#else
	BitBlt (hdcDst, x * LED_WIDTH, MAZE_H * BLOCK_HEIGHT + STAT_HEIGHT, LED_WIDTH, LED_HEIGHT,
			hdcSrc, 0, 0, SRCCOPY);
#endif
	SelectObject (hdcDst, oldDst);
	SelectObject (hdcSrc, oldSrc);
}								/* GUI_DrawTimeLed  */

/*
 * end of file w32_tile.c
 */
