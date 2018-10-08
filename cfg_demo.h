/*
 * file cfg_demo.h - configuration data for recorded demos
 *
 * $Id: cfg_demo.h,v 1.6 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_CFG_DEMO_H
#define XBLAST_CFG_DEMO_H

/*
 * type definitions
 */

/* demo parameters */
typedef struct
{
	int numPlayers;				/* number of players in game */
	int numFrames;				/* number of frames recorded */
	int numLives;				/* number of lives */
	int frameRate;				/* DEMOFIX */
	XBBool randomPlayers;		/* use random player positions */
	XBAtom level;				/* atom for level file_name */
	int randomSeed;				/* seed for random generator */
	time_t time;				/* start of recording */
	int winner;					/* winner of game */
} CFGDemoInfo;

/* demo entry data for GUI */
typedef struct
{
	XBAtom atom;				/* name of demo file */
	XBAtom level;				/* level played */
	int numPlayers;				/* # of players */
	time_t time;				/* recording time */
} CFGDemoEntry;

/*
 * global variables
 */

/* database for current demo playback/recording */
extern DBRoot *dbDemo;

/*
 * global prototypes
 */

/* managing all demo data */
extern void LoadDemoConfig (void);
extern void SaveDemoConfig (void);
extern void FinishDemoConfig (void);
extern CFGDemoEntry *CreateDemoList (size_t * num);

/* current demo: from/to file */
extern XBBool LoadDemoFromFile (XBAtom atom);
extern void SaveCurrentDemo (const char *file);

/* current demo: config data */
extern void StoreDemoConfig (const CFGDemoInfo * cfgDemo);
extern XBBool RetrieveDemoConfig (CFGDemoInfo * cfgDemo);

/* current demo: frame data */
extern void StoreDemoFrame (int gameTime, int numPlayer, const unsigned char *buf);
extern XBBool RetrieveDemoFrames (int numPlayer, unsigned char (*buf)[MAX_PLAYER]);
extern void DeleteDemoFrames (void);

#endif
/*
 * end of file cfg_demo.h
 */
