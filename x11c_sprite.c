/*
 * file x11c_sprite.c - drawing sprites under X11
 *
 * $Id: x11c_sprite.c,v 1.3 2004/05/14 10:00:36 alfie Exp $
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
#include "x11c_sprite.h"
#include "gui.h"

#include "x11_common.h"
#include "x11c_image.h"

#include "geom.h"
#include "image.h"
#include "sprite.h"
#include "cfg_player.h"

/*
 * local variables
 */
static GC gcSpriteMask;
static GC gcSpriteBits;

static Pixmap pixBombMask[MAX_BOMBS][MAX_BOMB_ANIME];
static Pixmap pixBombBits[MAX_BOMBS][MAX_BOMB_ANIME];
static Pixmap pixExplMask[MAX_EXPLOSION];
static Pixmap pixExplBits[MAX_EXPLOSION];
static Pixmap pixEpmSpriteBits[MAX_PLAYER][MAX_ANIME_EPM];
static Pixmap pixEpmSpriteMask[MAX_PLAYER][MAX_ANIME_EPM];
static Pixmap pixPpmSpriteBits[MAX_ANIME_PPM];
static Pixmap pixPpmSpriteMask[MAX_ANIME_PPM];
static Pixmap pixIconBits[MAX_ICON_SPRITES];
static Pixmap pixIconMask[MAX_ICON_SPRITES];

static CFGPlayerGraphics gfxPlayer[MAX_PLAYER];

static const XBColor colorIcon[MAX_ICON_SPRITES] = {
  COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, 
  COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_RED,          COLOR_SPRING_GREEN, 
  COLOR_GRAY_75,      COLOR_RED,          COLOR_GREEN,        COLOR_BLUE, 
};
/* 
 * local function init_sprites_color
 */
XBBool
InitSprites (void)
{
  XGCValues xgcv;
  int i, j;

  /* gc for drawing mask */
  xgcv.foreground = whitePixel;
  xgcv.fill_style = FillStippled;
  gcSpriteMask  = XCreateGC(dpy, pix, GCFillStyle | GCForeground, &xgcv);
  /* gc for drawing sprite bits */
  xgcv.fill_style = FillTiled;
  gcSpriteBits  = XCreateGC(dpy, pix, GCFillStyle, &xgcv);

  /* bomb sprites */
  for (i = 0; i < MAX_BOMBS; i++) {
    for (j=0; j < MAX_BOMB_ANIME; j++) {
      pixBombMask[i][j] = ReadPbmBitmap (imgPathExpl, imgFileBomb[i][j]);
      pixBombBits[i][j] = ReadRgbPixmap (imgPathExpl, imgFileBomb[i][j]);
      if ( (None == pixBombBits[i][j]) || (None == pixBombMask[i][j]) ) {
	return XBFalse;
      }
    }
  }
  /* create explosion sprites */
  for (i = 0; i < MAX_EXPLOSION; i++) {
    /* mask */
    pixExplMask[i] = ReadPbmBitmap(imgPathExpl, imgFileExpl[i]);
    pixExplBits[i] = ReadRgbPixmap(imgPathExpl, imgFileExpl[i]);
    if ( (None == pixExplBits[i]) || (None == pixExplMask[i]) ) {
      return XBFalse;
    }
  }
  /* set default value for player sprites */
  for (i = 0; i < MAX_PLAYER; i ++) {
    for (j = 0; j < MAX_ANIME_EPM; j ++) {
      pixEpmSpriteBits[i][j] = pixEpmSpriteMask[i][j] = None;
      gfxPlayer[i].shape     = ATOM_INVALID;
      gfxPlayer[i].helmet    = COLOR_INVALID;      
      gfxPlayer[i].face      = COLOR_INVALID;      
      gfxPlayer[i].body      = COLOR_INVALID;      
      gfxPlayer[i].handsFeet = COLOR_INVALID;      
      gfxPlayer[i].armsLegs  = COLOR_INVALID;      
      gfxPlayer[i].white     = COLOR_INVALID;      
    }
  }
  /* create shared players sprites */
  for (j = 0; j < MAX_ANIME_PPM; j ++) {
    pixPpmSpriteBits[j] = ReadRgbPixmap (imgPathSprite, imgFileSpritePpm[j]);
    pixPpmSpriteMask[j] = ReadPbmBitmap (imgPathSprite, imgFileSpritePpm[j]);
    if ( (None == pixPpmSpriteBits[j]) || (None == pixPpmSpriteMask[j]) ) {
      return XBFalse;
    }
  }
  /* load all icons sprites */
  for (i = 0; i < MAX_ICON_SPRITES; i ++) {
    if (i == ISA_Target)
        pixIconBits[i] = ReadCchPixmap (imgPathMisc, imgFileIcon[i], COLOR_BLACK, COLOR_GOLD, COLOR_SADDLE_BROWN);
    else if (i == ISA_TargetAboutToWin)
        pixIconBits[i] = ReadCchPixmap (imgPathMisc, imgFileIcon[i], COLOR_BLACK, COLOR_GOLD, COLOR_RED);
    else if (i == ISA_Loser)
        pixIconBits[i] = ReadCchPixmap (imgPathMisc, imgFileIcon[i], COLOR_BLACK, COLOR_SADDLE_BROWN, SET_COLOR(10, 4, 1));
    else
        pixIconBits[i] = ReadCchPixmap (imgPathMisc, imgFileIcon[i], COLOR_BLACK, colorIcon[i], COLOR_LIGHT_GOLDENROD);
    pixIconMask[i] = ReadPbmBitmap (imgPathMisc, imgFileIcon[i]);
  }
  /* that's all */
  return XBTrue;
} /* Init Sprites */

/*
 * load a single player sprite animation
 */
XBBool
GUI_LoadPlayerSprite (int player, int anime, const CFGPlayerGraphics *config)
{
  int i;
  const char *epmName;

  assert (player < MAX_PLAYER);
  assert (config != NULL);

  if (! ComparePlayerGraphics (config, gfxPlayer + player) ) {
    /* graphics has changed => delete all loaded pixmaps */
    for (i = 0; i < MAX_ANIME_EPM; i ++) {
      if (None != pixEpmSpriteBits[player][i]) {
	XFreePixmap (dpy, pixEpmSpriteBits[player][i]);
	pixEpmSpriteBits[player][i] = None;
      }
      if (None != pixEpmSpriteMask[player][i]) {
	XFreePixmap (dpy, pixEpmSpriteMask[player][i]);
	pixEpmSpriteMask[player][i] = None;
      }
    }
    gfxPlayer[player] = *config;
  }
  /* check if loading of pixmap is needed */
  if (ATOM_INVALID == config->shape) {
    pixEpmSpriteBits[player][anime] = None;
    pixEpmSpriteMask[player][anime] = None;
  } else {
    epmName = ImgFileSpriteEpm (config->shape, anime);
    if  (None == pixEpmSpriteBits[player][anime]) {
      pixEpmSpriteBits[player][anime] = ReadEpmPixmap (imgPathSprite, epmName, NUM_PLAYER_COLORS, &config->helmet);
      if (None == pixEpmSpriteBits[player][anime]) {
	return XBFalse;
      }
    }
    if (None == pixEpmSpriteMask[player][anime]) {
      pixEpmSpriteMask[player][anime] = ReadPbmBitmap (imgPathSprite, epmName);
      if (None == pixEpmSpriteMask[player][anime]) {
	return XBFalse;
      }
    }
  }
  return XBTrue;
} /* GUI_LoadPlayerSprite */

/*
 *
 */
void
GUI_LoadIconSprite (int index, XBColor color)
{
  assert (index >= 0);
  assert (index < MAX_COLOR_SPRITES);
  /* load sprite */
  if (pixIconBits[index] != None) {
    XFreePixmap (dpy, pixIconBits[index]);
  }
  if (pixIconMask[index] != None) {
    XFreePixmap (dpy, pixIconMask[index]);
  }
  pixIconBits[index] = ReadCchPixmap (imgPathMisc, imgFileIcon[index], COLOR_BLACK, color, COLOR_LIGHT_GOLDENROD);
  pixIconMask[index] = ReadPbmBitmap (imgPathMisc, imgFileIcon[index]);
} /* GUI_LoadColorSprite */

/*
 * draw a masked sprite
 */
static void
DrawSprite (const BMRectangle *rect, Pixmap bits, Pixmap mask)
{
  /* test values */
  assert (rect != NULL);
  assert (mask != None);
  assert (bits != None);
  /* draw it */
  XSetClipOrigin (dpy, gcSpriteBits, rect->x, rect->y);
  XSetTSOrigin (dpy, gcSpriteBits, rect->x, rect->y);
  XSetClipMask (dpy, gcSpriteBits, mask);
  XSetTile (dpy, gcSpriteBits, bits);
  XFillRectangle (dpy, pix, gcSpriteBits, rect->x, rect->y, rect->w, rect->h);
} /* DrawSprite */

/*
 * draw mask of sprite
 */
static void
DrawMask (const BMRectangle *rect, Pixmap mask)
{
  /* test values */
  assert (rect != NULL);
  assert (mask != None);
  /* draw it */
  XSetTSOrigin (dpy, gcSpriteMask, rect->x, rect->y);
  XSetStipple (dpy, gcSpriteMask, mask);
  XFillRectangle (dpy, pix, gcSpriteMask, rect->x, rect->y, rect->w, rect->h);
} /* DrawMask */

/*
 *
 */
void
CopyExplosion (Pixmap pix_tile, int i)
{
  static XGCValues xgcv;

  xgcv.clip_mask     = pixExplMask[i];
  xgcv.clip_y_origin = 0;
  xgcv.clip_x_origin = 0;
  xgcv.tile          = pixExplBits[i];
  xgcv.ts_y_origin   = 0;
  xgcv.ts_x_origin    = 0;
  XChangeGC (dpy, gcSpriteBits, GCClipMask|GCClipXOrigin|GCClipYOrigin| GCTile|GCTileStipXOrigin|GCTileStipYOrigin, &xgcv);
  XFillRectangle(dpy, pix_tile, gcSpriteBits, 0, 0, BLOCK_WIDTH, BLOCK_HEIGHT);
} /* CopyExplosion */

/* 
 * public function : draw_explosion 
 */
void 
GUI_DrawExplosionSprite (int x, int y, int block)
{
  BMRectangle rect;

  assert (block < MAX_EXPLOSION);
  rect.x = x*BLOCK_WIDTH;
  rect.y = y*BLOCK_HEIGHT;
  rect.w = BLOCK_WIDTH;
  rect.h = BLOCK_HEIGHT;
  DrawSprite (&rect, pixExplBits[block], pixExplMask[block]);
} /* GUI_DrawExplosionSprite */

/*
 * draw bomb 
 */
void
GUI_DrawBombSprite (const Sprite *ptr)
{ 
  int anime = SpriteAnime (ptr);
  int bomb  = SpriteBomb (ptr);

  assert (anime < MAX_BOMB_ANIME);
  assert (bomb < MAX_BOMBS);

  if (SpriteIsMasked (ptr)) {
    DrawMask (SpriteRectangle (ptr), pixBombMask[bomb][anime]);
  } else {
    DrawSprite (SpriteRectangle (ptr), pixBombBits[bomb][anime], pixBombMask[bomb][anime]);
  }
} /* GUI_DrawBombSprite */

/*
 *
 */
void
GUI_DrawPlayerSprite (const Sprite *ptr)
{
  Pixmap bits;
  Pixmap mask;
  int    anime  = SpriteAnime (ptr);
  int    player = SpritePlayer (ptr);
  
  assert (anime < MAX_ANIME);
  assert (player < MAX_PLAYER);

  if (anime >= MAX_ANIME_EPM) {
    bits = pixPpmSpriteBits[anime - MAX_ANIME_EPM];
    mask = pixPpmSpriteMask[anime - MAX_ANIME_EPM];
  } else {
    bits = pixEpmSpriteBits[player][anime];
    mask = pixEpmSpriteMask[player][anime];
  }
  if (SpriteIsMasked (ptr)) {
    DrawMask (SpriteRectangle (ptr), mask);
  } else {
    DrawSprite (SpriteRectangle (ptr), bits, mask);
  }
} /* GUI_DrawPlayerSprite */

/*
 *
 */
void
GUI_DrawIconSprite (const Sprite *ptr)
{
  int anime = SpriteAnime (ptr);               
  
  assert (anime < MAX_ICON_SPRITES);
  if (! SpriteIsMasked (ptr) ) {
    DrawSprite (SpriteRectangle (ptr), pixIconBits[anime], pixIconMask[anime]);
  } 
} /* GUI_DrawColorSprite */

/*
 *  draw sprite routine for text sprites
 */
void
GUI_DrawTextSprite (const Sprite *ptr)
{
  GUI_DrawTextbox (SpriteText (ptr), SpriteAnime (ptr), SpriteRectangle (ptr));
} /* GUI_DrawTextSprite */

/*
 * end of x11c_sprite.h
 */
