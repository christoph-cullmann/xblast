/*
 * file player.c - ingame player mangment 
 *
 * $Id: player.h,v 1.16 2004/11/29 14:44:49 lodott Exp $
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
#ifndef XBLAST_PLAYER_H
#define XBLAST_PLAYER_H

#include "sprite.h"
#include "cfg_game.h"
#include "cfg_player.h"
#include "ini_file.h"
#include "action.h"

/*
 * global macros
 */
#define TELEPORT_TIME     20 /* Define > 1 */
#define EXTRA_GHOST_TIME 256
#define EXTRA_THROUGH_TIME 64
#define MAX_JUNKIE_TIME (384 + GameRandomNumber(31))
#define NEW_INVINCIBLE    64
/* AbsInt */
#define ILLINIT_TIME      64
/* AbsInt end */
#define EXTRA_INVINCIBLE 160
#define EXTRA_INVISIBLE  256
#define STUN_TIME         16
#define SMELLY_TIME 30
#define  DALEIF_TIME 5 /* (galatius) Sky */
#define BONUSEVIL (2*EXTRA_INVISIBLE)
#define ILLDEATHTIME (GAME_TIME/8)
#define EXTRA_ELECTRIFY_COUNT 4
/*
 * player health
 */
typedef enum {
  Healthy = 0, IllBomb, IllSlow, IllRun, IllMini, IllEmpty, IllInvisible, 
  IllMalfunction, IllReverse, IllReverse2, IllTeleport,
  MAX_ILL
} BMHealth;

typedef enum {
  PM_Same = 0, PM_Polar, PM_Right, PM_Inner, PM_LeftRight, PM_Below, 
  PM_Horizontal, PM_Vertical, PM_Circle,
  MAX_PM
} BMPosMod;
extern int Original;
/*
 * type definitions
 */
typedef struct _bmplayer {
  int y, x;
  int id, team;
  int number;
  int local;
  int localDisplay;
  /* Added by VVL (Chat) 12/11/99 : Begin */
  char chatstring[CHAT_LEN];
  int chatlen;
  int chatmode;
  /* Added by VVL (Chat) 12/11/99 : End */
  Sprite *sprite;
  int disp;
  BMDirection d_soll,d_ist,d_look;
  int invincible;
  int dying;
  int stunned;
  BMHealth health;
  BMHealth illness;
  /* AbsInt start */
  int ai_revived;
  /* AbsInt end */
  int speed;
  int illtime;
  int junkie;
  int lives;
  int stop;
  int evilill;
  int score;
  int range;
  int choice_bomb_type ;
  int bombs;
  int PID; // XBCC
  int suck_button;
  unsigned int iniextra_flags;
  unsigned int revextra_flags;
  int special_bombs;
  XBBool remote_control;
  XBBool kick;
  int air_button;
  int victories;
  XBBool teleport;
  int cloaking;
  int num_extras;
  PlayerAbort abort;
  XBBool morphed;
  int num_morph;
  XBBool in_active;
  int jump_button; /* EPFL */
  /* Skywalker */
  int revive;
  int frogger;
  int laola;
  int looser;
  int ghost;
  XBBool bot;
  int num_snipe;
  int sniping;int daleif; /* Player will daleif (galatius,sky)*/
  int daleifing; /* Player has daleifed (galatius,sky)*/

  /* farter (galatius) */
  int farted; /* player has been farted on */
  int farter; /* player has the farter */
  int bfarter; /* player can fart bombs and others */
  int smelly;/* Just farted others */
  int electrify;
  int throughCount;
  int through;
  int phantom;
  /* */
  Sprite *targetSprite;
} BMPlayer;

typedef struct {
  char *name;
  char *tag;
  char *pause;
  char *winlevel;
  char *wingame;
  char *loselife;
  char *loselevel;
  char *gloat;
  char *loser;
  char *laola;
  char *welcome;
  char *abort;
  char *abortcancel;
} PlayerStrings;

/*
 * global variables
 */
extern BMPlayer player_stat[2*MAX_PLAYER];
extern PlayerStrings p_string[2*MAX_PLAYER];

/*
 * prototypes
 */
extern XBBool ParseLevelPlayers (const DBSection *section, unsigned gameMode, DBSection *warn);
extern void ConfigLevelPlayers (const DBSection *section, XBBool allowRandomPos, unsigned gameMode);
extern void WelcomePlayers (void);
extern int NumSpecialBombs (void);
extern void InitPlayers (XBPlayerHost, const CFGGame *, const CFGPlayer *);
extern void FinishPlayers (void);
extern void DeletePlayerSprites (void);
extern void DropBomb (BMPlayer *ps, int type);
extern void DoJunkie (void);
extern void InfectOtherPlayers (int *active_player);
extern void KillPlayerAtGhost (int block,int x, int y);
extern void KillPlayerAt (int x, int y);
extern int KillOtherPlayers (int team);
extern void SetMsgLaola  (int player);
extern void SetMsgLaola  (int player);
extern int StunOtherPlayers (int team, int time);
extern XBBool CheckPlayerNear (int x, int y);
extern void DoAllPlayers (int game_time, int *active_player);
extern void CheckPlayerHit (void);
extern void DoEvilIll (void);
extern int ElectrifyOtherPlayers(int nplayer);
extern int StealBombsOtherPlayers (int team);
extern int StealRangeOtherPlayers (int team);
extern void SetMsgLoser  (int player);
extern void SetMsgLaola  (int player);
extern int FartOnOtherPlayers (BMPlayer *ps);
extern void SwapColorOtherPlayers (int team);
extern void SwapPositionOtherPlayers (int team);

#endif
/*
 * end of file player.h
 */
