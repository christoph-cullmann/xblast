/*
 * file w32_sprite.c - drawing sprites
 *
 * $Id: w32_sprite.c,v 1.2 2004/05/14 10:00:36 alfie Exp $
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
#include "w32_sprite.h"
#include "gui.h"

#include "w32_pixmap.h"
#include "w32_image.h"

#include "geom.h"
#include "image.h"

/*
 * local variables
 */
static HDC hdcPix    = NULL;
static HDC hdcSprite = NULL;

static HBITMAP pixBombMask[MAX_BOMBS][MAX_BOMB_ANIME];
static HBITMAP pixBombBits[MAX_BOMBS][MAX_BOMB_ANIME];
static HBITMAP pixExplMask[MAX_EXPLOSION];
static HBITMAP pixExplBits[MAX_EXPLOSION];
static HBITMAP pixEpmSpriteBits[MAX_PLAYER][MAX_ANIME_EPM];
static HBITMAP pixEpmSpriteMask[MAX_PLAYER][MAX_ANIME_EPM];
static HBITMAP pixPpmSpriteBits[MAX_ANIME_PPM];
static HBITMAP pixPpmSpriteMask[MAX_ANIME_PPM];
static HBITMAP pixIconBits[MAX_ICON_SPRITES];
static HBITMAP pixIconMask[MAX_ICON_SPRITES];

/*
 * 
 */
static void
ClipSpriteBitmap (HBITMAP bits, HBITMAP mask, int width, int height)
{
  HBITMAP clip;
  HGDIOBJ oldSprite;
  HGDIOBJ oldPix;

  /* create clipping bitmap */
  clip       = CreateBitmap (width, height, 1, 1, NULL);
  assert (clip != NULL);
  oldPix    = SelectObject (hdcPix, clip);
  oldSprite = SelectObject (hdcSprite, mask);
  BitBlt (hdcPix, 0, 0, width, height, hdcSprite, 0, 0, NOTSRCCOPY);
  /* now clip bits */
  SelectObject (hdcSprite, bits);
  BitBlt (hdcSprite, 0, 0, width, height, hdcPix, 0, 0, SRCAND);
  /* clean up */
  SelectObject (hdcSprite, oldSprite);
  SelectObject (hdcPix, oldPix);
  DeleteObject (clip);
} /* ClipSpriteBitmap */

/*
 *
 */
XBBool
InitSprites (void)
{
  int i, j;
  HDC hdc;

  /* create own device contextes */
  hdc       = GetDC (window);
  hdcPix    = CreateCompatibleDC (hdc);
  hdcSprite = CreateCompatibleDC (hdc);
  ReleaseDC (window, hdc);
  /* zero all arrays */
  memset (pixBombMask,      0, sizeof (pixBombMask));
  memset (pixBombBits,      0, sizeof (pixBombBits));
  memset (pixExplMask,      0, sizeof (pixExplMask));
  memset (pixExplBits,      0, sizeof (pixExplBits));
  memset (pixEpmSpriteMask, 0, sizeof (pixEpmSpriteMask));
  memset (pixEpmSpriteBits, 0, sizeof (pixEpmSpriteBits));
  memset (pixPpmSpriteMask, 0, sizeof (pixPpmSpriteMask));
  memset (pixPpmSpriteBits, 0, sizeof (pixPpmSpriteBits));
  memset (pixIconMask,      0, sizeof (pixIconMask));
  memset (pixIconBits,      0, sizeof (pixIconBits));
  /* bomb sprites */
  for (i = 0; i < MAX_BOMBS; i++) {
    for (j=0; j < MAX_BOMB_ANIME; j++) {
      pixBombMask[i][j] = ReadPbmBitmap (imgPathExpl, imgFileBomb[i][j]);
      pixBombBits[i][j] = ReadRgbPixmap (imgPathExpl, imgFileBomb[i][j]);
      if (NULL ==  pixBombMask[i][j] || 
	  NULL ==  pixBombBits[i][j] ) {
	return XBFalse;
      }
      ClipSpriteBitmap (pixBombBits[i][j], pixBombMask[i][j], BLOCK_WIDTH, BLOCK_HEIGHT);
    }
  }
  /* create explosion sprites */
  for (i = 0; i < MAX_EXPLOSION; i++) {
    pixExplMask[i] = ReadPbmBitmap (imgPathExpl, imgFileExpl[i]);
    pixExplBits[i] = ReadRgbPixmap (imgPathExpl, imgFileExpl[i]);
    if (NULL ==  pixExplMask[i] || 
	NULL ==  pixExplBits[i] ) {
      return XBFalse;
    }
    ClipSpriteBitmap (pixExplBits[i], pixExplMask[i], BLOCK_WIDTH, BLOCK_HEIGHT);
  }
  /* load icon sprites */
  for (i = 0; i < MAX_ICON_SPRITES; i ++) {
    pixIconMask[i] = ReadPbmBitmap (imgPathMisc, imgFileIcon[i]);
    pixIconBits[i] = ReadCchPixmap (imgPathMisc, imgFileIcon[i], COLOR_BLACK,
				      (i != ISA_Abort) ? COLOR_SPRING_GREEN : COLOR_RED,
				      COLOR_LIGHT_GOLDENROD);
    if (NULL ==  pixIconMask[i] || 
	NULL ==  pixIconBits[i] ) {
      return XBFalse;
    }
    ClipSpriteBitmap (pixIconBits[i], pixIconMask[i], imgRectIcon[i].w, imgRectIcon[i].h);
  }
  /* create shared players sprites */
  for (j = 0; j < MAX_ANIME_PPM; j ++) {
    pixPpmSpriteBits[j] = ReadRgbPixmap (imgPathSprite, imgFileSpritePpm[j]);
    pixPpmSpriteMask[j] = ReadPbmBitmap (imgPathSprite, imgFileSpritePpm[j]);
    if ( NULL == pixPpmSpriteBits[j] || 
	 NULL == pixPpmSpriteMask[j] ) {
      return XBFalse;
    }
    ClipSpriteBitmap (pixPpmSpriteBits[j], pixPpmSpriteMask[j], 
                      imgRectSprite[j + MAX_ANIME_EPM].w, imgRectSprite[j + MAX_ANIME_EPM].h);
  }
  /* everything is fine */
  return XBTrue;
} /* InitSprites */

/*
 * clan up the mess
 */
void
FinishSprites (void)
{
  int i, j;

  /* bomb sprites */
  for (i = 0; i < MAX_BOMBS; i ++) {
    for (j = 0; j < MAX_BOMB_ANIME; j ++) {
      if (NULL != pixBombMask[i][j]) {
        DeleteObject (pixBombMask[i][j]);
      }
      if (NULL != pixBombBits[i][j]) {
        DeleteObject (pixBombBits[i][j]);
      }
    }
  }
  /* explosion sprites */
  for (i = 0; i < MAX_EXPLOSION; i ++) {
    if (NULL != pixExplMask[i]) {
      DeleteObject (pixExplMask[i]);
    }
    if (NULL != pixExplBits[i]) {
      DeleteObject (pixExplBits[i]);
    }
  }
  /* player sprites */
  for (i = 0; i < MAX_PLAYER; i ++) {
    for (j = 0; j < MAX_ANIME_EPM; j ++) {
      if (NULL != pixEpmSpriteMask[i][j]) {
        DeleteObject (pixEpmSpriteMask[i][j]);
      }
      if (NULL != pixEpmSpriteBits[i][j]) {
        DeleteObject (pixEpmSpriteBits[i][j]);
      }
    }
  }
  for (i = 0; i < MAX_ANIME_PPM; i ++) {
    if (NULL != pixPpmSpriteMask[i]) {
      DeleteObject (pixPpmSpriteMask[i]);
    }
    if (NULL != pixEpmSpriteBits[i][j]) {
      DeleteObject (pixPpmSpriteBits[i]);
    }
  }
  /* icons */
  for (i = 0; i < MAX_ICON_SPRITES; i ++) {
    if (NULL != pixIconBits[i]) {
      DeleteObject (pixIconBits[i]);
    }
    if (NULL != pixIconMask[i]) {
      DeleteObject (pixIconMask[i]);
    }
  }
  /* device contextes */
  if (NULL != hdcPix) {
    DeleteDC (hdcPix);
  }
  if (NULL != hdcSprite) {
    DeleteDC (hdcSprite);
  }
} /* FinishSprites */

/*
 * load sprites for given player
 */
XBBool
GUI_LoadPlayerSprite (int player, int i, const CFGPlayerGraphics *config)
{
  const char *epmName;

  assert (player < MAX_PLAYER);
  assert (i < MAX_ANIME_EPM);
  assert (config != NULL);

  /* delete old sprite */
  if (NULL != pixEpmSpriteMask[player][i]) {
    DeleteObject (pixEpmSpriteMask[player][i]);
    pixEpmSpriteMask[player][i] = NULL;
  }
  if (NULL != pixEpmSpriteBits[player][i]) {
    DeleteObject (pixEpmSpriteBits[player][i]);
    pixEpmSpriteBits[player][i] = NULL;
  }
  /* load sprite */
  if (ATOM_INVALID == config->shape) {
    pixEpmSpriteBits[player][i] = NULL;
    pixEpmSpriteMask[player][i] = NULL;
  } else {
    epmName = ImgFileSpriteEpm (config->shape, i);
    pixEpmSpriteBits[player][i] = ReadEpmPixmap (imgPathSprite, epmName, NUM_PLAYER_COLORS, &config->helmet);
    pixEpmSpriteMask[player][i] = ReadPbmBitmap (imgPathSprite, epmName);
    if ( NULL == pixEpmSpriteBits[player][i] || 
	 NULL == pixEpmSpriteMask[player][i] ) {
      /* error while loading */
      return XBFalse;
    }
    ClipSpriteBitmap (pixEpmSpriteBits[player][i], pixEpmSpriteMask[player][i], 
		      imgRectSprite[i].w, imgRectSprite[i].h);
  }
  return XBTrue;
} /* GUI_LoadPlayerSprite */

/*
 * load sprite for color selector
 */
void 
GUI_LoadIconSprite (int index, XBColor color)
{
  assert (index >= 0);
  assert (index < MAX_COLOR_SPRITES);

  /* free bitmaps if necessary */
  if (NULL != pixIconBits[index]) {
    DeleteObject (pixIconBits[index]);
  }
  if (NULL != pixIconMask[index]) {
    DeleteObject (pixIconMask[index]);
  }
  /* now load it */
  pixIconBits[index] = ReadCchPixmap (imgPathMisc, imgFileIcon[index], COLOR_BLACK, color, COLOR_LIGHT_GOLDENROD);
  pixIconMask[index] = ReadPbmBitmap (imgPathMisc, imgFileIcon[index]);
  ClipSpriteBitmap (pixIconBits[index], pixIconMask[index], imgRectIcon[index].w, imgRectIcon[index].h);
} /* GUI_LoadColorSprite */

/*
 * draw sprite into pixmap buffer
 */
static void
DrawSprite (const BMRectangle *rect, HBITMAP bits, HBITMAP mask)
{
  HGDIOBJ oldPix;
  HGDIOBJ oldSprite;

  assert (rect != NULL);
  assert (bits != NULL);
  assert (mask != NULL);
  oldPix = SelectObject (hdcPix, pix);
  /* draw mask */
  oldSprite = SelectObject (hdcSprite, mask);
  BitBlt (hdcPix, rect->x, rect->y, rect->w, rect->h, hdcSprite, 0, 0, SRCAND);
  /* draw bits */
  SelectObject (hdcSprite, bits);
  BitBlt (hdcPix, rect->x, rect->y, rect->w, rect->h, hdcSprite, 0, 0, SRCPAINT);
  /* finish drawing */
  SelectObject (hdcPix, oldPix);
  SelectObject (hdcSprite, oldSprite);
} /* DrawSprite */

/*
 * draw sprite mask into pixmap buffer
 */
static void
DrawMask (const BMRectangle *rect, HBITMAP mask)
{
  HGDIOBJ oldPix;
  HGDIOBJ oldSprite;

  assert (rect != NULL);
  assert (mask != NULL);
  oldPix = SelectObject (hdcPix, pix);
  /* draw mask */
  oldSprite = SelectObject (hdcSprite, mask);
  BitBlt (hdcPix, rect->x, rect->y, rect->w, rect->h, hdcSprite, 0, 0, MERGEPAINT);
  /* finish drawing */
  SelectObject (hdcPix, oldPix);
  SelectObject (hdcSprite, oldSprite);
} /* DrawSprite */

/*
 * copy explosion sprite into a tile 
 */
void
CopyExplosion (HBITMAP pix_tile, int i)
{
  HGDIOBJ oldPix;
  HGDIOBJ oldSprite;
  
  /* prepare drawing */
  oldPix    = SelectObject (hdcPix, pix_tile);
  /* draw mask */
  oldSprite = SelectObject (hdcSprite, pixExplMask[i]);
  BitBlt (hdcPix, 0, 0, BLOCK_WIDTH, BLOCK_HEIGHT, hdcSprite, 0, 0, SRCAND);
  /* draw bits */
  SelectObject (hdcSprite, pixExplBits[i]);
  BitBlt (hdcPix, 0, 0, BLOCK_WIDTH, BLOCK_HEIGHT, hdcSprite, 0, 0, SRCPAINT);
  /* finish drawing */
  SelectObject (hdcSprite, oldSprite);
  SelectObject (hdcPix, oldPix);
} /* CopyExplosion */

/*
 * draw explosion as sprite (fopr intro)
 */
void 
GUI_DrawExplosionSprite (int x, int y, int block)
{
  BMRectangle rect;

  assert (block < MAX_EXPLOSION);
  /* set region */
  rect.x = x * BLOCK_WIDTH;
  rect.y = y * BLOCK_HEIGHT;
  rect.w = BLOCK_WIDTH;
  rect.h = BLOCK_HEIGHT;
  /* draw sprite */
  DrawSprite (&rect, pixExplBits[block], pixExplMask[block]);
} /* GUI_DrawExplosionSprite */

/*
 * draw a bomb sprite
 */
void
GUI_DrawBombSprite (const Sprite *ptr)
{ 
  int bomb  = SpriteBomb (ptr);
  int anime = SpriteAnime (ptr);

  assert (bomb  < MAX_BOMBS);
  assert (anime < MAX_BOMB_ANIME);
  if (SpriteIsMasked (ptr)) {
    DrawMask (SpriteRectangle (ptr), pixBombMask[bomb][anime]);
  } else {
    DrawSprite (SpriteRectangle (ptr), pixBombBits[bomb][anime], pixBombMask[bomb][anime]);
  }  
} /* GUI_DrawBombSprite */

/*
 * draw a player sprite
 */
void
GUI_DrawPlayerSprite (const Sprite *ptr)
{
  HBITMAP bits;
  HBITMAP mask;
  int     anime  = SpriteAnime (ptr);
  int     player = SpritePlayer (ptr);
  
  assert (anime  < MAX_ANIME);
  assert (player < MAX_PLAYER);
  /* get mask an bitmap */
  if (anime >= MAX_ANIME_EPM) {
    bits = pixPpmSpriteBits[anime - MAX_ANIME_EPM];
    mask = pixPpmSpriteMask[anime - MAX_ANIME_EPM];
  } else {
    bits = pixEpmSpriteBits[player][anime];
    mask = pixEpmSpriteMask[player][anime];
  }
  /* draw sprite */
  if (SpriteIsMasked (ptr)) {
    DrawMask (SpriteRectangle (ptr), mask);
  } else {
    DrawSprite (SpriteRectangle (ptr), bits, mask);
  }  
} /* GUI_DrawPlayerSprite */

/*
 * draw color sprite
 */
void
GUI_DrawIconSprite (const Sprite *ptr)
{
  int anime = SpriteAnime (ptr);

  assert (anime < MAX_ICON_SPRITES);
  if (! SpriteIsMasked (ptr)) {
    DrawSprite (SpriteRectangle (ptr), pixIconBits[anime], pixIconMask[anime]);
  }  
} /* GUI_DrawColorSprite */

/*
 * draw textbox sprite
 */
void
GUI_DrawTextSprite (const Sprite *ptr)
{
  GUI_DrawTextbox (SpriteText (ptr), SpriteAnime (ptr), SpriteRectangle (ptr));
} /* GUI_DrawTextSprite */

/*
 *  end of file w32_sprite.c
 */
