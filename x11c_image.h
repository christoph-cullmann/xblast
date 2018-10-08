/*
 * file x11c_image.h -
 *
 * $Id: x11c_image.h,v 1.5 2006/02/09 21:21:25 fzago Exp $
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
#ifndef _X11_COLOR_IMAGE_H
#define _X11_COLOR_IMAGE_H

/*
 * global prototypes
 */
extern XBBool InitImage (int visual_class);
extern Pixmap ReadPbmBitmap (const char *path, const char *filename);
extern Pixmap ReadRgbPixmap (const char *path, const char *filename);
extern Pixmap ReadCchPixmap (const char *path, const char *filename, XBColor fg, XBColor bg,
							 XBColor add);
extern Pixmap ReadEpmPixmap (const char *path, const char *filename, int n_colors,
							 const XBColor * color);

#endif
/*
 * end of file x11c_image.h
 */
