/*
 * file atom.h - predefined atoms fast database access
 *
 * $Id: atom.h,v 1.15 2005/01/11 22:44:41 lodott Exp $
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
#ifndef XBLAST_ATOM_H
#define XBLAST_ATOM_H

#include "gui.h"

/*
 * global variables
 */
extern XBAtom atomAllLevels;
extern XBAtom atomAllowNat;
extern XBAtom atomArmsLegs;
extern XBAtom atomAuthor;
extern XBAtom atomAuthor;
extern XBAtom atomBackpack;
extern XBAtom atomBeep;
extern XBAtom atomBody;
extern XBAtom atomBombClick;
extern XBAtom atomBombs;
extern XBAtom atomBot;
extern XBAtom atomBottom;
extern XBAtom atomBrowseLan;
extern XBAtom atomCentral; // XBCC
extern XBAtom atomCentralJoinName; // XBCC
extern XBAtom atomCentralJoinPort; // XBCC
extern XBAtom atomCentralLocal; // XBCC
extern XBAtom atomCentralRemote; // XBCC
extern XBAtom atomClient;
extern XBAtom atomColor;
extern XBAtom atomControl;
extern XBAtom atomDarkText1;
extern XBAtom atomDarkText2;
extern XBAtom atomDefault;
extern XBAtom atomDemo;
extern XBAtom atomDirection;
extern XBAtom atomDrawGame;
extern XBAtom atomExtra;
extern XBAtom atomExtraDistribution;
extern XBAtom atomFace;
extern XBAtom atomFixedUdpPort;
extern XBAtom atomFont;
extern XBAtom atomFontMini;
extern XBAtom atomFrameRate;
extern XBAtom atomFrames;
extern XBAtom atomFunc;
extern XBAtom atomFuseTime;
extern XBAtom atomGame;
extern XBAtom atomGamehis[10];
extern XBAtom atomGameMode;
extern XBAtom atomGameMode;
extern XBAtom atomGeneral;
extern XBAtom atomGeometry;
extern XBAtom atomGraphics;
extern XBAtom atomHandsFeet;
extern XBAtom atomHaunt;
extern XBAtom atomHelmet;
extern XBAtom atomHidden;
extern XBAtom atomHint;
extern XBAtom atomHost;
extern XBAtom atomIfRecLives;
extern XBAtom atomInfo;
extern XBAtom atomInfoTime; // LRF
extern XBAtom atomInitExtra;
extern XBAtom atomInitVirus;
extern XBAtom atomKey;
extern XBAtom atomKeyAbort;
extern XBAtom atomKeyAbortCancel;
extern XBAtom atomKeyBomb;
extern XBAtom atomKeyDown;
extern XBAtom atomKeyLeft;
extern XBAtom atomKeyPause;
extern XBAtom atomKeyRight;
extern XBAtom atomKeySpecial;
extern XBAtom atomKeyStop;
extern XBAtom atomKeyUp;
extern XBAtom atomLarge;
extern XBAtom atomLastPlayed;  // LRF Better random leves (hopefully)
extern XBAtom atomLeft;
extern XBAtom atomLeftKeyboard;
extern XBAtom atomLevel;
extern XBAtom atomLevelOrder; // LRF
extern XBAtom atomLevelShuffled;
extern XBAtom atomLevelSorted;
extern XBAtom atomLevelStat;
extern XBAtom atomLightText1;
extern XBAtom atomLightText2;
extern XBAtom atomLives;
extern XBAtom atomLocal;
extern XBAtom atomMap;
extern XBAtom atomMedium;
extern XBAtom atomMissing;
extern XBAtom atomMode;
extern XBAtom atomMsgGloat;
extern XBAtom atomMsgLaola;
extern XBAtom atomMsgLoseLevel;
extern XBAtom atomMsgLoseLife;
extern XBAtom atomMsgLoser;
extern XBAtom atomMsgWelcome;
extern XBAtom atomMsgWinGame;
extern XBAtom atomMsgWinLevel;
extern XBAtom atomMusic;
extern XBAtom atomName;
extern XBAtom atomNastyCeil;
extern XBAtom atomNastyGentle;
extern XBAtom atomNastyRange;
extern XBAtom atomNextNasty;
extern XBAtom atomNumBlocks;
extern XBAtom atomNumFrames;
extern XBAtom atomNumPlayers;
extern XBAtom atomOutOfTime;
extern XBAtom atomPass; // XBCC
extern XBAtom atomPID; // XBCC
extern XBAtom atomPlayer;
extern XBAtom atomPlayerClick;
extern XBAtom atomPlayerRating; // XBCC
extern XBAtom atomPlayerStat;
extern XBAtom atomPort;
extern XBAtom atomPorthis[10];
extern XBAtom atomProbBomb;
extern XBAtom atomProbHidden;
extern XBAtom atomProbRange;
extern XBAtom atomProbSpecial;
extern XBAtom atomProbVirus;
extern XBAtom atomRandomLevels;
extern XBAtom atomRandomPlayers;
extern XBAtom atomRandomSeed;
extern XBAtom atomRange;
extern XBAtom atomRatedGame; // XBCC
extern XBAtom atomRecLives;
extern XBAtom atomRecordDemo;
extern XBAtom atomRecorded;
extern XBAtom atomRemote;
extern XBAtom atomRemoteGame;
extern XBAtom atomRemotePlayer;
extern XBAtom atomResults;
extern XBAtom atomReviveExtra;
extern XBAtom atomReviveVirus;
extern XBAtom atomRight;
extern XBAtom atomRightKeyboard;
extern XBAtom atomScrambleDel;
extern XBAtom atomScrambleDraw;
extern XBAtom atomSelect;
extern XBAtom atomServer;
extern XBAtom atomShape;
extern XBAtom atomShrink;
extern XBAtom atomShuffle;
extern XBAtom atomSlowFlame;
extern XBAtom atomSmall;
extern XBAtom atomSound;
extern XBAtom atomSpecial;
extern XBAtom atomSpecialBombs;
extern XBAtom atomStatusBg;
extern XBAtom atomStatusFg;
extern XBAtom atomStatusLed;
extern XBAtom atomStereo;
/* AbsInt begin */
extern XBAtom atomBeep;
/* AbsInt end */
extern XBAtom atomTeamMode;
extern XBAtom atomTime;
extern XBAtom atomTimeRatings;
extern XBAtom atomTitleBg;
extern XBAtom atomTitleFg;
extern XBAtom atomTop;
extern XBAtom atomTotal;
extern XBAtom atomTurnStepKeyboard;
extern XBAtom atomTurnStepJoystick;
extern XBAtom atomType;
extern XBAtom atomUseStopKey;
extern XBAtom atomVersionMajor;
extern XBAtom atomVersionMinor;
extern XBAtom atomVersionPatch;
extern XBAtom atomWallClick;
extern XBAtom atomWin32;
extern XBAtom atomWinner;
extern XBAtom atomWins;
extern XBAtom atomX11;
extern XBAtom atomXBCCRating;      // XBCC
extern XBAtom atomXBCCGamesPlayed;
extern XBAtom atomXBCCRealWins;
extern XBAtom atomXBCCRelativeWins;
extern XBAtom atomXBCCTimeUpdate;
extern XBAtom atomXBCCTimeRegister;
extern XBAtom atomXblast;

    /* Skywalker */
extern XBAtom atomKeyLaola 	 ;
extern XBAtom atomKeyLooser	  ;
extern XBAtom atomKeyBot	  ;

extern XBAtom atomKeyChatStart;
extern XBAtom atomKeyChatSend;
extern XBAtom atomKeyChatCancel;
extern XBAtom atomKeyChatChangeReceiver;
    /* */
extern XBAtom atomArrayBlock00[MAX_BLOCK];
extern XBAtom atomArrayControl0[MAX_PLAYER+1];
extern XBAtom atomArrayHost0[MAX_HOSTS];
extern XBAtom atomArrayPlayer0[MAX_PLAYER+1];
extern XBAtom atomArrayPos0[MAX_PLAYER+1];
extern XBAtom atomArrayPos000[MAZE_W*MAZE_H];
extern XBAtom atomArrayRow00[MAZE_H];
extern XBAtom atomArrayTeam0[MAX_PLAYER+1];

/*
 * global prototypes
 */
extern void InitDefaultAtoms (void);

#endif
/*
 * end of file atom.h
 */
