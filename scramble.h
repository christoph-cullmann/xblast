/*
 * file scramble.h - scrambling blocks
 *
 * $Id: scramble.h,v 1.5 2004/11/29 14:44:49 lodott Exp $
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
#ifndef __SCRAMBLE_H
#define __SCRAMBLE_H

#include "ini_file.h"

/*
 * global prototypes
 */
extern XBBool ParseLevelScramble (const DBSection *sectionDraw, const DBSection *sectionDel, DBSection *warn1, DBSection *warn2);
extern void DoScramble (int gameTime);
 
extern int getScrambleTimes(int p_time); // XBCC
/* Scramble Structure */

typedef struct {
  int      time;
  unsigned row[MAZE_H];
} XBScrambleData;
extern XBScrambleData scrambleDraw;
extern XBScrambleData scrambleDel;
extern XBScrambleData *GetScrDel ();
extern XBScrambleData *GetScrDraw ();

#endif
/*
 * end of file scramble.h
 */
