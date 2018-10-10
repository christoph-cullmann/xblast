/*
 * file level.h - Setting up level given in database
 *
 * $Id: level.h,v 1.5 2004/12/04 06:01:13 lodott Exp $
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
#ifndef _LEVEL_H
#define _LEVEL_H

/*
 * type definitions
 */
#include "ini_file.h"

/*
 * global prototypes
 */
extern XBBool ParseLevel (const DBRoot *level);
extern XBBool ConfigLevel (const DBRoot *level);
extern void FinishLevel (void);

extern const char *GetLevelName   (const DBRoot *level);
extern const char *GetLevelAuthor (const DBRoot *level);
extern const char *GetLevelHint   (const DBRoot *level);
extern unsigned GetWarningCount();

#endif
/*
 * end of file level.h
 */
