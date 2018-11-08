/*
 * File intro.c - misc. intros and inbetween screens
 *
 * $Id: intro.c,v 1.12 2005/01/06 21:44:45 lodott Exp $
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
#ifdef SMPF
#include <math.h>
#endif

#include "intro.h"

#include "bomb.h"
#include "event.h"
#include "info.h"
#include "introdat.h"
#include "level.h"
#include "status.h"
#include "random.h"

#include "snd.h"
/*
 * local constants
 */
#ifdef WMS
#ifdef SMPF
#define M_PI 3.14159265358979323846f
#endif
#endif

#define TIMEOUT_GAME_INTRO  10
#define TIMEOUT_LEVEL_INTRO 15
#define TIMEOUT_SCOREBOARD   6
#define TIMEOUT_LEVEL_END    3

#define MAX_AUDIENCE (3*(MAZE_W+1))+100

#define SCORE_NAME_WIDTH  (2*BLOCK_WIDTH+4*BASE_X)
#define SCORE_NAME_HEIGHT (5*BASE_Y)

/*
 * local types
 */
typedef struct  {
  int     xPos;
  int     yPos;
  Sprite *sprite;
  int     team;
  int     numData;
  int     phase;
  const   BMSpriteAnimation *data;
} BMAudience;

/*
 * local variables
 */
static BMPosition scorePos[MAX_PLAYER];
#ifndef SMPF
static BMPosition namePos[MAX_PLAYER];
#endif
/* audience */
static int        numAudience = 0;
static BMAudience audience[MAX_AUDIENCE];
static const int  audienceSpriteAnime[] = {
  SpriteStopDown, 
  SpriteLooser, SpriteLooser1, SpriteLooser2, 
  SpriteWinner, SpriteWinner2, SpriteWinner3,
  MAX_ANIME
};
/* animation for winner at level end*/
static int     winnerCount;
//static int     winnerTeam;
static int     winnerNum;
static Sprite *winnerSprite[MAX_PLAYER];

/*
 * 
 */
static void
DrawIntroTextBoxes (const IntroTextBox *box)
{
  assert (box != NULL);

  while (box->text != NULL) {
    GUI_DrawTextbox (box->text, box->flags, &box->rect);
    box ++;
  }
} /* DrawIntroTextBoxes */

/*
 *
 */
void
DoFade (XBFadeMode fadeMode, int maxLine)
{
  XBEventData eData;

  /* inits */
  GUI_SetTimer (FRAME_TIME, XBTrue);
  GUI_InitFade (fadeMode, maxLine);
  /* do it */
  while (GUI_DoFade ()) {
    while (XBE_TIMER != GUI_WaitEvent (&eData) ) continue;
  }
  GUI_SetTimer (0, XBTrue);
} /* DoFade */

/*
 * wait for user input, return XBFalse if user wants to quit
 */
static XBBool
DoWait (int sec, XBBool countdown, PFV func) 
{
  XBEventData data;
  int    count = sec * 20;

  GUI_SetKeyboardMode (KB_MENU);
  GUI_SetMouseMode (XBTrue);
  GUI_SetTimer (50, XBTrue);
  /* wait for events */
  for (;;) {
    switch (GUI_WaitEvent (&data)) {
      /* timeout and countdown handling */
    case XBE_TIMER:
      /* exec user function */
      if (NULL != func) {
	(*func) ();
      }
      if (0 == sec) {
	break;
      }
      /* handle countdown */
      count --;
      if (count <= 0) {
	return XBTrue;
      }
      if (! countdown) {
	break;
      }
      /* display countdown */
      switch (count) {
      case 61: SetMessage ("3", XBTrue); break;
      case 41: SetMessage ("2", XBTrue); break;
      case 21: SetMessage ("1", XBTrue); break;
      case  1: SetMessage ("0", XBTrue); break;
      default: break;
      }
      break;
      /* keypresses and joystick ... */
    case XBE_MENU:
      switch (data.value) {
      case XBMK_SELECT:  /* space or button */
      case XBMK_DEFAULT: /* return */
	return XBTrue;
      case XBMK_ABORT:   /* escape */
	    // ignore escape return XBFalse;
	    return XBTrue;
      default: 
	break;
      }
      break;
      /* any mouse button press */
    case XBE_MOUSE_1:
    case XBE_MOUSE_2:
    case XBE_MOUSE_3:
      return XBTrue;
    default:
      break;
    }
  }
  return XBFalse;
} /* DoWait */

/*
 *
 */
void
DoIntro (void)
{
  XBEventData data;
  BMRectangle rect;
  XBEventCode event;
  double pfactor;
  int count;
  int x, y, w, h, lw;

  rect.x = BLOCK_WIDTH*6;
  rect.y = BLOCK_HEIGHT*5;
  rect.w = BLOCK_WIDTH*3;
  rect.h = BLOCK_HEIGHT;
    
  /* clear pixmap with default bitmap */
  GUI_ClearPixmap ();
  /* standard init text */
  DrawIntroTextBoxes (introBox);
  /* Flush graphics */
  GUI_FlushPixmap (0);
  /* Syn to Display */
  GUI_Sync ();
  /* load sounds for intro */
  SND_Load (SND_EXPL);
  SND_Load (SND_INTRO);
  /* wait for kb event */
  GUI_SetTimer (FRAME_TIME, XBTrue);
  GUI_SetKeyboardMode (KB_MENU);
  GUI_SetMouseMode (XBTrue);
  for (count = 0; count < INTRO_LENGTH; count ++) {
    if (! (count % CHAR_ANIME) ) {
      SND_Play (SND_EXPL, (count/CHAR_ANIME)*3+1);
    }
    
    /* draw growing poly */
    pfactor = (double)(count+1)/INTRO_LENGTH;
    x = (int) (0.5 + (7.5 - pfactor * 4.5) * BLOCK_WIDTH);
    y = (int) (0.5 + (7.5 - pfactor * 6.0) * BLOCK_HEIGHT);
    w = (int) (0.5 + pfactor *  9.0 * BLOCK_WIDTH);
    h = (int) (0.5 + pfactor * 12.0 * BLOCK_HEIGHT);
    lw = (int) (0.5 + pfactor * 8);
       GUI_DrawPolygon (x, y, w, h, lw, pointx, SIZE_OF_X, XBTrue);
           MarkMazeRect (x-lw/2, y-lw/2, w+lw, h+lw);

    if (count < CHAR_ANIME) {
      CopyExplBlock ( 0, 5, blockB[count]);
    } else if (count < (CHAR_ANIME*2)) {
      CopyExplBlock ( 0, 5, blockB[CHAR_ANIME-1]);
      CopyExplBlock ( 3, 5, blockL[count-CHAR_ANIME]);
    } else if (count < (CHAR_ANIME*3)) {
      CopyExplBlock ( 0, 5, blockB[CHAR_ANIME-1]);
      CopyExplBlock ( 3, 5, blockL[CHAR_ANIME-1]);
      CopyExplBlock ( 6, 5, blockA[count-CHAR_ANIME*2]);
    } else if (count < (CHAR_ANIME*4)) {
      CopyExplBlock ( 0, 5, blockB[CHAR_ANIME-1]);
      CopyExplBlock ( 3, 5, blockL[CHAR_ANIME-1]);
      CopyExplBlock ( 6, 5, blockA[CHAR_ANIME-1]);
      CopyExplBlock ( 9, 5, blockS[count-CHAR_ANIME*3]);
    } else {
      CopyExplBlock ( 0, 5, blockB[CHAR_ANIME-1]);
      CopyExplBlock ( 3, 5, blockL[CHAR_ANIME-1]);
      CopyExplBlock ( 6, 5, blockA[CHAR_ANIME-1]);
      CopyExplBlock ( 9, 5, blockS[CHAR_ANIME-1]);
      CopyExplBlock (12, 5, blockT[count-CHAR_ANIME*4]);
    }

    /* update sound */
    SND_Flush ();
    /* set rectangles to redrawn */
    SetRedrawRectangles ();
    /* redraw all blocks */
    GUI_FlushBlocks ();
    /* update explosions animations */
    UpdateExpl ();
    /* copyright text */
    if (count == INTRO_LENGTH -1) {
      DrawIntroTextBoxes (creditsBox);
    }
    /* update window from pixmap */
    GUI_FlushPixmap (XBTrue);
    /* clear the redraw map */
      ClearRedrawMap ();

    /* check event */
    while (XBE_TIMER != (event = GUI_WaitEvent (&data) ) ) {
      switch (event) {
      case XBE_MENU:
      case XBE_MOUSE_1:
      case XBE_MOUSE_2:
      case XBE_MOUSE_3:
	if (count < INTRO_LENGTH - 2) {
	  count = INTRO_LENGTH - 3;
	} 
	break;
      default:
	break;
      }
    }
  }
  SND_Play (SND_INTRO,SOUND_MIDDLE_POSITION);
  /* wait for keystroke or timer */
  DoWait (TIMEOUT_GAME_INTRO, XBFalse, NULL);
  /* unload sounds */
  SND_Stop (SND_INTRO);
  SND_Stop (SND_EXPL);
  SND_Unload (SND_EXPL);
  SND_Unload (SND_INTRO);
  /* fade out screen */
  DoFade (XBFM_BLACK_OUT, PIXH+SCOREH);
} /* DoIntro */

/*
 *
 */
static void
LoadUpdateWindow (void) 
{
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
} /* ScoreUpdateWindow */

/*
 * poll events
 */
XBBool
LoadPoll (void)
{
  XBEventCode eCode;
  XBEventData eData;

  while (XBE_NONE != (eCode = (GUI_PeekEvent (&eData) ) ) ) {
    if (XBE_MENU   == eCode &&
	XBMK_ABORT == eData.value) {
      return XBFalse;
    }
  }
  return XBTrue;
} /* Poll */

/*
 *
 */
XBBool
InitPlayerSprites (int numPlayers, const CFGPlayer *cfgPlayer)
{
  int     i, j, x;
  XBBool  ok;
  Sprite *sprite[MAX_PLAYER+1];

  assert (cfgPlayer != NULL);
  /* inits */
  memset (&sprite, 0, sizeof (sprite));
  /* setup background graphics */
  ConfigScoreGraphics (graphicsLoadSprite);
  ConfigScoreMap (mapLoadSprite);
  ClearStatusBar (BTFree, BTFree);
  /* only escape-key to poll */
  GUI_SetTimer (0, XBTrue);
  GUI_SetMouseMode (XBFalse);
  GUI_SetKeyboardMode (KB_MENU);
  /* load sounds for intro */
  SND_Load (SND_WHIRL);
  SND_Play (SND_WHIRL, SOUND_MIDDLE_POSITION);
  /* text message */
  sprite[MAX_PLAYER] = CreateTextSprite ("Loading ...", PIXW/2 - 3*BLOCK_WIDTH, PIXH/2 + BLOCK_HEIGHT,
					 6*BLOCK_WIDTH, BLOCK_HEIGHT, FF_White | FF_Large | FF_Outlined,
					 SPM_MAPPED);
  assert (sprite[MAX_PLAYER] != NULL);
#ifdef DEBUG
  Dbg_StartClock ();
#endif
  ok = XBFalse;
  /* show first animation for each player */
  x = (PIXW - numPlayers*BLOCK_WIDTH) / 2;
  for (i = 0; i < numPlayers; i ++) {
    GUI_LoadPlayerSprite (i, 0, &cfgPlayer[i].graphics);
    if (! LoadPoll ()) {
      goto Finish;
    }
    sprite[i] = CreatePlayerSprite (i, x, PIXH/2 - BLOCK_HEIGHT, SpriteStopDown, SPM_MAPPED);
    assert (sprite[i] != NULL);
    x += BLOCK_WIDTH;
  }
  LoadUpdateWindow ();
  /* load rest of animation */
  for (i = 0; i < numPlayers; i ++) {
    for (j = 1; j < MAX_ANIME_EPM; j ++) {
      GUI_LoadPlayerSprite (i, j,  &cfgPlayer[i].graphics);
      if (! LoadPoll ()) {
	goto Finish;
      }
    }
    GUI_LoadPlayerScoreTiles (i,  &cfgPlayer[i].graphics);
    SetSpriteAnime (sprite[i], SpriteWinner);
    LoadUpdateWindow ();
    if (! LoadPoll ()) {
      goto Finish;
    }
  }
  /* successful ;-) */
#ifdef DEBUG
  Dbg_Out ("%d sprites loaded in %lu msec\n", numPlayers, Dbg_FinishClock () );
#endif
  ok = XBTrue;
Finish:
  /* delete old sprites */
  for (i = 0; i <= MAX_PLAYER; i ++) {
    if (NULL != sprite[i]) {
      DeleteSprite (sprite[i]);
    }
  }  
  FinishLevelGraphics ();
  /* stop sound */ 
  SND_Stop (SND_WHIRL);
  SND_Unload (SND_WHIRL);
  /* that's all */
  return ok;
} /* LoadPlayerSprites */

/*
 *
 */
XBBool
LevelIntro (int numPlayers, const DBRoot *level, int timeOut)
{
  int          i, player;
  BMPlayer    *ps;
  int          numInfo;
  const char **info;
  static char  tmp[128];

  ResetStatusBar (player_stat, "Press Space or Return", XBFalse);
  /* draw player positions */
  for (player=0, ps = player_stat; player < numPlayers; player++, ps++) {
    MoveSprite (ps->sprite, ps->x, ps->y);
    SetSpriteMode (ps->sprite, ps->in_active ? SPM_UNMAPPED : SPM_MAPPED);
    SetSpriteAnime (ps->sprite, SpriteStopDown);
  }
  /* Update Window */
  DrawMaze ();
  DrawAllSprites ();
  /* Get Level info */
  /* draw level title und hint*/
  titleBox[1].text = GetLevelName (level);
  sprintf (tmp, "created by %s", GetLevelAuthor (level));
  titleBox[2].text = tmp;
  titleBox[3].text = GetLevelHint (level);
  DrawIntroTextBoxes (titleBox);

  /* player info */
  info = GetPlayerInfo (&numInfo);
  for (i = 0; i < numInfo; i ++) {
    playerInfoBox[2+i].text = info[i];
  }
  playerInfoBox[2+i].text = NULL;
  DrawIntroTextBoxes (playerInfoBox);
  /* level info */
  info = GetLevelInfo (&numInfo);
  for (i = 0; i < numInfo; i ++) {
    levelInfoBox[2+i].text = info[i];
  }
  levelInfoBox[2+i].text = NULL;
  DrawIntroTextBoxes (levelInfoBox);
  /* extra info */
  info = GetExtraInfo (&numInfo);
  for (i = 0; i < numInfo; i ++) {
    extraInfoBox[2+i].text = info[i];
  }
  extraInfoBox[2+i].text = NULL;
  DrawIntroTextBoxes (extraInfoBox);
  /* update window from pixmap */
  DoFade (XBFM_IN, PIXH+SCOREH);
  GUI_FlushPixmap (XBFalse);
  /* clear the redraw map */
  ClearRedrawMap();
  /* negative timeout, simulate keypress */
  if (timeOut < 0) {
    return XBTrue;
  }
  /* now wait for select */
  Dbg_Out("Gonna wait %i seconds\n", timeOut);
  return DoWait (timeOut, XBTrue, LoadUpdateWindow);
} /* LevelIntro */

/*
 *
 */
static void
LevelAnimateWinner (void)
{
  int i;

  /* animate winner sprites */
  for (i = 0; i < winnerNum; i ++) {
    SetSpriteMode  (winnerSprite[i], SPM_MAPPED);
    SetSpriteAnime (winnerSprite[i], winnerAnime[winnerCount]);
  }
  /* next frames */
  winnerCount ++;
  if (winnerCount >= NUM_WINNER_ANIME) {
    winnerCount = 0;
  }
  /* graphics update etc */
  LoadUpdateWindow ();
} /* LevelAnimateWinner */

/*
 *
 */
XBBool
LevelEnd (int numPlayers, int lastTeam, const char *msg, int timeOut)
{
  int i;

  int player;
  for (player = 0; player <numPlayers; player ++) {
    ((player_stat[player].sprite)->player).player=player_stat[player].id; 
  }

  /* counter for winner animation */
  winnerCount = 0;
  /* which sprites do we animate */
  winnerNum = 0;
  for (i = 0; i < numPlayers; i ++) {
    if (lastTeam == player_stat[i].team) {
      winnerSprite[winnerNum] = player_stat[i].sprite;
      winnerNum ++;
    }
  }
  /* show level result */
  SetMessage (msg, XBTrue);
  /* player applause */
  if (lastTeam >= 0) {
    SND_Play (SND_WON, SOUND_MIDDLE_POSITION);
  }
  /* negative timeout, just return */
  if (timeOut < 0) {
    return XBTrue;
  }
  /* zero timeout: wait space, otherwise space or timeout */
  return DoWait (timeOut ? TIMEOUT_LEVEL_END : 0, XBFalse, LevelAnimateWinner);
} /* LevelEnd */

/*
 *
 */
void 
InitScoreBoard (int numPlayers, int numWins)
{
  int i, j, x, y;
#ifndef SMPF
  int yStart, yStep;
#endif
  int yAudience, xStart;

  /* determine start pos */
#ifdef SMPF
  yAudience=3;
#else
  switch (numPlayers) {
  case 2:  yStart = 5; yStep = 4; yAudience = 3; break;
  case 3:  yStart = 4; yStep = 3; yAudience = 3; break;
  case 4:  yStart = 4; yStep = 2; yAudience = 3; break;
  case 5:  yStart = 3; yStep = 2; yAudience = 3; break;
  default: yStart = 1; yStep = 2; yAudience = 1; break;
  }
#endif
  /* fill map */
  for (y = 0; y < yAudience; y ++) {
    for (x = 0; x < MAZE_W; x ++) {
      mapScoreBoard[x][y] = 7;
    }
  }
  /* clear the rest */
  for (; y < MAZE_H; y ++) {
    for (x = 0; x < MAZE_W; x ++) {
      mapScoreBoard[x][y] = 6;
    }
  }
#ifndef SMPF
  /* draw podests */
  for (i = 0; i < numPlayers; i ++) {
    /* higher part */
    for (x = 0; x < 3; x ++) {
      mapScoreBoard[x][yStart+yStep*i]   = 0;
      mapScoreBoard[x][yStart+yStep*i+1] = 1;
    }
    /* steps */
    mapScoreBoard[x][yStart+yStep*i]   = 2;
    mapScoreBoard[x][yStart+yStep*i+1] = 3;
    x ++;
    /* lower part */
    for (; x < MAZE_W && x < (6 + numWins); x ++) {
      mapScoreBoard[x][yStart+yStep*i]   = 4;
      mapScoreBoard[x][yStart+yStep*i+1] = 5;
    }
  }
  /* set player positions */
  for (i = 0; i < numPlayers; i ++) {
    scorePos[i].x = BLOCK_WIDTH*4;
    scorePos[i].y = (yStart + yStep*i - 1) * BLOCK_HEIGHT + 4*BASE_Y;
    namePos[i].x  = BLOCK_WIDTH/4;
    namePos[i].y  = (yStart + yStep*i + 1) * BLOCK_HEIGHT + 3*BASE_Y/2;
  }
#endif
  /* load additional sprites for audience */
  for (i = numPlayers; i < NUM_DEFAULT_PLAYERS; i ++) {
    const CFGPlayerGraphics *gfx = DefaultPlayerGraphics (i);
    for (j = 0; audienceSpriteAnime[j] != MAX_ANIME; j ++) {
      GUI_LoadPlayerSprite (i, audienceSpriteAnime[j], gfx);
    }
  }
  /* set audience positions and sprite */
  numAudience = 0;
  for (y = 0; y < BLOCK_HEIGHT * yAudience; y += BLOCK_HEIGHT) {
    xStart = (y == BLOCK_HEIGHT) ? -BLOCK_WIDTH/2 : 0;
    for (x = xStart; x < PIXW; x += BLOCK_WIDTH) {
      audience[numAudience].xPos = x + OtherRandomNumber (BASE_X) - BASE_X/2;
      audience[numAudience].yPos = y - OtherRandomNumber (BASE_Y) - 4*BLOCK_HEIGHT/3;
      audience[numAudience].team = OtherRandomNumber (MAX(NUM_DEFAULT_PLAYERS, numPlayers));
      numAudience ++;
    } 
  }
  /* SMPF circle test
  for(i=0; i< 16; i++) {
    audience[numAudience].xPos = (int) floor(3*PIXW*cos(M_PI+(2*M_PI * i / 16))/12+PIXW/2-BLOCK_WIDTH/2);
    audience[numAudience].yPos = (int) floor(3*PIXH*sin(M_PI+(2*M_PI * i / 16))/12+PIXH/2-BLOCK_HEIGHT/2);
    audience[numAudience].team = OtherRandomNumber (MAX(NUM_DEFAULT_PLAYERS, numPlayers));
    numAudience ++;
    } */
} /* InitScoreBoard */

/*
 *
 */
void 
InitWinner (int numPlayers)
{
  int i, x, y;
  int xStart, xStep;
  int yAudience;

  /* determine start pos */
  yAudience = 3;
  switch (numPlayers) {
  case 2:  xStart = 7; xStep = 4; break;
  case 3:  xStart = 5; xStep = 4; break;
  case 4:  xStart = 3; xStep = 4; break;
  case 5:  xStart = 1; xStep = 4; break;
  default: xStart = 1; xStep = 3; break;
  }
  /* fill map */
  /* audience */
  for (y = 1; y <= yAudience; y ++) {
    for (x = 0; x < MAZE_W; x ++) {
      mapScoreBoard[x][y] = 7;
    }
  }
  for (x = 0; x < MAZE_W; x ++) {
    mapScoreBoard[x][0] = 1;
    mapScoreBoard[x][y] = 8;
  }
  y ++;
  /* clear the rest */
  for (; y < MAZE_H; y ++) {
    for (x = 0; x < MAZE_W; x ++) {
      mapScoreBoard[x][y] = 6;
    }
  }
  /* winner */
  for (x = MAZE_W/2-1; x < MAZE_W/2+2; x ++) {
    /* draw big podests */
    //mapScoreBoard[x][6] = 0;
    //mapScoreBoard[x][7] = 0;
    //mapScoreBoard[x][8] = 1;
    /* winner position */
    scorePos[0].x = (PIXW - BLOCK_WIDTH)/2;
    scorePos[0].y = 4 * BLOCK_HEIGHT + 6 * BASE_Y;
  }
  /* other players */
  for (i = 1, x = xStart; i < numPlayers; i ++) {
    /* draw small podest */
    // mapScoreBoard[x][10] = 4;
    // mapScoreBoard[x][11] = 5;
    /* player position */
    scorePos[i].x =  x * BLOCK_WIDTH;;
    scorePos[i].y =  9 * BLOCK_HEIGHT + 4 * BASE_Y;
    /* next one */
    x += xStep;
  }
  /* set audience positions and sprite */
  numAudience = 0;
  for (y = BLOCK_HEIGHT; y <= BLOCK_HEIGHT * yAudience; y += BLOCK_HEIGHT) {
    xStart = (y == 2*BLOCK_HEIGHT) ? -BLOCK_WIDTH/2 : 0;
    for (x = xStart; x < PIXW; x += BLOCK_WIDTH) {
      audience[numAudience].xPos = x + OtherRandomNumber (BASE_X) - BASE_X/2;
      audience[numAudience].yPos = y - OtherRandomNumber (BASE_Y) - 4*BLOCK_HEIGHT/3;
      audience[numAudience].team = OtherRandomNumber (MAX(numPlayers,NUM_DEFAULT_PLAYERS));
      numAudience ++;
    } 
  }
  /* SMPF circle test
  for(i=0; i< 16; i++) {
    audience[numAudience].xPos = (int) floor(3*PIXW*cos(M_PI+(2*M_PI * i / 16))/12+PIXW/2-BLOCK_WIDTH/2);
    audience[numAudience].yPos = (int) floor(3*PIXH*sin(M_PI+(2*M_PI * i / 16))/12+PIXH/2+BLOCK_HEIGHT);
    audience[numAudience].team = OtherRandomNumber (MAX(NUM_DEFAULT_PLAYERS, numPlayers));
    numAudience ++;
    } */
} /* InitScoreBoard */

/*
 *
 */
static void
ShowAudience (int numPlayers, int lastTeam)
{
  int i;
  BMAudience *ptr;

  for (i = 0, ptr = audience; i < numAudience; i ++, ptr ++) {
    /* set animation */
    if (ptr->team < numPlayers) {
      if ((1<<ptr->team) & lastTeam) {
	/* his team has won the game */
	ptr->data    = winnerAnime;
	ptr->numData = NUM_WINNER_ANIME;
      } else {
	/* his team has lost the game */
	ptr->data    = looserAnime;
	ptr->numData = NUM_LOOSER_ANIME;
      }
    } else {
      if (MAX_PLAYER == lastTeam) {
	/* draw game or timeout */
	ptr->data    = otherLooserAnime;
	ptr->numData = NUM_OTHER_LOOSER_ANIME;
      } else {
	/* some has won */
	ptr->data    = otherWinnerAnime;
	ptr->numData = NUM_OTHER_WINNER_ANIME;
      }
    }
    ptr->phase = OtherRandomNumber (ptr->numData);
    /* create sprite */
    ptr->sprite = CreatePlayerSprite (ptr->team, ptr->xPos, ptr->yPos, ptr->data[ptr->phase], SPM_MAPPED);
  }
} /* ShowAudience */

/*
 *
 */
static void
ShowLaOla (void)
{
  int i;
  BMAudience *ptr;

  for (i = 0, ptr = audience; i < numAudience; i ++, ptr ++) {
    /* set animation */
    ptr->data    = laOlaAnime;
    ptr->numData = NUM_LAOLA_ANIME;
    ptr->phase   = (2*NUM_LAOLA_ANIME - 2*ptr->xPos/BLOCK_WIDTH + OtherRandomNumber (5)) % NUM_LAOLA_ANIME;
    /* create sprite */
    ptr->sprite  = CreatePlayerSprite (ptr->team, ptr->xPos, ptr->yPos, ptr->data[ptr->phase], SPM_MAPPED);
  }
} /* ShowLaOla */

/*
 *
 */
static void
AnimateAudience (void)
{
  int i;
  BMAudience *ptr;

  /* create audience sprites */
  for (i = 0, ptr = audience; i < numAudience; i ++, ptr ++) {
    /* set new animation frame */
    SetSpriteAnime (ptr->sprite, ptr->data[ptr->phase]);
    /* increment frame counter */
    ptr->phase ++;
    if (ptr->phase >= ptr->numData) {
      ptr->phase = 0;
    }
  }  
} /* AnimateAudience */

/*
 *
 */
static void
HideAudience (void)
{
  int i;

  for (i = 0; i < numAudience; i ++) {
    assert (audience[i].sprite != NULL);
    DeleteSprite (audience[i].sprite);
  }
} /* HideAudience */

/*
 *
 */
static void
ScoreUpdateWindow (void) 
{
  /* sprite animations */
  AnimateAudience ();
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
} /* ScoreUpdateWindow */

/*
 *
 */
XBBool
ShowScoreBoard (int lastTeam, int maxNumWins, int numPlayers, BMPlayer *playerStat, int timeOut)
{
  int       i, j, n=0;
  BMPlayer *ps;
  int       numTrophies = 0;
  XBBool    result = XBTrue;
  Sprite   *trophy[MAX_PLAYER*MAX_VICTORIES];
#ifndef SMPF
  Sprite   *name[MAX_PLAYER];
#endif
  int      winningTeam = 0;

  /* load score board map */
  ResetInfo ();
  ConfigScoreGraphics (graphicsScoreBoard);
  ConfigScoreMap (mapScoreBoard);
  /* draw maze in pximap */
  DrawMaze ();
  ResetStatusBar (playerStat, "Scoreboard", XBFalse);
  for (i = 0, ps = playerStat; i <  numPlayers; i ++, ps ++) {
#ifdef SMPF
    ps->x = (int) floor(3*PIXW*cos(M_PI+(2*M_PI * i / numPlayers))/12+PIXW/2-BLOCK_WIDTH/2);
    ps->y = (int) floor(3*PIXH*sin(M_PI+(2*M_PI * i / numPlayers))/12+PIXH/2-BLOCK_HEIGHT/2);
#else
    ps->x = scorePos[i].x;
    ps->y = scorePos[i].y;
#endif
    /* draw player sprites */
    MoveSprite (ps->sprite, ps->x, ps->y);
    SetSpriteMode (ps->sprite, SPM_MAPPED);
    if (ps->in_active) {
      SetSpriteAnime (ps->sprite, SpriteStopUp);
    } else if (ps->team == lastTeam) {
      SetSpriteAnime (ps->sprite, SpriteWinner);
    } else if (ps->victories == maxNumWins) {
      SetSpriteAnime (ps->sprite, SpriteStopDown);
    } else{
      SetSpriteAnime (ps->sprite, SpriteLooser);
    }
    n = (n > ps->victories) ? n: ps->victories;
    /* draw "score" bombs */
#ifndef SMPF
    for (j = 0; j < ps->victories; j ++) {
      trophy[numTrophies ++] = CreateBombSprite (BB_NORMAL, (6+j)*BLOCK_WIDTH, ps->y + BLOCK_HEIGHT, 0, SPM_MAPPED);
    }
    /* draw player names */
    name[i] = CreateTextSprite (p_string[i].name, namePos[i].x, namePos[i].y, SCORE_NAME_WIDTH, SCORE_NAME_HEIGHT, 
				FF_White|FF_Medium|FF_Boxed, SPM_MAPPED);
#endif
  }
#ifdef SMPF
  for (j = 0; j < n; j ++) {
    trophy[numTrophies ++] = CreateBombSprite (BB_NORMAL,(j-n/2)*BLOCK_WIDTH+BLOCK_WIDTH*MAZE_W/2, 
					       BLOCK_HEIGHT * (MAZE_H-2)+BLOCK_HEIGHT/2, 0, SPM_MAPPED);
  } 
#endif

  /* now show audience */
  for (i = 0, ps = playerStat; i <  numPlayers; i ++, ps ++) {
    if(ps->team == lastTeam) {      winningTeam|=1<<i; }
  }
  Dbg_Out("The players that won are: %i\n", winningTeam);
  ShowAudience (numPlayers, winningTeam);
  /* draw it */
  DrawAllSprites ();
  /* update window */
  GUI_FlushScoreBoard ();
  DoFade (XBFM_IN, PIXH);
  GUI_FlushPixmap (XBFalse);
  /* applause */
  SND_Play (SND_APPL, SOUND_MIDDLE_POSITION);
  /* non-negative timeout, wait till keypress or timeout */
  if (timeOut >= 0) {
    result = DoWait (timeOut ? TIMEOUT_SCOREBOARD : 0, XBTrue, ScoreUpdateWindow);
  }
  /* fade out screen */
  DoFade (XBFM_BLACK_OUT, PIXH);
  /* clean up */
  for (i = 0; i < numTrophies; i++) {
    assert (trophy[i] != NULL);
    DeleteSprite (trophy[i]);
  }
#ifndef SMPF
  for (i = 0; i < numPlayers; i++) {
    assert (name[i] != NULL);
    DeleteSprite (name[i]);
  }
#endif
  HideAudience ();
  FinishLevelGraphics ();
  ClearRedrawMap ();
  
  return result;
} /* ShowScoreBoard */

/*
 *
 */
void
ShowWinner (int lastTeam, int numPlayers, BMPlayer *playerStat)
{
  int i, j, k, x, n, s;
  BMPlayer *ps;
  Sprite *name=NULL;
  char *msg = NULL;
  int xStart, xStep;
  char totalName[256];
  int nTeamPlayers[MAX_HOSTS];
  int teamStart[MAX_HOSTS];
  int nTeamPlayersSet[MAX_HOSTS];
  int teams=0;

  /* load score board map */
  for(i=0; i < MAX_HOSTS; i++) {
    nTeamPlayers[i]=0;
    nTeamPlayersSet[i]=0;
  }

  for (i=0, ps = playerStat; i < numPlayers; ps ++, i++) {
    nTeamPlayers[ps->team]++;
    if(nTeamPlayers[ps->team]==1) {
      teams++;
    }
  }
  
  if((numPlayers-nTeamPlayers[lastTeam])>=5) { 
    xStep=2;
  } else {
    xStep=3;
  }

  Dbg_Out("Team configuration:\n");
  s=0;
  for(i=0; i < MAX_HOSTS; i++) {
    if(i== lastTeam) {
      Dbg_Out("Winning team %i, %i players:\n",i,nTeamPlayers[i]);
    } else {
      Dbg_Out("Loosing team %i, %i players:\n",i,nTeamPlayers[i]);
      teamStart[i]=s;
      if(nTeamPlayers[i]>0) {
	s=s+2*nTeamPlayers[i]-1+xStep;
      }
    }
  }
  s=s-3; // last team so no between space!

  n=nTeamPlayers[lastTeam];
  switch (n) {
  case 1:  xStart = 7; xStep = 2; break;
  case 2:  xStart = 6; xStep = 2; break;
  case 3:  xStart = 5; xStep = 2; break;
  case 4:  xStart = 4; xStep = 2; break;
  case 5:  xStart = 3; xStep = 2; break;
  default: xStart = 3; xStep = 1; break;
  }
 
  /* winners */

#ifndef SMPF
  for(x=xStart-1;x<=xStart+xStep*(n-1)+1;x++) {
    /* winner position */
    mapScoreBoard[x][6] = 0;
    mapScoreBoard[x][7] = 0;
    mapScoreBoard[x][8] = 1;
  }
#endif

  for (x = xStart, i=0; i<n; i++) {
    /* draw big podests */
    scorePos[i].x = x * BLOCK_WIDTH;
#ifdef SMPF
    scorePos[i].y = 7 * BLOCK_HEIGHT + 0 * BASE_Y;
#else
    scorePos[i].y = 4 * BLOCK_HEIGHT + 6 * BASE_Y;
#endif
    x+=xStep;
  }

#ifdef SMPF
  for (i=0, j=0, ps = playerStat; i < numPlayers; ps ++, i++) {
    if(ps->team!=lastTeam) {
      scorePos[j+n].x = (int) floor(3*PIXW*cos(M_PI+(2*M_PI * j / (numPlayers-n)))/12+PIXW/2-BLOCK_WIDTH/2);
      scorePos[j+n].y = (int) floor(3*PIXH*sin(M_PI+(2*M_PI * j / (numPlayers-n)))/12+PIXH/2+BLOCK_HEIGHT);
      j++;
    }
  }
#else
  s=s/2;
  for (i=0, j=0, ps = playerStat; i < numPlayers; ps ++, i++) {
    if(ps->team!=lastTeam) {
      x=7+teamStart[ps->team]-s+nTeamPlayersSet[ps->team]*2;
      Dbg_Out("x = %d\n",x);
      nTeamPlayersSet[ps->team]++;
      mapScoreBoard[x][10] = 4;
      mapScoreBoard[x][11] = 5;
      if(nTeamPlayersSet[ps->team]<nTeamPlayers[ps->team]) {
	mapScoreBoard[x+1][10] = 4;
	mapScoreBoard[x+1][11] = 5;
      }
      scorePos[j+n].x =  x * BLOCK_WIDTH;
      scorePos[j+n].y =  9 * BLOCK_HEIGHT + 4 * BASE_Y;
      j++;
    }
  }
#endif

  ResetInfo ();
  ConfigScoreGraphics (graphicsScoreBoard);
  ConfigScoreMap (mapScoreBoard);
  /* draw maze in pximap */
  DrawMaze ();
      

  /* init */
  for(i=0;i<256;i++) {
    totalName[i]=0;
  }
  for (i = 0, j = 0, k = 0, ps = playerStat; i <  numPlayers; i ++, ps ++) {
    if (ps->team == lastTeam) {
      /* show winner sprite*/
      ps->x = scorePos[k].x;
      ps->y = scorePos[k].y;
      SetSpriteAnime (ps->sprite, SpriteBigWinner);
      /* show winner name */
      if(k>0) { strcat(totalName," & "); }
      strcat(totalName,p_string[i].name);
      /* set winning message */
      msg = p_string[i].wingame;
      k++;
    } else {
      //Dbg_Out("Draw one at %i %u\n", (j+n) , (scorePos[j+n].x)/BLOCK_WIDTH);
      ps->x = scorePos[j+n].x;
      ps->y = scorePos[j+n].y;
      if (ps->in_active) {
	SetSpriteAnime (ps->sprite, SpriteStopUp);
      } else if (ps->victories == 0) {
	SetSpriteAnime (ps->sprite, SpriteDamagedDown);
      } else{
	SetSpriteAnime (ps->sprite, SpriteLooser);
      }    
      j ++;
    }
    /* draw player sprites */
    MoveSprite (ps->sprite, ps->x, ps->y);
    SetSpriteMode (ps->sprite, SPM_MAPPED);
  }
#ifndef SMPF
  name=CreateTextSprite (totalName, (30-(n*3))*BLOCK_WIDTH/4, 49*BLOCK_HEIGHT/6, n*3*BLOCK_WIDTH/2,  2*BLOCK_HEIGHT/3, 
			        FF_White|FF_Medium|FF_Boxed, SPM_MAPPED);
#endif
  /* display message */
  assert (msg != NULL);
  ResetStatusBar (playerStat, msg, XBFalse);
  /* now show audience */
  ShowLaOla ();
  /* draw it */
  DrawAllSprites ();
  /* update window */
  GUI_FlushScoreBoard ();
  DoFade (XBFM_IN, PIXH);
  GUI_FlushPixmap (XBFalse);
  /* sound */
  SND_Load (SND_FINALE);
  SND_Play (SND_FINALE, SOUND_MIDDLE_POSITION);
  /* wait for select key to continue */
  DoWait (0, XBFalse, ScoreUpdateWindow);
  /* fade out screen */
  DoFade (XBFM_WHITE_OUT, PIXH+SCOREH);
  /* clean up */
  if (NULL != name) {
    DeleteSprite (name);
  }
  HideAudience ();
  FinishLevelGraphics ();
  SND_Stop (SND_FINALE);
  SND_Unload (SND_FINALE);
} /* ShowWinner */

/*
 * end of file intro.c
 */
