/*
 * file x11_common.h -
 *
 * $Id: x11_common.h,v 1.4 2004/05/16 16:52:46 iskywalker Exp $
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
#ifndef XBLAST_X11_COMMON_H
#define XBLAST_X11_COMMON_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

#include "common.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#if defined (sun)
#include <sys/sockio.h>
#endif

#include "xblast.h"
/*
 * global constants
 */
#define NUM_FONTS 3

/*
 * global variables;
 */
extern Display  *dpy;
extern int       defDepth;
extern int       bitsPerPixel;
extern Window    win;
extern Colormap  cmap;
extern Visual   *defVisual;
extern int       whitePixel;
extern int 	 blackPixel;
extern int       iconified;

extern char     *xblastResName;
extern char     *xblastResClass;

extern GC 	 gcFromPix;
extern GC 	 gcWindow;
extern GC        gcClearPix;

extern Pixmap    pix;

#endif
/*
 * end of file common.h
 */
