/*
 * file x11c_pixmap.c - double buffer for drawing
 *
 * $Id: sdl_pixmap.c,v 1.5 2006/03/28 11:50:25 fzago Exp $
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
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "xblast.h"

#include "sdl_common.h"
#include "sdl_image.h"

/*
 * local constants
 */
#define CLEAR_WIDTH  (24*BASE_X)
#define CLEAR_HEIGHT (24*BASE_Y)
#define FADE_STEP 16

/*
 * local variables
 */

/* Changed by VVL (Chat) 12/11/99 */
#ifdef SMPF
static SDL_Rect xrec[MAZE_W * (MAZE_H + 3) + STAT_W * 4];
#else
static SDL_Rect xrec[MAZE_W * (MAZE_H + 2) + STAT_W * 4];
#endif
int counter = 0;
static SDL_Rect *xrecMax = xrec;

/* fading */
static XBFadeMode fadeMode;
static int fadeMax;				/* maximum y coordinate */
static int fadeStep;			/* step width between lines */

/* GC to use*/
static Uint32 fadeColor;
static SDL_Surface *clearPix;

/* This SDL_Surface is used as a work around to blit back the screen after a
   fading. The right fix would be to force an entire drawing after fading.
   TODO: remove this hack and force entire drawing where needed.
*/

static SDL_Surface *screen_copy;

/*
 *
 */
XBBool
InitPixmap (void)
{

	SDL_Surface *temp;
	temp =
		ReadCchPixmap (imgPathMisc, imgFileTitle, COLOR_BLACK, COLOR_GRAY_75, COLOR_MIDNIGHT_BLUE);
	if (!temp) {
		fprintf (stderr, "Error: file not found.\n");
		return XBFalse;
	}
	clearPix = SDL_DisplayFormat (temp);
	SDL_FreeSurface (temp);
	if (clearPix == NULL) {
		fprintf (stderr, "ReadCchPixmap Failed in InitPixmap\n");
		return XBFalse;
	};

	screen_copy = SDL_CreateRGBSurface (screen->flags, screen->w, screen->h,
										screen->format->BitsPerPixel, screen->format->Rmask,
										screen->format->Gmask, screen->format->Bmask,
										screen->format->Amask);
	if (screen_copy == NULL) {
		fprintf (stderr, "Error: failed to init screen_copy. Reason: %s\n", SDL_GetError ());
		return XBFalse;
	};

	return XBTrue;

}								/* InitPixmap */

/* 
 * Clear screen with clearpix
 */
void
GUI_ClearPixmap (void)
{

	int x;
	int y;

	SDL_Rect DstRect;
	DstRect.x = 0;
	DstRect.y = 0;
	DstRect.w = PIXW;
	for (DstRect.x = 0; DstRect.x < PIXW; x += CLEAR_WIDTH) {
		for (DstRect.y = 0; DstRect.y < PIXH + SCOREH; y += CLEAR_HEIGHT) {
			SDL_BlitSurface (clearPix, NULL, screen, &DstRect);
			DstRect.y += clearPix->h;
		}
		DstRect.x += clearPix->w;
	}
	SDL_Flip (screen);

}								/* GUI_ClearPixmap */

/*
 *
 */
void
GUI_AddMazeRectangle (int x, int y)
{
	xrecMax->x = BLOCK_WIDTH * x;
	xrecMax->y = BLOCK_HEIGHT * y;
	xrecMax->w = BLOCK_WIDTH;
	xrecMax->h = BLOCK_HEIGHT;

	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;
		if ((prev->y == xrecMax->y) && ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;
		}
	}

	xrecMax++;
}								/* GUI_AddMazeRectangle */

/*
 *
 */
void
GUI_AddStatRectangle (int x, int y)
{
	xrecMax->x = x * STAT_WIDTH;
	xrecMax->y = MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT;
	xrecMax->w = STAT_WIDTH;
	xrecMax->h = (y < STAT_H) ? STAT_HEIGHT : LED_HEIGHT;

	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;
		/* try to join rectangles */
		if ((prev->y == xrecMax->y) && ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;
			counter--;
		}
	}

	xrecMax++;
}								/* GUI_AddStatRectangle */

/* Added by VVL (Chat) 12/11/99 : Begin */
/* Added by VVL (Chat) 12/11/99 : Begin */
void
GUI_AddChatRectangle (int x, int y)
{
#ifdef SMPF
	int i = 2;
#else
	int i = 1;
#endif
	int j;
	j = MAZE_W * (MAZE_H + 1) + STAT_W * 4;
	xrecMax->h = i * STAT_HEIGHT + BLOCK_HEIGHT + 8;
	xrecMax->x = x * STAT_WIDTH;
	xrecMax->y = (MAZE_H + i) * BLOCK_HEIGHT;
	xrecMax->w = STAT_WIDTH;

	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;
		if ((prev->y == xrecMax->y)
			&& ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;

		}
	}
	if (xrec + MAZE_W * (MAZE_H + 1) + STAT_W * 4 == xrecMax) {
		return;
	}
	xrecMax++;

}

/* Added by VVL (Chat) 12/11/99 : End */
/* Added by VVL (Chat) 12/11/99 : End */
/* Added by VVL (Chat) 12/11/99 : Begin */
/*
void 
AddTilesRectangle (int x, int y)
{  
} */

void
GUI_AddTilesRectangle (int x, int y)
{
#ifdef SMPF
	int i = 0;
#else
	int i = 0;
#endif
	xrecMax->h = i * STAT_HEIGHT;
	xrecMax->x = x * STAT_WIDTH;
	xrecMax->y = (MAZE_H + i) * BLOCK_HEIGHT;
	xrecMax->w = STAT_WIDTH;
	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;

		if ((prev->y == xrecMax->y)
			&& ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;
		}
	}

	xrecMax++;

}

/* Added by VVL (Chat) 12/11/99 : End */

/*
 *
 */
void
GUI_FlushPixmap (XBBool flag)
{
	SDL_Flip (screen);

	if (flag) {
		counter = 0;
		xrecMax = xrec;
	}
}								/* GUI_FlushPixmap  */

/*
 *
 */
void
GUI_FlushScoreBoard (void)
{
}								/* GUI_FlushScoreBoard  */

/*
 *
 */
// maxLines are not used in this SDL implementation.
void
GUI_InitFade (XBFadeMode mode, int maxLines)
{
	fadeMax = maxLines;
	fadeStep = FADE_STEP;
	fadeMode = mode;
	switch (mode) {
	case XBFM_BLACK_OUT:
		fadeColor = SDL_MapRGB (screen->format, 0, 0, 0);
		break;
	case XBFM_WHITE_OUT:
		fadeColor = SDL_MapRGB (screen->format, 0xFF, 0xFF, 0xFF);
		break;
	case XBFM_IN:				// just to keep compiler happy
		break;
	}
	if (mode != XBFM_IN)		// save a copy of the current screen (before fade)
	{
		SDL_BlitSurface (screen, NULL, screen_copy, NULL);
	}
}								/* GUI_InitFade */

/*
 * 
 */

XBBool
GUI_DoFade (void)
{
	int y;
	SDL_Rect r;

	if (fadeStep <= 0) {
		if (fadeMode != XBFM_IN)
			SDL_FillRect (screen, NULL, fadeColor);
		SDL_Flip (screen);
		SDL_Delay (200);
		if (fadeMode != XBFM_IN)
			SDL_BlitSurface (screen_copy, NULL, screen, NULL);
		return XBFalse;
	}

	r.x = 0;
	r.w = screen->w;
	r.h = (FADE_STEP + 1) - fadeStep;
	for (y = 0; y < screen->h; y += FADE_STEP) {
		r.y = y;
		if (fadeMode == XBFM_IN)
			SDL_UpdateRect (screen, r.x, r.y, r.w, r.h);
		else					// fade out
			SDL_FillRect (screen, &r, fadeColor);
	}

	if (fadeMode != XBFM_IN)
		SDL_Flip (screen);

	/* preparing next fade. */
	fadeStep -= 2;

	/* thats all */
	return XBTrue;
}								/* GUI_FadeOut */

/*
 * end of file x11c_pixmap.c
 */
