/*
 * file game_client.c - run game as client
 *
 * $Id: game_client.c,v 1.29 2005/01/18 15:36:15 lodott Exp $
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
#include "game_client.h"

#include "atom.h"
#include "bomb.h"
#include "client.h"
#include "intro.h"
#include "demo.h"
#include "cfg_level.h"
#include "game.h"
#include "level.h"
#include "status.h"
#include "geom.h"
#include "bot.h"
#include "snd.h"

/*
 * local variables
 */
typedef struct {
  BMPlayer *ps;   /* player stat pointer */
  int cnt;        /* player stat index */
} BotData;

static CFGGame      clientGame;
static PlayerAction clientAction[MAX_PLAYER];
static XBBool       playerLinked[MAX_PLAYER];
static int          pa[MAX_PLAYER];
static int          teamActive;
static int          numActive;

static BotData      bot;
static char         exitMsg[100];
/*
 * initialize linked players
 */
static void
InitPlayerLink (void)
{
  int i;
  assert (clientGame.players.num <= MAX_PLAYER);
  for (i = 0; i < clientGame.players.num; i ++) {
    playerLinked[i] = XBTrue;
  }
  for (; i < MAX_PLAYER; i ++) {
    playerLinked[i] = XBFalse;
  }
} /* InitPlayerLink */

/*
 * set unlinked active players to inactive
 */
static XBBool
UpdatePlayerLink (void)
{
  int    i;
  XBBool result = XBFalse;
  for (i = 0; i < clientGame.players.num; i ++) {
    if (! playerLinked[i] &&
	! player_stat[i].in_active) {
      player_stat[i].in_active = XBTrue;
      player_stat[i].lives = 0;
      result = XBTrue;
    }
  }
  return result;
} /* UpdatePlayerLink */

/*
 * determine active players and counts
 */
static void
GetActivePlayers (int *pa, int *pl, int *tm)
{
  int i, reinco;
  *pl = 0;
  *tm = 0;
  reinco=0;
  for (i = 0; i < clientGame.players.num; i ++) {
    pa[i] = !player_stat[i].in_active;
    if (! player_stat[i].in_active) {
      *pl = *pl +1;
      if (clientGame.setup.teamMode) {
	if (! ( reinco & (1 << player_stat[i].team) ) ) {
	  reinco|=1 << player_stat[i].team;
	  *tm = *tm +1;;
	}
      } else {
	*tm = *tm +1;;
      }
    }
  }
  Dbg_Game("%i active player, %i active teams (%s mode)\n", *pl, *tm, clientGame.setup.teamMode ? "team" : "chaos");
} /* GetActivePlayers */

/*
 * determine local player
 */
static void
InitBotData ()
{
  for (bot.ps = player_stat, bot.cnt = 0; bot.ps < player_stat + clientGame.players.num; bot.ps ++, bot.cnt++) {
    if (bot.ps->local) {
      return;
    }
  }
  bot.cnt = -1;
} /* InitBotData */

/*
 * get action for server bot, if necessary
 */
static void
PlayBot(int *away)
{
  /* check if we have a local player at all */
  if (bot.cnt < 0) {
    return;
  }
  /* clear away flag and deactivate bot, if player pressed a key */
  if (! clientAction[bot.cnt].away) {
    *away = 0;
    bot.ps->bot=XBFalse;
  }
  /* determine bot action if requested */
  if (clientGame.setup.bot || bot.ps->bot == XBTrue) {
    gestionBot (player_stat,clientAction, bot.cnt, clientGame.players.num);
  }
} /* PlayBot */

/*
 * wait for an event from server
 */
static XBBool
WaitForServerEvent (XBNetworkEvent waitEvent, XBBool needFlush)
{
  XBEventData    eData;
  XBNetworkEvent netEvent;
  unsigned       id;
  XBPlayerHost   host;
  int            i;

  /* set timer */
  GUI_SetTimer (FRAME_TIME, XBTrue);
  /* no keys/mouse */
  GUI_SetKeyboardMode (KB_NONE);
  GUI_SetMouseMode (XBFalse);
  /* get events */
  while (1) {
    /* update window */
    GameUpdateWindow ();
    /* wait till tmer triggers */
    if (XBE_TIMER == GUI_WaitEvent (&eData) ) {
      /* try to flush udp connections */
      if (needFlush) {
	needFlush = Client_FlushPlayerAction ();
      }
      /* check for incoming network events */
      while (XBNW_None != (netEvent = Network_GetEvent (&id) ) ) {
	if (netEvent == waitEvent) {
	  return XBTrue;
	} else {
	  switch (netEvent) {
	  case XBNW_Disconnected:
	    if (id == 0 && waitEvent != XBNW_None) {
	      sprintf(exitMsg, "server disconnected");
	      Dbg_Game("exit - %s\n", exitMsg);
	      return XBFalse;
	    } else {
	      Dbg_Game("client #%u disconnected\n", id);
	      host = XBPH_Client1 + id - 1;
	      for (i = 0; i < clientGame.players.num; i ++) {
		if (clientGame.players.host[i] == host) {
		  playerLinked[i] = XBFalse;
		  Dbg_Game ("unlink player %d\n", i);
		}
	      }
	    }
	    break;
	  case XBNW_Error:
	    sprintf(exitMsg, "network error to server");
	    Dbg_Game("exit - %s\n", exitMsg);
	    return XBFalse;
	  default:
	    break;
	  }
	}
      }
      /* network queue is now emptied, check if that was waited for */
      if (waitEvent == XBNW_None) {
	return XBTrue;
      }
    }
  }
  return XBFalse;
} /* WaitForServerEvent */

/*
 * send sync signal to server and wait for acknowledgement
 */
static XBBool
SyncWithServer (XBNetworkEvent syncEvent, XBBool needFlush, XBBool showMsg)
{
  Dbg_Game("waiting for others \n");
  if (showMsg) {
    SetMessage ("Waiting for others ...", XBTrue);
  }
  Client_SendSync (syncEvent);
  if (!WaitForServerEvent (syncEvent, XBFalse)) {
    return XBFalse;
  }
  return XBTrue;
} /* SyncWithServer */

/*
 * wait for level data from server
 */
static const DBRoot *
LevelFromServer (void)
{
  int cnt = 0;
  do {
    Dbg_Game("level negotiations running\n");
    if (! WaitForServerEvent (XBNW_LevelConfig, XBFalse) ) {
      Dbg_Game("%i level proposals processed\n", cnt/2);
      return NULL;
    }
    cnt++;
    if (!Client_FixedLevel()) {
      Dbg_Game("received level data from server\n");
      if (Client_RejectsLevel()) {
	Dbg_Game("rejecting level, resetting\n");
	ClearRemoteLevelConfig();
      } else {
	Dbg_Game("accepting level proposal from server\n");
      }
      Client_SendLevelCheck();
    }
  } while (!Client_FixedLevel());
  Dbg_Game("received level activation by server\n");
  if (Client_RejectsLevel()) {
    sprintf(exitMsg, "rejected activated level, protocol error!");
    Dbg_Game("exit - %s\n", exitMsg);
    return NULL;
  }
  Dbg_Game("level negotiations successful\n");
  return GetRemoteLevelConfig ();
} /* LevelFromServer */

/*
 * run a level as client
 */
static int
ClientRunLevel (int numActive, const DBRoot *level)
{
  int          gameTime;
  int          pauseStatus;
  int          lastTeam, winner;
  const char  *msg;
  int  away=1;
  XBEventData  data;
  /* sanity check */
  assert (NULL != level);
  /* necesary inits */
  winner      = -1;
  gameTime    = 0;
  pauseStatus = -1;
  lastTeam    = -1;
  /* start demo recording if requested */
  if (clientGame.setup.recordDemo) {
    DemoInitLevel (DB_Atom (level));
  }
  /* Config level */
  if (!ConfigLevel (level)) {
    Dbg_Game("level config failed!\n");
  }
  /* clear async flag */
  Client_SetLevelAsync(XBFalse);
  /* clear actions */
  Client_ResetPlayerAction ();
  Client_ClearPlayerAction ();
  InitBotData();
  Dbg_Game("cleared client game data\n");
  /* show level info */
  if (! LevelIntro (clientGame.players.num, level, bot.ps->bot ? -1 : clientGame.setup.infoTime)) {
    Dbg_Game("game aborted in level intro\n");
    goto Exit;
  }
  /* sync with server */
  Dbg_Game("syncing level-intro with server\n");
  if (!SyncWithServer (XBNW_SyncLevelIntro, XBFalse, XBTrue) ) {
    Dbg_Game("sync failed, disconnecting\n");
    goto Exit;
  }
  Dbg_Game("received level-intro sync\n");
  /* update active players/teams */
  UpdatePlayerLink();
  GetActivePlayers(pa,&numActive,&teamActive);
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
    /* clear input data */
    ClearPlayerAction (clientAction);
    /* get events */
    if (! GameEventLoop (XBE_SERVER, &data) ) {
      Dbg_Game("game aborted in game event loop\n");
      goto Exit;
    }
    /* handle vents */
    switch ((XBServerEvent) data.value) {
    case XBSE_FINISH:
      Dbg_Game("server sent level finish\n");
      goto Finish;
    case XBSE_ERROR:
      sprintf(exitMsg, "server error received during level");
      Dbg_Game("exit - %s\n", exitMsg);
      goto Exit;
    default:
      /* show next frame */
      if (gameTime+1 < data.value) {
	Dbg_Game ("received %u, expected %u\n", data.value, gameTime+1);
	/* requeue event, to use later */
	GUI_SendEventValue (XBE_SERVER, gameTime+1);
      } else if (gameTime+1 > data.value) {
	Dbg_Game("ignoring data for past frame %u, current %u\n", data.value, gameTime+1);
      }
      break;
    }
    /* increment game clock */
    gameTime ++;
    /* bot */
    PlayBot(&away);
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
    (void) GameEvalAction (clientGame.players.num, clientAction);
    /* update window */
    GameUpdateWindow ();
  } while (1);

 Finish:
  Dbg_Game("level ended\n");
  /* stop music if necessary */
  if (clientGame.setup.Music){
    SND_Stop(clientGame.setup.Music);
  }
  /* activate bot if client was idle */
  if (away==1) {
    bot.ps->bot=XBTrue;
  }
  /* send finish message */
  Dbg_Game("sending level finish\n");
  Client_FinishPlayerAction (gameTime + 1);
  /* determine last team, don't store */
  LevelResult (gameTime, &lastTeam, clientGame.players.num, level, XBFalse);
  /* finish demo file if needed */
  if (clientGame.setup.recordDemo) {
    DemoFinishLevel (gameTime,winner);
  }
  /* send winner team */
  Dbg_Game("sending winner team %u\n", lastTeam);
  Client_SendWinner(lastTeam);
  /* wait for server to send response */
  Dbg_Game("waiting for server level-result sync\n");
  if (!WaitForServerEvent (XBNW_SyncLevelResult, XBFalse)) {
    Dbg_Game("sync failed\n");
    goto Exit;
  }
  Dbg_Game("level-result sync received\n");
  /* check if async was responded */
  if (Client_LevelAsynced()) {
    Dbg_Game("level async received, ignoring result\n");
    GUI_ErrorMessage("Async level, making it a draw\n");
    lastTeam=MAX_PLAYER;
  } else {
    Dbg_Game("level was sync\n");
    /* now store result */
    msg = LevelResult (gameTime, &lastTeam, clientGame.players.num, level, XBTrue);
    /* show message and winner Animation */
    if (! LevelEnd (clientGame.players.num, lastTeam, msg, bot.ps->bot ? -1 : 1) ) {
      Dbg_Game("aborted game during winner animation\n");
      lastTeam = -1;
    }
  }
  /* clean up */
 Exit:
  /* stop music if necessary */
  if (clientGame.setup.Music){
    SND_Stop(clientGame.setup.Music);
  }
  DeleteAllExplosions ();
  FinishLevel ();
  /* fade out image */
  DoFade (XBFM_BLACK_OUT, PIXH+1);
  /* that's all */
  return lastTeam;
} /* ClientRunLevel */

/*
 * run game as client
 */
void
RunClientGame (XBPlayerHost hostType)
{
  const DBRoot  *level;
  int lastTeam;
  int i, maxNumWins;
  /* clear exit message */
  memset(exitMsg, 0, sizeof(exitMsg));
  /* get game configs */
  if (! RetrieveGame (CT_Remote, atomArrayHost0[0], &clientGame) ) {
    Dbg_Game("failed to get game config!\n");
    sprintf(exitMsg, "game config error");
    Dbg_Game("exit - %s\n", exitMsg);
    goto Disconnect;
  }
  /* common inits */
  if (! InitGame (hostType, CT_Remote, &clientGame, clientAction)) {
    sprintf(exitMsg, "game init error");
    Dbg_Game("exit - %s\n", exitMsg);
    goto Disconnect;
  }
  /* init local data */
  maxNumWins = 0;
  numActive  = 0;
  teamActive = 0;
  /* init player link flags */
  InitPlayerLink ();
  /* clear level data for next run */
  ClearRemoteLevelConfig ();
  Client_LevelFix(XBFalse);
  Client_LevelRejection(XBTrue);
  /* beep once if requested */
  if (clientGame.setup.beep) {
    fprintf(stderr,"\a");
  }
  /* sync with server */
  Dbg_Game("waiting for server end-of-init sync\n");
  if (!SyncWithServer (XBNW_SyncEndOfInit, XBFalse, XBFalse) ) {
    Dbg_Game("sync failed\n");
    goto Exit;
  }
  Dbg_Game("end-of-init sync received\n");
  /* update active players/teams */
  UpdatePlayerLink ();
  GetActivePlayers(pa,&numActive,&teamActive);
  /* start the game */
  do {
    /* load and run next level */
    if (NULL == (level = LevelFromServer () ) ) {
      Dbg_Game("level negotiations failed, exiting\n");
      goto Exit;
    }
    /* update active players/teams */
    UpdatePlayerLink ();
    GetActivePlayers(pa,&numActive,&teamActive);
    /* run the level */
    Dbg_Game ("starting level\n");
    lastTeam = ClientRunLevel (teamActive, level);
    /* check for quick exit */
    if (-1 == lastTeam) {
      Dbg_Game("game aborted during level\n");
      goto Exit;
    }
    /* update current winner */
    for (i = 0; i < clientGame.players.num; i ++) {
      if (player_stat[i].victories > maxNumWins) {
	maxNumWins = player_stat[i].victories;
      }
    }
    /* sync before showing score board */
    Dbg_Game("waiting for server sync level-end\n");
    if (!SyncWithServer (XBNW_SyncLevelEnd, XBFalse, XBTrue) ) {
      Dbg_Game("sync failed!\n");
      goto Exit;
    }
    Dbg_Game("level-end sync received\n");
    /* show scores */
    if (! ShowScoreBoard (lastTeam, maxNumWins, clientGame.players.num, player_stat, bot.ps->bot ? -1 : 1)) {
      Dbg_Game("game aborted during scoreboard\n");
      goto Exit;
    }
    /* clear level data for next run, must be before scoreboard sync */
    ClearRemoteLevelConfig ();
    Client_LevelFix(XBFalse);
    Client_LevelRejection(XBTrue);
    Dbg_Game("reset level data, now waiting for server sync scoreboard\n");
    if (!SyncWithServer (XBNW_SyncScoreboard, XBFalse, XBTrue) ) {
      Dbg_Game("sync failed!\n");
      goto Exit;
    }
    /* determine active players/teams */
    UpdatePlayerLink();
    GetActivePlayers(pa,&numActive,&teamActive);
  } while (numActive > 1 && teamActive > 1 &&
	   maxNumWins < clientGame.setup.numWins);
  Dbg_Game("game ended\n");
  SetMessage ("Waiting for server", XBTrue);
  /* show winner */
  InitWinner (clientGame.players.num);
  ShowWinner (lastTeam, clientGame.players.num, player_stat);
  /* wait till network queue is cleared */
  Dbg_Game("waiting for server to shutdown!\n");
  (void) WaitForServerEvent (XBNW_None, XBFalse);
 Exit:
  FinishGame (&clientGame);
 Disconnect:
  Client_Disconnect ();
  if (strlen(exitMsg)>0) {
    GUI_ErrorMessage(exitMsg);
  }
  return;
} /* RunClientGame */

/*
 * end of file game_client.c
 */
