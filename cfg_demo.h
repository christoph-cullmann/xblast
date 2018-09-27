/*
 * file cfg_demo.h - configuration data for recorded demos
 *
 * $Id: cfg_demo.h,v 1.3 2004/05/14 10:00:33 alfie Exp $
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

#include "ini_file.h"

/*
 * type definitions
 */
typedef struct {
  int    numPlayers; 	/* number of players in game */
  int    numFrames;  	/* number of frames recorded */
  int    numLives;   	/* number of lives */
  int    frameRate;     /* DEMOFIX */
  XBBool randomPlayers; /* use random player positions */
  XBAtom level;         /* atom for level file_name */
  int    randomSeed;    /* seed for random generator */
  time_t time;          /* start of recording */
  int    winner;        /* winner of game */
} CFGDemoInfo;

typedef struct {
  XBAtom atom;       /* name of demo file */
  XBAtom level;      /* level played */
  int    numPlayers; /* # of players */
  time_t time;       /* recording time */
} CFGDemoEntry;

/*
 * global variables
 */
extern DBRoot *dbDemo;

/*
 * global prototypes
 */
extern void LoadDemoConfig   (void);
extern void SaveDemoConfig   (void);
extern void FinishDemoConfig (void);

extern void StoreDemoConfig    (const CFGDemoInfo *cfgDemo);
extern void StoreDemoFrame     (int gameTime, int numPlayer, const unsigned char *buf);
extern void DeleteDemoFrames  (void);
extern void SaveCurrentDemo   (const char *file);

extern XBBool RetrieveDemoConfig (CFGDemoInfo *cfgDemo);
extern XBBool RetrieveDemoFrames (int numPlayer, unsigned char (*buf)[MAX_PLAYER]);

extern CFGDemoEntry *CreateDemoList (size_t *num);

extern XBBool LoadDemoFromFile (XBAtom atom);

#endif
/*
 * end of file cfg_demo.h
 */
