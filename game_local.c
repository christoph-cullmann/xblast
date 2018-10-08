/*
 * file game_local.c - run a local game
 *
 * $Id: game_local.c,v 1.17 2006/02/24 22:10:22 fzago Exp $
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
static CFGGame localGame;
static PlayerAction localAction[MAX_PLAYER];

/*
 *
 */
static int
LocalRunLevel (int numActive, const DBRoot * level)
{
	int gameTime;
	int pauseStatus;
	int lastTeam;
	int frameTime;
	int oldaction;
	int winner, counter;
	BMPlayer *ps = NULL;
	const char *msg;
	int away = 1;
	XBEventData eData;
    /* AbsInt start */
    int         aborted;
    /* AbsInt end */

	/* sanity check */
	assert (level != NULL);
	/* necesary inits */
	Dbg_Out ("Level playing is: %s\n", GetLevelName (level));

	winner = -1;
	gameTime = 0;
	pauseStatus = -1;
	lastTeam = -1;
	frameTime = localGame.setup.frameRate ? 1000 / localGame.setup.frameRate : 0;
	/* start demo recording */
	SeedRandom (time (NULL));
	if (localGame.setup.recordDemo) {
		DemoInitLevel (DB_Atom (level));
	}
	/* Config level */
	if (!ConfigLevel (level)) {
		goto Exit;
	}
    /* AbsInt begin */
    SND_Beep();
    /* AbsInt end */
	/* show level info */
	if (!LevelIntro (localGame.players.num, level, 0)) {
		goto Exit;
	}
	/* init level display */
	LevelBegin (GetLevelName (level));
	/* init events */
	GUI_SetTimer (frameTime, XBTrue);
	GUI_SetKeyboardMode (KB_XBLAST);
	GUI_SetMouseMode (XBFalse);
	if (localGame.setup.Music) {
		SND_Load (localGame.setup.Music);
		SND_Play (localGame.setup.Music, SOUND_MIDDLE_POSITION);
	}
	ClearPlayerAction (localAction);
	oldaction = PlayerActionToByte (&localAction[0]);
	/* now start */
	do {
		if (pauseStatus != -1) {
			/* 
			 * PAUSE mode 
			 */
			/* check if pause is to end */
			if (pauseStatus < 0) {
				pauseStatus++;
				if (-1 == pauseStatus) {
					GUI_Bell ();
					ResetMessage ();
				}
			}
			/* check input */
			ClearPlayerAction (localAction);
			if (!GameEventLoop (XBE_TIMER, &eData)) {
				goto Exit;
			}
			/* check if player has ended pause mode */
			if (PauseEvalAction (localGame.players.num, localAction, pauseStatus)) {
				pauseStatus = -PAUSE_DELAY;
				SetMessage ("Continue", XBTrue);
				GUI_Bell ();
			}
		}
		else {
			/*
			 * INGAME mode
			 */
			/* check input */
			// fprintf(stderr,"action1 %i \n",localAction[0]);
			ClearPlayerAction (localAction);
			if (!GameEventLoop (XBE_TIMER, &eData)) {
				goto Exit;
			}
			/* increment game clock */
			gameTime++;			/* bot */
			counter = 0;
			for (ps = player_stat, counter = 1; ps < player_stat + localGame.players.num;
				 ps++, counter++) {
				if (ps->local) {
					break;

				}
			}

			if (ps->bot == XBTrue) {
				gestionBot (player_stat, localAction, counter - 1, localGame.players.num);
			}
			/* do the game */
			GameTurn (gameTime, localGame.players.num, &numActive);
			if (localGame.setup.bot) {
				gestionBot (player_stat, localAction, 1, localGame.players.num);
			}
			/* record actions for demo data */
			if (localGame.setup.recordDemo) {
				DemoRecordFrame (gameTime, localAction);
			}
			/* evaluate player actions */
			if (PlayerActionToByte (&localAction[0]) != oldaction) {
				oldaction = PlayerActionToByte (&localAction[0]);
				away = 0;
				ps->bot = XBFalse;
			}
            /* AbsInt start */
			if (-1 != (pauseStatus = GameEvalAction (localGame.players.num, localAction, &aborted))) {
                /* AbsInt end */
				/* pause was activated */
				assert (pauseStatus < localGame.players.num);
				SetMessage (p_string[pauseStatus].pause, XBTrue);
			}
		}
		/* update window */
		GameUpdateWindow ();
	} while (gameTime < GAME_TIME &&
			 numActive > 0 && (numActive > 1 || NumberOfExplosions () != 0));
	if (away == 1) {
		ps->bot = XBTrue;
	}
	LevelResult (gameTime, &lastTeam, localGame.players.num, level, XBFalse);
	if (lastTeam <= MAX_PLAYER) {
		for (ps = player_stat, counter = 1; ps < player_stat + localGame.players.num;
			 ps++, counter++) {
			if (ps->team == lastTeam) {
				winner = counter;
			}
		}
	}
/* finish demo file if needed */
	if (localGame.setup.recordDemo) {
		DemoFinishLevel (gameTime, winner, "l");
	}
	/* calc result etc */
	msg = LevelResult (gameTime, &lastTeam, localGame.players.num, level, XBTrue);
	/* show message and winner Animation */
	if (!LevelEnd (localGame.players.num, lastTeam, msg, 0)) {
		/* abort */
		lastTeam = -1;
	}
	if (localGame.setup.Music) {
		SND_Stop (localGame.setup.Music);
	}
	/* clean up */
  Exit:
	if (localGame.setup.Music) {
		SND_Stop (localGame.setup.Music);
	}
	FinishLevel ();
	DeleteAllExplosions ();
	/* fade out image */
	DoFade (XBFM_BLACK_OUT, PIXH + 1);
	/* that´s all */
	return lastTeam;
}								/* LocalRunLevel */

/*
 *
 */
void
RunLocalGame (void)
{
	const DBRoot *level;
	int lastTeam;
	int i, maxNumWins;
	int numActive;
	XBBool initDone = XBFalse;

	/* retrieve game configs */
	if (!RetrieveGame (CT_Local, atomLocal, &localGame)) {
		goto Exit;
	}
	/* select levels to plays */
	if (!InitLevels (&localGame)) {
		goto Exit;
	}
	/* init rest of data */
	if (!InitGame (XBPH_Local, CT_Local, &localGame, localAction)) {
		goto Exit;
	}
	initDone = XBTrue;

	GUI_ShowCursor(XBFalse);

	/* game loop */
	maxNumWins = 0;
	numActive = localGame.players.num;
	do {
		/* load and run next level */
		level = LoadLevelFile (GetNextLevel ());
		lastTeam = LocalRunLevel (numActive, level);
		/* check for quick exit */
		if (-1 == lastTeam) {
			goto Exit;
		}
		/* calculate new maxmium # of victories */
		for (i = 0; i < localGame.players.num; i++) {
			if (player_stat[i].victories > maxNumWins) {
				maxNumWins = player_stat[i].victories;
			}
		}
		/* show scores */
		if (!ShowScoreBoard (lastTeam, maxNumWins, localGame.players.num, player_stat, 0)) {
			goto Exit;
		}
		/* determine number of active players */
		numActive = 0;
		for (i = 0; i < localGame.players.num; i++) {
			if (!player_stat[i].in_active) {
				numActive++;
			}
		}
		Dbg_Out ("%d active players\n", numActive);
	} while (numActive > 1 && maxNumWins < localGame.setup.numWins);

	GUI_ShowCursor(XBTrue);

	/* end screen */
	InitWinner (localGame.players.num);
	ShowWinner (lastTeam, localGame.players.num, player_stat);
	/* that's all */
	FinishGame (&localGame);
	return;

  Exit:

	GUI_ShowCursor(XBTrue);

	if (initDone) {
		FinishGame (&localGame);
	}
	return;
}								/* StartLocalGame */

/*
 * end of file game_local.c
 */
