/*
 * file util.h - file and directory i/o
 *
 * $Id: util.h,v 1.8 2006/02/09 21:21:25 fzago Exp $
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

/*
 *  type definitions
 */

/* structure with directory entries */
typedef struct _xb_dir
{
	char *name;
	time_t mtime;
	struct _xb_dir *next;
} XBDir;

/*
 * function prototypes
 */

/* loading config file */
extern FILE *FileOpen (const char *path, const char *name, const char *ext, const char *mode);

/* create/destroy dir lists */
extern XBDir *CreateFileList (const char *path, const char *ext, XBBool rec);
extern void DeleteFileList (XBDir * list);

/* loading images */
extern unsigned char *ReadPbmFile (const char *path, const char *file, int *width, int *height);
extern unsigned char *ReadPpmFile (const char *path, const char *file, int *width, int *height);
extern unsigned char *ReadEpmFile (const char *path, const char *file, int *width, int *height,
								   int *depth);
extern char *ReadRawFile (const char *path, const char *filename, size_t * len);

#endif
/*
 * end of file util.h
 */
