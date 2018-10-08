/*
 * file x11c_tile.c - draw map tiles 
 *
 * $Id: sdl_tile.c,v 1.6 2006/03/28 11:41:19 fzago Exp $
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

#include "sdl_common.h"

/*
 * local variables
 */
#define MAX_LEDS 5				// 2+3 XBCC

SDL_Surface *ClearPix;

static SDL_Surface *pixBlock[MAX_TILE];
static SDL_Surface *pixLeds[MAX_LEDS];
static SDL_Surface *pixScore[MAX_SCORE_TILES];
static SDL_Surface *pixExplBlock[MAX_EXPLOSION];

static SDL_Rect voidList[MAZE_W * (MAZE_H + 2)];
static SDL_Rect blockList[MAX_TILE][MAZE_W * (MAZE_H + 2)];
static SDL_Rect explList[MAX_EXPLOSION][MAZE_W * (MAZE_H + 2)];

static SDL_Rect *voidLast = voidList;
static SDL_Rect *blockLast[MAX_TILE];
static SDL_Rect *explLast[MAX_EXPLOSION] = {
	explList[0],
	explList[1],
	explList[2],
	explList[3],
	explList[4],
	explList[5],
	explList[6],
	explList[7],
	explList[8],
	explList[9],
	explList[10],
	explList[11],
	explList[12],
	explList[13],
	explList[14],
	explList[15],
};

/*
 * 
 */
XBBool
InitTiles (void)
{
	const CFGColor *cfgColor;
	int i;
	SDL_Surface *temp;

	/* load config */
	cfgColor = GetColorConfig ();
	assert (cfgColor != NULL);
	/* init led tiles  */
	i = 0;
	pixLeds[0] =
		ReadCchPixmap (imgPathScore, imgFileScoreLed[i++], COLOR_BLACK, cfgColor->statusFg,
					   cfgColor->statusLed);
	pixLeds[1] =
		ReadCchPixmap (imgPathScore, imgFileScoreLed[i++], COLOR_BLACK, cfgColor->statusFg,
					   cfgColor->statusLed);
	pixLeds[2] =
		ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, cfgColor->statusFg,
					   COLOR_RED);
	pixLeds[3] =
		ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, cfgColor->statusFg,
					   COLOR_ORANGE);
	pixLeds[4] =
		ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, cfgColor->statusFg,
					   COLOR_BLUE);

	/* init other score tiles */
	for (i = 0; i < SBDead; i++) {
		pixScore[i] =
			ReadCchPixmap (imgPathScore, imgFileScoreTile[i], COLOR_BLACK, cfgColor->statusFg,
						   cfgColor->statusBg);
		if (NULL == pixScore[i]) {
			return XBFalse;
		}
	}

	/* init drawing lists */
	for (i = 0; i < MAX_TILE; i++) {
		pixBlock[i] = NULL;
		blockLast[i] = &blockList[i][0];
	}

	for (i = 0; i < MAX_EXPLOSION; i++) {
		explLast[i] = explList[i];
	}

	temp =
		ReadCchPixmap (imgPathMisc, imgFileTitle, COLOR_BLACK, COLOR_GRAY_75, COLOR_MIDNIGHT_BLUE);
	ClearPix = SDL_DisplayFormat (temp);
	SDL_FreeSurface (temp);

	return XBTrue;
}								/* InitTiles */

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
		// fprintf(stderr," block %i %i %i %p %i %i\n",x,y,block,blockLast[block],
		//      x*BLOCK_WIDTH,y*BLOCK_HEIGHT);
		(blockLast[block])->x = x * BLOCK_WIDTH;
		(blockLast[block])->y = y * BLOCK_HEIGHT;
		(blockLast[block])->w = BLOCK_WIDTH;
		(blockLast[block])->h = BLOCK_HEIGHT;

		(blockLast[block])++;
	}
	else {
		(voidLast)->x = x * BLOCK_WIDTH;
		(voidLast)->y = y * BLOCK_HEIGHT;
		(voidLast)->w = BLOCK_WIDTH;
		(voidLast)->h = BLOCK_HEIGHT;

		(voidLast)++;
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

	(explLast[block])->x = x * BLOCK_WIDTH;
	(explLast[block])->y = y * BLOCK_HEIGHT;
	(explLast[block])->w = BLOCK_WIDTH;
	(explLast[block])->h = BLOCK_HEIGHT;

	(explLast[block])++;
}								/* GUI_DrawExplosion */

/*
 *
 */
static void
MultiBlt (SDL_Surface * img, SDL_Rect * rect, int n_rect)
{
	int i;
	for (i = 0; i < n_rect; i++) {
		SDL_BlitSurface (img, NULL, screen, &rect[i]);
	}
}								/* MultiBlt */

/* 
 *
 */
void
GUI_FlushBlocks (void)
{
	int i;

	/* void blocks */
	if (voidLast != voidList) {
		MultiBlt (ClearPix, voidList, voidLast - voidList);
		voidLast = voidList;
	}
	/* normal blocks */
	for (i = 0; i < MAX_TILE; i++) {
		if (pixBlock[i] != NULL && blockLast[i] != blockList[i]) {
			MultiBlt (pixBlock[i], blockList[i], blockLast[i] - blockList[i]);
		}
		blockLast[i] = blockList[i];
	}
	/* explosion blocks */
	for (i = 0; i < MAX_EXPLOSION; i++) {
		if (pixExplBlock[i] != NULL) {
			if (explLast[i] != explList[i]) {
				MultiBlt (pixExplBlock[i], explList[i], explLast[i] - explList[i]);
				explLast[i] = explList[i];
			}
		}
	}

}								/* GUI_FlushBlocks */

/*
 *
 */
void
GUI_LoadBlockRgb (int id, const char *name)
{
	assert (id >= 0);
	assert (id < MAX_TILE);
	assert (pixBlock[id] == NULL);

	pixBlock[id] = ReadRgbPixmap (imgPathBlock, name);
	if (pixBlock[id] == NULL) {
		Dbg_Out (" Faile to load rgb Block %s.ppm \n", name);
		pixBlock[id] = ReadRgbPixmap (imgPathBlock, "unknown-file");
	}
	Dbg_Out (" loaded rgb Block %s.ppm \n", name);
}								/* GUI_LoadBlock */

/*
 *
 */
void
GUI_LoadBlockCch (int id, const char *name, XBColor fg, XBColor bg, XBColor add)
{
	assert (id >= 0);
	assert (id < MAX_TILE);
	assert (pixBlock[id] == NULL);

	pixBlock[id] = ReadCchPixmap (imgPathBlock, name, fg, bg, add);
	if (pixBlock[id] == NULL) {
		Dbg_Out (" Faile to load Block %s.ppm \n", name);
		pixBlock[id] = ReadCchPixmap (imgPathBlock, "unknown-file", fg, bg, add);
	}
	Dbg_Out (" loaded cch Block %s.ppm %i\n", name, id);
}								/* GUI_LoadBlock */

/*
 *
 */
void
GUI_InitExplosionBlocks (void)
{
	int i;

	for (i = 0; i < MAX_EXPLOSION; i++) {
		pixExplBlock[i] = SDL_CreateRGBSurface (screen->flags,
												BLOCK_WIDTH,
												BLOCK_HEIGHT,
												screen->format->BitsPerPixel,
												screen->format->Rmask,
												screen->format->Gmask,
												screen->format->Bmask, screen->format->Amask);

		SDL_BlitSurface (pixBlock[0], NULL, pixExplBlock[i], NULL);

		/* now copy explosion into it */
		CopyExplosion (pixExplBlock[i], i);
	}
}								/* GUI_InitExplosionBlocks */

/* 
 *
 */
void
GUI_FreeBlock (int in_pix)
{
	assert (in_pix >= 0);
	assert (in_pix < MAX_TILE);
	/* free pixmap */
	if (NULL != pixBlock[in_pix]) {
		SDL_FreeSurface (pixBlock[in_pix]);
		pixBlock[in_pix] = NULL;
	}
}								/* GUI_FreeBlock */

/* 
 *
 */
void
GUI_FreeExplosionBlocks (void)
{
	int i;

	for (i = 0; i < MAX_EXPLOSION; i++) {
		SDL_FreeSurface (pixExplBlock[i]);
	}
}								/* GUI_FreeExplosionBlocks */

/*
 *
 */
void
GUI_LoadPlayerScoreTiles (int player, const CFGPlayerGraphics * config)
{
	const CFGColor *cfgColor;
	XBColor scoreColors[NUM_PLAYER_COLORS];

	assert (player < MAX_PLAYER);
	assert (config != NULL);
	/* get colors */
	cfgColor = GetColorConfig ();
	assert (NULL != cfgColor);
	/* copy colors */
	scoreColors[0] = config->helmet;
	scoreColors[1] = config->face;
	scoreColors[2] = config->handsFeet;
	scoreColors[3] = config->armsLegs;
	scoreColors[4] = cfgColor->statusFg;
	scoreColors[5] = cfgColor->statusBg;
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
GUI_DrawScoreBlock (int x, int y, int block)	// SMPF
{
	SDL_Rect Rect;

	assert (block < MAX_SCORE_TILES);
	assert (pixScore[block] != NULL);

	Rect.x = x * STAT_WIDTH;
#ifdef SMPF
	Rect.y = MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT;
#else
	Rect.y = MAZE_H * BLOCK_HEIGHT;
#endif
	Rect.w = pixScore[block]->w;
	Rect.h = pixScore[block]->h;

	SDL_BlitSurface (pixScore[block], NULL, screen, &Rect);

}								/* GUI_DrawScoreBlock */

/* 
 *
 */
void
GUI_DrawTimeLed (int x, int block)
{
	SDL_Rect Rect;

	assert (block >= 0);
	assert (block < 5);
	assert (pixLeds[block] != NULL);

	Rect.x = x * LED_WIDTH;
#ifdef SMPF
	Rect.y = MAZE_H * BLOCK_HEIGHT + STAT_HEIGHT * 2;
#else
	Rect.y = MAZE_H * BLOCK_HEIGHT + STAT_HEIGHT;
#endif
	Rect.w = pixLeds[block]->w;
	Rect.h = pixLeds[block]->h;

	SDL_BlitSurface (pixLeds[block], NULL, screen, &Rect);

}								/* GUI_DrawTimeLed  */

/*
 * end of file x11c_tile.c
 */
