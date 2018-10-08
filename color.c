/*
 * file color.c - color and image manipulation
 *
 * $Id: color.c,v 1.5 2006/02/09 21:21:23 fzago Exp $
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

/*
 * global function: CchToPpm
 * description:    converts ppm pixel data interpreting them as color/color/highlight
 * parameters:     ppm    - pixel data in ppm format
 *                 width  - image width
 *                 height - image height
 *                 fg     - base color (black most of the time)
 *                 bg     - first color (for red pixels)
 *                 add    - seconed color (for green pixels)
 */
void
CchToPpm (unsigned char *ppm, int width, int height, XBColor fg, XBColor bg, XBColor add)
{
	int x, y;
	unsigned red, green, blue;
	unsigned fgRgb[3];
	unsigned bgRgb[3];
	unsigned addRgb[3];

	/* convert colors */
	fgRgb[0] = GET_RED (fg);
	fgRgb[1] = GET_GREEN (fg);
	fgRgb[2] = GET_BLUE (fg);

	bgRgb[0] = GET_RED (bg);
	bgRgb[1] = GET_GREEN (bg);
	bgRgb[2] = GET_BLUE (bg);

	addRgb[0] = GET_RED (add);
	addRgb[1] = GET_GREEN (add);
	addRgb[2] = GET_BLUE (add);

	/* convert pixels */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/* blue is highlight only */
			ppm[0] -= ppm[2];
			ppm[1] -= ppm[2];
			/* calc 16bit colors */
			red =
				31 * ppm[2] + fgRgb[0] * 255 + bgRgb[0] * ppm[0] - fgRgb[0] * ppm[0] +
				addRgb[0] * ppm[1];
			green =
				31 * ppm[2] + fgRgb[1] * 255 + bgRgb[1] * ppm[0] - fgRgb[1] * ppm[0] +
				addRgb[1] * ppm[1];
			blue =
				31 * ppm[2] + fgRgb[2] * 255 + bgRgb[2] * ppm[0] - fgRgb[2] * ppm[0] +
				addRgb[2] * ppm[1];
			/* cut off */
			if (red > 8191) {
				red = 8191;
			}
			if (green > 8191) {
				green = 65535;
			}
			/* blue */
			if (blue > 8191) {
				blue = 8191;
			}
			/* rgb values are 13bit here */
			ppm[0] = red >> 5;
			ppm[1] = green >> 5;
			ppm[2] = blue >> 5;

			ppm += 3;
		}
	}
}								/* CchToPpm */

/*
 * global function: EpmToPpm
 * description:	    converts ppm pixel data interpreting them as color/color/highlight
 * parameters: 	    ppm    - pixel data in ppm format
 *             	    width  - image width
 *             	    height - image height
 *             	    fg     - base color (black most of the time)
 *             	    bg     - first color (for red pixels)
 *             	    add    - seconed color (for green pixels)
 */
void
EpmToPpm (unsigned char *epm, unsigned char *ppm, int width, int height, int ncolors,
		  const XBColor * color)
{
	int i, x, y;
	unsigned red, green, blue;
	unsigned *cRed, *cGreen, *cBlue;
	unsigned char **cEpm;

	assert (epm != NULL);
	assert (ppm != NULL);
	/* alloc temp color arrays */
	cRed = malloc (ncolors * sizeof (unsigned));
	assert (cRed != NULL);
	cGreen = malloc (ncolors * sizeof (unsigned));
	assert (cGreen != NULL);
	cBlue = malloc (ncolors * sizeof (unsigned));
	assert (cBlue != NULL);
	cEpm = malloc (ncolors * sizeof (unsigned char *));
	assert (cEpm != NULL);
	/* convert colors */
	for (i = 0; i < ncolors; i++) {
		cRed[i] = GET_RED (color[i]);
		cGreen[i] = GET_GREEN (color[i]);
		cBlue[i] = GET_BLUE (color[i]);
		cEpm[i] = epm + i * width * height;
	}
	/* convert pixels */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			red = blue = green = 0;
			for (i = 0; i < ncolors; i++) {
				red += cEpm[i][0] * cRed[i];
				green += cEpm[i][0] * cGreen[i];
				blue += cEpm[i][0] * cBlue[i];
				cEpm[i]++;
			}
			/* cut off */
			if (red > 8191) {
				red = 8191;
			}
			if (green > 8191) {
				green = 8191;
			}
			/* blue */
			if (blue > 8191) {
				blue = 8191;
			}
			/* rgb values are 13bit here */
			ppm[0] = red >> 5;
			ppm[1] = green >> 5;
			ppm[2] = blue >> 5;
			ppm += 3;
		}
	}
	free (cEpm);
	free (cBlue);
	free (cGreen);
	free (cRed);
}								/* EpmToPpm */

/*
 * convert color to string
 */
const char *
ColorToString (XBColor color)
{
	static char string[32];

	assert (color != COLOR_INVALID);
	sprintf (string, "#%02x%02x%02x", GET_RED (color) << 3, GET_GREEN (color) << 3,
			 GET_BLUE (color) << 3);
	return string;
}								/* ColorToString */

/*
 * parse color from string
 */
XBColor
StringToColor (const char *string)
{
	char hash;
	unsigned red, green, blue;

	assert (string != NULL);
	if (4 != sscanf (string, "%c%2x%2x%2x", &hash, &red, &green, &blue)) {
		return COLOR_INVALID;
	}
	if ('#' != hash) {
		return COLOR_INVALID;
	}
	return SET_COLOR (red >> 3, green >> 3, blue >> 3);
}								/* StringToColor */

/*
 * create lighter version of color
 */
XBColor
LighterColor (XBColor color)
{
	unsigned r, g, b;

	/* read colors */
	r = GET_RED (color);
	g = GET_GREEN (color);
	b = GET_BLUE (color);
	/* convert */
	r += r / 4 + XBCOLOR_DEPTH / 4;
	g += g / 4 + XBCOLOR_DEPTH / 4;
	b += b / 4 + XBCOLOR_DEPTH / 4;
	/* that'S all */
	return SET_COLOR (r, g, b);
}								/* LighterColor */

/*
 * create lighter version of color
 */
XBColor
DarkerColor (XBColor color)
{
	unsigned r, g, b;

	/* read colors */
	r = GET_RED (color);
	g = GET_GREEN (color);
	b = GET_BLUE (color);
	/* convert */
	r -= r / 2;
	g -= g / 2;
	b -= b / 2;
	/* that'S all */
	return SET_COLOR (r, g, b);
}								/* LighterColor */

/*
 * create a random color
 */
XBColor
RandomColor (void)
{
	return SET_COLOR (OtherRandomNumber (3) * XBCOLOR_DEPTH / 2,
					  OtherRandomNumber (3) * XBCOLOR_DEPTH / 2,
					  OtherRandomNumber (3) * XBCOLOR_DEPTH / 2);
}								/* RandomColor */

/*
 * end of file color.c
 */
