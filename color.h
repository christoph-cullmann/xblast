/*
 * file color.h - color and image manipulation
 *
 * $Id: color.h,v 1.4 2005/01/11 17:36:29 iskywalker Exp $
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
#ifndef _COLOR_H
#define _COLOR_H

#ifndef HAVECONFIG_H
#include "config.h"
#define HAVECONFIG_H
#endif

/*
 * type definitions
 */
typedef unsigned short XBColor;

/*
 * macros for color manipulation
 */
#define XBCOLOR_DEPTH 31

#define SET_COLOR(r,g,b) ((XBColor) (((r) & 0x001F) << 10) | (XBColor) (((g) & 0x001F) <<  5) | (XBColor)  ((b) & 0x001F) )
			
#define GET_RED(c)   (((c) >> 10) & 0x001F)  
#define GET_GREEN(c) (((c) >>  5) & 0x001F)  
#define GET_BLUE(c)  ( (c)        & 0x001F)  

/*
 * predefined colors
 */
#define COLOR_INVALID          0xFFFF
#define COLOR_BLACK            0x0000
#define COLOR_WHITE            0x7FFF

#define COLOR_RED              	SET_COLOR (31,  0,  0)
#define COLOR_GREEN             SET_COLOR ( 0, 31,  0)
#define COLOR_BLUE              SET_COLOR ( 0,  0, 31)

#define COLOR_DARK_BLUE         SET_COLOR ( 0,  0, 17)
#define COLOR_DARK_SEA_GREEN    SET_COLOR (17, 23, 17)
#define COLOR_DARK_SLATE_GRAY_4 SET_COLOR (10, 17, 17)
#define COLOR_DEEP_PINK        	SET_COLOR (31,  2, 18)
#define COLOR_FIRE_BRICK_1     	SET_COLOR (31,  6,  6)
#define COLOR_FOREST_GREEN     	SET_COLOR ( 4, 17,  4)
#define COLOR_GOLD             	SET_COLOR (31, 26,  0)
#define COLOR_GRAY_25          	SET_COLOR ( 8,  8,  8) 
#define COLOR_GRAY_75          	SET_COLOR (24, 24, 24)
#define COLOR_GREEN_YELLOW     	SET_COLOR (21, 31,  5)
#define COLOR_INDIAN_RED       	SET_COLOR (25, 11, 11)
#define COLOR_LIGHT_GOLDENROD  	SET_COLOR (29, 27, 16)
#define COLOR_LIGHT_SALMON     	SET_COLOR (31, 20, 15)
#define COLOR_LIGHT_STEEL_BLUE 	SET_COLOR (22, 24, 27)
#define COLOR_LIGHT_YELLOW     	SET_COLOR (31, 31, 28)
#define COLOR_MIDNIGHT_BLUE    	SET_COLOR ( 3,  3, 14)
#define COLOR_NAVY_BLUE        	SET_COLOR ( 0,  0, 16)
#define COLOR_ORANGE           	SET_COLOR (31, 20,  0)
#define COLOR_ORANGE_RED       	SET_COLOR (31,  8,  0)
#define COLOR_ORCHID           	SET_COLOR (27, 14, 26)
#define COLOR_ROYAL_BLUE       	SET_COLOR ( 8, 13, 28)
#define COLOR_SADDLE_BROWN     	SET_COLOR (17,  8,  2)
#define COLOR_SPRING_GREEN     	SET_COLOR ( 0, 31, 15)
#define COLOR_TAN              	SET_COLOR (26, 22, 17)
#define COLOR_TURQUOISE        	SET_COLOR ( 8, 28, 21)
#define COLOR_YELLOW           	SET_COLOR (31, 31,  0)

/*
 * global prototypes
 */
extern void CchToPpm (unsigned char *ppm, int width, int height, 
                      XBColor fg, XBColor bg, XBColor add);
extern void EpmToPpm (unsigned char *epm, unsigned char *ppm, int width, int height,
		      int ncolors, const XBColor *color);
extern const char *ColorToString (XBColor color);
extern XBColor StringToColor (const char *string);
extern XBColor LighterColor (XBColor color);
extern XBColor DarkerColor (XBColor color);
extern XBColor RandomColor (void);

#endif
/*
 * end of color.h
 */
