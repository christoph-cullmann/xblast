/*
 * file demo.c - recording and playback of xblast games
 *
 * $Id: demo.c,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
 *
 */
#include "demo.h"

#include "atom.h"
#include "cfg_demo.h"
#include "cfg_player.h"
#include "random.h"

/*
 * local variables
 */
static unsigned char buffer[GAME_TIME+1][MAX_PLAYER];
static CFGDemoInfo   cfgDemo;

/*
 * initialize demo recording and databases
 */
void
//DemoInitGame (XBPlayerHost hostType, const CFGGame *cfgGame)
DemoInitGame (CFGType cfgType, const CFGGame *cfgGame) // DEMOFIX
{
  int       i;
  CFGPlayer cfg;

  /* sanity check */
  assert (NULL != cfgGame);
  /* store player data */
  for (i = 0; i < MAX_PLAYER; i ++) {
    DeletePlayerConfig (CT_Demo, atomArrayPlayer0[i+1]);
  }
  for (i = 0; i < cfgGame->players.num; i ++) {
    /* retrive config */
    //    (void) RetrievePlayer ((hostType == XBPH_Local) ? CT_Local : CT_Remote, 
    (void) RetrievePlayer (cfgType, // DEMOFIX
			   cfgGame->players.player[i], cfgGame->players.teamColor[i], &cfg);
    /* store it */
    Dbg_Out("%s\n",cfg.name);
    StorePlayer (CT_Demo, atomArrayPlayer0[i+1], &cfg);
  }
  /* setup demo info */
  cfgDemo.numPlayers    = cfgGame->players.num;
  cfgDemo.numLives      = cfgGame->setup.numLives;
  cfgDemo.randomPlayers = cfgGame->setup.randomPlayers;
  cfgDemo.frameRate     = cfgGame->setup.frameRate;
  cfgDemo.numFrames  	= 0;
  cfgDemo.randomSeed 	= 0;
  cfgDemo.level      	= ATOM_INVALID;
  cfgDemo.time       	= 0;
  cfgDemo.winner       	= 0; /* draw */
} /* DemoInit */

/*
 *
 */
void
DemoFinishGame ()
{
} /* DemoFinishGame */

/*
 * record single frame for demo
 */
void 
DemoRecordFrame (int gameTime, const PlayerAction *pa)
{
  int i;

  /* sanity check */
  assert (gameTime  <= GAME_TIME);
  assert (pa != NULL);
  /* store data */
  for (i = 0; i < cfgDemo.numPlayers; i ++) {
    buffer[gameTime][i] = PlayerActionToByte (pa + i);
  }
} /* DemoRecordFrame */

/*
 * start single level
 */
void
DemoInitLevel (XBAtom level)
{
  cfgDemo.randomSeed = (int) GetRandomSeed ();
  cfgDemo.level      = level;
  cfgDemo.time       = time (NULL);
  /* */
  DeleteDemoFrames ();
} /* DemoInitLevel */

/*
 * finish demo recording for one level
 */
static void
SavePlayerAction (int gameTime)
{
  int i, j;

  assert (gameTime <= GAME_TIME + 1);
  for (i = 0; i < gameTime; i ++) {
    for (j = 0; j < cfgDemo.numPlayers; j ++) {
      if (buffer[i][j] != 0) {
	StoreDemoFrame (i, cfgDemo.numPlayers, buffer[i]);
	break;
      }
    }
  }
  return;
} /* SaveDemoKeys */

/*
 * level has finsihed, store demo data to disk
 */
void
DemoFinishLevel (int gameTime, int winner)
{
  struct tm *tm;
  char tmp[256];

  /* try to save key (aka player actions) to disk */
  SavePlayerAction (gameTime);
  /* complete and store demo info */
  cfgDemo.numFrames = gameTime;
  cfgDemo.winner = winner;
  
  StoreDemoConfig (&cfgDemo);
  /* create file name */
  tm = localtime (&cfgDemo.time);
  assert (NULL != tm);
  sprintf (tmp, "%04d_%02d_%02d_%02d%02d_%02d_%s", 
	   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
	   tm->tm_hour, tm->tm_min,tm->tm_sec,
	   GUI_AtomToString (cfgDemo.level));
  /* save it to disk */
  SaveCurrentDemo (tmp);
} /* DemoFinishLevel */

/*
 * get game config for demo playback
 */
XBBool
DemoPlaybackConfig (CFGGame *cfgGame)
{
  int         i;

  /* sanity check */
  assert (NULL != cfgGame);
  /* get info on current demo */
  if (! RetrieveDemoConfig (&cfgDemo) ) {
    return XBFalse;
  }
  memset (cfgGame, 0, sizeof (*cfgGame));
  /* set game config */
  cfgGame->setup.numLives      = cfgDemo.numLives;
  cfgGame->setup.numWins       = 0;
  cfgGame->setup.frameRate     = cfgDemo.frameRate;
  cfgGame->setup.allLevels     = XBFalse;
  cfgGame->setup.randomLevels  = XBFalse;
  cfgGame->setup.randomPlayers = cfgDemo.randomPlayers;
  cfgGame->setup.recordDemo    = XBFalse;
  /* set player config */
  cfgGame->players.num = cfgDemo.numPlayers;
  for (i = 0; i < cfgGame->players.num; i ++) {
    cfgGame->players.player[i]    = atomArrayPlayer0[i+1];
    cfgGame->players.control[i]   = XBPC_None;
    cfgGame->players.host[i]      = XBPH_Demo;
    cfgGame->players.team[i]      = XBPT_None;
    cfgGame->players.teamColor[i] = COLOR_INVALID;
  }
  return XBTrue;
} /* DemoPlaybackConfig */

/*
 * level for demo  playback
 */
XBAtom
DemoPlaybackLevel (void)
{
  /* set random seed properly */
  SeedRandom (cfgDemo.randomSeed);
  return cfgDemo.level;
} /* DemoPlaybackLevel */

/*
 * init frames for demo playback
 */
XBBool
DemoPlaybackStart (void)
{
  /* clear buffer */
  memset (buffer, 0, sizeof (buffer));
  /* get frames data */
  return RetrieveDemoFrames (cfgDemo.numPlayers, buffer);
} /* DemoPlaybackInitFrames */

/*
 * get single frame for playback
 */
XBBool
DemoPlaybackFrame (int gameTime, PlayerAction *pa)
{
  int i;

  assert (NULL != pa);
  assert (gameTime <= GAME_TIME);

  if (gameTime >= cfgDemo.numFrames) {
    return XBFalse;
  }
  for (i = 0; i < cfgDemo.numPlayers; i ++) {
    PlayerActionFromByte (pa + i, buffer[gameTime][i]);
  }
  return XBTrue;
} /* DemoPlaybackFrame */

/*
 * end of file demo.c
 */
