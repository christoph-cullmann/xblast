/*
 * file image.h - sprite image siizes etc
 *
 * $Id: image.h,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
#ifndef _IMAGE_H
#define _IMAGE_H

#include "xblast.h"
#include "sprite.h"
#include "event.h"

/* 
 * some constants 
 */
#define MAX_BOMBS 2

/*
 * external variables
 */
/* search path */
extern const char *imgPathBlock;
extern const char *imgPathExpl;
extern const char *imgPathMisc;
extern const char *imgPathScore;
extern const char *imgPathSprite;
/* file names */
extern const char *imgFileTitle;
extern const char *imgFileTextBg, *imgFileTextFg;
extern const char *imgFileScoreLed[2];
extern const char *imgFileScoreTile[];
extern const char *imgFileScorePlayer[];
extern const char *imgFileExpl[MAX_EXPLOSION];
extern const char *imgFileBomb[MAX_BOMBS][MAX_BOMB_ANIME];
extern const char *imgFileSpritePpm[MAX_ANIME_PPM];
extern const char *imgFileIcon[MAX_ICON_SPRITES];
/* sprite regions */
extern const BMRectangle imgRectSprite[MAX_ANIME];
extern const BMRectangle imgRectIcon[MAX_ICON_SPRITES];
/*
 * global prototypes
 */ 
extern const char *ImgFileSpriteEpm (XBAtom shape, int anime);
extern const XBAtom *GetShapeList (int *pNum);
extern void ClearShapeList (void);

#endif
/*
 * end of file image.h
 */
