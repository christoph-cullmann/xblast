/*
 * file game.c - play the game
 *
 * $Id: game.c,v 1.28 2005/01/18 15:33:04 lodott Exp $
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
#include "game.h"

#include "atom.h"
#include "bomb.h"
#include "cfg_level.h"
#include "cfg_stat.h"
#include "client.h"
#include "demo.h"
#include "event.h"
#include "func.h"
#include "geom.h"
#include "intro.h"
#include "level.h"
#include "map.h"
#include "network.h"
#include "player.h"
#include "scramble.h"
#include "server.h"
#include "shrink.h"
#include "status.h"
#include "util.h"

#include "gui.h"
#include "snd.h"
#include "bot.h"

#include "random.h"
#ifdef DEBUG_RANDOM
#endif

#define TEAM_NOWINNER 255
#define TEAM_LOCALASYNC 254

/*
 * local variables
 */

/* player input */
static PlayerAction *actionTable[NUM_XBE];
static XBBool        useStopKey[NUM_XBE];
static int           turnX[MAX_PLAYER];
static int           turnY[MAX_PLAYER];
static int           numOfMessages=MAX_PLAYER+2;
static char          chatNames[MAX_PLAYER+2][256]; /* publiic and team */
static int localPlayer=0;
static int           numOfPlayers;

/* sounds to play */
static SND_Id levelSounds[] = {
  SND_BAD,
  SND_DROP,
  SND_NEWBOMB,
  SND_NEWKICK,
  SND_NEWPUMP,
  SND_NEWRC,
  SND_MOREFIRE,
  SND_DEAD,
  SND_EXPL,
  SND_KICK,
  SND_PUMP,
  SND_OUCH,
  SND_BUTT,
  SND_SHOOT,
  SND_INVIS,
  SND_INVINC,
  SND_NEWTELE,
  SND_TELE,
  SND_INJ,
  SND_MINIBOMB,
  SND_HAUNT,
  SND_SPIRAL,
  SND_SPBOMB,
  SND_SLIDE,
  SND_STUN,
  SND_WARN,
  SND_COMPOUND,
  SND_TELE1,
  SND_TELE2,
  SND_HOLY,
  SND_ENCLOAK,
  SND_DECLOAK,
  SND_FAST,
  SND_SLOW,
  SND_SLAY,
  SND_LIFE,
  SND_NEWCLOAK,
  SND_BOMBMORPH,
  SND_APPL,
  SND_APPL2,
  SND_WON,
  SND_STEP1,
  SND_STEP2,
  SND_STEP3,
  SND_STEP4,
  SND_STEP5,
  SND_STEP6,
  SND_MAX
};

/*
 * init ingame sound
 */
static void
InitSounds (void)
{
  SND_Id *pSnd;

  for (pSnd = levelSounds; *pSnd != SND_MAX; pSnd ++) {
    SND_Load (*pSnd);
  }
} /* InitSounds */

/*
 * finish ingame sound
 */
static void
FinishSounds (void)
{
  SND_Id *pSnd;

  for (pSnd = levelSounds; *pSnd != SND_MAX; pSnd ++) {
    SND_Stop (*pSnd);
    SND_Unload (*pSnd);
  }
} /* FinishSounds */

/*
 * init player input (keybord and joystick)
 */
static void
InitPlayerInput (XBPlayerHost hostType, const CFGGamePlayers *gamePlayers, const CFGPlayer *cfgPlayer, PlayerAction *playerAction)
{
  int           i;
  assert (NULL != gamePlayers);
  assert (NULL != cfgPlayer);
  /* clear data */
  memset (actionTable, 0, sizeof (actionTable));
  memset (useStopKey, 0, sizeof (useStopKey));
  /* chat */
  /* for (i = 0; i < gamePlayers->num; i ++) { ;

  strcpy (name[i], GUI_AtomToString (gamePlayers->player[i]) );
  }*/
  /* set controls for each player */
  for (i = 0; i < gamePlayers->num; i ++) {
    sprintf(chatNames[i],"%s",p_string[i].name);
    assert (ATOM_INVALID != gamePlayers->player[i]);
    if (hostType == gamePlayers->host[i]){
	  localPlayer=i;
    }

    /* connect player to event type */
    switch (gamePlayers->control[i]) {
    case XBPC_RightKeyboard:
      if (hostType == gamePlayers->host[i]) {
	actionTable[XBE_KEYB_1] = &playerAction[i];
	useStopKey[XBE_KEYB_1]  = cfgPlayer[i].misc.useStopKey;
      }
      turnX[i] = cfgPlayer[i].misc.turnStepKeyboard * BASE_X;
      turnY[i] = cfgPlayer[i].misc.turnStepKeyboard * BASE_Y;
      break;

    case XBPC_LeftKeyboard:
      if (hostType == gamePlayers->host[i]) {
	actionTable[XBE_KEYB_2] = &playerAction[i];
	useStopKey[XBE_KEYB_2]  = cfgPlayer[i].misc.useStopKey;
      }
      turnX[i] = cfgPlayer[i].misc.turnStepKeyboard * BASE_X;
      turnY[i] = cfgPlayer[i].misc.turnStepKeyboard * BASE_Y;
      break;

    case XBPC_Joystick1:
      if (hostType == gamePlayers->host[i]) {
	actionTable[XBE_JOYST_1] = &playerAction[i];
	useStopKey[XBE_JOYST_1]  = XBTrue;
      }
      turnX[i] = cfgPlayer[i].misc.turnStepJoystick * BASE_X;
      turnY[i] = cfgPlayer[i].misc.turnStepJoystick * BASE_Y;
      break;

    case XBPC_Joystick2:
      if (hostType == gamePlayers->host[i]) {
	actionTable[XBE_JOYST_2] = &playerAction[i];
	useStopKey[XBE_JOYST_2]  = XBTrue;
      }
      turnX[i] = cfgPlayer[i].misc.turnStepJoystick * BASE_X;
      turnY[i] = cfgPlayer[i].misc.turnStepJoystick * BASE_Y;
      break;

    default:
      break;
    }
  }
  numOfMessages = gamePlayers->num;
  sprintf(chatNames[numOfMessages++],"Public");
  sprintf(chatNames[numOfMessages++],"Team");
} /* InitPlayerInput */

/*
 * clear player action data
 */
void
ClearPlayerAction (PlayerAction *playerAction)
{
  int player;

  for (player = 0; player < MAX_PLAYER; player ++) {
    playerAction[player].player  = player;
    playerAction[player].dir     = GoDefault;
    playerAction[player].bomb    = XBFalse;
    playerAction[player].special = XBFalse;
    playerAction[player].pause   = XBFalse;
    playerAction[player].suicide = XBFalse;
    playerAction[player].abort   = ABORT_NONE;
    playerAction[player].laola = XBFalse;
    playerAction[player].looser = XBFalse;
    playerAction[player].away = XBTrue;
  }
} /* ClearPlayerAction */

/*
 * update game display
 */
void
GameUpdateWindow (void)
{
  /* update sound */
  SND_Flush ();
  /* shuffle sprites and mark them */
  ShuffleAllSprites ();
  /* set rectangles to be redrawn */
  SetRedrawRectangles ();
  /* shuffle sprites and mark them */
  MarkAllSprites ();
  /* update maze pixmap */
  UpdateMaze ();
  /* draw sprites into pixmap */
  DrawAllSprites ();
  /* update window from pixmap */
  GUI_FlushPixmap (XBTrue);
  /* clear the redraw map */
  ClearRedrawMap();
} /* GameUpdateWindow */

/*
 * evaluate events for game until wait event
 */
static  XBBool
GameEvent (XBEventCode waitCode, XBEventData *data)
{
  XBEventCode code;
  BMPlayer         *ps;
  int counter=0;
  static unsigned int target=0, init=0;
  char temp[40];

  assert (data != NULL);
  /* wait for specific event */
  if (init == 0) {
    target = numOfPlayers;
    init = 1;
  }
  GUI_SetMouseMode (XBTrue);
  while (waitCode != (code = GUI_WaitEvent (data) ) ) {
    /* doesn't appear to be needed */
    if (code == XBE_XBLAST) {
        // simply discard this event as it only delivers the Escape key press
        continue;
    }

    /* a player has made an action */
    if (NULL != actionTable[code]) {
      /* loop through all local players */
      for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers && counter < localPlayer; ps ++,counter++) ;
      if (ps->chatmode != 1) {
	/* player not in chat mode */
	switch (data->value) {
	case XBGK_GO_UP:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->dir     = GoUp;
	  break;
	case XBGK_GO_DOWN:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->dir     = GoDown;
	  break;
	case XBGK_GO_LEFT:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->dir     = GoLeft;
	  break;
	case XBGK_GO_RIGHT:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->dir     = GoRight;
	  break;
	case XBGK_BOMB:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->bomb    = XBTrue;
	  break;
	case XBGK_SPECIAL:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->special = XBTrue;
	  break;
	case XBGK_PAUSE:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->pause   = XBTrue;
	  break;
	case XBGK_ABORT:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->abort   = ABORT_TRUE;
	  break;
	case XBGK_ABORT_CANCEL:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->abort   = ABORT_CANCEL;
	  break;
	case XBGK_LAOLA:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->laola = XBTrue;
	  for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers && counter < localPlayer; ps ++,counter++) ;
	  SetMsgLaola(counter);
	  break;
	case XBGK_LOOSER:
	  actionTable[code]->away=XBFalse;
	  actionTable[code]->looser = XBTrue;
	  SetMsgLoser(counter);
	  break;
	case XBGK_BOT:
	  if (ps->bot == XBTrue) {
	    SetMessage("Bot Deactivated",XBFalse);
	    ps->bot = XBFalse;
	  } else {
	    SetMessage("Bot Activated",XBFalse);
	    ps->bot = XBTrue;
	  }
	  break;
	case XBGK_STOP_ALL:
	  actionTable[code]->away=XBFalse;
	  if (useStopKey[code]) {
	    actionTable[code]->dir = GoStop;
	  }
	  break;
	case XBGK_STOP_UP:
	case XBGK_STOP_DOWN:
	case XBGK_STOP_LEFT:
	case XBGK_STOP_RIGHT:
	  actionTable[code]->away=XBFalse;
	  if (! useStopKey[code]) {
	    actionTable[code]->dir = GoStop;
	  }
	  break;
	case XBGK_CHAT_START:
	  ps->chatmode = 1;
	  ps->chatlen = 0;
	  memset(ps->chatstring,0,CHAT_LEN-1);
	  ps->chatstring[CHAT_LEN-1]=(char)0;
	  SetGet("Start Chatting",XBTrue);
	  if (ps->lives==0) {
	    //  actionTable[code]->chatmode=XBTrue;
	  }
	  break;
	case XBGK_CHAT_SEND:
	  if (ps->chatmode==1) {
	    Dbg_Out("Chat Send!(action) %s\n",ps->chatstring);
	    ps->chatmode=2;
	    Client_SendChat(counter,target%numOfMessages,0,target%numOfMessages, ps->chatstring);
	    Server_SendChat(counter,target%numOfMessages,0,target%numOfMessages, ps->chatstring);
	    SetChat(ps->chatstring,XBTrue);
	    memset(ps->chatstring,0,CHAT_LEN-1);
	    ps->chatstring[CHAT_LEN-1]=(char)0;
	    ps->chatmode=0;
	  }
	  break;
	case XBGK_CHAT_CANCEL:
	  ps->chatmode=3;
	  SetGet("Canceling Chat",XBTrue);
	  ps->chatmode=0;
	  // actionTable[code]->chatmode=XBFalse;
	  break;
	case XBGK_CHAT_CHANGE_RECEIVER:
	  target++;
	  Dbg_Out(temp,"Sending msg to %s  %i %i %i",chatNames[target%(numOfMessages)],target,numOfMessages,target%(numOfMessages));
	  sprintf(temp,"Sending msg to %s",chatNames[target%(numOfMessages)]);
	  SetGet(temp,XBTrue);
	  break;
	}
      } else {
	/* player in chat mode */
	switch (data->value) {
	case XBGK_CHAT_START:
	  ps->chatmode=1;
	  SetGet("Start Chatting",XBTrue);
	  break;
	case XBGK_CHAT_SEND:
	  if (ps->chatmode == 1) {
	    /* again this line! */
	    for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers&&counter<localPlayer; ps ++,counter++) ;
	    ps->chatmode=2;
	    sprintf(temp,"%s:%s",p_string[counter].name,ps->chatstring);
	    Dbg_Out("Chat Send!(else) %s %i\n",temp,counter);
	    Client_SendChat(counter,target%numOfMessages,0,target%numOfMessages, temp);
	    Server_SendChat(counter,target%numOfMessages,0,target%numOfMessages, temp);
	    SetChat(temp,XBTrue);
	    memset(ps->chatstring,0,CHAT_LEN-1);
	    ps->chatmode=0;
	  }
	  break;
	case XBGK_CHAT_CANCEL:
	  ps->chatmode=3;
	  SetGet("Canceling Chat",XBTrue);
	  ps->chatmode=0;
	  break;
	case XBGK_CHAT_CHANGE_RECEIVER:
	  target++;
	  Dbg_Out(temp,"Sendind msg to %s  %i %i %i",chatNames[target%(numOfMessages)],target,numOfMessages,target%(numOfMessages));
	  sprintf(temp,"Sendind msg to %s",chatNames[target%(numOfMessages)]);
	  SetGet(temp,XBTrue);
	  break;
	}
      }
    }
    /* a ctrl key has been pressed */
    if (code == XBE_CTRL){
      switch (data->value){
      case XBCK_ESCAPE:
	data->value=XBXK_EXIT;
	return XBTrue;
	break;
      case XBCK_BACKSPACE:
	for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers&&counter<localPlayer; ps ++,counter++) ;
	ps->chatstring[--ps->chatlen]=0;
	SetGet(ps->chatstring,XBTrue);
	break;
      case XBCK_RETURN:
	for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers&&counter<localPlayer; ps ++,counter++) ;
	if(ps->chatmode==1){
	  ps->chatmode=2;
	  sprintf(temp,"%s:%s",p_string[counter].name,ps->chatstring);
	  Dbg_Out("Chat Send! %s \n",temp);
	  Client_SendChat(counter,target%numOfMessages,0,target%numOfMessages,temp);
	  Server_SendChat(counter,target%numOfMessages,0,target%numOfMessages,temp);
	  SetChat(temp,XBTrue);
	  memset(ps->chatstring,0,CHAT_LEN-1);
	  ps->chatmode=0;
	}
	break;
      }
    }
    /* an ascii key has been pressed */
    if (code == XBE_ASCII) {
      for (ps = player_stat,counter=0; ps < player_stat + numOfPlayers&&counter<localPlayer; ps ++,counter++) ;
      if (ps->chatmode==1) {
	if (ps->chatlen < CHAT_LEN-strlen(p_string[counter].name+4)) {
	  ps->chatstring[ps->chatlen++]=data->value;
	  SetGet(ps->chatstring,XBTrue);
	} else {
	  ps->chatmode=2;
	  ps->chatstring[CHAT_LEN-1]=(char)0;
	  SetChat(ps->chatstring,XBTrue);
	  Client_SendChat(counter,target%numOfMessages,0,target%numOfMessages,temp);
	  Server_SendChat(counter,target%numOfMessages,0,target%numOfMessages,temp);
	  ps->chatlen=0;
	  memset(ps->chatstring,0,CHAT_LEN-1);
	  ps->chatstring[CHAT_LEN-1]=(char)0;
	  ps->chatmode=1;
	  ps->chatstring[ps->chatlen++]=data->value;
	}
      }
    }
  }
  return XBFalse;
} /* GameEvent */

/*
 * event loop for game
 */
XBBool
GameEventLoop (XBEventCode eCode, XBEventData *eData)
{
  assert (NULL != eData);
  while (GameEvent (eCode, eData)) {
    if (XBXK_EXIT == eData->value) {
      return XBFalse;
    }
  }
  return XBTrue;
} /* GameEventLoop */


/*
 * Evaluate player action in pause
 */
XBBool
PauseEvalAction (int numPlayer, PlayerAction *playerAction, int pauseStatus)
{
  int player;

  for (player = 0; player < numPlayer; player ++) {
    /* check if player has deactivated pause mode */
    if (player == pauseStatus &&
	player_stat[player].lives > 0 &&
	playerAction[player].pause) {
      return XBTrue;
    }
  }
  return XBFalse;
} /* PauseEvalAction */

/*
 * Evaluate player action in game
 */
int
GameEvalAction (int numPlayer, PlayerAction *playerAction)
{
  int    player;
  int    result  = -1;
  XBBool checkAbort = XBFalse;

  for (player=0; player < numPlayer; player++) {
    BMPlayer     *ps = player_stat + player;
    PlayerAction *pa = playerAction + player;
    /* if player lives */
    if (ps->lives > 0) {
      /* suicide, unless already dying */
      if (pa->suicide && ! ps->dying) {
	ps->dying = DEAD_TIME;
	ps->lives = 1;
      }
      /* toggle pause mode */
      if (pa->pause) {
	result = player;
      }
      /* drop bomb if needed */
      if (pa->bomb) {
	DropBomb (ps, BMTdefault);
      }
      /* execute special key function */
      if (pa->special) {
	(*specialKeyFunc) (ps);
      }
      /* do laola */
      if(pa->laola){
	ps->laola=6;
	SetMsgLaola(player);
      }
      /* activate bot */
      if (pa->bot){
	ps->bot=XBTrue;
      }
      /* do the loser */
      if (pa->looser){
	SetMsgLoser(player);
	ps->looser=10;
      }
      /* try to abort game if needed */
      if (pa->abort == ABORT_CANCEL) {
	if (ps->abort != ABORT_NONE) {
	  SetMessage (p_string[player].abortcancel, XBFalse);
	}
	ps->abort = ABORT_NONE;
      } else if (pa->abort == ABORT_TRUE) {
	SetMessage (p_string[player].abort, XBFalse);
	ps->abort = ABORT_TRUE;
	checkAbort = XBTrue;
      }
      /* get direction */
      if (pa->dir != GoDefault) {
	/* reverse direction  if needed */
	if (ps->illness == IllReverse) {
	  switch (pa->dir) {
	  case GoUp:    pa->dir = GoDown;  break;
	  case GoDown:  pa->dir = GoUp;    break;
	  case GoLeft:  pa->dir = GoRight; break;
	  case GoRight: pa->dir = GoLeft;  break;
	  default:      break;
	  }
	}
	/* reverse2 direction, if needed */
	if (ps->illness == IllReverse2) {
	  switch (pa->dir) {
	  case GoDown:  pa->dir = GoLeft;    break;
	  case GoUp:    pa->dir = GoRight;   break;
	  case GoLeft:  pa->dir = GoUp;	   break;
	  case GoRight: pa->dir = GoDown;    break;
	  default:      break;
	  }
	}
	/* set new course */
	ps->d_soll = pa->dir;
	/* check if player changes course */
	switch (ps->d_ist) {
	case GoUp:
	  if ( ( ( GoRight == ps->d_soll || GoLeft  == ps->d_soll ) &&
		 ps->y % BLOCK_HEIGHT >= BLOCK_HEIGHT - turnY[player] ) ||
	       GoDown == ps->d_soll) {
	    ps->d_ist = GoDown;
	  }
	  break;
	case GoDown:
	  if ( ( ( GoRight == ps->d_soll ||
		   GoLeft  == ps->d_soll ) &&
		 ps->y % BLOCK_HEIGHT <= turnY[player]) ||
	       GoUp == ps->d_soll) {
	    ps->d_ist = GoUp;
	  }
	  break;
	case GoRight:
	  if ( ( ( GoUp   == ps->d_soll ||
		   GoDown == ps->d_soll ) &&
		 ps->x % BLOCK_WIDTH <= turnX[player]) ||
	       GoLeft == ps->d_soll ) {
	    ps->d_ist = GoLeft;
	  }
	  break;
	case GoLeft:
	  if ( ( ( GoUp   == ps->d_soll ||
		   GoDown == ps->d_soll ) &&
		 ps->x % BLOCK_WIDTH >= BLOCK_WIDTH - turnX[player] ) ||
	       GoRight == ps->d_soll ) {
	    ps->d_ist = GoRight;
	  }
	  break;
	default:
	  break;
	}
      }
    }
  }
  /* abort check if one player requested */
  if (checkAbort) {
    XBBool doAbort = XBTrue;
    for (player = 0; player < numPlayer; player ++) {
      if (player_stat[player].lives > 0) {
	if (player_stat[player].abort == ABORT_NONE) {
	  doAbort = XBFalse;
	  break;
	}
      }
    }
    if (doAbort) {
      /* Only if everyone has agreed */
      SetMessage ("* * Level Aborted * *", XBTrue);
      for (player = 0; player < numPlayer; player ++) {
	if (player_stat[player].lives > 0) {
	  player_stat[player].lives = 1;
	  player_stat[player].dying = DEAD_TIME;
	}
      }
    }
  }
  return result;
} /* GameEvalAction */

/*
 * just before one level
 */
void
LevelBegin (const char *name)
{
  /* Set status bar to new level */
  ResetStatusBar (player_stat, name, XBTrue);
  /* Show welcome message */
  WelcomePlayers ();
  /* draw it */
  DrawMaze ();
  /* fade in */
  DoFade (XBFM_IN, PIXH+1);
  GUI_FlushPixmap (XBFalse);
} /* LevelBegin */

/*
 * after one level
 */
const char *
LevelResult (int gameTime, int *lastTeam, int numPlayers, const DBRoot *level, XBBool store)
{
  int         i,j;
  XBBool      won[MAX_PLAYER];
  XBBool      outOfTime   = XBFalse;
  XBBool      drawGame    = XBFalse;
  int         numWinners  = 0;
  int         numLoosers  = 0;
  double      pointWinner = 0.0;
  double      pointLooser = 0.0;
  const char *msg;

  /* sanity check */
  assert (lastTeam != NULL);
  *lastTeam = TEAM_NOWINNER;
  /* check each player */
  for (i = 0; i < numPlayers; i ++) {
    /* let dying players die */
    if (player_stat[i].dying) {
      player_stat[i].dying = 0;
      player_stat[i].lives --;
    }
    /* still alive, then local winner */
    won[i] = (player_stat[i].lives > 0);
    Dbg_Game(" player %i, team %i -> %s\n",i,player_stat[i].team, won[i] ? "won" : "lost");
    /* update winner counts */
    if (won[i]) {
      numWinners++;
      switch (*lastTeam) {
      case TEAM_NOWINNER: *lastTeam = player_stat[i].team; break;
      case TEAM_LOCALASYNC: break;
      default:
	if (*lastTeam != player_stat[i].team) {
	  *lastTeam = TEAM_LOCALASYNC;
	}
      }
    } else {
      numLoosers ++;
    }
  }
  Dbg_Game("num of winners = %i\n",numWinners);
  /* determine message  */
  msg="Test";
  if (gameTime >= (GAME_TIME-1)) {
    Dbg_Game("game timed out\n");
    msg       = "Out of Time";
    outOfTime = XBTrue;
  } else if (*lastTeam == TEAM_NOWINNER) {
    Dbg_Game("draw game\n");
    msg       = "Draw Game";
    drawGame  = XBTrue;
  } else if (*lastTeam == TEAM_LOCALASYNC) {
    Dbg_Game("game asynced locally\n");
    msg       = "Local async";
  } else {
    assert (numWinners > 0);
    /* update player victories, if store requested */
    if (store) {
      for (i = 0; i < numPlayers; i ++) {
	if (*lastTeam == player_stat[i].team) {
	  player_stat[i].victories ++;
	}
      }
    }
    Dbg_Game("winner team = %u\n", *lastTeam);
    if (numWinners == 1) {
      /* one player/team has won */
      for (i = 0; i < numPlayers; i ++) {
	if (won[i]) {
	  Dbg_Game("Only player %i won: %s\n", i, p_string[i].name);
	  msg = p_string[i].winlevel;
	}
      }
    } else {
      /* multiple winner, random win msg */
      j = OtherRandomNumber(numWinners)+1;
      for (i = 0; j>0 && i < numPlayers; i ++) {
	if (won[i]) {
	  j--;
	  if (j==0) {
	    Dbg_Game("random win msg, player %i = %s\n", i,msg);
	    msg = p_string[i].winlevel;
	  }
	}
      }
    }
    pointWinner = numLoosers / numWinners;
    pointLooser = -1;
  }
  /* store level statistics if requested */
  if (store) {
    /* log player result */
    for (i = 0; i < numPlayers; i ++) {
      StoreLevelStat (DB_Atom (level), GUI_StringToAtom (p_string[i].name), won[i], won[i] ? pointWinner : pointLooser);
    }
    /* log result for draws and out of time */
    StoreLevelStat (DB_Atom (level), atomOutOfTime, outOfTime, 0.0);
    StoreLevelStat (DB_Atom (level), atomDrawGame,  drawGame,  0.0);
  }
  /* that's all */
  return msg;
} /* LevelResult */

#if 0
/*
 * after one level
 */
void
LevelEnd (int numPlayers, int lastTeam, const char *msg, XBBool timeOut)
{
  BMPlayer         *ps;
  XBEventData       eData;
  XBBool            flag;
  int               count;
  XBBool            done = XBFalse;
  BMSpriteAnimation anime;

  SND_Play (SND_WON, SOUND_MIDDLE_POSITION);
  /* setup animations */
  flag  = XBTrue;
  count = 0;
  /* setup event handling */
  SetMessage (msg, XBTrue);

  GUI_SetTimer (FRAME_TIME, XBTrue);
  GUI_SetKeyboardMode (KB_MENU);
  GUI_SetMouseMode (XBFalse);
  while (! done) {
    /* update window */
    GameUpdateWindow ();
    /* wait for next event */
    switch (GUI_WaitEvent (&eData) ) {
    case XBE_TIMER:
      if (0 == count % 16) {
	/* toggle message */
	SetMessage (flag ? msg : "Press Space or Return", XBTrue);
	flag = ! flag;
      }
      /* animate winner sprites */
      anime = winnerAnime[count % NUM_WINNER_ANIMES];
      if (lastTeam <= MAX_PLAYER) {
	for (ps = player_stat; ps < player_stat + numPlayers; ps ++) {
	  if (ps->team == lastTeam) {
	    SetSpriteMode (ps->sprite, SPM_MAPPED);
	    SetSpriteAnime (ps->sprite, anime);
	  }
	}
      }
      /* check for timeout */
      if (timeOut &&
	  count > 1000*TIMEOUT_LEVEL_END / FRAME_TIME) {
	done = XBTrue;
      }
      /* --- */
      count ++;
      break;
    case XBE_MENU:
      if (eData.value == XBMK_SELECT ||
	  eData.value == XBMK_DEFAULT) {
	done = XBTrue;
      }
      break;
    case XBE_MOUSE_1:
    case XBE_MOUSE_2:
    case XBE_MOUSE_3:
      done = XBTrue;
      break;
    default:
      break;
    }
  }

  /* clear the redraw map */
  ClearRedrawMap();
  /* fade out image */
  DoFade (XBFM_BLACK_OUT, PIXH+1);
} /* LevelEnd */
#endif

/*
 * play a turn
 */
void
GameTurn (int gameTime, int numPlayer, int *numActive)
{
#ifdef DEBUG_RANDOM
  int        newSeed;
  static int oldSeed = 0;
#endif

  /* set bot time */
  SetBotTime(gameTime) ;
  /* handle players */
  DoAllPlayers (gameTime, numActive);
  /* Shrink functions */
  DoShrink (gameTime);
  /* Scramble blocks */
  DoScramble (gameTime);
  /* Game functions */
  DoNastyWalls (gameTime);
  /* check bombs */
  DoBombs (player_stat, numPlayer);
  /* check explosions */
  DoExplosions();
  /* check if bombs are ignited by other bombs */
  IgniteBombs ();
  /* stun players hit by other bombs */
  StunPlayers (player_stat, numPlayer);
  /* check if and Virus or Junkie infects another player */
  InfectOtherPlayers(numActive);
  /*evil grail */
  DoEvilIll();
  /* do junkie countdown  */
  DoJunkie ();
  /* check if player was hit by any explosions */
  CheckPlayerHit ();
  /* update status bar */
  UpdateStatusBar (player_stat, gameTime);
#ifdef DEBUG_RANDOM
  newSeed = GetRandomSeed ();
  if (newSeed != oldSeed) {
    Dbg_Out ("time=%d seed=%u\n", gameTime, newSeed);
    oldSeed = newSeed;
  }
#endif
} /* GameTurn */

/*
 * return number of players in game
 */
int
GetNumOfPlayers(){
  return numOfPlayers;
} /* GetNumOfPlayers */

/*
 * initialize game data
 */
XBBool
InitGame (XBPlayerHost hostType, CFGType cfgType, const CFGGame *cfgGame, PlayerAction *playerAction)
{
  int       i;
  CFGPlayer cfgPlayer[MAX_PLAYER];
  /* sanity check */
  assert (XBPH_None != hostType);
  assert (NULL      != cfgGame);
  assert (NULL      != playerAction);
  numOfPlayers=cfgGame->players.num;
  /* get player graphics */
  for (i = 0; i < cfgGame->players.num; i ++) {
    if (! RetrievePlayer (cfgType,
			  cfgGame->players.player[i],
			  cfgGame->players.teamColor[i],
			  &cfgPlayer[i]) ) {
      return XBFalse;
    }
  }
  /* check for demo recording */
  if (cfgGame->setup.recordDemo) {
    DemoInitGame (cfgType, cfgGame);
  }
  /* init player graphics */
  if (! InitPlayerSprites (cfgGame->players.num, cfgPlayer) ) {
    return XBFalse;
  }
  /* init players sprites and status icons */
  InitPlayers (hostType, cfgGame, cfgPlayer);
  /* init status bar at bottom of screen */
  InitStatusBar (cfgGame->players.num);
  /* setup scorboard (between levels ) */
  InitScoreBoard (cfgGame->players.num, cfgGame->setup.numWins);
  /* setup key mappings */
  InitPlayerInput (hostType, &cfgGame->players, cfgPlayer, playerAction);
  /* load sounds needed */
  InitSounds ();
  /* that's all */
  return XBTrue;
} /* InitGame */

/*
 * clean up game data
 */
void
FinishGame (const CFGGame *cfgGame)
{
  /* sanity check */
  assert (NULL != cfgGame);
  /* shutdown demo recorded if needed */
  if (cfgGame->setup.recordDemo) {
    DemoFinishGame ();
  }
  /* some cleaning up */
  FinishSounds ();
  DeletePlayerSprites ();
  FinishPlayers ();
} /* FinishGame */

/*
 * end of file game.c
 */
