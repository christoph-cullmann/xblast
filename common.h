/*
 * file common.h - system header files common to all xblast modules
 *
 * $Id: common.h,v 1.9 2005/01/15 17:41:43 iskywalker Exp $
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
#ifndef XBLAST_COMMON_H
#define XBLAST_COMMON_H

#ifndef HAVECONFIG_H
#include "config.h"
#define HAVECONFIG_H
#endif

#ifdef HAVE_CONFIG
#include "xbconfig.h"
#define HAVE_CONFIG
#endif/*
#ifndef __USE_W32_SOCKETS
#include <sys/types.h>
#endif
#ifndef W32
#include <sys/types.h>
#endif*/
#ifdef W32
#ifndef WMS
#include <windows.h>
#include <winsock2.h>
#endif
#endif

#include <assert.h>
#include <ctype.h>
#ifdef WMS
#include "timeval.h"
#else
#include <dirent.h>
#endif
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>


#ifndef __USE_W32_SOCKETS

#ifndef W32
#include <sys/socket.h>
#endif
#endif

#include "debug.h"

#endif
/*
 * end of file common.h
 */
