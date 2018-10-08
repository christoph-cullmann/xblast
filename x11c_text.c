/*
 * file x11c_text.c - draw text strings 
 *
 * $Id: x11c_text.c,v 1.8 2006/05/04 17:26:16 fzago Exp $
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
#include "x11_common.h"

/*
 * local variables
 */
static GC gcTextBlack;
static GC gcTextWhite;
static XFontStruct *fontStruct[NUM_FONTS];

/*
 * load fonts struct by font name
 */
static XFontStruct *
LoadFont (const char *fontName)
{
	XFontStruct *fontStruct;
	if (NULL == (fontStruct = XLoadQueryFont (dpy, fontName))) {
		fprintf (stderr, "could not load font %s\n", fontName);

		/* otherwise get default font struct */
		fontStruct = XQueryFont (dpy, XGContextFromGC (gcTextBlack));
	}
	return fontStruct;
}								/* LoadFont */

/*
 *
 */
XBBool
InitFonts (void)
{
	XGCValues xgcv;
	const CFGFont *cfgFont;
	const CFGColor *cfgColor;

	/* get config */
	cfgFont = GetFontConfig ();
	assert (cfgFont != NULL);
	cfgColor = GetColorConfig ();
	assert (cfgColor != NULL);

	/* gc black text */
	xgcv.fill_style = FillTiled;
	xgcv.tile =
		ReadCchPixmap (imgPathMisc, imgFileTextBg, COLOR_BLACK, cfgColor->darkText1,
					   cfgColor->darkText2);
	xgcv.line_width = BASE_X / 4;
	gcTextBlack = XCreateGC (dpy, pix, GCTile | GCFillStyle | GCLineWidth, &xgcv);

	/* gc white text */
	xgcv.fill_style = FillTiled;
	xgcv.tile =
		ReadCchPixmap (imgPathMisc, imgFileTextFg, COLOR_BLACK, cfgColor->lightText1,
					   cfgColor->lightText2);
	xgcv.line_width = BASE_X / 4;
	gcTextWhite = XCreateGC (dpy, pix, GCTile | GCFillStyle | GCLineWidth, &xgcv);

	/* try to load fonts */
	fontStruct[FF_Large] = LoadFont (cfgFont->large);
	fontStruct[FF_Medium] = LoadFont (cfgFont->medium);
	fontStruct[FF_Small] = LoadFont (cfgFont->small);

	/* check load */
	assert (NULL != fontStruct[FF_Large]);
	assert (NULL != fontStruct[FF_Medium]);
	assert (NULL != fontStruct[FF_Small]);

	/* that's all */
	return XBTrue;
}								/* InitFonts */

/*
 * draw simple textbox (in game)
 */
void
GUI_DrawSimpleTextbox (const char *text, unsigned flags, const BMRectangle * rect)
{
	XFontStruct *font;
	int x, y;
	int width, height;
	GC gcFg, gcBg;
	XRectangle clip;

	/* first get used font */
	font = fontStruct[FM_Size & flags];

	/* set gc for foreground and background */
	if (flags & FM_Color) {
		gcFg = gcTextWhite;
		gcBg = gcTextBlack;
	}
	else {
		gcFg = gcTextBlack;
		gcBg = gcTextWhite;
	}

	/* draw boxes if needed */
	XSetTSOrigin (dpy, gcFg, 0, rect->y + (rect->h - BLOCK_HEIGHT) / 2);
	if (flags & FM_Boxed) {
		XSetTSOrigin (dpy, gcBg, 0, rect->y + (rect->h - BLOCK_HEIGHT) / 2);
		XFillRectangle (dpy, pix, gcBg, rect->x, rect->y, rect->w, rect->h);
		XDrawRectangle (dpy, pix, gcFg, rect->x, rect->y, rect->w, rect->h);
	}

	/* draw string */
	if (NULL != text) {

		/* set clipping rectangles */
		clip.x = rect->x;
		clip.y = rect->y;
		clip.width = rect->w;
		clip.height = rect->h;
		XSetClipRectangles (dpy, gcFg, 0, 0, &clip, 1, YXBanded);

		/* dimensions of text */
		width = XTextWidth (font, text, strlen (text));
		height = font->max_bounds.ascent - font->max_bounds.descent;

		/* x- location */
		x = rect->x + (rect->w - width) / 2;

		/* y-location */
		y = rect->y + (height + rect->h) / 2;

		/* draw it */
		XSetFont (dpy, gcFg, font->fid);
		XDrawString (dpy, pix, gcFg, x, y, text, strlen (text));

		/* reset clip mask */
		XSetClipMask (dpy, gcFg, None);
	}
}								/* GUI_DrawSimpleTextbox */

/*
 *
 */
static int
DrawAlignedText (const char *text, int len, unsigned flags, int x, int y, int w, GC gcFg, GC gcBg,
				 XFontStruct * font)
{
	int width;
	int dx, dy;
	int step;
	assert (text != NULL);
	assert (font != NULL);

	/* standard horizontal text */
	len++;

	do {
		len--;
		width = XTextWidth (font, text, len);
	} while (width > w - 2 * BASE_X);

	/* x- location */
	switch (flags & FM_Align) {
	case FF_Left:
		x += BASE_X;
		break;
	case FF_Right:
		x += w - width - BASE_X;
		break;
	default:
		x += (w - width) / 2;
		break;
	}

	/* y-location */
	if (flags & FM_Outlined) {
		if (FF_Small == (flags & FM_Size)) {
			step = 1;
		}
		else {
			step = BASE_X / 4;
		}
		for (dx = -step; dx <= step; dx += step) {
			for (dy = -step; dy <= step; dy += step) {
				if (dx || dy) {
					XDrawString (dpy, pix, gcBg, x + dx, y + dy, text, len);
				}
			}
		}
	}
	XDrawString (dpy, pix, gcFg, x, y, text, len);

	/* return values */
	return x + width - 1;
}								/* DrawAlignedText */

/*
 *
 */
void
GUI_DrawTextbox (const char *text, unsigned flags, const BMRectangle * rect)
{
	XFontStruct *font;
	int y, height;
	int right;
	int len;
	GC gcFg, gcBg;
	XRectangle clip;

	/* first get used font */
	font = fontStruct[FM_Size & flags];

	/* set gc for foreground and background */
	if (flags & FM_Color) {
		gcFg = gcTextWhite;
		gcBg = gcTextBlack;
	}
	else {
		gcFg = gcTextBlack;
		gcBg = gcTextWhite;
	}

	/* draw boxes if needed */
	XSetTSOrigin (dpy, gcFg, 0, rect->y + (rect->h - BLOCK_HEIGHT) / 2);
	if (flags & FM_Boxed) {
		if (!(flags & FM_Transparent)) {
			XSetTSOrigin (dpy, gcBg, 0, rect->y + (rect->h - BLOCK_HEIGHT) / 2);
			XFillRectangle (dpy, pix, gcBg, rect->x, rect->y, rect->w, rect->h);
		}
		else {
			XGCValues xgcv;
			xgcv.line_width = 1;
			XChangeGC (dpy, gcBg, GCLineWidth, &xgcv);
			for (y = 0; y < rect->h; y += 2) {
				XDrawLine (dpy, pix, gcBg, rect->x, rect->y + y, rect->x + rect->w - 1,
						   rect->y + y);
			}
			xgcv.line_width = 3;
			XChangeGC (dpy, gcBg, GCLineWidth, &xgcv);
		}
		XDrawRectangle (dpy, pix, gcFg, rect->x, rect->y, rect->w, rect->h);
	}

	/* draw string */
	if (NULL != text) {
		len = strlen (text);

		/* set clipping rectangles */
		clip.x = rect->x;
		clip.y = rect->y;
		clip.width = rect->w;
		clip.height = rect->h;
		XSetClipRectangles (dpy, gcFg, 0, 0, &clip, 1, YXBanded);
		XSetClipRectangles (dpy, gcBg, 0, 0, &clip, 1, YXBanded);

		/* set font */
		XSetFont (dpy, gcFg, font->fid);
		XSetFont (dpy, gcBg, font->fid);

		/* dimensions of text */
		height = font->max_bounds.ascent - font->max_bounds.descent;
		y = rect->y + (height + rect->h) / 2;
		if (flags & FF_Vertical) {

			/* vertical text layout */
			int i;
			for (i = 0; i < len; i++) {
				int h2 = font->max_bounds.ascent + font->max_bounds.descent;
				int y2 = rect->y + (height + rect->h) / 2 + (2 * i - len + 1) * (h2) / 2;
				(void)DrawAlignedText (text + i, 1, flags, rect->x, y2, rect->w, gcFg, gcBg, font);
		}}
		else {
			right = DrawAlignedText (text, len, flags, rect->x, y, rect->w, gcFg, gcBg, font);

			/* cursor for string editors */
			if (flags & FF_Cursor) {
				XDrawLine (dpy, pix, gcFg, right + 2, rect->y + 2, right + 2,
						   rect->y + rect->h - 3);
			}
		}

		/* reset clip mask */
		XSetClipMask (dpy, gcFg, None);
		XSetClipMask (dpy, gcBg, None);
	}
}								/* GUI_DrawTextbox */

/*
 * draw a filled and outlinded polygon
 */
void
GUI_DrawPolygon (int x, int y, int w, int h, int lw, const BMPoint * points, int npoints,
				 XBBool black_white)
{
	XPoint *xp;
	int i;
	XGCValues xgcv;

	/* convert and scale to xpoints */
	xp = (XPoint *) calloc (sizeof (XPoint), npoints + 1);
	for (i = 0; i < npoints; i++) {
		xp[i].x = (int)(x + w * points[i].x);
		xp[i].y = (int)(y + h * points[i].y);
	} xp[npoints] = xp[0];

	/* set line width */
	xgcv.line_width = lw;

	/* now draw it */
	if (black_white) {
		XChangeGC (dpy, gcTextWhite, GCLineWidth, &xgcv);
		XFillPolygon (dpy, pix, gcTextBlack, xp, npoints, Complex, CoordModeOrigin);
		XDrawLines (dpy, pix, gcTextWhite, xp, npoints + 1, CoordModeOrigin);
		xgcv.line_width = 2;
		XChangeGC (dpy, gcTextWhite, GCLineWidth, &xgcv);
	}
	else {
		XChangeGC (dpy, gcTextBlack, GCLineWidth, &xgcv);
		XFillPolygon (dpy, pix, gcTextWhite, xp, npoints, Complex, CoordModeOrigin);
		XDrawLines (dpy, pix, gcTextBlack, xp, npoints + 1, CoordModeOrigin);
		xgcv.line_width = 2;
		XChangeGC (dpy, gcTextWhite, GCLineWidth, &xgcv);
	}

	/* that's all */
	free (xp);
}								/* GUI_DrawPolygon */

/*
 * end of file x11c_text.c
 */
