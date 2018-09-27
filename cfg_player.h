/*
 * cfg_player.h - player configuration data
 * 
 * $Id: cfg_player.h,v 1.7 2004/10/15 22:08:56 lodott Exp $
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
#ifndef XBLAST_CFG_PLAYER_H
#define XBLAST_CFG_PLAYER_H

#include "event.h"
#include "color.h"
#include "net_tele.h"
#include "cfg_main.h"

/*
 * macros
 */
#define NUM_PLAYER_COLORS   7
#define NUM_DEFAULT_PLAYERS 6

/*
 * type definitions
 */

/* player graphics */
typedef struct {
  XBAtom  shape;
  XBColor helmet;
  XBColor face;
  XBColor body;
  XBColor handsFeet;
  XBColor armsLegs;
  XBColor backpack;
  XBColor white; /* needed for convenience */
  XBColor bodySave;
  XBColor handsFeetSave;
} CFGPlayerGraphics;

/* player messages */
typedef struct {
  const char *msgWinLevel;
  const char *msgWinGame;
  const char *msgLoseLife;
  const char *msgLoseLevel;
  const char *msgLaola;
  const char *msgLoser;
  const char *msgGloat;
  const char *msgWelcome;
} CFGPlayerMessages;

/* misc player options */
typedef struct {
  XBBool useStopKey;
  int    turnStepKeyboard;
  int    turnStepJoystick;
} CFGPlayerMisc;

/* player identity */
typedef struct {
  int        PID;
  const char *pass;
} CFGPlayerID;

/* player rating */
typedef struct {
  double     rating;
  int        gamesPlayed;
  int        realWins;
  int        relativeWins;
  time_t     timeUpdate;
  time_t     timeRegister;
} CFGPlayerRating;

/* all in one */
typedef struct {
  CFGPlayerGraphics graphics;
  CFGPlayerMessages messages;
  CFGPlayerMisc     misc;
  CFGPlayerID       id;
  const char       *name;
} CFGPlayer;

/* all in one */
typedef struct {
  CFGPlayerGraphics graphics;
  CFGPlayerMessages messages;
  CFGPlayerMisc     misc;
  CFGPlayerID       id;
  CFGPlayerRating   rating;
  const char       *name;
} CFGPlayerEx;

/*
 * function prototypes
 */
extern void   LoadPlayerCentral (XBBool amCentral); // XBCC 
extern void   SavePlayerCentral (void);
extern void   FinishPlayerCentral (void);

extern void   LoadPlayerConfig (void);
extern void   SavePlayerConfig (void);
extern void   FinishPlayerConfig (void);

extern void   RemoveAllPlayers (CFGType);

extern XBBool ComparePlayerGraphics       (const CFGPlayerGraphics *, const CFGPlayerGraphics *);

extern int         GetNumPlayerConfigs (CFGType);
extern XBAtom      GetPlayerAtom       (CFGType, int index);
extern const char *GetPlayerName       (CFGType, XBAtom atom);
extern int FindDoubleName(CFGType cfgType, XBAtom newplayer);

extern void   StorePlayer         (CFGType, XBAtom, const CFGPlayer *);
extern void   StorePlayerEx       (CFGType, XBAtom, const CFGPlayerEx *); // XBCC extended player format
extern void   StorePlayerRating   (CFGType, XBAtom, const CFGPlayerRating *); // XBCC extended player format
extern void   StorePlayerID       (CFGType, XBAtom, const CFGPlayerID *); // XBCC
extern void   StorePlayerGraphics (CFGType, XBAtom, const CFGPlayerGraphics *);
extern void   StorePlayerMessages (CFGType, XBAtom, const CFGPlayerMessages *);
extern void   StorePlayerMisc     (CFGType, XBAtom, const CFGPlayerMisc *);

extern XBBool RetrievePlayer         (CFGType, XBAtom, XBColor, CFGPlayer *);
extern XBBool RetrievePlayerEx       (CFGType, XBAtom, CFGPlayerEx *); // XBCC extended player format
extern XBBool RetrievePlayerRating   (CFGType, XBAtom, CFGPlayerRating *); // XBCC extended player format
extern XBBool RetrievePlayerID       (CFGType, XBAtom, CFGPlayerID *); // XBCC
extern XBBool RetrievePlayerMessages (CFGType, XBAtom, CFGPlayerMessages *);
extern XBBool RetrievePlayerGraphics (CFGType, XBAtom, XBColor, CFGPlayerGraphics *);
extern XBBool RetrievePlayerMisc     (CFGType, XBAtom, CFGPlayerMisc *);

extern XBAtom CreateNewPlayerConfig (CFGType, const char *name);
extern XBAtom RenamePlayerConfig    (CFGType, XBAtom atom, const char *name);
extern void   DeletePlayerConfig    (CFGType, XBAtom atom);
extern XBBool SendPlayerConfig      (CFGType, XBSndQueue *sndQueue, XBTeleCOT cot, XBTeleIOB iob, XBAtom atom, XBBool toCentral);
extern void   AddToPlayerConfig     (CFGType, XBAtom atom, const char *text);

extern XBBool RetrievePlayerID (CFGType cfgType, XBAtom atom, CFGPlayerID *);
extern void StorePlayerID (CFGType cfgType, XBAtom atom, const CFGPlayerID *);

extern void StoreGameResult (CFGType cfgType, XBAtom atom, int k, int *regPl, int *PID, int *Score); // XBST
extern void AppendGameResult (CFGType cfgType, XBAtom fname, XBAtom atom, int k, int *regPl, int *PID, int *Score);

extern void StoreTimePlayerRating (CFGType cfgType, XBAtom atom, int k, int *regPl, int *PID, float *rating); // XBST
extern void AppendTimePlayerRating (CFGType cfgType, XBAtom fname, XBAtom atom, int k, int *regPl, int *PID, float *rating);

/*
 * global variables
 */
extern const CFGPlayerGraphics *DefaultPlayerGraphics (size_t);
#endif
/*
 * end of file cfg_player.h
 */
