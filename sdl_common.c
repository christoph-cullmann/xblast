/*
 * file SDL_common.c - global variables for Win32 engine
 *
 * $Id: sdl_common.c,v 1.3 2004/09/13 22:33:45 tenderflake Exp $
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
#include "sdl_common.h"


/*
 * global variables
 */
SDL_Surface *screen = NULL;
SDL_Surface *pix    = NULL;


const char *xblastClass = "XBlast";

/*
 * end of file SDL_common.c
 */
