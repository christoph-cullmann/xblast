/*
 * file game_demo.c - playback demo game
 *
 * $Id: game_demo.c,v 1.6 2006/02/09 21:21:24 fzago Exp $
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

#include "xblast.h"

/*
 * local variables
 */
static CFGGame demoGame;
static PlayerAction demoAction[MAX_PLAYER];

/*
 *
 */
static int
DemoRunLevel (int numActive, const DBRoot * level)
{
	int gameTime;
	int pauseStatus;
	int lastTeam;
	int frameTime;
	const char *msg;
	XBEventData eData;
    /* AbsInt start */
    int         aborted;
    /* AbsInt end */

	/* sanity check */
	assert (level != NULL);
	/* necesary inits */
	gameTime = 0;
	pauseStatus = -1;
	lastTeam = -1;
	frameTime = demoGame.setup.frameRate ? 1000 / demoGame.setup.frameRate : 0;
	/* load all frame data */
	DemoPlaybackStart ();
	/* Config level */
	ConfigLevel (level);
	/* init level display */
	LevelBegin (GetLevelName (level));
	/* inti events */
	GUI_SetTimer (frameTime, XBTrue);
	GUI_SetKeyboardMode (KB_XBLAST);
	GUI_SetMouseMode (XBFalse);
	/* now start */
	do {
		/* check input */
		ClearPlayerAction (demoAction);
		if (!GameEventLoop (XBE_TIMER, &eData)) {
			goto Exit;
		}
		/* increment game clock */
		gameTime++;
		/* do the game */
		GameTurn (gameTime, demoGame.players.num, &numActive);
		/* get player action from file */
		DemoPlaybackFrame (gameTime, demoAction);
        /* AbsInt start */
		(void)GameEvalAction (demoGame.players.num, demoAction, &aborted);
        /* AbsInt end */
		/* update window */
		GameUpdateWindow ();
	} while (gameTime < GAME_TIME &&
			 numActive > 0 && (numActive > 1 || NumberOfExplosions () != 0));
	/* calc result etc */
	msg = LevelResult (gameTime, &lastTeam, demoGame.players.num, level, XBFalse);
	/* show message and winner Animation */
	if (!LevelEnd (demoGame.players.num, lastTeam, msg, 1)) {
		return -1;
	}
	/* clean up */
  Exit:
	FinishLevel ();
	DeleteAllExplosions ();
	/* fade out image */
	DoFade (XBFM_BLACK_OUT, PIXH + 1);
	/* that´s all */
	return lastTeam;
}								/* DemoRunLevel */

/*
 *
 */
void
RunDemoGame (void)
{
	const DBRoot *level;

	/* retrieve config */
	if (!DemoPlaybackConfig (&demoGame)) {
		return;
	}
	/* standard inits */
	if (!InitGame (XBPH_Demo, CT_Demo, &demoGame, demoAction)) {
		return;
	}
	/* load level */
	if (NULL != (level = LoadLevelFile (DemoPlaybackLevel ()))) {
		/* run single demo */
		(void)DemoRunLevel (demoGame.players.num, level);
	}
	FinishGame (&demoGame);
}								/* StartDemoGame */

/*
 * end of file game_demo.c
 */
