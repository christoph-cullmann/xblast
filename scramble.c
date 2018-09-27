/*
 * file scramble.c - scrambling blocks
 *
 * $Id: scramble.c,v 1.6 2004/11/29 14:44:49 lodott Exp $
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
#include "scramble.h"

#include "atom.h"
#include "map.h"
#include "player.h"

#include "snd.h"

/*
 * local macros
 */
#define SCRAMBLE_RAISE 4

/*
 * local types
 */

/*
 * local variables - retrieved from level config
 */
 XBScrambleData scrambleDraw;
 XBScrambleData scrambleDel;

int
getScrambleTimes(int p_time)
{
  static int i;
  if(p_time < 0) {
    i=0;
  }
  switch(i++) {
  case 0: return -scrambleDel.time; // first delete in case they are on the same time 
  case 1: return scrambleDraw.time;
  default: return 0;
  }
}

/*
 * set scramble data
 */
static XBBool
SetScrambleData (XBScrambleData *scramble, const DBSection *section, DBSection *warn)
{
  double     fTime;
  int        i, numBlocks;
  BMPosition pos;

  assert (scramble != NULL);
  /* set default values */
  scramble->time = GAME_TIME + SCRAMBLE_RAISE + 1;
  memset (scramble->row, 0, sizeof (scramble->row));
  /* check if any scramble is defined */
  if (NULL == section) {
    return XBTrue;
  }
  /* time for scramble */
  if (! DB_GetEntryFloat (section, atomTime, &fTime) ) {
    return XBTrue;
  }
  scramble->time = fTime * GAME_TIME;
  /* number of blocks */
  if (! DB_GetEntryInt (section, atomNumBlocks, &numBlocks) ) {
    Dbg_Out("LEVEL:  critical failure, %s\n", DB_SectionEntryString(section, atomNumBlocks));
    DB_CreateEntryString(warn,atomNumBlocks,"missing!");
    return XBFalse;
  }
  for (i = 0; i < numBlocks; i ++) {
    if (! DB_GetEntryPos (section, atomArrayPos000[i], &pos) ) {
      Dbg_Out("LEVEL:  critical failure, %s\n", DB_SectionEntryString(section, atomArrayPos000[i]));
      DB_CreateEntryString(warn,atomArrayPos000[i],"missing!");
      return XBFalse;
    }
    if (pos.x < 0 || pos.x >= MAZE_W ||
	pos.y < 0 || pos.y >= MAZE_H) {
      Dbg_Out("+++LEVEL+++ invalid block pos x=%hd y=%hd\n", pos.x, pos.y);
      return XBFalse;
    }
    scramble->row[pos.y] |= (1 << pos.x);
  }
  return XBTrue;
} /* SetScrambleData */

XBScrambleData *
GetScrDel ()
{
  return & scrambleDel;
}

XBScrambleData *
GetScrDraw ()
{
  return & scrambleDraw;
}
/*
 * parse level data
 */
XBBool
ParseLevelScramble (const DBSection *sectionDraw, const DBSection *sectionDel, DBSection *warn1, DBSection *warn2)
{
  if (! SetScrambleData (&scrambleDraw, sectionDraw, warn1)) {
    return XBFalse;
  }
  if (! SetScrambleData (&scrambleDel, sectionDel, warn2)) {
    return XBFalse;
  }
  return XBTrue;
} /* ParseLevelScramble */

/*
 * work a single scramble pattern
 */
static void
DoSingleScramble (const XBScrambleData *scramble, BMMapTile block)
{
  int x, y;
  unsigned mask;

  assert (NULL != scramble);
  /* --- */
  for (y = 0; y < MAZE_H; y ++) {
    if (0 != scramble->row[y]) {
      for (x = 0, mask = 1; x < MAZE_W; x ++, mask <<= 1) {
	if (mask & scramble->row[y]) {
	  SetMazeBlock (x, y, block);
	  if (block != BTFree) {
	    KillPlayerAt (x, y);
	  } 
	  if (block == BTBlockRise) {
	    SND_Play (SND_SPIRAL, x);
	  }
	}
      }
    }
  }
} /* DoSingleScramble */

/*
 * ingame polling function
 */
void
DoScramble (int gameTime)
{
  /* check for raising blocks */
  if (gameTime == (scrambleDraw.time - 4) ) {
    DoSingleScramble (&scrambleDraw, BTBlockRise);
  /* check for drawing blocks */
  } else if (gameTime == scrambleDraw.time ) {
    DoSingleScramble (&scrambleDraw, BTBlock);
  }
  /* check for deleting blocks */
  if (gameTime == scrambleDel.time ) {
    DoSingleScramble (&scrambleDel, BTFree);
  }
} /* DoScramble */

/*
 * end of scramble.c
 */
