/*
 * file x11_common.c -
 *
 * $Id: x11_common.c,v 1.3 2004/05/14 10:00:36 alfie Exp $
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

#include "x11_common.h"

/*
 * global variables
 */
Display  *dpy = NULL;   /* connection to X11 display */
Window    win;          /* the xblast window */
Colormap  cmap;         /* colormap used */
Visual   *defVisual;    /* used visual */
int       defDepth;     /* color depth (in bits) */
int       bitsPerPixel; /* pixel memroy size (in bits) */
int 	  whitePixel;   /* id for color white */
int 	  blackPixel;   /* id for color black */
int       iconified;    /* flag if window is iconfified */

char *xblastResName  = "xblast"; 
char *xblastResClass = "XBlast";

GC gcFromPix;  /* context to draw from pixmap into window */
GC gcWindow;   /* context to clear window */
GC gcClearPix; /* context to clear pixmap */

Pixmap pix;    /* pixmap for double buffering */

/*
 * end of x11_file common.c
 */
