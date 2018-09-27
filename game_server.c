/*
 * file game_server.c - run game as server
 *
 * $Id: game_server.c,v 1.29 2005/01/18 15:36:15 lodott Exp $
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
#include "game_server.h"
#include <time.h>
#include "random.h"
#include "atom.h"
#include "cfg_level.h"
#include "bomb.h"
#include "demo.h"
#include "game.h"
#include "geom.h"
#include "intro.h"
#include "level.h"
#include "server.h"
#include "status.h"
#include "bot.h"
#include "snd.h"
#include "user.h"

#ifndef MAX_REJECTS
#define MAX_REJECTS 50
#endif

/*
 * local variables
 */
typedef struct {
  BMPlayer *ps;   /* player stat pointer */
  int cnt;        /* player stat index */
} BotData;

static CFGGame      serverGame;
static PlayerAction serverAction[MAX_PLAYER];
static XBBool       playerLinked[MAX_PLAYER];
static int          pa[MAX_PLAYER];
static int          numActive;
static int          teamActive;
static BotData      bot;

/*
 * mark all external hosts
 */
static void
InitPlayerLink (void)
{
  int i;
  assert (serverGame.players.num <= MAX_PLAYER);
  for (i = 0; i < serverGame.players.num; i ++) {
    playerLinked[i] = (serverGame.players.host[i] != XBPH_Server &&
		       serverGame.players.host[i] != XBPH_Local  &&
		       serverGame.players.host[i] != XBPH_Demo);
  }
  for (; i < MAX_PLAYER; i ++) {
    playerLinked[i] = XBFalse;
  }
} /* InitPlayerLink */

/*
 * set unlinked active external players to inactive
 */
static XBBool
UpdatePlayerLink (void)
{
  int    i;
  XBBool result = XBFalse;
  for (i = 0; i < serverGame.players.num; i ++) {
    if (serverGame.players.host[i] != XBPH_Server &&
	serverGame.players.host[i] != XBPH_Local  &&
	! playerLinked[i] &&
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
  for (i = 0; i < serverGame.players.num; i ++) {
    pa[i] = !player_stat[i].in_active;
    if (! player_stat[i].in_active) {
      *pl = *pl +1;
      if (serverGame.setup.teamMode) {
	if (! ( reinco & (1 << player_stat[i].team) ) ) {
	  reinco|=1 << player_stat[i].team;
	  *tm = *tm +1;;
	}
      } else {
	*tm = *tm +1;;
      }
    }
  }
  Dbg_Game("%i active player, %i active teams (%s mode)\n", *pl, *tm, serverGame.setup.teamMode ? "team" : "chaos");
} /* GetActivePlayers */

/*
 * update the central game with current result
 */
static void
UpdateCentralGame()
{
  int i;
  static char res[20];
  memset(res,0,sizeof(res));
  for (i = 0; i < serverGame.players.num; i ++) {
    sprintf(&res[i],"%i",player_stat[i].victories);
  }
  res[i] = '-';
  Dbg_Game("updating central game entry, current result %s\n", res);
  Server_RestartNewGame(0,res);
} /* UpdateCentralGame */

/*
 * determine local player
 */
static void
InitBotData ()
{
  for (bot.ps = player_stat, bot.cnt = 0; bot.ps < player_stat + serverGame.players.num; bot.ps ++, bot.cnt++) {
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
  if (! serverAction[bot.cnt].away) {
    *away = 0;
    bot.ps->bot=XBFalse;
  }
  /* determine bot action if requested */
  if (serverGame.setup.bot || bot.ps->bot == XBTrue) {
    gestionBot (player_stat,serverAction, bot.cnt, serverGame.players.num);
  }
} /* PlayBot */

/*
 * check connections
 */
static void
CheckConnections()
{
  int pl, id;
  for (pl = 0; pl < serverGame.players.num; pl ++) {
    id = serverGame.players.host[pl] + 1 - XBPH_Client1;
    if (serverGame.players.host[pl] != XBPH_Server &&
	serverGame.players.host[pl] != XBPH_Local  &&
	playerLinked[pl] &&
	! player_stat[pl].in_active &&
	Network_GetHostState(id) == XBHS_None) {
      /* override player action as suicide */
      playerLinked[pl] = XBFalse;
      serverAction[pl].suicide = XBTrue;
      Dbg_Game("player %u at host %u disconnected, suiciding\n",pl,id);
    }
  }
} /* CheckConnections */

/*
 * output flag array
 */
static char *
ShowPlayerFlags(XBBool *arr)
{
  static char tmp[MAX_PLAYER+1];
  int i;
  memset(tmp,0,sizeof(tmp));
  for (i=0; i<serverGame.players.num; i++) {
    tmp[i] = arr[i] ? 'x' : '-';
  }
  return tmp;
} /* ShowPlayerFlags */

/*
 * server waits until all clients sent given event
 */
static XBBool
WaitForClientEvent (XBNetworkEvent waitEvent, XBBool needFlush)
{
  int            i;
  long           num;
  unsigned       id;
  XBEventData    eData;
  XBNetworkEvent netEvent;
  XBBool         playerWait[MAX_PLAYER];

  /* determine for which players we have to wait */
  memcpy (playerWait, playerLinked, sizeof (playerWait));
  Dbg_Game("linked players  |%s|\n",ShowPlayerFlags(playerLinked));
  /* set timer, disable keys/mouse */
  GUI_SetTimer (FRAME_TIME, XBTrue);
  GUI_SetKeyboardMode (KB_NONE);
  GUI_SetMouseMode (XBFalse);
  /* loop until all clients have reported back */
  do {
    /* determine how many player are not ready */
    num = 0;
    for (i = 0; i < serverGame.players.num; i ++) {
      if (playerWait[i]) {
	num ++;
      }
    }
    Dbg_Game("waiting for players |%s|, remaining = %u\n",ShowPlayerFlags(playerWait),num);
    /* update window */
    GameUpdateWindow ();
    /* check for network events when timer triggers */
    if (XBE_TIMER == GUI_WaitEvent (&eData) ) {
      /* try to flush udp connections, if requested */
      if (needFlush) {
	needFlush = Server_FlushPlayerAction ();
      }
      /* check for network events */
      netEvent = Network_GetEvent (&id);
      if (netEvent == waitEvent ||
	  netEvent == XBNW_Error ||
	  netEvent == XBNW_Disconnected) {
	/* host returned sync or disconnected */
	XBPlayerHost host = XBPH_Client1 + id - 1;
	/* loop through all players on that host */
	for (i = 0; i < serverGame.players.num; i ++) {
	  if (serverGame.players.host[i] == host) {
	    /* mark as having responded */
	    playerWait[i] = XBFalse;
	    /* unlink if disconnected */
	    switch (netEvent) {
	    case XBNW_Error:
	      Dbg_Game("unlinked player %i (host %i), network error\n",i,id);
	      playerLinked[i] = XBFalse;
	      break;
	    case XBNW_Disconnected: break;
	      Dbg_Game("unlinked player %i, (host %i), disconnected\n",i,id);
	      playerLinked[i] = XBFalse;
	      break;
	    default: break;
	    }
	  }
	}
      }
      /* TODO: limit timer events and disconnect all non-responding hosts */
    }
  } while (num > 0);
  return XBTrue;
} /* WaitForClientEvent */

/*
 * server waits for specific event from all clients and acknowledges
 */
static XBBool
SyncWithClients (XBNetworkEvent syncEvent, XBBool needFlush, XBBool showMsg)
{
  if (showMsg) {
    SetMessage ("Waiting for others ...", XBTrue);
  }
  WaitForClientEvent (syncEvent, needFlush);
  /* acknowledge receiving all syncs and go on */
  Server_SendSync(syncEvent);
  return XBTrue;
} /* SyncWithClients */

/*
 * insert keys from clients
 */
static void
InsertClientAction (const CFGGamePlayers *cfgPlayers, PlayerAction *serverAction)
{
  int i;
  assert (NULL != cfgPlayers);
  assert (NULL != serverAction);
  for (i = 0; i < cfgPlayers->num; i ++) {
    switch (cfgPlayers->host[i]) {
    case XBPH_Client1: Server_GetPlayerAction (1, i, serverAction + i); break;
    case XBPH_Client2: Server_GetPlayerAction (2, i, serverAction + i); break;
    case XBPH_Client3: Server_GetPlayerAction (3, i, serverAction + i); break;
    case XBPH_Client4: Server_GetPlayerAction (4, i, serverAction + i); break;
    case XBPH_Client5: Server_GetPlayerAction (5, i, serverAction + i); break;
#ifdef SMPF
    case XBPH_Client6: Server_GetPlayerAction (6, i, serverAction + i); break;
    case XBPH_Client7: Server_GetPlayerAction (7, i, serverAction + i); break;
    case XBPH_Client8: Server_GetPlayerAction (8, i, serverAction + i); break;
    case XBPH_Client9: Server_GetPlayerAction (9, i, serverAction + i); break;
    case XBPH_Client10: Server_GetPlayerAction (10, i, serverAction + i); break;
    case XBPH_Client11: Server_GetPlayerAction (11, i, serverAction + i); break;
    case XBPH_Client12: Server_GetPlayerAction (12, i, serverAction + i); break;
    case XBPH_Client13: Server_GetPlayerAction (13, i, serverAction + i); break;
    case XBPH_Client14: Server_GetPlayerAction (14, i, serverAction + i); break;
    case XBPH_Client15: Server_GetPlayerAction (15, i, serverAction + i); break;
#endif
    default:           break;
    }
  }
  Server_ClearPlayerAction ();
} /* InsertClientAction */

/*
 * run a level
 */
static int
ServerRunLevel (int numActive, const DBRoot *level)
{
  int	       gameTime;
  int	       pauseStatus;
  int	       lastTeam, counter,winner;
  int	       frameTime;
  BMPlayer     *ps;
  XBBool       async;
  const char   *msg;
  int          away = 1;
  XBEventData  eData;

  /* sanity check */
  assert (level != NULL);
  /* necessary inits */
  winner      = -1;
  gameTime    = 0;
  pauseStatus = -1;
  lastTeam    = -1;
  frameTime   = serverGame.setup.frameRate ? 1000/serverGame.setup.frameRate : 0;
  /* start demo recording if requested */
  if (serverGame.setup.recordDemo) {
    DemoInitLevel (DB_Atom (level));
  }
  /* Config level */
  if (!ConfigLevel(level)) {
    Dbg_Game("level config failed!\n");
    goto Exit;
  }
  /* prepare async check at end of level*/
  Server_ClearLevelWinners();
  /* init bot data */
  InitBotData();
  /* clean up player actions */
  Server_ClearPlayerAction ();
  Server_ResetPlayerAction ();
  /* level intro */
  if (! LevelIntro (serverGame.players.num, level, bot.ps->bot ? -1 : serverGame.setup.infoTime)) {
    Dbg_Game("abort in level intro\n");
    goto Exit;
  }
  /* wait for clients to show level info */
  Dbg_Game("waiting for clients to show level intro\n");
  SyncWithClients (XBNW_SyncLevelIntro, XBFalse, XBTrue);
  Dbg_Game("clients show level intro\n");
  /* determine active players/teams */
  UpdatePlayerLink();
  GetActivePlayers(pa,&numActive,&teamActive);
  if (teamActive <= 1) {
    GUI_ErrorMessage ("Only one team left after level-intro sync!");
    goto Exit;
  }
  /* show level map */
  LevelBegin (GetLevelName (level));
  /* set timer for frames */
  GUI_SetTimer (frameTime, XBTrue);
  /* process key events */
  GUI_SetKeyboardMode (KB_XBLAST);
  GUI_SetMouseMode (XBFalse);
  /* play music, if requested */
  if (serverGame.setup.Music){
    SND_Load (serverGame.setup.Music);
    SND_Play (serverGame.setup.Music, SOUND_MIDDLE_POSITION);
  }
  Dbg_Game("starting level!\n");
  /* update central entry */
  UpdateCentralGame();
  /* now start level */
  do {
    /* ready input */
    ClearPlayerAction (serverAction);
    /* handle all event until timer triggers */
    if (! GameEventLoop (XBE_TIMER, &eData) ) {
      Dbg_Game("game aborted during level\n");
      goto Exit;
    }
    /* increment game clock */
    gameTime ++;
    /* update game entry occasionally */
    if ( (gameTime % 1024) == 0) {
      UpdateCentralGame();
    }
    /* server bot */
    PlayBot(&away);
    /* handle game turn */
    GameTurn (gameTime, serverGame.players.num, &numActive);
    /* insert any data received from clients */
    InsertClientAction (&serverGame.players, serverAction);
    /* trigger suicides for disconnected clients */
    CheckConnections();
    /* send all data on player actions to clients */
    Server_SendPlayerAction (gameTime, serverAction);
    /* record demo data if requested */
    if (serverGame.setup.recordDemo) {
      DemoRecordFrame (gameTime, serverAction);
    }
    /* evaluate player action */
    (void) GameEvalAction (serverGame.players.num, serverAction);
    /* update window */
    GameUpdateWindow ();
  } while (gameTime < GAME_TIME &&
	   numActive > 0 &&
	   (numActive > 1 || NumberOfExplosions () != 0) );
  /* tell client game is over */
  Server_FinishPlayerAction (gameTime + 1);
  /* enable bot if server appears to be away */
  if (away == 1) {
    bot.ps->bot=XBTrue;
  }
  /* calc last team for async check, do not store yet */
  LevelResult (gameTime, &lastTeam, serverGame.players.num, level, XBFalse);
  /* count number of players in winner team */
  if (lastTeam <= MAX_PLAYER) {
    for (ps = player_stat,counter=1; ps < player_stat +serverGame.players.num; ps ++,counter++) {
      if (ps->team == lastTeam) {
	winner=counter;
      }
    }
  }
  /* finish demo file if requested */
  if (serverGame.setup.recordDemo) {
    DemoFinishLevel (gameTime,winner);
  }
  Dbg_Game("waiting for clients to send winner\n");
  Server_SetLevelWinners(0,lastTeam);
  WaitForClientEvent(XBNW_SyncLevelResult, XBTrue);
  /* determine active players/teams */
  UpdatePlayerLink();
  GetActivePlayers(pa,&numActive,&teamActive);
  if (teamActive <= 1) {
    GUI_ErrorMessage ("Only one team left after async check!");
    goto Exit;
  }
  async = Server_LevelAsync();
  if (async) {
    Dbg_Game("async result determined, informing clients!\n");
    Server_SendLevelAsync();
    GUI_ErrorMessage("Async level, making it a draw\n");
    lastTeam=MAX_PLAYER;
  } else {
    Dbg_Game("results sync, informing clients!\n");
    Server_SendLevelSync();
    /* now store the level result */
    msg = LevelResult (gameTime, &lastTeam, serverGame.players.num, level, XBTrue);
    if (! LevelEnd (serverGame.players.num, lastTeam, msg, bot.ps->bot ? -1 : 1)) {
      lastTeam = -1;
    }
  }
 Exit:
  /* stop music if necessary */
  if (serverGame.setup.Music) {
    SND_Stop(serverGame.setup.Music);
  }
  FinishLevel ();
  DeleteAllExplosions ();
  /* fade out image */
  DoFade (XBFM_BLACK_OUT, PIXH+1);
  /* that's all */
  return lastTeam;
} /* ServerRunLevel */

/*
 * send level data to clients
 */
static XBBool
SendLevelToClients (const DBRoot **level)
{
  int okay = MAX_REJECTS;
  /* send level data to clients */
  while (okay>0) {
    *level    = LoadLevelFile (GetNextLevel ());
    Dbg_Game ("Proposed level is: %s\n", GetLevelName(*level));
    Server_SendLevel (*level);
    Server_ClearLevelStatus();
    Server_SetLevelStatus(0,XBTrue);
    WaitForClientEvent (XBNW_LevelConfig, XBTrue);
    if (Server_LevelApproved()) {
      Dbg_Game ("Level accepted by all clients\n");
      /* now set and send fresh random seed so that it arrives before the activate */
      SeedRandom(time(NULL));
      Server_SendRandomSeed();
      Server_SendLevelActivate();
      Dbg_Game("negotiations finished, proceeding\n");
      return(0);
    } else {
      okay--;
      Dbg_Game ("Level rejected (%i attempt(s) remaining)\n",okay);
      Server_SendLevelReset();
    }
  }
  Dbg_Game("negotiations failed!\n");
  return(-1);
} /* SendLevelToClients */

/*
 * run the game as server
 */
void
RunServerGame (void)
{
  const DBRoot  *level;
  int lastTeam, winner, maxNumWins;
  int i;
  XBBool         centralConnect=XBFalse;
  CFGCentralSetup central;

  /* get setup */
  if (! RetrieveGame (CT_Remote, atomArrayHost0[0], &serverGame) ) {
    Dbg_Game("failed to get game setup!\n");
    goto Disconnect;
  }
  /* select levels to play */
  if (! InitLevels (&serverGame) ) {
    Dbg_Game("failed to initialize levels!\n");
    goto Disconnect;
  }
  /* common inits */
  if (! InitGame (XBPH_Server, CT_Remote, &serverGame, serverAction)) {
    Dbg_Game("failed to initialize game!\n");
    goto Disconnect;
  }
  /* local data */
  maxNumWins = 0;
  winner = -1;
  numActive  = 0;
  teamActive = 0;
  Dbg_Game("server game initialized\n");
  /* mark external hosts */
  InitPlayerLink ();
  /* wait for clients to initialize game */
  Dbg_Game("waiting for clients to init game\n");
  SyncWithClients (XBNW_SyncEndOfInit, XBFalse, XBFalse);
  Dbg_Game("clients have initialized game\n");
  /* determine active players/teams */
  UpdatePlayerLink();
  GetActivePlayers(pa,&numActive,&teamActive);
  if (teamActive <= 1) {
    GUI_ErrorMessage ("Only one team left after game init!");
    return;
  }
  /* Connect to central */
  if (serverGame.setup.rated) {
    SetMessage("Connecting to central...", XBTrue);
    Dbg_Game("rated game requested\n");
    RetrieveCentralSetup (&central);
    if (User_Connect(&central)) {
      Dbg_Game("Connection to central established\n");
      centralConnect=XBTrue;
    } else {
      centralConnect=XBFalse;
      Dbg_Game("failed to establish connection to central, unrated game\n");
    }
  } else {
    Dbg_Game("unrated game requested\n");
    centralConnect=XBFalse;
  }
  /* play levels */
  do {
    /* negotiate next level */
    if (SendLevelToClients(&level) == -1) {
      GUI_ErrorMessage("Level negotiations failed!");
      goto Exit;
    }
    /* update active players/teams */
    UpdatePlayerLink();
    GetActivePlayers(pa,&numActive,&teamActive);
    if (teamActive <= 1) {
      GUI_ErrorMessage ("Only one team left after level negotiation!");
      goto Exit;
    }
    /* play level */
    lastTeam = ServerRunLevel (teamActive, level);
    /* check for quick exit */
    if (-1 == lastTeam) {
      Dbg_Game("server aborted game\n");
      goto Exit;
    }
    /* update current winner */
    Dbg_Game("team #%i won the level\n", lastTeam);
    for (i = 0; i < serverGame.players.num; i ++) {
      if (player_stat[i].victories > maxNumWins) {
	maxNumWins = player_stat[i].victories;
	winner = i;
      }
    }
    /* wait for clients to reach level end */
    Dbg_Game("waiting for clients to show level end\n");
    SyncWithClients (XBNW_SyncLevelEnd, XBFalse, XBTrue);
    Dbg_Game("waiting for clients to show level end\n");
    /* update active players/teams */
    UpdatePlayerLink();
    GetActivePlayers(pa,&numActive,&teamActive);
    if (teamActive <= 1) {
      GUI_ErrorMessage ("Only one team left after level-end sync!");
      goto Exit;
    }
    /* send level stats to central if rated game */
    if (centralConnect) {
      if (User_Connected()) {
	Dbg_Game ("sending level results to central\n");
	User_SendGameStat(serverGame.players.num, player_stat, pa);
      } else {
	Dbg_Game("connection to central has broken down, no more ratings will be sent\n");
	centralConnect = XBFalse;
      }
    }
    /* show scores */
    if (! ShowScoreBoard (lastTeam, maxNumWins, serverGame.players.num, player_stat, bot.ps->bot ? -1 : 1)) {
      Dbg_Game("game exit during Scoreboard\n");
      goto Exit;
    }
    /* wait for clients to show scoreboard */
    Dbg_Game("waiting for clients to show score\n");
    SyncWithClients (XBNW_SyncScoreboard, XBFalse, XBTrue);
    Dbg_Game("clients show score\n");
    /* determine number of active players */
    UpdatePlayerLink ();
    GetActivePlayers(pa, &numActive, &teamActive);
  } while (numActive > 1 && teamActive > 1 && maxNumWins < serverGame.setup.numWins);
  /* and the winner is ... */
  if (maxNumWins >= serverGame.setup.numWins) {
    Dbg_Game("team #%i won the game!\n", winner);
    if (centralConnect) {
      Dbg_Game ("Sending final game result to central\n");
      for (i = 0; i < serverGame.players.num; i ++) {
	pa[i] = 1;
	if (player_stat[i].victories == serverGame.setup.numWins) {
	  player_stat[i].lives=-player_stat[i].victories;
	} else {
	  player_stat[i].lives=player_stat[i].victories;
	}
      }
      User_SendGameStat(-serverGame.players.num, player_stat, pa);
    }
    /* determine and show winner */
    InitWinner (serverGame.players.num);
    ShowWinner (lastTeam, serverGame.players.num, player_stat);
  } else {
    Dbg_Game("game finished, too few players - current winner %i\n", winner);
    GUI_ErrorMessage ("Not enough players/teams left in the game");
  }
 Exit:
  FinishGame (&serverGame);
 Disconnect:
  Dbg_Game("disconnecting all clients\n");
  Server_SendDisconnectAll ();
  if (centralConnect) {
    Dbg_Game("disconnecting result link to central\n");
    User_SendDisconnect();
    User_Disconnect();
    centralConnect=XBFalse;
  }
  Dbg_Game("closing game entry in central\n");
  Server_CloseNewGame();
  return;
} /* StartServerGame */

/*
 * end of file game_server.c
 */
