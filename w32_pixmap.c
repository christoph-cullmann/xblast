/*
 * file w32_pixmap.c - double buffer for drawing
 *
 * $Id: w32_pixmap.c,v 1.11 2006/02/19 13:33:01 lodott Exp $
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
#include "w32_pixmap.h"
#include "gui.h"

#include "w32_image.h"

#include "geom.h"
#include "image.h"

/*
 * local constants
 */
#define CLEAR_WIDTH  (24*BASE_X)
#define CLEAR_HEIGHT (24*BASE_Y)
#define FADE_STEP    16
/* Changed by VVL (Chat) 12/11/99 */
/* Added by Fouf on 01/19/00 15:44:43 */
#define MAX_RECT     (MAZE_W*(MAZE_H) + STAT_W*4)
//#define MAX_RECT     (MAZE_W*MAZE_H + 4*STAT_W)

/*
 * local types
 */
typedef struct
{
	RGNDATAHEADER rdh;
	RECT rect[MAX_RECT];
} RegionData;

/*
 * local variables
 */
/* pixmap fro double buffering */
static HDC hdcPix = NULL;
static HBITMAP clearPix = NULL;
/* update rectangles for redraw */
static RegionData rgnData;
/* maximum y coordinate */
static int fadeMax;
/* step width between lines */
static int fadeStep;
/* fade mode*/
static XBFadeMode fadeMode;

/*
 * global function: GUI_ClearPixmap 
 * description:     clear pixmap buffer with standard pattern
 * parameters:      none
 * return value:    none
 */
void
GUI_ClearPixmap (void)
{
	HDC hdcSrc;
	HGDIOBJ oldPix;
	int x, y;

	/* get context for destination */
	hdcSrc = CreateCompatibleDC (hdcPix);
	oldPix = SelectObject (hdcPix, pix);
	(void)SelectObject (hdcSrc, clearPix);
	/* draw */
	for (x = 0; x < PIXW; x += CLEAR_WIDTH) {
		for (y = 0; y < PIXH + SCOREH; y += CLEAR_HEIGHT) {
			BitBlt (hdcPix, x, y, 192, 144, hdcSrc, 0, 0, SRCCOPY);
		}
	}
	/* get rid of the device contextes */
	(void)SelectObject (hdcPix, oldPix);
	DeleteDC (hdcSrc);
}								/* GUI_ClearPixmap */

/*
 * library function: ClearRactnagles
 * description:      clear given rectangles in pixmap
 * parameters:       rect   - point to array of rectangles
 *                   n_rect - number of rectangles in array
 * return value:     none
 */
void
ClearRectangles (HDC hdcDst, HDC hdcSrc, RECT * rect, int nRect)
{
	HGDIOBJ oldSrc;
	int i;

	assert (rect != NULL);
	/* get context for destination */
	oldSrc = SelectObject (hdcSrc, clearPix);
	/* draw */
	for (i = 0; i < nRect; i++) {
		BitBlt (hdcDst, rect->left, rect->top, BLOCK_WIDTH, BLOCK_HEIGHT,
				hdcSrc, rect->left % CLEAR_WIDTH, rect->top % CLEAR_HEIGHT, SRCCOPY);
		rect++;
	}
	/* get rid of the device contextes */
	(void)SelectObject (hdcSrc, oldSrc);
}								/* ClearRectangles */

/*
 * redraw window by painting parts of pixmap into it (for WM_PAINT)
 */
void
PaintPixmap (HWND window)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HGDIOBJ oldPix;
	HPALETTE oldPal = NULL;
	unsigned i;

	HRGN hRgn = CreateRectRgn (0, 0, 0, 0);
	assert (hRgn != NULL);
	if (GetUpdateRgn (window, hRgn, FALSE)) {
		/* get graphics context for window */
		hdc = BeginPaint (window, &ps);
		/* get region data */
		if (0 == GetRegionData (hRgn, sizeof (rgnData), (RGNDATA *) & rgnData) ||
			RDH_RECTANGLES != rgnData.rdh.iType) {
			/* update full window */
			rgnData.rdh.nCount = 1;
			rgnData.rect->left = 0;
			rgnData.rect->top = 0;
			rgnData.rect->right = PIXW;
			rgnData.rect->bottom = PIXH + SCOREH;
		}
		/* draw it */
		oldPix = SelectObject (hdcPix, pix);
		if (NULL != palette) {
			oldPal = SelectPalette (hdc, palette, FALSE);
			RealizePalette (hdc);
		}
		for (i = 0; i < rgnData.rdh.nCount; i++) {
			BitBlt (hdc, rgnData.rect[i].left, rgnData.rect[i].top,
					rgnData.rect[i].right - rgnData.rect[i].left,
					rgnData.rect[i].bottom - rgnData.rect[i].top, hdcPix, rgnData.rect[i].left,
					rgnData.rect[i].top, SRCCOPY);
		}
		if (NULL != palette) {
			(void)SelectPalette (hdc, oldPal, FALSE);
		}
		(void)SelectObject (hdcPix, oldPix);
		/* finish drawing */
		EndPaint (window, &ps);
	}
	DeleteObject (hRgn);
}								/* UpdatePixmapRect */

/*
 * library function: GUI_FlushPixmap
 * description:      copy pixmap to window
 * parameters:       flag - XBTrue only changed parts, XBFalse all of it
 * return value:     none
 */
void
GUI_FlushPixmap (XBBool flag)
{
	if (!flag) {
		InvalidateRect (window, NULL, FALSE);
	}
	UpdateWindow (window);
}								/* GUI_FlushPixmap */

/*
 *
 */
void
GUI_FlushScoreBoard (void)
{
	static const RECT rect = {
		0, PIXH, PIXW, PIXH + SCOREH
	};

	InvalidateRect (window, &rect, FALSE);
	UpdateWindow (window);
}								/* GUI_FlushScoreBoard */

/*
 * global function:  GUI_AddMazeRectangle
 * description:      add a map tile to list of rectangles, which are to be redrawn
 * parameters:       x - column of tile
 *                   y - row of tile
 * return value:     none
 */
void
GUI_AddMazeRectangle (int x, int y)
{
	RECT rect;

	rect.left = BLOCK_WIDTH * x;
	rect.top = BLOCK_HEIGHT * y;
	rect.right = BLOCK_WIDTH * (x + 1);
	rect.bottom = BLOCK_HEIGHT * (y + 1);

	InvalidateRect (window, &rect, FALSE);
}								/* GUI_AddMazeRectangle */

/*
 * global function: GUI_AddStatRectangle
 * description:      add a statusbar tile to list of rectangles, which are to be redrawn
 * parameters:       x - column of tile
 *                   y - row of tile
 * return value:     none
 */
void
GUI_AddStatRectangle (int x, int y)
{
	RECT rect;

	rect.left = STAT_WIDTH * x;
	rect.right = STAT_WIDTH * (x + 1);
	rect.top = MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT;
	if (++y < STAT_H) {
		rect.bottom = MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT;
	}
	else {
		rect.bottom = MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT + LED_HEIGHT;
	}
	//Dbg_Out("add rect %i %i %i %i max %i\n",rect.left ,rect.right ,rect.top ,rect.bottom,MAX_RECT );

	InvalidateRect (window, &rect, FALSE);
}

/* GUI_AddStatRectangle */
/* Added by VVL (Chat) 12/11/99 : Begin */
void
GUI_AddChatRectangle (int x, int y)
{
	int i = 0, j = 1;
	RECT rect;
	rect.left = x * STAT_WIDTH;
	rect.right = STAT_WIDTH * (x + 1);
#ifdef SMPF
	i = 0;
	j = 3;
#else
	i = 0;
#endif
	rect.top = (MAZE_H + j) * BLOCK_HEIGHT + i * STAT_HEIGHT + LED_HEIGHT + 8;
	rect.bottom = (MAZE_H + j) * BLOCK_HEIGHT + (i + 1) * STAT_HEIGHT + LED_HEIGHT + 8;
	//  Dbg_Out("add rect1 %i %i %i %i\n",rect.left ,rect.right ,rect.top ,rect.bottom );

	InvalidateRect (window, &rect, FALSE);
}

/* GUI_AddStatRectangle */
/* Added by VVL (Chat) 12/11/99 : Begin */
void
GUI_AddTilesRectangle (int x, int y)
{
	int i = 0, j = 0;
	RECT rect;
	rect.left = x * STAT_WIDTH;
	rect.right = STAT_WIDTH * (x + 1);
#ifdef SMPF
	i = 0;
	j = 0;
#else
	i = 0;
#endif
	rect.top = (MAZE_H + j) * BLOCK_HEIGHT + i * STAT_HEIGHT + LED_HEIGHT + 8;
	rect.bottom = (MAZE_H + j) * BLOCK_HEIGHT + (i + 1) * STAT_HEIGHT + LED_HEIGHT + 8;
	//  Dbg_Out("add rect1 %i %i %i %i\n",rect.left ,rect.right ,rect.top ,rect.bottom );

	InvalidateRect (window, &rect, FALSE);
}

/* Added by VVL (Chat) 12/11/99 : End */

/*
 * library function: InitPixmap 
 * description:      creates bitmap for double buffering 
 * parameters:       none
 * return value:     0 on success, -1 on failure;
 */
XBBool
InitPixmap (void)
{
	HDC hdc;

	/* get device context of window */
	hdc = GetDC (window);
	if (NULL == hdc) {
		return XBFalse;
	}
	/* create device context for drawing */
	hdcPix = CreateCompatibleDC (hdc);
	if (NULL == hdcPix) {
		return XBFalse;
	}
	/* now create compatible bitmap */
	pix = CreateCompatibleBitmap (hdc, PIXW, PIXH + SCOREH);
	if (NULL == pix) {
		return XBFalse;
	}
	/* Load Bitmap for clearing pixmap */
	clearPix =
		ReadCchPixmap (imgPathMisc, imgFileTitle, COLOR_BLACK, COLOR_GRAY_75, COLOR_MIDNIGHT_BLUE);
	if (NULL == clearPix) {
		return XBFalse;
	}
	/* give back the window device context */
	ReleaseDC (window, hdc);
	return XBTrue;
}								/* InitPixmap */

/*
 *
 */
void
FinishPixmap (void)
{
	if (NULL != hdcPix) {
		DeleteDC (hdcPix);
	}
	if (NULL != clearPix) {
		DeleteObject (clearPix);
	}
	if (NULL != pix) {
		DeleteObject (pix);
	}
}								/* FinishPixmap */

/*
 *
 */
void
GUI_InitFade (XBFadeMode mode, int maxLines)
{
	assert (maxLines <= PIXH + SCOREH);
	fadeMax = maxLines;
	fadeStep = FADE_STEP;
	fadeMode = mode;
}								/* GUI_InitFade */

/*
 * 
 */
XBBool
GUI_DoFade (void)
{
	HDC hdc;
	int y, yStep;

	if (fadeStep <= 0) {
		return XBFalse;
	}
	/* setup lines to draw */
	if (fadeStep == FADE_STEP) {
		y = 0;
		yStep = FADE_STEP;
	}
	else {
		y = fadeStep;
		yStep = 2 * fadeStep;
	}
	/* prepare drawing */
	hdc = GetDC (window);
	/* draw ... */
	if (fadeMode == XBFM_IN) {
		HGDIOBJ oldPix;
		oldPix = SelectObject (hdcPix, pix);
		for (; y < fadeMax; y += yStep) {
			BitBlt (hdc, 0, y, PIXW, 1, hdcPix, 0, y, SRCCOPY);
		}
		SelectObject (hdcPix, oldPix);
	}
	else {
		HGDIOBJ oldPen;
		HGDIOBJ newPen;

		if (fadeMode == XBFM_WHITE_OUT) {
			newPen = GetStockObject (WHITE_PEN);
		}
		else {
			newPen = GetStockObject (BLACK_PEN);
		}
		oldPen = SelectObject (hdc, GetStockObject (WHITE_PEN));
		for (; y < fadeMax; y += yStep) {
			MoveToEx (hdc, 0, y, NULL);
			LineTo (hdc, PIXW - 1, y);
		}
		SelectObject (hdc, oldPen);
		DeleteObject (newPen);
	}
	ReleaseDC (window, hdc);
	/* prepare next step */
	fadeStep /= 2;
	/* that´s all */
	return XBTrue;
}								/* GUI_FadeOut */

/*
 * end of file w32_pixmap.c
 */
