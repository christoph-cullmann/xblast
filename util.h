/*
 * file util.h - file and directory i/o
 *
 * $Id: util.h,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#ifndef _UTIL_H
#define _UTIL_H

#include "common.h"

/*
 *  type definitions
 */

/* structure with directory entries */
typedef struct _xb_dir {
  char           *name;
  time_t          mtime;
  struct _xb_dir *next;
} XBDir;

/*
 * function prototypes
 */
extern unsigned char *ReadPbmFile (const char *path, const char *file, int *width, int *height);
extern unsigned char *ReadPpmFile (const char *path, const char *file, int *width, int *height);
extern unsigned char *ReadEpmFile (const char *path, const char *file, int *width, int *height, int  *depth);
extern char *ReadRawFile (const char *path, const char *filename, size_t *len);

extern FILE  *FileOpen (const char *path, const char *name, const char *ext, const char *mode);
extern XBDir *CreateFileList (const char *path, const char *ext);

extern void DeleteFileList (XBDir *list);

#endif
/*
 * end of file util.h
 */
