/*
 * file x11c_image.c - convert images to window system
 *
 * $Id: x11c_image.c,v 1.6 2006/02/09 21:21:25 fzago Exp $
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
#include "x11_common.h"

/* 
 * local variables
 */
static int rgb_color[6][6][6];
static unsigned long rmask, gmask, bmask;
static int rbits, rshift;
static int gbits, gshift;
static int bbits, bshift;
static int rbase, gbase, bbase;

/* pointer to conversion functions (rgb to pixel) */
static void (*ppm_to_image) (unsigned char *ppm, int width, int height, XImage * image);

/*
 *
 */
static void
AnalyzeColorMask (unsigned long mask, int *bits, int *shift)
{
	*shift = 0;
	*bits = 0;

	if (mask) {
		while (0 == (mask % 2)) {
			(*shift)++;
			mask >>= 1;
		}
		while (1 == (mask % 2)) {
			(*bits)++;
			mask >>= 1;
		}
	}
}								/* AnalyzeColorMask */

/*
 *
 */
static void
PpmToImageRgb (unsigned char *ppm, int width, int height, XImage * image)
{
	int x, y, i;
	int pixel;
	int base[3], scale[3], shift[3], mask[3];

	base[0] = rbase;
	base[1] = gbase;
	base[2] = bbase;
	scale[0] = 65535 - base[0];
	scale[1] = 65535 - base[1];
	scale[2] = 65535 - base[2];
	shift[0] = 16 - rshift - rbits;
	shift[1] = 16 - gshift - gbits;
	shift[2] = 16 - bshift - bbits;
	mask[0] = rmask;
	mask[1] = gmask;
	mask[2] = bmask;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/* calculate renormalized rgb values */
			pixel = 0;
			/* loop over rgb */
			for (i = 0; i < 3; i++) {
				/* EPFL: shift[0] or shift[2] can be positive or negative, depending of the colordepth and hardware */
				if (shift[i] > 0) {
					pixel |= (((scale[i] * (*ppm)) / 255 + base[i]) >> shift[i]) & mask[i];
				}
				else {
					pixel |= (((scale[i] * (*ppm)) / 255 + base[i]) << -shift[i]) & mask[i];
				}
				ppm++;
			}
			XPutPixel (image, x, y, pixel);
		}
	}
}								/* PpmToImageRgb */

/*
 *
 */
static void
AllocRgbColor (int red, int green, int blue)
{
	XColor colorUsed;
#ifdef DEBUG
	static int num_rgb_colors = 0;

	fprintf (stderr, "Allocating RGB color %d\n", ++num_rgb_colors);
#endif

	/* default pixel is white */
	rgb_color[red][green][blue] = whitePixel;
	/* try to alloc a new color cell */
	colorUsed.red = (65535 - rbase) * red / 5 + rbase;
	colorUsed.green = (65535 - gbase) * green / 5 + gbase;
	colorUsed.blue = (65535 - bbase) * blue / 5 + bbase;
	colorUsed.flags = DoRed | DoGreen | DoBlue;
	if (XAllocColor (dpy, cmap, &colorUsed)) {
		rgb_color[red][green][blue] = colorUsed.pixel;
	}
	else {
		/* if private colormap is in not in use,try agaian white */
		if (cmap == DefaultColormap (dpy, DefaultScreen (dpy))) {
			/* create private color map */
			cmap = XCopyColormapAndFree (dpy, cmap);
			XSetWindowColormap (dpy, win, cmap);
			/* alloc again */
			colorUsed.red = 65535 * red / 5;
			colorUsed.green = 65535 * green / 5;
			colorUsed.blue = 65535 * blue / 5;
			colorUsed.flags = DoRed | DoGreen | DoBlue;
			if (XAllocColor (dpy, cmap, &colorUsed)) {
				rgb_color[red][green][blue] = colorUsed.pixel;
			}
		}
	}
}								/* AllocRgbColor */

/*
 *
 */
static void
PpmToImage8Bit (unsigned char *ppm, int width, int height, XImage * image)
{
	int x, y;
	unsigned char *ptr;
	int red, green, blue;

	/* upper left pixels */
	ptr = ppm;
	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			/* calculate renormalized rgb values */
			red = (20 * ptr[0] / 255) / 4;
			green = (20 * ptr[1] / 255) / 4;
			blue = (20 * ptr[2] / 255) / 4;
			if (rgb_color[red][green][blue] < 0) {
				AllocRgbColor (red, green, blue);
			}
			XPutPixel (image, x, y, rgb_color[red][green][blue]);
			ptr += 6;
		}
		ptr += 3 * width;
	}
	/* upper right pixels */
	ptr = ppm + 3;
	for (y = 0; y < height; y += 2) {
		for (x = 1; x < width; x += 2) {
			/* calculate renormalized rgb values */
			red = (20 * ptr[0] / 255 + 1) / 4;
			green = (20 * ptr[1] / 255 + 1) / 4;
			blue = (20 * ptr[2] / 255 + 1) / 4;
			if (rgb_color[red][green][blue] < 0) {
				AllocRgbColor (red, green, blue);
			}
			XPutPixel (image, x, y, rgb_color[red][green][blue]);
			ptr += 6;
		}
		ptr += 3 * width;
	}
	/* lower left pixels */
	ptr = ppm + 3 * width;
	for (y = 1; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			/* calculate renormalized rgb values */
			red = (20 * ptr[0] / 255 + 2) / 4;
			green = (20 * ptr[1] / 255 + 2) / 4;
			blue = (20 * ptr[2] / 255 + 2) / 4;
			if (rgb_color[red][green][blue] < 0) {
				AllocRgbColor (red, green, blue);
			}
			XPutPixel (image, x, y, rgb_color[red][green][blue]);
			ptr += 6;
		}
		ptr += 3 * width;
	}
	/* lower right pixels */
	ptr = ppm + 3 * width + 3;
	for (y = 1; y < height; y += 2) {
		for (x = 1; x < width; x += 2) {
			/* calculate renormalized rgb values */
			red = (20 * ptr[0] / 255 + 3) / 4;
			green = (20 * ptr[1] / 255 + 3) / 4;
			blue = (20 * ptr[2] / 255 + 3) / 4;
			if (rgb_color[red][green][blue] < 0) {
				AllocRgbColor (red, green, blue);
			}
			XPutPixel (image, x, y, rgb_color[red][green][blue]);
			ptr += 6;
		}
		ptr += 3 * width;
	}
}								/* PpmToImage8Bit */

/*
 *
 */
XBBool
InitImage (int visual_class)
{
	XVisualInfo vinfo;
	XVisualInfo *result;
	int i, nitems;

	if (defDepth > 8) {
		/* get visual for true color display */
		vinfo.class = visual_class;
		result = XGetVisualInfo (dpy, VisualClassMask, &vinfo, &nitems);
		for (i = 0; i < nitems; i++) {
			if (result[i].depth == defDepth) {
				break;
			}
		}
		if (i == nitems) {
			fprintf (stderr, "cannot find VisalInfo for current bpp. Expect color problems");
			i = 0;
		}
		/* get pixel format */
		rmask = result[i].red_mask;
		gmask = result[i].green_mask;
		bmask = result[i].blue_mask;
		AnalyzeColorMask (rmask, &rbits, &rshift);
		AnalyzeColorMask (gmask, &gbits, &gshift);
		AnalyzeColorMask (bmask, &bbits, &bshift);
		XFree (result);
#ifdef DEBUG
		fprintf (stderr, "Red:   mask=%08lx, bits=%d, shift=%d\n", rmask, rbits, rshift);
		fprintf (stderr, "Green: mask=%08lx, bits=%d, shift=%d\n", gmask, gbits, gshift);
		fprintf (stderr, "Blue:  mask=%08lx, bits=%d, shift=%d\n", bmask, bbits, bshift);
#endif
		ppm_to_image = PpmToImageRgb;
	}
	else {
		/* 8 bit display with color map */
		int r, g, b;

		for (r = 0; r < 6; r++) {
			for (g = 0; g < 6; g++) {
				for (b = 0; b < 6; b++) {
					rgb_color[r][g][b] = -1;
				}
			}
		}

		ppm_to_image = PpmToImage8Bit;
	}
	/* set up offset for r,g,b values */
	rbase = gbase = bbase = 0x1000;
	/* that's all */
	return XBTrue;
}								/* InitImage */

/*
 *
 */
Pixmap
ReadCchPixmap (const char *path, const char *filename, XBColor fg, XBColor bg, XBColor add)
{
	XImage *image;
	Pixmap tmp;
	int width, height;
	unsigned char *ppm;
	char *data;

	/* load ppm file */
	if (NULL == (ppm = ReadPpmFile (path, filename, &width, &height))) {
		fprintf (stderr, "failed to load pixmap %s\n", filename);
		return None;
	}
	/* alloc ppm and image data */
	if (NULL == (data = malloc (((bitsPerPixel + 7) / 8) * width * height))) {
		fprintf (stderr, "failed to alloc image data\n");
		return None;
	}
	/* create image */
	image = XCreateImage (dpy, defVisual, defDepth, ZPixmap, 0, data, width, height, 32, 0);
	if (image == NULL) {
		fprintf (stderr, "create image failed\n");
		return None;
	}
	/* recolor ppm image */
	CchToPpm (ppm, width, height, fg, bg, add);
	/* convert ppm to image */
	(*ppm_to_image) (ppm, width, height, image);
	/* free ppm data */
	free (ppm);
	/* create pixmap */
	tmp = XCreatePixmap (dpy, win, width, height, defDepth);
	/* put image */
	XPutImage (dpy, tmp, gcWindow, image, 0, 0, 0, 0, width, height);
	/* delete image */
#ifdef DEBUG_ALLOC
	Dbg_Vfree (__FILE__, __LINE__, data);
#endif
	XDestroyImage (image);
	/* thats 'all */
	return tmp;
}								/* ReadCchPixmap */

/*
 *
 */
Pixmap
ReadEpmPixmap (const char *path, const char *filename, int n_colors, const XBColor * color)
{
	XImage *image;
	Pixmap tmp;
	int width, height;
	unsigned char *epm, *ppm;
	char *data;
	int depth;

	/* load ppm file */
	if (NULL == (epm = ReadEpmFile (path, filename, &width, &height, &depth))) {
		fprintf (stderr, "failed to load pixmap %s\n", filename);
		return None;
	}
	/* adjust number of colors */
	if (n_colors > depth) {
		n_colors = depth;
	}
	/* alloc ppm and image data */
	data = malloc (((bitsPerPixel + 7) / 8) * width * height);
	assert (data != NULL);
	ppm = malloc (depth * width * height);
	assert (ppm != NULL);

	/* create image */
	image = XCreateImage (dpy, defVisual, defDepth, ZPixmap, 0, data, width, height, 32, 0);
	if (image == NULL) {
		fprintf (stderr, "create image failed\n");
		return None;
	}
	/* recolor ppm image */
	EpmToPpm (epm, ppm, width, height, n_colors, color);
	/* convert ppm to image */
	(*ppm_to_image) (ppm, width, height, image);
	/* free ppm data */
	free (epm);
	free (ppm);
	/* create pixmap */
	tmp = XCreatePixmap (dpy, win, width, height, defDepth);
	/* put image */
	XPutImage (dpy, tmp, gcWindow, image, 0, 0, 0, 0, width, height);
	/* delete image */
#ifdef DEBUG_ALLOC
	Dbg_Vfree (__FILE__, __LINE__, data);
#endif
	XDestroyImage (image);
	/* thats 'all */
	return tmp;
}								/* ReadEpmPixmap */

/*
 *
 */
Pixmap
ReadRgbPixmap (const char *path, const char *filename)
{
	XImage *image;
	Pixmap tmp;
	int width, height;
	char *data;
	unsigned char *ppm;

	/* load ppm file */
	if (NULL == (ppm = ReadPpmFile (path, filename, &width, &height))) {
		fprintf (stderr, "Failed to load pixmap %s\n", filename);
		return None;
	}
	/* alloc ppm and image data */
	if (NULL == (data = malloc (((bitsPerPixel + 7) / 8) * width * height))) {
		fprintf (stderr, "failed to alloc image data\n");
		return None;
	}
	/* create image */
	image = XCreateImage (dpy, defVisual, defDepth, ZPixmap, 0, data, width, height, 32, 0);
	if (image == NULL) {
		fprintf (stderr, "create image failed\n");
		return None;
	}
	/* convert ppm to image */
	(*ppm_to_image) (ppm, width, height, image);
	/* free ppm data */
	free (ppm);
	/* create pixmap */
	tmp = XCreatePixmap (dpy, win, width, height, defDepth);
	/* put image */
	XPutImage (dpy, tmp, gcWindow, image, 0, 0, 0, 0, width, height);
	/* delete image */
#ifdef DEBUG_ALLOC
	Dbg_Vfree (__FILE__, __LINE__, data);
#endif
	XDestroyImage (image);

	return tmp;
}								/* ReadRgbPixmap */

/*
 *
 */
Pixmap
ReadPbmBitmap (const char *path, const char *filename)
{
	int width, height;
	unsigned char *pbm;
	Pixmap tmp;

	/* load ppm file */
	if (NULL == (pbm = ReadPbmFile (path, filename, &width, &height))) {
		fprintf (stderr, "Failed to load bitmap %s\n", filename);
		return None;
	}
	/* create bitmap data */
	tmp = XCreateBitmapFromData (dpy, win, (char *)pbm, width, height);
	/* free ppm data */
	free (pbm);

	return tmp;
}								/* ReadPbmBitmap */

/*
 * convert colorname to value
 */
XBColor
GUI_ParseColor (const char *name)
{
	XColor color;

	if (!XParseColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), name, &color)) {
		return COLOR_INVALID;
	}
	return SET_COLOR (color.red >> 11, color.green >> 11, color.blue >> 11);
}								/* GUI_ParseColor */

/*
 * end of file x11c_image.c
 */
