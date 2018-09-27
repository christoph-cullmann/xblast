/*
 * cfg_game.h - game configuration data
 *
 * $Id: cfg_game.h,v 1.13 2005/01/12 00:20:57 lodott Exp $
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
#ifndef XBLAST_CFG_GAME_H
#define XBLAST_CFG_GAME_H

#include "cfg_main.h"
#include "color.h"
#include "event.h"
#include "net_tele.h"
#include "snd.h"
#include "version.h"

/* player controls */
typedef enum {
  XBPC_None,
  XBPC_RightKeyboard,
  XBPC_LeftKeyboard,
  XBPC_Joystick1,
  XBPC_Joystick2,
  XBPC_Joystick3,
  XBPC_Joystick4,
  /* no new elements after this line */
  XBPC_NUM
} XBPlayerControl;

/*player hosts */
typedef enum {
  XBPH_None,
  XBPH_Demo,
  XBPH_Local,
  XBPH_Server,
  XBPH_Client1,
  XBPH_Client2,
  XBPH_Client3,
  XBPH_Client4,
  XBPH_Client5,
#ifdef SMPF
  XBPH_Client6, 
  XBPH_Client7, 
  XBPH_Client8, 
  XBPH_Client9, 
  XBPH_Client10,
  XBPH_Client11,
  XBPH_Client12,
  XBPH_Client13,
  XBPH_Client14,
  XBPH_Client15, 
#endif
  XBPH_Central,
  /* no new elements after this line */
  XBPH_NUM
} XBPlayerHost;

/*player teams */
typedef enum {
#ifndef OLDMENUS
  XBPT_Invalid,
#endif
  XBPT_None,
  XBPT_Red,
  XBPT_Green,
  XBPT_Blue,
  /* no new elements after this line */
  XBPT_NUM
} XBPlayerTeam;

/* game player config  */
typedef struct {
  int             num;
  int             PID[MAX_PLAYER];
  int             playerID[MAX_PLAYER];
  XBAtom          player[MAX_PLAYER];
  XBPlayerControl control[MAX_PLAYER];
  XBPlayerHost    host[MAX_PLAYER];
  XBPlayerTeam    team[MAX_PLAYER];
  XBColor         teamColor[MAX_PLAYER];
} CFGGamePlayers;

/* team modes */
typedef enum {
  XBTM_None,        /* normal game */
  XBTM_Team,        /* teams, assignment is random */
  XBTM_Hunt,        /* all on one (3 to 6 players) */
  /* no new elements after this line */
  XBTM_NUM
} XBTeamMode;

/* game parameters */
typedef struct {
  XBBool     ifRecLives;
  int        numLives;
  int        numWins;
  int        frameRate;
  XBBool     allLevels;
  XBBool     randomLevels;
  XBBool     randomPlayers;
  int        levelOrder; // LRF
  int        infoTime; 
  XBBool     recordDemo;
  XBTeamMode teamMode;
  XBBool     rated; // XBCC
  XBBool     bot;
  XBBool     beep;
  SND_Id     Music;
  int        recLives;
} CFGGameSetup;

/* connection to host */
typedef struct {
  const char *name;
  int         port;
  XBBool      fixedUdpPort;
  XBBool      browseLan;
  XBBool      allowNat;
  XBBool      central;  // XBCC
  XBBool      beep;
  SND_Id      Music;
  const char *game;
} CFGGameHost;

/* all in one */
typedef struct {
  CFGGameSetup   setup;
  CFGGamePlayers players;
  CFGGameHost    host;
} CFGGame;

/* 
 * global prototypes
 */
extern void LoadGameConfig (void);
extern void SaveGameConfig (void);
extern void FinishGameConfig (void);

extern void StoreGame        (CFGType, XBAtom atom, const CFGGame *);
extern void StoreGameHost    (CFGType, XBAtom atom, const CFGGameHost *);
extern void StoreGameSetup   (CFGType, XBAtom atom, const CFGGameSetup *);
extern void StoreGamePlayers (CFGType, XBAtom atom, const CFGGamePlayers *);
extern void StoreGameVersion (CFGType cfgType, XBAtom atom, const XBVersion *ver);

extern XBBool RetrieveGame        (CFGType, XBAtom atom, CFGGame *);
extern XBBool RetrieveGamePlayers (CFGType, XBAtom atom, CFGGamePlayers *);
extern XBBool RetrieveGameSetup   (CFGType, XBAtom atom, CFGGameSetup   *);
extern XBBool RetrieveGameHost    (CFGType, XBAtom atom, CFGGameHost  *);
extern XBBool RetrieveGameVersion (CFGType cfgType, XBAtom atom, XBVersion *ver);

extern XBBool SendGameConfig   (CFGType, XBSndQueue *sndQueue, XBTeleCOT cot, XBTeleIOB iob, XBAtom atom);
extern void AddToGameConfig    (CFGType, XBAtom atom, const char *text);
extern void DeleteGameConfig   (CFGType, XBAtom);
extern const char *GetHostName (CFGType, XBAtom);
extern XBBool RetrieveIpHistory (CFGGameHost game[10], XBAtom atom);
extern void StoreIpHistory (CFGGameHost *host, XBAtom atom);
 

#endif
/*
 * end of file cfg_game.h
 */
