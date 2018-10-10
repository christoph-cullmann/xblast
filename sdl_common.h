/*
 * file SDL_common.h -  global variables for Win32 engine
 *
 * $Id: sdl_common.h,v 1.2 2004/09/10 19:03:08 tenderflake Exp $
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

#ifndef XBLAST_SDL_COMMON_H
#define XBLAST_SDL_COMMON_H

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define RMASK 0x0000FF
#define GMASK 0x00FF00
#define BMASK 0xFF0000
#else
#define RMASK 0xFF0000
#define GMASK 0x00FF00
#define BMASK 0x0000FF
#endif


/*
 * global constants
 */
#define NUM_FONTS 3

/*
 * global variables
 */
extern SDL_Surface *screen;


/* extern HANDLE      instance; */
/* extern HWND        window; */
/* extern HBITMAP     pix; */
/* extern HPALETTE    palette; */

extern const char *xblastClass;



#endif
/*
 * end of file SDL_common.h
 */
