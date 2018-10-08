/*
 * file x11_config.h - x11 specific configuration
 *
 * $Id: x11_config.h,v 1.4 2006/02/09 18:31:47 fzago Exp $
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
#ifndef _X11_CONFIG_H
#define _X11_CONFIG_H

#include "color.h"

/*
 * type defintions
 */

/* font configuration */
typedef struct
{
	const char *small;
	const char *medium;
	const char *large;
} CFGFont;
/* colors */
typedef struct
{
	XBColor titleFg;
	XBColor titleBg;
	XBColor darkText1;
	XBColor darkText2;
	XBColor lightText1;
	XBColor lightText2;
	XBColor statusFg;
	XBColor statusBg;
	XBColor statusLed;
} CFGColor;

/*
 * prototypes
 */
extern const CFGFont *GetFontConfig (void);
extern const CFGColor *GetColorConfig (void);

#endif
/*
 * end of file x11_config.h
 */
