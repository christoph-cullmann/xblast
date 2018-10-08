/*
 * file game_client.c - run game as client
 *
 * $Id: game_client.c,v 1.45 2006/02/24 22:09:59 fzago Exp $
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
static CFGGame clientGame;
static PlayerAction clientAction[MAX_PLAYER];
static XBBool playerLinked[MAX_PLAYER];
static int pa[MAX_PLAYER];
static int teamActive;
static int numActive;
static XBBool autoPress;
static char exitMsg[100];
/*
 * initialize linked players
 */
static void
InitPlayerLink (void)
{
	int i;
	assert (clientGame.players.num <= MAX_PLAYER);
	for (i = 0; i < clientGame.players.num; i++) {
		playerLinked[i] = XBTrue;
	}
	for (; i < MAX_PLAYER; i++) {
		playerLinked[i] = XBFalse;
	}
}								/* InitPlayerLink */

/*
 * set unlinked active players to inactive
 */
static XBBool
UpdatePlayerLink (void)
{
	int i;
	XBBool result = XBFalse;
	for (i = 0; i < clientGame.players.num; i++) {
		if (!playerLinked[i] && !player_stat[i].in_active) {
			player_stat[i].in_active = XBTrue;
			player_stat[i].lives = 0;
			result = XBTrue;
		}
	}
	return result;
}								/* UpdatePlayerLink */

/*
 * determine active players and counts
 */
static void
GetActivePlayers (int *pa, int *pl, int *tm)
{
	int i, reinco;
	*pl = 0;
	*tm = 0;
	reinco = 0;
	for (i = 0; i < clientGame.players.num; i++) {
		pa[i] = !player_stat[i].in_active;
		if (!player_stat[i].in_active) {
			*pl = *pl + 1;
			if (clientGame.setup.teamMode) {
				if (!(reinco & (1 << player_stat[i].team))) {
					reinco |= 1 << player_stat[i].team;
					*tm = *tm + 1;;
				}
			}
			else {
				*tm = *tm + 1;;
			}
		}
	}
	Dbg_Game ("%i active player, %i active teams (%s mode)\n", *pl, *tm,
			  clientGame.setup.teamMode ? "team" : "chaos");
}								/* GetActivePlayers */

/*
 * wait for an event from server
 */
static XBBool
WaitForServerEvent (XBNetworkEvent waitEvent, XBBool needFlush)
{
	XBEventCode xbEvent;
	XBEventData eData;
	XBNetworkEvent netEvent;
	unsigned id;
	XBPlayerHost host;
	int i;

	/* set timer */
	GUI_SetTimer (FRAME_TIME, XBTrue);
	/* no keys/mouse */
	GUI_SetKeyboardMode (KB_XBLAST);
	GUI_SetMouseMode (XBFalse);
	/* get events */
	while (1) {
		/* update window */
		GameUpdateWindow ();
		/* wait for xblast event */
		xbEvent = GUI_WaitEvent (&eData);
		switch (xbEvent) {
		case XBE_XBLAST:
			/* check for escape */
			if (eData.value == XBXK_EXIT) {
				return XBFalse;
			}
			break;
		case XBE_TIMER:
			/* try to flush udp connections */
			if (needFlush) {
				needFlush = Client_FlushPlayerAction ();
			}
			/* loop through all network events that accumulated */
			while (XBNW_None != (netEvent = Network_GetEvent (&id))) {
				if (netEvent == waitEvent) {
					/* event waited for, return success */
					return XBTrue;
				}
				else {
					/* otherwise check for disconnect */
					switch (netEvent) {
					case XBNW_Disconnected:
						if (id == 0 && waitEvent != XBNW_None) {
							/* server disconnected, but client still waiting, return failure */
							sprintf (exitMsg, "server disconnected");
							Dbg_Game ("exit - %s\n", exitMsg);
							return XBFalse;
						}
						else {
							/* another client disconnected, unlike its players */
							Dbg_Game ("client #%u disconnected\n", id);
							host = XBPH_Client1 + id - 1;
							for (i = 0; i < clientGame.players.num; i++) {
								if (clientGame.players.host[i] == host) {
									playerLinked[i] = XBFalse;
									Dbg_Game ("unlink player %d\n", i);
								}
							}
						}
						break;
					case XBNW_Error:
						/* network error, return failure */
						sprintf (exitMsg, "network error to server");
						Dbg_Game ("exit - %s\n", exitMsg);
						return XBFalse;
					default:
						break;
					}
				}
				/* network queue is now emptied, check if that was waited for */
				if (waitEvent == XBNW_None) {
					return XBTrue;
				}
				break;
		default:
				/* check for chat event */
				(void)Chat_Event (xbEvent, eData);
				break;
			}
		}
	}
	return XBFalse;
}								/* WaitForServerEvent */

/*
 * send sync signal to server and wait for acknowledgement
 */
static XBBool
SyncWithServer (XBNetworkEvent syncEvent, XBBool needFlush, XBBool showMsg)
{
	Dbg_Game ("waiting for others \n");
	if (showMsg) {
		SetMessage ("Waiting for others ...", XBTrue);
	}
	Client_SendSync (syncEvent);
	if (!WaitForServerEvent (syncEvent, XBFalse)) {
		return XBFalse;
	}
	return XBTrue;
}								/* SyncWithServer */

/*
 * wait for level data from server
 */
static const DBRoot *
LevelFromServer (void)
{
	int cnt = 0;
	do {
		Dbg_Game ("level negotiations running\n");
		if (!WaitForServerEvent (XBNW_LevelConfig, XBFalse)) {
			Dbg_Game ("%i level proposals processed\n", cnt / 2);
			return NULL;
		}
		cnt++;
		if (!Client_FixedLevel ()) {
			Dbg_Game ("received level data from server\n");
			if (Client_RejectsLevel ()) {
				Dbg_Game ("rejecting level, resetting\n");
				ClearRemoteLevelConfig ();
			}
			else {
				Dbg_Game ("accepting level proposal from server\n");
			}
			Client_SendLevelCheck ();
		}
	} while (!Client_FixedLevel ());
	Dbg_Game ("received level activation by server\n");
	if (Client_RejectsLevel ()) {
		sprintf (exitMsg, "rejected activated level, protocol error!");
		Dbg_Game ("exit - %s\n", exitMsg);
		return NULL;
	}
	Dbg_Game ("level negotiations successful\n");
	return GetRemoteLevelConfig ();
}								/* LevelFromServer */

/*
 * run a level as client
 */
static int
ClientRunLevel (int numActive, const DBRoot * level)
{
	int gameTime;
	int pauseStatus;
	int lastTeam, winner;
	const char *msg;
	XBEventData data;
    /* AbsInt start */
    int         aborted;
    /* AbsInt end */
	/* sanity check */
	assert (NULL != level);
	/* necesary inits */
	winner = -1;
	gameTime = 0;
	pauseStatus = -1;
	lastTeam = -1;
	/* start demo recording if requested */
	if (clientGame.setup.recordDemo) {
		DemoInitLevel (DB_Atom (level));
	}
	/* Config level */
	if (!ConfigLevel (level)) {
		Dbg_Game ("level config failed!\n");
	}
	/* clear async flag */
	Client_SetLevelAsync (XBFalse);
	/* clear action array of client */
	Client_ClearPlayerAction ();
	Dbg_Game ("cleared client game data\n");
    /* AbsInt begin */
    SND_Beep();
    /* AbsInt end */
	/* show level info */
	if (!LevelIntro (clientGame.players.num, level, autoPress ? -1 : clientGame.setup.infoTime)) {
		Dbg_Game ("game aborted in level intro\n");
		goto Exit;
	}
	/* sync with server */
	Dbg_Game ("syncing level-intro with server\n");
	if (!SyncWithServer (XBNW_SyncLevelIntro, XBFalse, XBTrue)) {
		Dbg_Game ("sync failed, disconnecting\n");
		goto Exit;
	}
	Dbg_Game ("received level-intro sync\n");
	/* reset dgram socket after sync, so that early server data triggers no events */
	Client_ResetPlayerAction ();
	/* update active players/teams */
	UpdatePlayerLink ();
	GetActivePlayers (pa, &numActive, &teamActive);
	/* show level map */
	LevelBegin (GetLevelName (level));
	/* set status bar to new level */
	ResetStatusBar (player_stat, GetLevelName (level), XBTrue);
	/* init events */
	GUI_SetTimer (0, XBFalse);
	GUI_SetKeyboardMode (KB_XBLAST);
	GUI_SetMouseMode (XBTrue);
	/* play music if requested */
	if (clientGame.setup.Music) {
		SND_Load (clientGame.setup.Music);
		SND_Play (clientGame.setup.Music, SOUND_MIDDLE_POSITION);
	}
	/* now start level */
	do {
		/* increment game clock */
		gameTime++;
		Dbg_Action ("----------\n");
		/* clear input data */
		ClearPlayerAction (clientAction);
	  Wait:
		/* get events */
		Dbg_Action ("waiting for server event at gt=%u\n", gameTime);
		if (!GameEventLoop (XBE_SERVER, &data)) {
			Dbg_Game ("game aborted in game event loop\n");
			goto Exit;
		}
		/* handle vents */
		switch ((XBServerEvent) data.value) {
		case XBSE_FINISH:
			Dbg_Action ("server sent level finish\n");
			goto Finish;
		case XBSE_ERROR:
			sprintf (exitMsg, "server error received during level");
			Dbg_Game ("exit - %s\n", exitMsg);
			goto Exit;
		default:
			/* show next frame */
			Dbg_Action ("received action for gt=%u, current=%u\n", data.value, gameTime);
			if (gameTime < data.value) {
				/* requeue event, for accelerated processing of intermediate game times */
				Dbg_Action ("requeue %u\n", data.value);
				GUI_SendEventValue (XBE_SERVER, data.value);
			}
			else if (gameTime == data.value) {
				Dbg_Action ("expected\n");
			}
			else {
				Dbg_Action ("repeat\n");
				goto Wait;
			}
			break;
		}
		Dbg_Action ("do game stuff for gt=%u\n", gameTime);
		/* bot */
		Player_BotAction (clientAction);
		/* handle game turn */
		GameTurn (gameTime, clientGame.players.num, &numActive);
		/* send own keys to server */
		Client_SendPlayerAction (gameTime, clientAction);
		/* insert keys from server */
		Client_GetPlayerAction (gameTime, clientAction);
		/* record demo if needed */
		if (clientGame.setup.recordDemo) {
			DemoRecordFrame (gameTime, clientAction);
		}
		/* evaluate input data */
        /* AbsInt start */
		(void)GameEvalAction (clientGame.players.num, clientAction, &aborted);
        /* AbsInt end */
		/* update window */
		GameUpdateWindow ();
	} while (1);

  Finish:
	Dbg_Game ("level ended\n");
	/* stop music if necessary */
	if (clientGame.setup.Music) {
		SND_Stop (clientGame.setup.Music);
	}
	/* activate bot if client was idle */
	Player_CheckLocalAway ();
	/* now update autoPress */
	autoPress = Player_CheckLocalBot ();
	if (autoPress) {
		Dbg_Out ("AUTOPRESS!!!\n");
	}
	/* send finish message */
	Dbg_Game ("sending level finish\n");
	Client_FinishPlayerAction (gameTime);
	/* determine last team, don't store */
	LevelResult (gameTime, &lastTeam, clientGame.players.num, level, XBFalse);
	/* finish demo file if needed */
	if (clientGame.setup.recordDemo) {
		DemoFinishLevel (gameTime, winner, "c");
	}
	/* send winner team */
	Dbg_Game ("sending winner team %u\n", lastTeam);
	Client_SendWinner (lastTeam);
	/* wait for server to send response */
	Dbg_Game ("waiting for server level-result sync\n");
	if (!WaitForServerEvent (XBNW_SyncLevelResult, XBFalse)) {
		Dbg_Game ("sync failed\n");
		goto Exit;
	}
	Dbg_Game ("level-result sync received\n");
	/* check if async was responded */
	if (Client_LevelAsynced ()) {
		Dbg_Game ("level async received, ignoring result\n");
		GUI_ErrorMessage ("Async level, making it a draw\n");
		lastTeam = MAX_PLAYER;
	}
	else {
		Dbg_Game ("level was sync\n");
		/* now store result */
		msg = LevelResult (gameTime, &lastTeam, clientGame.players.num, level, XBTrue);
		/* show message and winner Animation */
		if (!LevelEnd (clientGame.players.num, lastTeam, msg, autoPress ? -1 : 1)) {
			Dbg_Game ("aborted game during winner animation\n");
			lastTeam = -1;
		}
	}
	/* clean up */
  Exit:
	/* stop music if necessary */
	if (clientGame.setup.Music) {
		SND_Stop (clientGame.setup.Music);
	}
	DeleteAllExplosions ();
	FinishLevel ();
	/* fade out image */
	DoFade (XBFM_BLACK_OUT, PIXH + 1);
	/* that's all */
	return lastTeam;
}								/* ClientRunLevel */

/*
 * run game as client
 */
void
RunClientGame (XBPlayerHost hostType)
{
	const DBRoot *level;
	int lastTeam;
	int i, maxNumWins;
	/* clear exit message */
	memset (exitMsg, 0, sizeof (exitMsg));
	/* get game configs */
	if (!RetrieveGame (CT_Remote, SERVERGAMECONFIG, &clientGame)) {
		Dbg_Game ("failed to get game config!\n");
		sprintf (exitMsg, "game config error");
		Dbg_Game ("exit - %s\n", exitMsg);
		goto Disconnect;
	}
	/* common inits */
	if (!InitGame (hostType, CT_Remote, &clientGame, clientAction)) {
		sprintf (exitMsg, "game init error");
		Dbg_Game ("exit - %s\n", exitMsg);
		goto Disconnect;
	}
	/* init local data */
	maxNumWins = 0;
	numActive = 0;
	teamActive = 0;
	autoPress = Player_CheckLocalBot ();
	/* init player link flags */
	InitPlayerLink ();
	/* clear level data for next run */
	ClearRemoteLevelConfig ();
	Client_LevelFix (XBFalse);
	Client_LevelRejection (XBTrue);
	/* beep once if requested */
    /* AbsInt begin */
    /*
	if (clientGame.setup.beep) {
		SND_Beep ();
	}
    */
    /* AbsInt end */
	/* sync with server */
	Dbg_Game ("waiting for server end-of-init sync\n");
	if (!SyncWithServer (XBNW_SyncEndOfInit, XBFalse, XBFalse)) {
		Dbg_Game ("sync failed\n");
		goto Exit;
	}
	Dbg_Game ("end-of-init sync received\n");
	/* update active players/teams */
	UpdatePlayerLink ();
	GetActivePlayers (pa, &numActive, &teamActive);

	GUI_ShowCursor(XBFalse);

	/* start the game */
	do {
		/* load and run next level */
		if (NULL == (level = LevelFromServer ())) {
			Dbg_Game ("level negotiations failed, exiting\n");
			goto Exit;
		}
		/* update active players/teams */
		UpdatePlayerLink ();
		GetActivePlayers (pa, &numActive, &teamActive);
		/* run the level */
		Dbg_Game ("starting level\n");
		lastTeam = ClientRunLevel (teamActive, level);
		/* check for quick exit */
		if (-1 == lastTeam) {
			Dbg_Game ("game aborted during level\n");
			goto Exit;
		}
		/* update current winner */
		for (i = 0; i < clientGame.players.num; i++) {
			if (player_stat[i].victories > maxNumWins) {
				maxNumWins = player_stat[i].victories;
			}
		}
		/* sync before showing score board */
		Dbg_Game ("waiting for server sync level-end\n");
		if (!SyncWithServer (XBNW_SyncLevelEnd, XBFalse, XBTrue)) {
			Dbg_Game ("sync failed!\n");
			goto Exit;
		}
		Dbg_Game ("level-end sync received\n");
		/* show scores */
		if (!ShowScoreBoard
			(lastTeam, maxNumWins, clientGame.players.num, player_stat, autoPress ? -1 : 1)) {
			Dbg_Game ("game aborted during scoreboard\n");
			goto Exit;
		}
		/* clear level data for next run, must be before scoreboard sync */
		ClearRemoteLevelConfig ();
		Client_LevelFix (XBFalse);
		Client_LevelRejection (XBTrue);
		Dbg_Game ("reset level data, now waiting for server sync scoreboard\n");
		if (!SyncWithServer (XBNW_SyncScoreboard, XBFalse, XBTrue)) {
			Dbg_Game ("sync failed!\n");
			goto Exit;
		}
		/* determine active players/teams */
		UpdatePlayerLink ();
		GetActivePlayers (pa, &numActive, &teamActive);
	} while (numActive > 1 && teamActive > 1 && maxNumWins < clientGame.setup.numWins);

	GUI_ShowCursor(XBTrue);

	Dbg_Game ("game ended\n");
	SetMessage ("Waiting for server", XBTrue);
	/* show winner */
	InitWinner (clientGame.players.num);
	ShowWinner (lastTeam, clientGame.players.num, player_stat);
	/* wait till network queue is cleared */
	Dbg_Game ("waiting for server to shutdown!\n");
	(void)WaitForServerEvent (XBNW_None, XBFalse);

  Exit:
	GUI_ShowCursor(XBTrue);
	FinishGame (&clientGame);

  Disconnect:
	Client_Disconnect ();
	if (strlen (exitMsg) > 0) {
		GUI_ErrorMessage (exitMsg);
	}
	return;
}								/* RunClientGame */

/*
 * end of file game_client.c
 */
