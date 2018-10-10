/*
 * file x11c_pixmap.c - double buffer for drawing
 *
 * $Id: x11c_pixmap.c,v 1.7 2004/10/19 17:59:19 iskywalker Exp $
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
#include "x11c_pixmap.h"
#include "gui.h"

#include "x11_common.h"
#include "x11c_image.h"
#include "x11_config.h"

#include "geom.h"
#include "color.h"
#include "image.h"

/*
 * local constants
 */
#define FADE_STEP 16

/*
 * local variables
 */
//static XRectangle  xrec[MAZE_W*MAZE_H + STAT_W*STAT_H];
/* Changed by VVL (Chat) 12/11/99 */
#ifdef SMPF 
 static XRectangle xrec[MAZE_W*(MAZE_H+2) + STAT_W*4];
#else
/* Added by Fouf on 01/19/00 15:44:43 */ static XRectangle xrec[MAZE_W*(MAZE_H+1) + STAT_W*4];
#endif
static XRectangle *xrecMax = xrec;
/* fading */
static int fadeMax;  /* maximum y coordinate */
static int fadeStep; /* step width between lines */
/* GC to use*/
static GC fadeGC;

/*
 *
 */
XBBool 
InitPixmap (void)
{
  XGCValues xgcv;
  const CFGColor *cfgColor;

  cfgColor = GetColorConfig ();
  assert (cfgColor != NULL);
  /* where to draw pixmap */
  pix             = XCreatePixmap (dpy, win, PIXW, PIXH+SCOREH, defDepth);
  /* gc : copy pixmap to window */
  xgcv.fill_style = FillTiled;
  xgcv.tile       = pix;
  gcFromPix       = XCreateGC (dpy, win, GCTile | GCFillStyle, &xgcv);
  /* gc : clear pixmap */
  xgcv.fill_style = FillTiled;
  xgcv.tile       = ReadCchPixmap (imgPathMisc, imgFileTitle, COLOR_BLACK, cfgColor->titleFg, cfgColor->titleBg);
  gcClearPix      = XCreateGC (dpy, pix, GCFillStyle | GCTile, &xgcv);
  /* that's all */
  return XBTrue;
} /* InitPixmap */

/* 
 *
 */
void 
GUI_ClearPixmap (void)
{
  XFillRectangle (dpy, pix, gcClearPix, 0, 0, PIXW, PIXH+SCOREH);
} /* GUI_ClearPixmap */

/*
 *
 */
void 
GUI_AddMazeRectangle (int x, int y)
{
  xrecMax->x      = BLOCK_WIDTH  * x;
  xrecMax->y      = BLOCK_HEIGHT * y;
  xrecMax->width  = BLOCK_WIDTH;
  xrecMax->height = BLOCK_HEIGHT;

  if (xrecMax != xrec) {
    XRectangle *prev = xrecMax - 1;
    if ( (prev->y == xrecMax->y) && ((xrecMax->x - prev->x) == prev->width) ) {
      prev->width += BLOCK_WIDTH;
      xrecMax = prev;
    }
  }

  xrecMax ++;
} /* GUI_AddMazeRectangle */

/*
 *
 */
void 
GUI_AddStatRectangle (int x, int y)
{
  xrecMax->x      = x * STAT_WIDTH;
  xrecMax->y      = MAZE_H * BLOCK_HEIGHT + y*STAT_HEIGHT;
  xrecMax->width  = STAT_WIDTH;
  xrecMax->height = (y <  STAT_H ) ? STAT_HEIGHT : LED_HEIGHT;

  if (xrecMax != xrec) {
    XRectangle *prev = xrecMax - 1;
    /* try to join rectangles */
    if ( (prev->y == xrecMax->y) && ((xrecMax->x - prev->x) == prev->width) ) {
      prev->width += BLOCK_WIDTH;
      xrecMax = prev;
    }
  }

  xrecMax ++;
} /* GUI_AddStatRectangle */
/* Added by VVL (Chat) 12/11/99 : Begin */
void 
GUI_AddChatRectangle (int x, int y)
{  
#ifdef SMPF
  int i=2;
#else
  int i=1;
#endif
  xrecMax->height = i*STAT_HEIGHT+BLOCK_HEIGHT+8;
  xrecMax->x      = x*STAT_WIDTH;
  xrecMax->y      = (MAZE_H+i)*BLOCK_HEIGHT;
  xrecMax->width  = STAT_WIDTH; 
  if (xrecMax != xrec) {
    XRectangle *prev = xrecMax - 1;
    
    if ( (prev->y == xrecMax->y) 
	 && ((xrecMax->x - prev->x) == prev->width) ) {
      prev->width += BLOCK_WIDTH;
      xrecMax = prev;
    }
  }
  
  
  xrecMax ++;
}
/* Added by VVL (Chat) 12/11/99 : End */
/* Added by VVL (Chat) 12/11/99 : Begin */
void 
GUI_AddTilesRectangle (int x, int y)
{  
#ifdef SMPF
  int i=0;
#else
  int i=0;
#endif
  xrecMax->height = i*STAT_HEIGHT;
  xrecMax->x      = x*STAT_WIDTH;
  xrecMax->y      = (MAZE_H+i)*BLOCK_HEIGHT;
  xrecMax->width  = STAT_WIDTH; 
  if (xrecMax != xrec) {
    XRectangle *prev = xrecMax - 1;
    
    if ( (prev->y == xrecMax->y) 
	 && ((xrecMax->x - prev->x) == prev->width) ) {
      prev->width += BLOCK_WIDTH;
      xrecMax = prev;
    }
  }
  
  
  xrecMax ++;
}
/* Added by VVL (Chat) 12/11/99 : End */

/*
 *
 */
void 
GUI_FlushPixmap (XBBool flag)
{
  int i;

  if (!flag) {
    /* Copy Pixmap to Window */
    for (i = 0; i < 4; i ++) {
      XFillRectangle (dpy, win, gcFromPix, 0, (PIXH + SCOREH)/4 * i, PIXW, (PIXH + SCOREH)/4 );
    }
  } else {
    if (!iconified) {
      XFillRectangles (dpy, win, gcFromPix, xrec, xrecMax - xrec);
    }
    xrecMax = xrec;
  }
  GUI_Sync ();
} /* GUI_FlushPixmap  */

/*
 *
 */
void 
GUI_FlushScoreBoard (void)
{
  XFillRectangle (dpy, win, gcFromPix, 0, PIXH, PIXW, SCOREH);
  GUI_Sync ();
} /* GUI_FlushScoreBoard  */

/*
 *
 */
void
GUI_InitFade (XBFadeMode mode, int maxLines)
{
  assert (maxLines <= PIXH + SCOREH);
  fadeMax  = maxLines;
  fadeStep = FADE_STEP;
  switch (mode) {
  case XBFM_IN:
    fadeGC = gcFromPix;
    break;
  case XBFM_BLACK_OUT:
    fadeGC = gcWindow;
    XSetForeground (dpy, gcWindow, blackPixel);
    break;
  case XBFM_WHITE_OUT:
    fadeGC = gcWindow;
    XSetForeground (dpy, gcWindow, whitePixel);
    break;
  }
} /* GUI_InitFade */

/*
 * 
 */
XBBool
GUI_DoFade (void)
{
  int y, yStep;

  if (fadeStep <= 0) {
    return XBFalse;
  }
  /* setup lines to draw */
  if (fadeStep == FADE_STEP) {
    y     = 0;
    yStep = FADE_STEP;
  } else {
    y     = fadeStep;
    yStep = 2*fadeStep;
  }
  for (y = fadeStep; y < fadeMax; y += yStep) {
    XFillRectangle (dpy, win, fadeGC, 0, y, PIXW, 1);
  }
  GUI_Sync ();
  /* prepare next step */
  fadeStep /= 2;
  /* that´s all */
  return XBTrue;
} /* GUI_FadeOut */

/*
 * end of file x11c_pixmap.c
 */
