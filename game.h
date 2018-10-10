/*
 * file game.h - rin the game 
 *
 * $Id: game.h,v 1.4 2004/08/04 04:46:15 iskywalker Exp $
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
#ifndef XBLAST_GAME_H
#define XBLAST_GAME_H

#include "action.h"
#include "cfg_game.h"
#include "cfg_player.h"
#include "ini_file.h"

/*
 * global macrors
 */
#define PAUSE_DELAY (TIME_STEP/2)

/*
 * global types
 */
typedef struct {
  CFGGame   game;
  CFGPlayer player[MAX_PLAYER];
} XBGameConfig;

/*
 * global prototypes
 */
extern void   	   ClearPlayerAction (PlayerAction *);
extern XBBool 	   GameEventLoop (XBEventCode eCode, XBEventData *eData);
extern XBBool 	   PauseEvalAction (int numPlayer, PlayerAction *playerAction, int pauseStatus);
extern void   	   GameTurn (int gameTime, int numPlayer, int *numActive);
extern int    	   GameEvalAction (int numPlayer, PlayerAction *playerAction);
extern void        GameUpdateWindow (void);
extern void        LevelBegin (const char *name);
extern const char *LevelResult (int gameTime, int *lastTeam, int numPlayers, const DBRoot *level, XBBool store);

extern XBBool InitGame (XBPlayerHost, CFGType, const CFGGame *, PlayerAction *);
extern void FinishGame (const CFGGame *);
extern int  GetNumOfPlayers (void);

#endif
/*
 * end of file game.h
 */
