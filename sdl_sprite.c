/*
 * file x11c_sprite.c - drawing sprites under X11
 *
 * $Id: sdl_sprite.c 112466 2009-07-06 08:37:37Z ingmar $
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
static SDL_Surface *SpriteBits;

static SDL_Surface *pixBombMask[MAX_BOMBS][MAX_BOMB_ANIME];
static SDL_Surface *pixBombBits[MAX_BOMBS][MAX_BOMB_ANIME];
static SDL_Surface *pixExplMask[MAX_EXPLOSION];
static SDL_Surface *pixExplBits[MAX_EXPLOSION];
static SDL_Surface *pixEpmSpriteBits[MAX_PLAYER][MAX_ANIME_EPM];
static SDL_Surface *pixEpmSpriteMask[MAX_PLAYER][MAX_ANIME_EPM];
static SDL_Surface *pixPpmSpriteBits[MAX_ANIME_PPM];
static SDL_Surface *pixPpmSpriteMask[MAX_ANIME_PPM];
static SDL_Surface *pixIconBits[MAX_ICON_SPRITES];
static SDL_Surface *pixIconMask[MAX_ICON_SPRITES];

static CFGPlayerGraphics gfxPlayer[MAX_PLAYER];

static const XBColor colorIcon[MAX_ICON_SPRITES] = {
	COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_SPRING_GREEN,
	COLOR_SPRING_GREEN, COLOR_SPRING_GREEN,
	COLOR_SPRING_GREEN, COLOR_SPRING_GREEN, COLOR_RED, COLOR_SPRING_GREEN,
	COLOR_GRAY_75, COLOR_RED, COLOR_GREEN, COLOR_BLUE,
};

static void
InvertBlackAndWhiteSurface (SDL_Surface * mask)
{

	int x, y;
	Uint32 pixel;
	for (y = 0; y < mask->h; y++) {
		for (x = 0; x < mask->w; x++) {
			pixel = getpixel (mask, x, y);
			putpixel (mask, x, y, !pixel * 0xffffffff);
		}
	}
	SDL_SetColorKey (mask, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);

	SDL_UpdateRect (mask, 0, 0, 0, 0);

	GUI_FlushPixmap (XBFalse);

}

/**
 * Apply the mask over the sprite image and set the 
 * transparent colorkey to specified sdl_color. 
 * sdl_color should respect the format specified
 * in sprite's surface->format.
 * ie. If sprite is 8bpp, keep sdl_color < 256.
 */
static void
MaskSprite (SDL_Surface * mask, SDL_Surface * sprite, Uint32 sdl_color)
{
	SDL_Color colors[2] = { {0, 0, 0, 0}, {0xFF, 0xFF, 0xFF, 0} };
	SDL_Color dst_colors[2] = { {0, 0, 0, 0}, {0xFF, 0xFF, 0xFF, 0} };

	SDL_Palette palette = { 2, colors };

	SDL_PixelFormat format = {
		&palette,
		8,
		1,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0,
		0
	};

	SDL_Surface *temp;
	Uint8 red, green, blue;

	temp = SDL_ConvertSurface (mask, &format, SDL_SRCCOLORKEY);

	assert (temp);

	SDL_GetRGB (sdl_color, sprite->format, &red, &green, &blue);

	dst_colors[1].r = red;
	dst_colors[1].g = green;
	dst_colors[1].b = blue;

	SDL_SetColors (temp, dst_colors, 0, 2);
	SDL_SetColorKey (temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, mask->format->colorkey);

	SDL_SetColorKey (sprite, SDL_SRCCOLORKEY | SDL_RLEACCEL, sdl_color);
	SDL_BlitSurface (temp, NULL, sprite, NULL);

	SDL_FreeSurface (temp);

}

/* does the same as above but tries to avoid the
problems of ppm has black and pbm has also black 
so letting some parts of the bomb be invisible like the 
shadow... */
static void
MaskBombSprite (SDL_Surface * mask, SDL_Surface * sprite, Uint32 sdl_color)
{
	int i, j;
	Uint32 bla, scolor;

	bla = SDL_MapRGB (mask->format, 0, 0, 0);
	scolor = SDL_MapRGB (sprite->format, 0xff, 0, 0);
	for (j = 0; j < mask->h; j++) {
		for (i = 0; i < mask->w; i++) {
			if (getpixel (mask, i, j) > bla) {
				putpixel (sprite, i, j, scolor);

			}
		}
	}
	SDL_SetColorKey (sprite, SDL_SRCCOLORKEY | SDL_RLEACCEL, scolor);

}

/* 
 * local function init_sprites_color
 */
XBBool
InitSprites (void)
{
	int i, j;

	// TODO: Choose a different ColorKey for transparency.
	// all whites are transparent :(

	SpriteBits = screen;

	/* bomb sprites */
	for (i = 0; i < MAX_BOMBS; i++) {
		for (j = 0; j < MAX_BOMB_ANIME; j++) {
			pixBombMask[i][j] = ReadPbmBitmap (imgPathExpl, imgFileBomb[i][j]);
			pixBombBits[i][j] = ReadRgbPixmap (imgPathExpl, imgFileBomb[i][j]);
			//MaskSprite(pixBombMask[i][j], pixBombBits[i][j], SDL_MapRGB(pixBombBits[i][j]->format, 1, 1, 0));
			MaskBombSprite (pixBombMask[i][j], pixBombBits[i][j],
							SDL_MapRGB (pixBombBits[i][j]->format, 1, 1, 0));
			InvertBlackAndWhiteSurface (pixBombMask[i][j]);
			if ((NULL == pixBombBits[i][j]) || (NULL == pixBombMask[i][j])) {
				return XBFalse;
			}
		}
	}
	/* create explosion sprites */
	for (i = 0; i < MAX_EXPLOSION; i++) {
		/* mask */
		pixExplMask[i] = ReadPbmBitmap (imgPathExpl, imgFileExpl[i]);
		pixExplBits[i] = ReadRgbPixmap (imgPathExpl, imgFileExpl[i]);
		MaskSprite (pixExplMask[i], pixExplBits[i], SDL_MapRGB (pixExplBits[i]->format, 0, 1, 0));
		if ((NULL == pixExplBits[i]) || (NULL == pixExplMask[i])) {
			return XBFalse;
		}
	}
	/* set default value for player sprites */
	for (i = 0; i < MAX_PLAYER; i++) {
		for (j = 0; j < MAX_ANIME_EPM; j++) {
			pixEpmSpriteBits[i][j] = pixEpmSpriteMask[i][j] = NULL;
			gfxPlayer[i].shape = ATOM_INVALID;
			gfxPlayer[i].helmet = COLOR_INVALID;
			gfxPlayer[i].face = COLOR_INVALID;
			gfxPlayer[i].body = COLOR_INVALID;
			gfxPlayer[i].handsFeet = COLOR_INVALID;
			gfxPlayer[i].armsLegs = COLOR_INVALID;
			gfxPlayer[i].white = COLOR_INVALID;
		}
	}
	/* create shared players sprites */
	for (j = 0; j < MAX_ANIME_PPM; j++) {
		pixPpmSpriteBits[j] = ReadRgbPixmap (imgPathSprite, imgFileSpritePpm[j]);
		pixPpmSpriteMask[j] = ReadPbmBitmap (imgPathSprite, imgFileSpritePpm[j]);
		MaskSprite (pixPpmSpriteMask[j], pixPpmSpriteBits[j],
					SDL_MapRGB (pixPpmSpriteBits[j]->format, 1, 1, 1));
		InvertBlackAndWhiteSurface (pixPpmSpriteMask[j]);
		if ((NULL == pixPpmSpriteBits[j]) || (NULL == pixPpmSpriteMask[j])) {
			return XBFalse;
		}
	}
	/* load all icons soprites */
	for (i = 0; i < MAX_ICON_SPRITES; i++) {
		pixIconBits[i] =
			ReadCchPixmap (imgPathMisc, imgFileIcon[i], COLOR_BLACK, colorIcon[i],
						   COLOR_LIGHT_GOLDENROD);
		pixIconMask[i] = ReadPbmBitmap (imgPathMisc, imgFileIcon[i]);
		MaskSprite (pixIconMask[i], pixIconBits[i], SDL_MapRGB (pixIconBits[i]->format, 1, 1, 1));
	}
	/* that's all */
	return XBTrue;
}								/* Init Sprites */

/*
 * load a single player sprite animation
 */
XBBool
GUI_LoadPlayerSprite (int player, int anime, const CFGPlayerGraphics * config)
{
	int i;
	const char *epmName;

	assert (player < MAX_PLAYER);
	assert (config != NULL);

	if (!ComparePlayerGraphics (config, gfxPlayer + player)) {
		/* graphics has changed => delete all loaded pixmaps */
		for (i = 0; i < MAX_ANIME_EPM; i++) {
			if (NULL != pixEpmSpriteBits[player][i]) {
				SDL_FreeSurface (pixEpmSpriteBits[player][i]);
				pixEpmSpriteBits[player][i] = NULL;
			}
			if (NULL != pixEpmSpriteMask[player][i]) {
				SDL_FreeSurface (pixEpmSpriteMask[player][i]);
				pixEpmSpriteMask[player][i] = NULL;
			}
		}
		gfxPlayer[player] = *config;
	}
	/* check if loading of pixmap is needed */
	if (ATOM_INVALID == config->shape) {
		pixEpmSpriteBits[player][anime] = NULL;
		pixEpmSpriteMask[player][anime] = NULL;
	}
	else {
		epmName = ImgFileSpriteEpm (config->shape, anime);
		if (NULL == pixEpmSpriteBits[player][anime]) {
			pixEpmSpriteBits[player][anime] =
				ReadEpmPixmap (imgPathSprite, epmName, NUM_PLAYER_COLORS, &config->helmet);
			if (NULL == pixEpmSpriteBits[player][anime]) {
				return XBFalse;
			}
		}
		if (NULL == pixEpmSpriteMask[player][anime]) {
			pixEpmSpriteMask[player][anime] = ReadPbmBitmap (imgPathSprite, epmName);
			InvertBlackAndWhiteSurface (pixEpmSpriteMask[player][anime]);
			if (NULL == pixEpmSpriteMask[player][anime]) {
				return XBFalse;
			}
		}
	}
	return XBTrue;
}								/* GUI_LoadPlayerSprite */

/*
 *
 */
void
GUI_LoadIconSprite (int index, XBColor color)
{
	assert (index >= 0);
	assert (index < MAX_COLOR_SPRITES);
	/* load sprite */
	if (pixIconBits[index] != NULL) {
		SDL_FreeSurface (pixIconBits[index]);
	}
	if (pixIconMask[index] != NULL) {
		SDL_FreeSurface (pixIconMask[index]);
	}
	pixIconBits[index] =
		ReadCchPixmap (imgPathMisc, imgFileIcon[index], COLOR_BLACK, color, COLOR_LIGHT_GOLDENROD);
	pixIconMask[index] = ReadPbmBitmap (imgPathMisc, imgFileIcon[index]);
}								/* GUI_LoadColorSprite */

/*
 * draw a masked sprite
 */
static void
DrawSprite (const BMRectangle * rect, SDL_Surface * bits, SDL_Surface * mask)
{
	SDL_Rect r;

	/* test values */
	assert (rect != NULL);
	assert (mask != NULL);
	assert (bits != NULL);
	/* draw it */

	r.x = rect->x;
	r.y = rect->y;
	r.w = rect->w;
	r.h = rect->h;

	SDL_BlitSurface (bits, NULL, screen, &r);

}								/* DrawSprite */

/*
 * draw mask of sprite
 */
static void
DrawMask (const BMRectangle * rect, SDL_Surface * mask)
{
	SDL_Rect r;

	assert (rect != NULL);
	assert (mask != NULL);

	r.x = rect->x;
	r.y = rect->y;
	r.w = rect->w;
	r.h = rect->h;

	SDL_BlitSurface (mask, NULL, screen, &r);

}								/* DrawMask */

/*
 *
 */
void
CopyExplosion (SDL_Surface * pix_tile, int i)
{

	SDL_BlitSurface (pixExplBits[i], NULL, pix_tile, NULL);

}								/* CopyExplosion */

/* 
 * public function : draw_explosion 
  */
void
GUI_DrawExplosionSprite (int x, int y, int block)
{
	BMRectangle rect;

	assert (block < MAX_EXPLOSION);
	rect.x = x * BLOCK_WIDTH;
	rect.y = y * BLOCK_HEIGHT;
	rect.w = BLOCK_WIDTH;
	rect.h = BLOCK_HEIGHT;
	DrawSprite (&rect, pixExplBits[block], pixExplMask[block]);
}								/* GUI_DrawExplosionSprite */

/*
 * draw bomb 
 */
void
GUI_DrawBombSprite (const Sprite * ptr)
{
	int anime = SpriteAnime (ptr);
	int bomb = SpriteBomb (ptr);

	assert (anime < MAX_BOMB_ANIME);
	assert (bomb < MAX_BOMBS);

	if (SpriteIsMasked (ptr)) {
		DrawMask (SpriteRectangle (ptr), pixBombMask[bomb][anime]);
	}
	else {
		DrawSprite (SpriteRectangle (ptr), pixBombBits[bomb][anime], pixBombMask[bomb][anime]);
	}
}								/* GUI_DrawBombSprite */

/*
 *
 */
void
GUI_DrawPlayerSprite (const Sprite * ptr)
{
	SDL_Surface *bits;
	SDL_Surface *mask;
	int anime = SpriteAnime (ptr);
	int player = SpritePlayer (ptr);

	assert (anime < MAX_ANIME);
	assert (player < MAX_PLAYER);

	if (anime >= MAX_ANIME_EPM) {
		bits = pixPpmSpriteBits[anime - MAX_ANIME_EPM];
		mask = pixPpmSpriteMask[anime - MAX_ANIME_EPM];
	}
	else {
		bits = pixEpmSpriteBits[player][anime];
		mask = pixEpmSpriteMask[player][anime];
	}
	if (SpriteIsMasked (ptr)) {
		DrawMask (SpriteRectangle (ptr), mask);
	}
	else {
		DrawSprite (SpriteRectangle (ptr), bits, mask);
	}
}								/* GUI_DrawPlayerSprite */

/*
 *
 */
void
GUI_DrawIconSprite (const Sprite * ptr)
{
	int anime = SpriteAnime (ptr);

	assert (anime < MAX_ICON_SPRITES);
	if (!SpriteIsMasked (ptr)) {
		DrawSprite (SpriteRectangle (ptr), pixIconBits[anime], pixIconMask[anime]);
	}
}								/* GUI_DrawColorSprite */

/*
 *  draw sprite routine for text sprites
 */
void
GUI_DrawTextSprite (const Sprite * ptr)
{
	GUI_DrawTextbox (SpriteText (ptr), SpriteAnime (ptr), SpriteRectangle (ptr));
}								/* GUI_DrawTextSprite */

/*
 * end of x11c_sprite.h
 */
