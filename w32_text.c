/*
 * file w32_text.c - drawing strings and boxes
 *
 * $Id: w32_text.c,v 1.5 2006/02/19 13:33:01 lodott Exp $
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
#include "w32_text.h"
#include "gui.h"

#include "w32_color.h"
#include "w32_image.h"

#include "geom.h"

/*
 * local macros
 */
#define COLOR_LIGHT (SET_COLOR(31,29,0))
#define COLOR_DARK  (SET_COLOR( 0, 0,0))

/*
 * local types
 */
/* pen types */
enum
{
	PEN_LightThick,
	PEN_LightThin,
	PEN_DarkThick,
	PEN_DarkThin,
  /*---*/
	NUM_PEN
};
/* brush types */
enum
{
	BRUSH_Light,
	BRUSH_Dark,
  /*---*/
	NUM_BRUSH
};
/* pen init data */
typedef struct
{
	int style;
	int width;
	XBColor color;
} XBInitPen;

/*
 * local variables
 */
static HDC hdcLight;
static HDC hdcDark;
static HPEN hPen[NUM_PEN];
static HBRUSH hBrush[NUM_BRUSH];
static HFONT hfont[NUM_FONTS];

#ifdef MINI_XBLAST
static int fontSize[NUM_FONTS] = {
	14, 12, 10
};
#else
static int fontSize[NUM_FONTS] = {
	32, 24, 18
};
#endif
static int fontHeight[NUM_FONTS] = {
	0, 0, 0
};
static XBInitPen initPen[NUM_PEN] = {
	{PS_SOLID, 3 * BASE_X / 8, COLOR_LIGHT,},
	{PS_SOLID, 1, COLOR_LIGHT,},
	{PS_SOLID, 3 * BASE_X / 8, COLOR_DARK,},
	{PS_SOLID, 1, COLOR_DARK,},
};
static XBColor initBrush[NUM_BRUSH] = {
	COLOR_LIGHT,
	COLOR_DARK,
};

/*
 * create dc for drwaing textbox
 */
static HDC
CreateTextDC (HDC hdc, HPEN hPen, HBRUSH hBrush, XBColor textColor)
{
	HDC hdcText;

	hdcText = CreateCompatibleDC (hdc);
	if (NULL == hdcText) {
		return NULL;
	}
	SelectObject (hdcText, hPen);
	SelectObject (hdcText, hBrush);
	SetTextColor (hdcText, COLOR_TO_COLORREF (textColor));
	SetBkMode (hdcText, TRANSPARENT);
	SetTextAlign (hdcText, TA_CENTER | TA_TOP);
	SetPolyFillMode (hdcText, WINDING);
	/* that's all */
	return hdcText;
}								/* CreateTextDC */

/*
 * library function: InitText
 * description:      initizialize windows resources for drawingh text
 * parameters:       none
 * return value:     XBTrue on success, XBFalse on error
 */
XBBool
InitText (void)
{
	int i;
	HDC hdc;
	HGDIOBJ old;
	SIZE size;

	/* initialize fields */
	hdcLight = hdcDark = NULL;
	memset (hPen, 0, sizeof (hPen));
	memset (hBrush, 0, sizeof (hBrush));
	memset (hfont, 0, sizeof (hfont));
	/* create pens */
	for (i = 0; i < NUM_PEN; i++) {
		hPen[i] =
			CreatePen (initPen[i].style, initPen[i].width, COLOR_TO_COLORREF (initPen[i].color));
	}
	/* create brushes */
	for (i = 0; i < NUM_BRUSH; i++) {
		hBrush[i] = CreateSolidBrush (COLOR_TO_COLORREF (initBrush[i]));
	}
	/* create logical fonts */
	hdc = GetDC (window);
	for (i = 0; i < NUM_FONTS; i++) {
		hfont[i] = CreateFont (fontSize[i], 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
							   OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
							   VARIABLE_PITCH | FF_SWISS, "MS SansSerif");
		if (NULL == hfont[i]) {
			return XBFalse;
		}
		/* get default height */
		old = SelectObject (hdc, hfont[i]);
		GetTextExtentPoint32 (hdc, "Xj", 2, &size);
		SelectObject (hdc, old);
		fontHeight[i] = size.cy;
	}
	/* create and config DC for light text */
	hdcLight = CreateTextDC (hdc, hPen[PEN_LightThick], hBrush[BRUSH_Dark], COLOR_LIGHT);
	/* create and config DC for dark text */
	hdcDark = CreateTextDC (hdc, hPen[PEN_DarkThick], hBrush[BRUSH_Light], COLOR_DARK);
	/* thats's all */
	ReleaseDC (window, hdc);
	return XBTrue;
}								/* InitText */

/*
 *
 */
void
FinishText (void)
{
	int i;

	/* pens */
	for (i = 0; i < NUM_PEN; i++) {
		if (NULL != hPen[i]) {
			DeleteObject (hPen[i]);
		}
	}
	/* brushes */
	for (i = 0; i < NUM_BRUSH; i++) {
		if (NULL != hBrush[i]) {
			DeleteObject (hBrush[i]);
		}
	}
	/* fonts */
	for (i = 0; i < NUM_FONTS; i++) {
		if (NULL != hfont[i]) {
			DeleteObject (hfont[i]);
		}
	}
	/* device contextes */
	if (NULL != hdcLight) {
		DeleteDC (hdcLight);
	}
	if (NULL != hdcDark) {
		DeleteDC (hdcDark);
	}
}								/* FinishText */

/*
 * global function: GUI_DrawSimpleTextbox
 * description:     draw a text string with optional box
 * parameters:      text  - 0 terminated string to draw
 *                  flags - draw mode flags see xblast.h FF_*
                    rect  - where to draw
 * return value:    none
 */
void
GUI_DrawSimpleTextbox (const char *text, unsigned flags, const BMRectangle * rect)
{
	HGDIOBJ old;
	int len;
	int ypos;
	int xpos;
	HDC hdc;

	/* sanity checks */
	assert (rect != NULL);
	/* get device context */
	hdc = (flags & FM_Color) ? hdcLight : hdcDark;
	old = SelectObject (hdc, pix);
	/* draw boxes */
	if (flags & FM_Boxed) {
		Rectangle (hdc, rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
	}
	/* Draw text */
	if (text != NULL) {
		len = strlen (text);
		ypos = rect->y + (rect->h - fontHeight[flags & FM_Size]) / 2;
		xpos = rect->x + rect->w / 2;
		/* x position */
		SetTextAlign (hdcLight, TA_CENTER | TA_TOP);
		SetTextAlign (hdcDark, TA_CENTER | TA_TOP);
		SelectObject (hdc, hfont[flags & FM_Size]);
		ExtTextOut (hdc, xpos, ypos, ETO_OPAQUE, NULL, text, len, NULL);
	}
	/* clean up */
	SelectObject (hdc, old);
}								/* GUI_DrawSimpleTextbox */

/*
 *
 */
static int
DrawAlignedText (const char *text, int len, unsigned flags, int x, int y, int w, HDC hdc_fg,
				 HDC hdc_bg, HGDIOBJ * pOld)
{
	SIZE textSize;
	int xpos;
	int dx, dy;
	int step;
	int right;

	assert (text != NULL);
	assert (pOld != NULL);
	/* Set Font */
	SelectObject (hdc_fg, hfont[flags & FM_Size]);
	/* determine length */
	len++;
	do {
		len--;
		GetTextExtentPoint32 (hdc_fg, text, len, &textSize);
	} while (textSize.cx > w - 2 * BASE_X);
	/* set text alignment */
	switch (flags & FM_Align) {
	case FF_Right:
		SetTextAlign (hdc_fg, TA_TOP | TA_RIGHT);
		SetTextAlign (hdc_bg, TA_TOP | TA_RIGHT);
		xpos = x + w - BASE_X;
		right = xpos;
		break;
	case FF_Left:
		SetTextAlign (hdc_fg, TA_TOP | TA_LEFT);
		SetTextAlign (hdc_bg, TA_TOP | TA_LEFT);
		xpos = x + BASE_X;
		right = xpos + textSize.cx;
		break;
	default:
		SetTextAlign (hdc_fg, TA_CENTER | TA_TOP);
		SetTextAlign (hdc_bg, TA_CENTER | TA_TOP);
		xpos = x + w / 2;
		right = xpos + textSize.cx / 2;
		break;
	}
	/* out lined output */
	if (flags & FM_Outlined) {
		/* determine size of outlining */
		if ((flags & FM_Size) == FF_Small) {
			step = 1;
		}
		else {
			step = BASE_X / 4;
		}
		SelectObject (hdc_fg, *pOld);
		*pOld = SelectObject (hdc_bg, pix);
		SelectObject (hdc_bg, hfont[flags & FM_Size]);
		for (dx = -step; dx <= step; dx += step) {
			for (dy = -step; dy <= step; dy += step) {
				if (dx || dy) {
					ExtTextOut (hdc_bg, xpos + dx, y + dy, ETO_OPAQUE, NULL, text, len, NULL);
				}
			}
		}
		SelectObject (hdc_bg, *pOld);
		*pOld = SelectObject (hdc_fg, pix);
	}
	/* text output */
	ExtTextOut (hdc_fg, xpos, y, ETO_OPAQUE, NULL, text, len, NULL);
	/* that's all */
	return right;
}								/* DrawAlignedText */

/*
 * global function: GUI_DrawTextbox
 * description:     draw a text string with optional box and outlining
 * parameters:      text  - 0 terminated string to draw
 *                  flags - draw mode flags see xblast.h FF_*
                    rect  - where to draw
 * return value:    none
 */
void
GUI_DrawTextbox (const char *text, unsigned flags, const BMRectangle * rect)
{
	HGDIOBJ old;
	int len;
	int ypos;
	HDC hdc;

	/* sanity checks */
	assert (rect != NULL);
	/* get device context */
	hdc = (flags & FM_Color) ? hdcLight : hdcDark;
	old = SelectObject (hdc, pix);
	/* draw boxes */
	if (flags & FM_Boxed) {
		if (flags & FM_Transparent) {
			HGDIOBJ oldPen;
			if (flags & FM_Color) {
				oldPen = SelectObject (hdc, hPen[PEN_DarkThin]);
			}
			else {
				oldPen = SelectObject (hdc, hPen[PEN_LightThin]);
			}
			for (ypos = 0; ypos < rect->h; ypos += 2) {
				MoveToEx (hdc, rect->x, rect->y + ypos, NULL);
				LineTo (hdc, rect->x + rect->w - 1, rect->y + ypos);
			}
			SelectObject (hdc, oldPen);
			MoveToEx (hdc, rect->x, rect->y, NULL);
			LineTo (hdc, rect->x + rect->w - 1, rect->y);
			LineTo (hdc, rect->x + rect->w - 1, rect->y + rect->h - 1);
			LineTo (hdc, rect->x, rect->y + rect->h - 1);
			LineTo (hdc, rect->x, rect->y);
		}
		else {
			Rectangle (hdc, rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
		}
	}
	/* Draw text */
	if (text != NULL) {
		HDC hdcInv;
		int right;

		len = strlen (text);
		ypos = rect->y + (rect->h - fontHeight[flags & FM_Size]) / 2;
		hdcInv = (flags & FM_Color) ? hdcDark : hdcLight;
		if (flags & FF_Vertical) {
			/* vertcial text layout */
			int i;
			for (i = 0; i < len; i++) {
				int h2 = fontHeight[flags & FM_Size];
				int y2 = rect->y + ( /*height + */ rect->h) / 2 + (2 * i - len + 1) * (h2) / 2;
				right =
					DrawAlignedText (text + i, 1, flags, rect->x, y2, rect->w, hdc, hdcInv, &old);
			}
		}
		else {
			right = DrawAlignedText (text, len, flags, rect->x, ypos, rect->w, hdc, hdcInv, &old);
			/* draw cursor if needed */
			if (flags & FF_Cursor) {
				MoveToEx (hdc, right + 2, rect->y + 4, NULL);
				LineTo (hdc, right + 2, rect->y + rect->h - 5);
			}
		}
	}
	/* clean up */
	SelectObject (hdc, old);
}								/* GUI_DrawTextbox */

/*
 * global function: GUI_DrawPolygon
 * description:     draw an outlined polygon 
 * parameters:      x           - left side of draw region
 *                  y           - top side of draw region
 *                  w           - width of draw region
 *                  h           - height of draw region
 *                  lw          - width of outline 
 *                  points      - relative coordinates (0,0 to 1,1)
 *                  npoints     - number of points
 *                  black_white - dark or light fill pattern
 * return value:    none
 */
void
GUI_DrawPolygon (int x, int y, int w, int h, int lw,
				 const BMPoint * points, int npoints, XBBool blackWhite)
{
	int i;
	POINT *wp;
	HDC hdc;
	HGDIOBJ old;

	assert (points != NULL);
	/* copy points to windows structure */
	wp = calloc (sizeof (POINT), npoints + 1);
	assert (wp != NULL);
	for (i = 0; i < npoints; i++) {
		wp[i].x = (LONG) (x + w * points[i].x);
		wp[i].y = (LONG) (y + h * points[i].y);
	}
	wp[npoints] = wp[0];
	/* get device context */
	hdc = blackWhite ? hdcLight : hdcDark;
	old = SelectObject (hdc, pix);
	/* draw polygon */
	Polygon (hdc, wp, npoints);
	/* clean up */
	free (wp);
	SelectObject (hdc, old);
}								/* GUI_DrawPolygon */

/*
 * end of file w32_text.c
 */
