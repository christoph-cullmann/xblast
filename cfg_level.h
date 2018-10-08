/*
 * cfg_level.c - managing level data
 *
 * $Id: cfg_level.h,v 1.11 2006/02/09 21:21:23 fzago Exp $
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

#ifndef _CFG_LEVEL_H
#define _CFG_LEVEL_H

/*
 * game mode flags for levels
 */
#define GM_Random     	(1<<0)
#define GM_2_Player   	(1<<1)
#define GM_3_Player   	(1<<2)
#define GM_4_Player   	(1<<3)
#define GM_5_Player   	(1<<4)
#define GM_6_Player   	(1<<5)
#define GM_Single     	(1<<6)
#define GM_Team       	(1<<7)
#define GM_Double     	(1<<8)
#define GM_LR_Players 	(1<<9)

#define GM_234_Player   (GM_2_Player|GM_3_Player|GM_4_Player)
#define GM_2456_Player  (GM_2_Player|GM_4_Player|GM_5_Player|GM_6_Player)
#define GM_2345_Player  (GM_234_Player|GM_5_Player)
#define GM_23456_Player (GM_2345_Player|GM_6_Player)

#define GM_All          (GM_Single|GM_Team|GM_Double|GM_LR_Players)
#define GM_SinglePlayer (GM_Single|GM_Team|GM_Double)

#define NUM_GM          11

/*
 * function prototypes
 */
extern void LoadLevelConfig (void);
extern void SaveLevelConfig (void);
extern void FinishLevelConfig (void);

extern int GetNumLevels (void);
extern XBAtom GetLevelAtom (int index);
extern const char *GetLevelNameByAtom (XBAtom atom);

extern XBBool GetLevelSelected (XBAtom atom);
extern void StoreLevelSelected (XBAtom atom, XBBool value);

extern XBBool InitLevels (const CFGGame *);

extern XBAtom GetNextLevel (void);
extern const DBRoot *LoadLevelFile (XBAtom atom);

extern void SendLevelConfig (XBSndQueue * queue, XBTeleCOT cot, const DBRoot * level);
extern void ClearRemoteLevelConfig (void);
extern void AddToRemoteLevelConfig (unsigned iob, const char *data);
extern XBBool CheckLocalLevelConfig (void);
extern XBBool CheckRemoteLevelConfig (void);
extern XBBool CheckAnyLevelConfig (const DBRoot *);
extern XBBool CheckAllLevelConfigs (void);
extern const DBRoot *GetRemoteLevelConfig (void);

#endif
/*
 * end of file cfg_level.h
 */
