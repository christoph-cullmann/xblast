/*
 * file w32_image.c - image conversion (rgb to pixel)
 *
 * $Id: w32_image.c,v 1.5 2006/02/19 13:33:01 lodott Exp $
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
#include "w32_image.h"

#include "gui.h"
#include "util.h"

/*
 * local variables
 */
static unsigned char *bitConvTable = NULL;

/*
 * library function: InitImages
 * description:      initializes data structure neede for image conversion
 * parameters:       none
 * return value:     0 on success, -1 on failure 
 */
XBBool
InitImages (void)
{
	unsigned i, j;
	unsigned char tmp;

	/* create translation tables between pbm bits and windows mask */
	bitConvTable = malloc (256);
	if (NULL == bitConvTable) {
		return XBFalse;
	}
	for (i = 0; i < 256; i++) {
		tmp = 0;
		for (j = 0; j < 8; j++) {
			if (i & (1 << j)) {
				tmp |= (1 << (7 - j));
			}
		}
		bitConvTable[i] = ~tmp;
	}
	return XBTrue;
}								/* InitImages */

/*
 *
 */
void
FinishImages (void)
{
	if (NULL != bitConvTable) {
		free (bitConvTable);
	}
}								/* FinishImages */

/*
 * local function: BitmapFromRGBPixel
 * description:    creates bitmap from  pixel data in  24 bit RGB format
 * parameters:     data   - 24 bit pixel data (b,g,r!)
 *                 width  - width of bitmap
 *                 height - height of bitmap
 * return value:   handle of bitmap, or NULL on failure
 */
static HBITMAP
BitmapFromRGBPixel (unsigned char *data, int width, int height)
{
	BITMAPINFO info;
	HDC hdc;
	HBITMAP bitmap;
	HPALETTE oldPal = NULL;
	unsigned char swap;
	int i, len;

	/* turn r,g,b around */
	len = 3 * width * height;
	for (i = 0; i < len; i += 3) {
		swap = data[i];
		data[i] = data[i + 2];
		data[i + 2] = swap;
	}
	/* set bitmap map info */
	info.bmiHeader.biSize = sizeof (info.bmiHeader);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = -height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 0;
	info.bmiHeader.biYPelsPerMeter = 2834;
	info.bmiHeader.biXPelsPerMeter = 2834;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
	/* get dvice context of window */
	hdc = GetDC (window);
	if (NULL == hdc) {
		return NULL;
	}
	/* select palette to use */
	if (NULL != palette) {
		oldPal = SelectPalette (hdc, palette, FALSE);
	}
	/* create bitmap */
	bitmap = CreateDIBitmap (hdc, &info.bmiHeader, CBM_INIT, data, &info, 0);
	ReleaseDC (window, hdc);
	if (NULL != palette) {
		SelectPalette (hdc, oldPal, FALSE);
	}
	/* that's all */
	return bitmap;
}								/* BitmapFromRGBPixel */

/*
 * library function: ReadPbmBitmap
 * description:      create a bitmap from a given pbm-file
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 * return value:     handle of bitmap, or NULL on failure
 */
HBITMAP
ReadPbmBitmap (const char *path, const char *filename)
{
	int width;
	int height;
	unsigned char *pbm;
	HBITMAP bitmap;
	int pbmLineLength;
	int w32LineLength;
	int i;
	int pbmSize;

	/* load pbm file */
	if (NULL == (pbm = ReadPbmFile (path, filename, &width, &height))) {
		GUI_ErrorMessage ("Failed to load bitmap %s\n", filename);
		return NULL;
	}
	pbmLineLength = (width + 7) / 8;
	w32LineLength = 2 * ((width + 15) / 16);
	pbmSize = pbmLineLength * height;
	for (i = 0; i < pbmSize; i++) {
		pbm[i] = bitConvTable[pbm[i]];
	}
	/* pbm data is byte aligned while bitmap data must be word aligned */
	if (pbmLineLength != w32LineLength) {
		int y;
		unsigned char *tmp;
		/* --- */
		tmp = calloc (height * w32LineLength, sizeof (unsigned char));
		assert (tmp != NULL);
		for (y = 0; y < height; y++) {
			memcpy (tmp + y * w32LineLength, pbm + y * pbmLineLength, pbmLineLength);
		}
		/* --- */
		free (pbm);
		pbm = tmp;
	}
	/* create monochrome bitmap */
	bitmap = CreateBitmap (width, height, 1, 1, pbm);
	/* free data */
	free (pbm);
	/* that's all */
	return bitmap;
}								/* ReadPbmBitmap */

/*
 * library function: ReadRgbPixmap
 * description:      create a bitmap from a ppm file (using rgb values)
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 * return value:     handle of bitmap, or NULL on failure
 */
HBITMAP
ReadRgbPixmap (const char *path, const char *filename)
{
	int width;
	int height;
	unsigned char *ppm;
	HBITMAP bitmap;

	/* load ppm file */
	if (NULL == (ppm = ReadPpmFile (path, filename, &width, &height))) {
		GUI_ErrorMessage ("Failed to load pixmap %s\n", filename);
		return NULL;
	}
	/* now create bitmap */
	bitmap = BitmapFromRGBPixel (ppm, width, height);
	/* free pixel data */
	free (ppm);
	/* that's all */
	return bitmap;
}								/* ReadRgbPixmap */

/*
 * library function: ReadCchPixmap
 * description:      create a bitmap from a ppm file (using red as bg, green as add 
 *                   and white as highlight)
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 *                   fg     - base color (black most of the time)
 *                   bg     - first color (for red pixels)
 *                   add    - seconed color (for green pixels)
 * return value:     handle of bitmap, or NULL on failure
 */
HBITMAP
ReadCchPixmap (const char *path, const char *filename, XBColor fg, XBColor bg, XBColor add)
{
	int width;
	int height;
	unsigned char *ppm;
	HBITMAP bitmap;

	/* load ppm file */
	if (NULL == (ppm = ReadPpmFile (path, filename, &width, &height))) {
		GUI_ErrorMessage ("Failed to load pixmap %s\n", filename);
		return NULL;
	}
	/* convert color */
	CchToPpm (ppm, width, height, fg, bg, add);
	/* now create bitmap */
	bitmap = BitmapFromRGBPixel (ppm, width, height);
	/* free pixel data */
	free (ppm);
	/* that's all */
	return bitmap;
}								/* ReadCchPixmap */

/*
 * library function: ReadEpmPixmap
 * description:      create a bitmap from a ppm file (using red as bg, green as add 
 *                   and white as highlight)
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 *                   n_colors - number of color layers
 *                   color    - arrays with colors foreach layer
 * return value:     handle of bitmap, or NULL on failure
 */
HBITMAP
ReadEpmPixmap (const char *path, const char *filename, int n_colors, const XBColor * color)
{
	int width;
	int height;
	int depth;
	unsigned char *epm;
	unsigned char *ppm;
	HBITMAP bitmap;

	assert (NULL != color);
	assert (NULL != path);
	assert (NULL != filename);
	/* load ppm file */
	if (NULL == (epm = ReadEpmFile (path, filename, &width, &height, &depth))) {
		GUI_ErrorMessage ("Failed to load pixmap %s\n", filename);
		return NULL;
	}
	/* check depth */
	if (depth < n_colors) {
		n_colors = depth;
	}
	/* create ppm array */
	ppm = malloc (width * height * 3);
	assert (ppm != NULL);
	/* convert color */
	EpmToPpm (epm, ppm, width, height, n_colors, color);
	/* now create bitmap */
	bitmap = BitmapFromRGBPixel (ppm, width, height);
	/* free pixel data */
	free (epm);
	free (ppm);
	/* that's all */
	return bitmap;
}								/* ReadEpmPixmap */

/*
 * convert colorname to value (not supported for win32)
 */
XBColor
GUI_ParseColor (const char *name)
{
	return COLOR_INVALID;
}								/* GUI_ParseColor */

/*
 * end of file w32_image.c
 */
