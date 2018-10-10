/*
 * file x11c_tile.c - draw map tiles 
 *
 * $Id: sdl_tile.c,v 1.1 2004/09/09 23:33:22 iskywalker Exp $
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
#include "x11c_tile.h"
#include "gui.h"

#include "x11_common.h"
#include "x11c_image.h"
#include "x11c_sprite.h"
#include "x11_config.h"

#include "geom.h"
#include "image.h"
#include "cfg_player.h"

/*
 * local variables
 */
#define MAX_LEDS 5 // 2+3 XBCC

static GC gcDrawBlock;

static SDL_Surface *  pixBlock[MAX_TILE];
static SDL_Surface *  pixExplBlock[MAX_EXPLOSION];
static SDL_Surface *  pixLeds[MAX_LEDS];
static SDL_Surface *  pixScore[MAX_SCORE_TILES];

static SDL_Surface * voidList[MAZE_W*(MAZE_H+2)];
static SDL_Surface *voidLast = voidList;
static SDL_Surface  blockList[MAX_TILE][MAZE_W*(MAZE_H+2)];
static SDL_Surface *blockLast[MAX_TILE];
static SDL_Surface explList[MAX_EXPLOSION][MAZE_W*(MAZE_H+2)];
static SDL_Surface *explLast[MAX_EXPLOSION] = {
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
  XGCValues       xgcv;
  int             i;

  /* load config */
  cfgColor = GetColorConfig ();
  assert (cfgColor != NULL);
  /* init led tiles  */
  for (i = 0; i < 2; i ++) {
    pixLeds[i] = ReadCchPixmap (imgPathScore, imgFileScoreLed[i], COLOR_BLACK, cfgColor->statusFg, cfgColor->statusLed);
    if (None == pixLeds[i]) {
      return XBFalse;
    }
  }
  // XBCC fuck colorconfig
  pixLeds[2] = ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, cfgColor->statusFg, COLOR_RED);
  pixLeds[3] = ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, cfgColor->statusFg, COLOR_ORANGE);
  pixLeds[4] = ReadCchPixmap (imgPathScore, imgFileScoreLed[1], COLOR_BLACK, cfgColor->statusFg, COLOR_BLUE);  
  // -XBCC
  /* init other score tiles */
  for (i = 0; i < SBDead ; i ++) {
    pixScore[i] = ReadCchPixmap (imgPathScore, imgFileScoreTile[i], COLOR_BLACK, cfgColor->statusFg, cfgColor->statusBg);
    if (None == pixScore[i]) {
      return XBFalse;
    }
  }
  /* gc : draw block */
  xgcv.fill_style = FillTiled;
  gcDrawBlock     = XCreateGC(dpy, pix, GCFillStyle, &xgcv );
  /* zero block and explosion tiles */
  for (i = 0; i < MAX_TILE; i ++) {
    pixBlock[i]  = None;
    blockLast[i] = &blockList[i][0];
  }
  for (i = 0; i < MAX_EXPLOSION; i ++) {
    pixExplBlock[i] = None;
    explLast[i]     = &explList[i][0];
  }
  return XBTrue;
} /* InitTiles */

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
  assert (y < MAZE_H+4);
#else
  assert (y < MAZE_H+3);
#endif
  assert (block >= -1);
  assert (block < MAX_TILE);

  if (block >= 0) {
    blockLast[block]->x      = x*BLOCK_WIDTH;
    blockLast[block]->y      = y*BLOCK_HEIGHT;
    blockLast[block]->width  = BLOCK_WIDTH;
    blockLast[block]->height = BLOCK_HEIGHT;

    blockLast[block] ++;
  } else {
    voidLast->x      = x*BLOCK_WIDTH;
    voidLast->y      = y*BLOCK_HEIGHT;
    voidLast->width  = BLOCK_WIDTH;
    voidLast->height = BLOCK_HEIGHT;

    voidLast ++;
  }
} /* GUI_DrawBlock */

/* 
 * 
 */
void 
GUI_DrawExplosion (int x, int y, int block)
{
  assert (block >= 0);
  assert (block < MAX_EXPLOSION);
  
  explLast[block]->x      = x*BLOCK_WIDTH;
  explLast[block]->y      = y*BLOCK_HEIGHT;
  explLast[block]->width  = BLOCK_WIDTH;
  explLast[block]->height = BLOCK_HEIGHT;
  
  explLast[block] ++;
} /* GUI_DrawExplosion */

/* 
 *
 */
void
GUI_FlushBlocks (void)
{
  int i;

  /* void blocks */
  if (voidLast != voidList) {
    XFillRectangles (dpy, pix, gcClearPix, voidList, voidLast - voidList);
    voidLast = voidList;
  }
  /* normal blocks */
  for (i=0; i<MAX_TILE; i++) {
    if (pixBlock[i]  != None &&
	blockLast[i] != blockList[i]) {
      XSetTile(dpy, gcDrawBlock, pixBlock[i]);
      XFillRectangles(dpy, pix, gcDrawBlock, blockList[i], blockLast[i] - blockList[i]);
    }
    blockLast[i] = blockList[i];
  }
  /* explosion blocks */
  for (i=0; i<MAX_EXPLOSION; i++) {
    if (pixExplBlock[i] != None) {
      if (explLast[i] != explList[i]) {
	XSetTile(dpy, gcDrawBlock, pixExplBlock[i]);
	XFillRectangles(dpy, pix, gcDrawBlock, explList[i], explLast[i] - explList[i]);
	explLast[i] = explList[i];
      }
    }
  }
} /* GUI_FlushBlocks */

/*
 *
 */
void 
GUI_LoadBlockRgb (int id, const char *name)
{
  assert (id >= 0);
  assert (id < MAX_TILE);
  assert (pixBlock[id] == None);

  pixBlock[id] = ReadRgbPixmap (imgPathBlock, name);
  if(pixBlock[id]==None){
    fprintf(stderr," Faile to load rgb Block %s.ppm \n",name);
     pixBlock[id] =ReadRgbPixmap (imgPathBlock, "unknown-file");
  }
} /* GUI_LoadBlock */

/*
 *
 */
void 
GUI_LoadBlockCch (int id, const char *name, XBColor fg, XBColor bg, XBColor add)
{
  assert (id >= 0);
  assert (id < MAX_TILE);
  assert (pixBlock[id] == None);  

  pixBlock[id] = ReadCchPixmap (imgPathBlock, name, fg, bg, add);
  if(pixBlock[id]==None){
    fprintf(stderr," Faile to load Block %s.ppm \n",name);
    pixBlock[id] = ReadCchPixmap (imgPathBlock, "unknown-file", fg, bg, add);
  }
} /* GUI_LoadBlock */

/*
 *
 */
void
GUI_InitExplosionBlocks (void)
{
  int i;

  for (i=0; i < MAX_EXPLOSION; i++) {
    pixExplBlock[i] = XCreatePixmap(dpy, pix, BLOCK_WIDTH, BLOCK_HEIGHT, defDepth);
    /* draw floor tile into it */
    XSetTile (dpy, gcDrawBlock, pixBlock[0]);
    XFillRectangle (dpy, pixExplBlock[i], gcDrawBlock, 0, 0, BLOCK_WIDTH, BLOCK_HEIGHT);
    /* now copy explosion into it */
    CopyExplosion (pixExplBlock[i], i);
  }
} /* GUI_InitExplosionBlocks */

/* 
 *
 */
void 
GUI_FreeBlock (int in_pix)
{
  assert (in_pix >= 0);
  assert (in_pix < MAX_TILE);
  /* free pixmap */
  if (None != pixBlock[in_pix]) {
    XFreePixmap(dpy, pixBlock[in_pix]);
    pixBlock[in_pix] = None;
  }
} /* GUI_FreeBlock */

/* 
 *
 */
void 
GUI_FreeExplosionBlocks (void)
{
  int i;
  
  for (i = 0; i < MAX_EXPLOSION; i ++ ) {
    XFreePixmap(dpy, pixExplBlock[i]);
  }
} /* GUI_FreeExplosionBlocks */

/*
 *
 */
void
GUI_LoadPlayerScoreTiles (int player, const CFGPlayerGraphics *config)
{
  const CFGColor *cfgColor;
  XBColor         scoreColors[NUM_PLAYER_COLORS];

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
  pixScore[SBDead + player]   = ReadEpmPixmap (imgPathScore, imgFileScorePlayer[0], NUM_PLAYER_COLORS, scoreColors);
  pixScore[SBSick + player]   = ReadEpmPixmap (imgPathScore, imgFileScorePlayer[1], NUM_PLAYER_COLORS, scoreColors);
  pixScore[SBPlayer + player] = ReadEpmPixmap (imgPathScore, imgFileScorePlayer[2], NUM_PLAYER_COLORS, scoreColors);
  pixScore[SBAbort + player]     = ReadEpmPixmap (imgPathScore, imgFileScorePlayer[3], NUM_PLAYER_COLORS, scoreColors);
  pixScore[SBSickAbort + player] = ReadEpmPixmap (imgPathScore, imgFileScorePlayer[4], NUM_PLAYER_COLORS, scoreColors);
 
} /* GUI_LoadPlayerScoreTile */

/*
 *
 */
//#ifdef SMPF
void 
GUI_DrawScoreBlock (int x, int y, int block) // SMPF
     /*#else
void 
GUI_DrawScoreBlock (int x, int block)
#endif*/
{
  assert (block < MAX_SCORE_TILES);
  assert (pixScore[block] != None);

  XSetTile (dpy, gcDrawBlock, pixScore[block]);
  //#ifdef SMPF
  XFillRectangle (dpy, pix, gcDrawBlock, x*STAT_WIDTH, MAZE_H*BLOCK_HEIGHT+y*STAT_HEIGHT, STAT_WIDTH, STAT_HEIGHT);
  /*#else
  XFillRectangle (dpy, pix, gcDrawBlock, x*STAT_WIDTH, MAZE_H*BLOCK_HEIGHT, STAT_WIDTH, STAT_HEIGHT);
  #endif*/
} /* GUI_DrawScoreBlock */

/* 
 *
 */
void 
GUI_DrawTimeLed (int x, int block)
{
  //  if(block > 1) { block = 1;}
  assert (block < MAX_LEDS);
  assert (pixLeds[block] != None);

  XSetTile(dpy, gcDrawBlock, pixLeds[block]);
#ifdef SMPF
  XFillRectangle(dpy, pix, gcDrawBlock, x*LED_WIDTH, MAZE_H*BLOCK_HEIGHT + STAT_HEIGHT*2, LED_WIDTH, LED_HEIGHT);
#else
  XFillRectangle(dpy, pix, gcDrawBlock, x*LED_WIDTH, MAZE_H*BLOCK_HEIGHT + STAT_HEIGHT  , LED_WIDTH, LED_HEIGHT);
#endif
} /* GUI_DrawTimeLed  */

/*
 * end of file x11c_tile.c
 */
