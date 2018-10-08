/*
 * file menu_level.c - level selection menu
 *
 * $Id: menu_level.h,v 1.7 2006/02/10 15:07:42 fzago Exp $
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
#ifndef XBLAST_MENU_LEVEL_H
#define XBLAST_MENU_LEVEL_H

/*
 * type defintions
 */
typedef struct
{
	const char *textAbort;
	MIC_button funcAbort;
	void *parAbort;
	const char *textDefault;
	MIC_button funcDefault;
	void *parDefault;
} XBMenuLevelPar;

/*
 * global prototypes
 */
extern XBBool CreateLevelMenu (void *par);
extern void FinishLevelSelection (void);

#endif
/*
 * end of file menu_level.h
 */
