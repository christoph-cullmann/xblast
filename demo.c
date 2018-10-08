/*
 * file demo.c - recording and playback of xblast games
 *
 * $Id: demo.c,v 1.8 2006/03/28 11:41:19 fzago Exp $
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

#include "xblast.h"

/*
 * local variables
 */
static unsigned char buffer[GAME_TIME + 1][MAX_PLAYER];
static CFGDemoInfo cfgDemo;

/*
 * initialize demo recording
 */
void
DemoInitGame (CFGType cfgType, const CFGGame * cfgGame)
{
	int i;
	CFGPlayer cfg;
	/* sanity check */
	assert (NULL != cfgGame);
	/* remove old demo players form database */
	for (i = 0; i < MAX_PLAYER; i++) {
		DeletePlayerConfig (CT_Demo, atomArrayPlayer0[i + 1]);
	}
	/* store current players */
	for (i = 0; i < cfgGame->players.num; i++) {
		/* get player data, ignore failures */
		(void)RetrievePlayer (cfgType, cfgGame->players.player[i], cfgGame->players.teamColor[i],
							  &cfg);
		/* store it */
		StorePlayer (CT_Demo, atomArrayPlayer0[i + 1], &cfg);
	}
	/* setup demo info */
	cfgDemo.numPlayers = cfgGame->players.num;
	cfgDemo.numLives = cfgGame->setup.numLives;
	cfgDemo.randomPlayers = cfgGame->setup.randomPlayers;
	cfgDemo.frameRate = cfgGame->setup.frameRate;
	cfgDemo.numFrames = 0;
	cfgDemo.randomSeed = 0;
	cfgDemo.level = ATOM_INVALID;
	cfgDemo.time = 0;
	cfgDemo.winner = 0;			/* draw */
}								/* DemoInit */

/*
 * finish demo recordings
 */
void
DemoFinishGame (void)
{
}								/* DemoFinishGame */

/*
 * record single frame for demo
 */
void
DemoRecordFrame (int gameTime, const PlayerAction * pa)
{
	int i;
	/* sanity check */
	assert (gameTime <= GAME_TIME);
	assert (pa != NULL);
	/* store in buffer */
	for (i = 0; i < cfgDemo.numPlayers; i++) {
		buffer[gameTime][i] = PlayerActionToByte (pa + i);
	}
}								/* DemoRecordFrame */

/*
 * initialize level recording
 */
void
DemoInitLevel (XBAtom level)
{
	/* level data */
	cfgDemo.randomSeed = (int)GetRandomSeed ();
	cfgDemo.level = level;
	cfgDemo.time = time (NULL);
	/* clear any old frames from database */
	DeleteDemoFrames ();
}								/* DemoInitLevel */

/*
 * copy buffered keys to file
 */
static void
StorePlayerActions (int gameTime)
{
	int i, j;
	/* sanity check */
	assert (gameTime <= GAME_TIME + 1);
	for (i = 0; i < gameTime; i++) {
		for (j = 0; j < cfgDemo.numPlayers; j++) {
			if (buffer[i][j] != 0) {
				StoreDemoFrame (i, cfgDemo.numPlayers, buffer[i]);
				break;
			}
		}
	}
}								/* StorePlayerActions */

/*
 * finish level recording
 */
void
DemoFinishLevel (int gameTime, int winner, const char *type)
{
	struct tm *tm;
	char tmp[256];
	/* store buffered keys */
	StorePlayerActions (gameTime);
	/* complete level info */
	cfgDemo.numFrames = gameTime;
	cfgDemo.winner = winner;
	/* store level info */
	StoreDemoConfig (&cfgDemo);
	/* create file name */
	tm = localtime (&cfgDemo.time);
	assert (NULL != tm);
	sprintf (tmp, "%s_%04d_%02d_%02d_%02d%02d_%02d_%s", type,
			 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			 tm->tm_hour, tm->tm_min, tm->tm_sec, GUI_AtomToString (cfgDemo.level));
	/* save it to disk */
	SaveCurrentDemo (tmp);
}								/* DemoFinishLevel */

/*
 * get game config for demo playback
 */
XBBool
DemoPlaybackConfig (CFGGame * cfgGame)
{
	int i;
	/* sanity check */
	assert (NULL != cfgGame);
	/* get demo config from database */
	if (!RetrieveDemoConfig (&cfgDemo)) {
		return XBFalse;
	}
	/* clear game config */
	memset (cfgGame, 0, sizeof (*cfgGame));
	/* set game config */
	cfgGame->setup.numLives = cfgDemo.numLives;
	cfgGame->setup.numWins = 0;
	cfgGame->setup.frameRate = cfgDemo.frameRate;
	cfgGame->setup.allLevels = XBFalse;
	cfgGame->setup.randomLevels = XBFalse;
	cfgGame->setup.randomPlayers = cfgDemo.randomPlayers;
	cfgGame->setup.recordDemo = XBFalse;
	/* set player configs */
	cfgGame->players.num = cfgDemo.numPlayers;
	for (i = 0; i < cfgGame->players.num; i++) {
		cfgGame->players.player[i] = atomArrayPlayer0[i + 1];
		cfgGame->players.control[i] = XBPC_None;
		cfgGame->players.host[i] = XBPH_Demo;
		cfgGame->players.team[i] = XBPT_None;
		cfgGame->players.teamColor[i] = COLOR_INVALID;
	}
	return XBTrue;
}								/* DemoPlaybackConfig */

/*
 * get level name of demo, set seed
 */
XBAtom
DemoPlaybackLevel (void)
{
	/* set random seed properly */
	SeedRandom (cfgDemo.randomSeed);
	return cfgDemo.level;
}								/* DemoPlaybackLevel */

/*
 * load demo player actions into buffer
 */
XBBool
DemoPlaybackStart (void)
{
	/* clear buffer */
	memset (buffer, 0, sizeof (buffer));
	/* get frames data */
	return RetrieveDemoFrames (cfgDemo.numPlayers, buffer);
}								/* DemoPlaybackStart */

/*
 * get single frame for playback
 */
XBBool
DemoPlaybackFrame (int gameTime, PlayerAction * pa)
{
	int i;
	assert (NULL != pa);
	assert (gameTime <= GAME_TIME);
	/* check for level end */
	if (gameTime >= cfgDemo.numFrames) {
		return XBFalse;
	}
	/* load player actions from buffer */
	for (i = 0; i < cfgDemo.numPlayers; i++) {
		PlayerActionFromByte (pa + i, buffer[gameTime][i]);
	}
	return XBTrue;
}								/* DemoPlaybackFrame */

/*
 * end of file demo.c
 */
