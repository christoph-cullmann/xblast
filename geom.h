/*
 * file geom.h -
 *
 * $Id: geom.h,v 1.4 2004/08/04 04:46:15 iskywalker Exp $
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

#ifndef _GEOM_H
#define _GEOM_H

#include "xblast.h"

#ifdef MINI_XBLAST
#define BASE_X 4
#define BASE_Y 3
#endif
/* Base Step in X direction */
#ifndef BASE_X 
#define BASE_X 8
#endif
/* Base Step in Y direction */
#ifndef BASE_Y
#define BASE_Y 6
#endif

/* size of map tiles */
#define BLOCK_WIDTH  (8*BASE_X) 
#define BLOCK_HEIGHT (8*BASE_Y)
/* size of elements in status */
#define STAT_WIDTH   (6*BASE_X) 
#define STAT_HEIGHT  (8*BASE_Y)
/* size of leds in status displays */
#define LED_WIDTH    (2*BASE_X)
#define LED_HEIGHT   (8*BASE_Y/3)
/* some window dimensions */
#define PIXW   	     (MAZE_W * BLOCK_WIDTH)
#define PIXH   	     (MAZE_H * BLOCK_HEIGHT)

#ifdef SMPF
#define SCOREH 	     (STAT_HEIGHT*3+LED_HEIGHT) // SMPF chat
#else
#define SCOREH 	     (STAT_HEIGHT*2+LED_HEIGHT) // chat
#endif

#endif
/*
 *
 */
