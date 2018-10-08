/*
 * file config.h - configuration types
 *
 * $Id: xbconfig.h,v 1.3 2006/02/09 21:21:25 fzago Exp $
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
#ifndef XBCONFIG_H
#define XBCONFIG_H

/*
 * constants
 */
#define NUM_PLAYER_COLORS 7

/*
 * type definitions
 */

/* player data shape, colors, strings etc */
typedef struct
{
	char name[16];
	char shape[16];
	XBColor helmet;
	XBColor face;
	XBColor body;
	XBColor arms_legs;
	XBColor hands_feet;
	XBColor backpack;
	XBColor white;
	XBBool useStopKey;
} XBPlayerConfig;

/* game config (not complete now) */
typedef struct
{
	int num_players;
	int num_lives;
	int num_wins;
	XBBool random_levels;
	XBBool random_players;
	XBBool *level_select;
} XBGameConfig;

#endif
/*
 * end of file config.h
 */
