/*
 * file atom .c - predefined atoms fast database access
 *
 * $Id: atom.c,v 1.16 2005/01/11 22:44:41 lodott Exp $
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
#include "atom.h"

/*
 * global variables
 */
XBAtom atomAllLevels;
XBAtom atomAllowNat;
XBAtom atomArmsLegs;
XBAtom atomAuthor;
XBAtom atomBackpack;
XBAtom atomBeep;
XBAtom atomBody;
XBAtom atomBombClick;
XBAtom atomBombs;
XBAtom atomBot;
XBAtom atomBottom;
XBAtom atomBrowseLan;
XBAtom atomCentral; // XBCC
XBAtom atomCentralJoinName; // XBCC
XBAtom atomCentralJoinPort; // XBCC
XBAtom atomCentralLocal; // XBCC
XBAtom atomCentralRemote; // XBCC
XBAtom atomClient;
XBAtom atomColor;
XBAtom atomControl;
XBAtom atomDarkText1;
XBAtom atomDarkText2;
XBAtom atomDefault;
XBAtom atomDemo;
XBAtom atomDirection;
XBAtom atomDrawGame;
XBAtom atomExtra;
XBAtom atomExtraDistribution;
XBAtom atomFace;
XBAtom atomFixedUdpPort;
XBAtom atomFont;
XBAtom atomFontMini;
XBAtom atomFrameRate;
XBAtom atomFrames;
XBAtom atomFunc;
XBAtom atomFuseTime;
XBAtom atomGame;
XBAtom atomGamehis[10];
XBAtom atomGameMode;
XBAtom atomGeneral;
XBAtom atomGeometry;
XBAtom atomGraphics;
XBAtom atomHandsFeet;
XBAtom atomHaunt;
XBAtom atomHelmet;
XBAtom atomHidden;
XBAtom atomHint;
XBAtom atomHost;
XBAtom atomIfRecLives;
XBAtom atomInfo;
XBAtom atomInfoTime; // LRF
XBAtom atomInitExtra;
XBAtom atomInitVirus;
XBAtom atomKey;
XBAtom atomKeyAbort;
XBAtom atomKeyAbortCancel;
    /* Skywalker */
XBAtom atomKeyLaola; 
XBAtom atomKeyLooser;
XBAtom atomKeyBot;
XBAtom atomKeyChatStart;
XBAtom atomKeyChatChangeReceiver;
XBAtom atomKeyChatSend;
XBAtom atomKeyChatCancel;
    /* */
XBAtom atomKeyBomb;
XBAtom atomKeyDown;
XBAtom atomKeyLeft;
XBAtom atomKeyPause;
XBAtom atomKeyRight;
XBAtom atomKeySpecial;
XBAtom atomKeyStop;
XBAtom atomKeyUp;
XBAtom atomLarge;
XBAtom atomLastPlayed; // LRF
XBAtom atomLeft;
XBAtom atomLeftKeyboard;
XBAtom atomLevel;
XBAtom atomLevelOrder; // LRF
XBAtom atomLevelShuffled;
XBAtom atomLevelSorted;
XBAtom atomLevelStat;
XBAtom atomLightText1;
XBAtom atomLightText2;
XBAtom atomLives;
XBAtom atomLocal;
XBAtom atomMap;
XBAtom atomMedium;
XBAtom atomMissing;
XBAtom atomMode;
XBAtom atomMsgGloat;
XBAtom atomMsgLaola;
XBAtom atomMsgLoseLevel;
XBAtom atomMsgLoseLife;
XBAtom atomMsgLoser;
XBAtom atomMsgWelcome;
XBAtom atomMsgWinGame;
XBAtom atomMsgWinLevel;
XBAtom atomMusic;
XBAtom atomName;
XBAtom atomNastyCeil;
XBAtom atomNastyGentle;
XBAtom atomNastyRange;
XBAtom atomNextNasty;
XBAtom atomNumBlocks;
XBAtom atomNumFrames;
XBAtom atomNumPlayers;
XBAtom atomOutOfTime;
XBAtom atomPlayer;
XBAtom atomPass; // XBCC
XBAtom atomPID; // XBCC
XBAtom atomPlayerClick;
XBAtom atomPlayerRating; // XBCC
XBAtom atomPlayerStat;
XBAtom atomPort;
XBAtom atomPorthis[10];
XBAtom atomProbBomb;
XBAtom atomProbHidden;
XBAtom atomProbRange;
XBAtom atomProbSpecial;
XBAtom atomProbVirus;
XBAtom atomRandomLevels;
XBAtom atomRandomPlayers;
XBAtom atomRandomSeed;
XBAtom atomRange;
XBAtom atomRatedGame; // XBCC
XBAtom atomRecLives;
XBAtom atomRecordDemo;
XBAtom atomRecorded;
XBAtom atomRemote;
XBAtom atomRemoteGame;
XBAtom atomRemotePlayer;
XBAtom atomResults;
XBAtom atomReviveExtra;
XBAtom atomReviveVirus;
XBAtom atomRight;
XBAtom atomRightKeyboard;
XBAtom atomScrambleDel;
XBAtom atomScrambleDraw;
XBAtom atomSelect;
XBAtom atomServer;
XBAtom atomShape;
XBAtom atomShrink;
XBAtom atomShuffle;
XBAtom atomSlowFlame;
XBAtom atomSmall;
XBAtom atomSound;
XBAtom atomSpecial;
XBAtom atomSpecialBombs;
XBAtom atomStatusBg;
XBAtom atomStatusFg;
XBAtom atomStatusLed;
XBAtom atomStereo;
/* AbsInt begin */
XBAtom atomBeep;
/* AbsInt end */
XBAtom atomTeamMode;
XBAtom atomTime;
XBAtom atomTimeRatings;
XBAtom atomTitleBg;
XBAtom atomTitleFg;
XBAtom atomTop;
XBAtom atomTotal;
XBAtom atomTurnStepKeyboard;
XBAtom atomTurnStepJoystick;
XBAtom atomType;
XBAtom atomUseStopKey;
XBAtom atomVersionMajor;
XBAtom atomVersionMinor;
XBAtom atomVersionPatch;
XBAtom atomWallClick;
XBAtom atomWin32;
XBAtom atomWinner;
XBAtom atomWins;
XBAtom atomX11;
XBAtom atomXBCCRating;       // XBCC
XBAtom atomXBCCGamesPlayed;  // XBCC
XBAtom atomXBCCRealWins;     // XBCC
XBAtom atomXBCCRelativeWins; // XBCC
XBAtom atomXBCCTimeUpdate;   // XBCC
XBAtom atomXBCCTimeRegister; // XBCC
XBAtom atomXblast;

XBAtom atomArrayBlock00[MAX_BLOCK];
XBAtom atomArrayControl0[MAX_PLAYER+1];
XBAtom atomArrayHost0[MAX_HOSTS];
XBAtom atomArrayPlayer0[MAX_PLAYER+1];
XBAtom atomArrayPos0[MAX_PLAYER+1];
XBAtom atomArrayPos000[MAZE_W*MAZE_H];
XBAtom atomArrayRow00[MAZE_H];
XBAtom atomArrayTeam0[MAX_PLAYER+1];

/*
 * initialize predefined atoms
 */
void
InitDefaultAtoms (void)
{
  int  i;
  /* allocate buffer long enough to hold "game9"/0 and "port9"/0 */
  char string[9];

  /* skalars */
  atomAllLevels      	= GUI_StringToAtom ("allLevels");
  atomAllowNat      	= GUI_StringToAtom ("allowNat");
  atomArmsLegs      	= GUI_StringToAtom ("armsLegs");
  atomAuthor        	= GUI_StringToAtom ("author");
  atomAuthor        	= GUI_StringToAtom ("author");
  atomBackpack      	= GUI_StringToAtom ("backpack");
  atomBeep         	= GUI_StringToAtom ("beep");
  atomBody          	= GUI_StringToAtom ("body");
  atomBombClick       	= GUI_StringToAtom ("bombClick");
  atomBombs         	= GUI_StringToAtom ("bombs");
  atomBot         	= GUI_StringToAtom ("bot");
  atomBottom         	= GUI_StringToAtom ("bottom");
  atomBrowseLan        	= GUI_StringToAtom ("browseLan");
  atomServer            = GUI_StringToAtom ("server");
  atomClient            = GUI_StringToAtom ("client");
  atomCentral           = GUI_StringToAtom ("central"); // XBCC
  atomCentralJoinName   = GUI_StringToAtom ("centraljoinname"); // XBCC
  atomCentralJoinPort   = GUI_StringToAtom ("centraljoinport"); // XBCC
  atomCentralLocal      = GUI_StringToAtom ("localStat"); // XBCC
  atomCentralRemote     = GUI_StringToAtom ("remoteStat"); // XBCC
  atomColor             = GUI_StringToAtom ("color");
  atomControl           = GUI_StringToAtom ("control");
  atomDarkText1         = GUI_StringToAtom ("darkText1");
  atomDarkText2         = GUI_StringToAtom ("darkText2");
  atomDefault      	= GUI_StringToAtom ("default");
  atomDemo      	= GUI_StringToAtom ("demo");
  atomDirection    	= GUI_StringToAtom ("direction");
  atomDrawGame    	= GUI_StringToAtom ("drawGame");
  atomExtra             = GUI_StringToAtom ("extra");
  atomExtraDistribution = GUI_StringToAtom ("extraDistribution");
  atomFace          	= GUI_StringToAtom ("face");
  atomFixedUdpPort     	= GUI_StringToAtom ("fixedUdpPort");
  atomFont              = GUI_StringToAtom ("font");
  atomFontMini          = GUI_StringToAtom ("fontMini");
  atomFrameRate         = GUI_StringToAtom ("frameRate");
  atomFrames            = GUI_StringToAtom ("frames");
  atomFunc          	= GUI_StringToAtom ("func");
  atomFuseTime     	= GUI_StringToAtom ("fuseTime");
  atomGame              = GUI_StringToAtom ("game");
  
  for(i=0;i<10;i++){
    sprintf(string,"game%i",i);
    atomGamehis[ i]             = GUI_StringToAtom (string);
    sprintf(string,"port%i",i);
    atomPorthis[i]    = GUI_StringToAtom (string);
  }

  atomGameMode      	= GUI_StringToAtom ("gameMode");
  atomGeneral      	= GUI_StringToAtom ("general");
  atomGeometry      	= GUI_StringToAtom ("geometry");
  atomGraphics      	= GUI_StringToAtom ("graphics");
  atomHandsFeet     	= GUI_StringToAtom ("handsFeet");
  atomHaunt        	= GUI_StringToAtom ("haunt");
  atomHelmet        	= GUI_StringToAtom ("helmet");
  atomHidden       	= GUI_StringToAtom ("hidden");
  atomHint          	= GUI_StringToAtom ("hint");
  atomHost          	= GUI_StringToAtom ("host");
  atomIfRecLives        = GUI_StringToAtom ("ifRecLives");
  atomInfo          	= GUI_StringToAtom ("info");
  atomInfoTime     	= GUI_StringToAtom ("infoTime");
  atomInitExtra         = GUI_StringToAtom ("initExtra");
  atomInitVirus     	= GUI_StringToAtom ("initVirus");
  atomKey         	= GUI_StringToAtom ("key");
  atomKeyAbort 	     	= GUI_StringToAtom ("keyAbort");
  atomKeyAbortCancel 	= GUI_StringToAtom ("keyAbortCancel");
    /* Skywalker */
  atomKeyLaola 	     	= GUI_StringToAtom ("keyLaola");
  atomKeyLooser	     	= GUI_StringToAtom ("keyLooser");
  atomKeyBot	     	= GUI_StringToAtom ("keyBot");
  atomKeyChatCancel	= GUI_StringToAtom ("keyChatCancel");
  atomKeyChatChangeReceiver	= GUI_StringToAtom ("keyChatChangeReceiver");
  atomKeyChatStart      = GUI_StringToAtom ("keyChatStart");
  atomKeyChatSend	= GUI_StringToAtom ("keyChatSend");
    /* */
  atomKeyBomb  	     	= GUI_StringToAtom ("keyBomb");
  atomKeyDown  	     	= GUI_StringToAtom ("keyDown");
  atomKeyLeft  	     	= GUI_StringToAtom ("keyLeft");
  atomKeyPause 	     	= GUI_StringToAtom ("keyPause");
  atomKeyRight 	     	= GUI_StringToAtom ("keyRight");
  atomKeySpecial     	= GUI_StringToAtom ("keySpecial");
  atomKeyStop  	     	= GUI_StringToAtom ("keyStop");
  atomKeyUp    	     	= GUI_StringToAtom ("keyUp");
  atomLarge             = GUI_StringToAtom ("large");
  atomLastPlayed        = GUI_StringToAtom ("lastPlayed"); // LRF
  atomLeft              = GUI_StringToAtom ("left");
  atomLeftKeyboard      = GUI_StringToAtom ("leftKeyboard");
  atomLevel             = GUI_StringToAtom ("level");
  atomLevelOrder        = GUI_StringToAtom ("levelOrder"); // LRF
  atomLevelShuffled    	= GUI_StringToAtom ("levelShuffled");
  atomLevelSorted     	= GUI_StringToAtom ("levelSorted");
  atomLevelStat     	= GUI_StringToAtom ("levelStat");
  atomLightText1        = GUI_StringToAtom ("lightText1");
  atomLightText2        = GUI_StringToAtom ("lightText2");
  atomLives         	= GUI_StringToAtom ("lives");
  atomLocal             = GUI_StringToAtom ("local");
  atomMap           	= GUI_StringToAtom ("map");
  atomMedium            = GUI_StringToAtom ("medium");
  atomMissing           = GUI_StringToAtom ("missing");
  atomMode              = GUI_StringToAtom ("mode");
  atomMsgGloat          = GUI_StringToAtom ("msgGloat");
  atomMsgLaola          = GUI_StringToAtom ("msgLaola");
  atomMsgLoseLevel      = GUI_StringToAtom ("msgLoseLevel");
  atomMsgLoseLife       = GUI_StringToAtom ("msgLoseLife");
  atomMsgLoser          = GUI_StringToAtom ("msgLoser");
  atomMsgWelcome        = GUI_StringToAtom ("msgWelcome");
  atomMsgWinGame        = GUI_StringToAtom ("msgWinGame");
  atomMsgWinLevel       = GUI_StringToAtom ("msgWinLevel");
  atomMusic             = GUI_StringToAtom ("music");
  atomName          	= GUI_StringToAtom ("name");
  atomNastyCeil     	= GUI_StringToAtom ("nastyCeil");
  atomNastyGentle     	= GUI_StringToAtom ("nastyGentle");
  atomNastyRange     	= GUI_StringToAtom ("nastyRange");
  atomNextNasty     	= GUI_StringToAtom ("nextNasty");
  atomNumBlocks     	= GUI_StringToAtom ("numBlocks");
  atomNumFrames     	= GUI_StringToAtom ("numFrames");
  atomNumPlayers     	= GUI_StringToAtom ("numPlayers");
  atomOutOfTime        	= GUI_StringToAtom ("outOfTime");
  atomPass        	= GUI_StringToAtom ("pass"); // XBCC
  atomPID        	= GUI_StringToAtom ("PID"); // XBCC
  atomPlayer        	= GUI_StringToAtom ("player");
  atomPlayerClick  	= GUI_StringToAtom ("playerClick");
  atomPlayerRating	= GUI_StringToAtom ("playerRating"); // XBCC
  atomPlayerStat   	= GUI_StringToAtom ("playerStat");
  atomPort        	= GUI_StringToAtom ("port");
  atomProbBomb      	= GUI_StringToAtom ("probBomb");
  atomProbHidden    	= GUI_StringToAtom ("probHidden");
  atomProbRange     	= GUI_StringToAtom ("probRange");
  atomProbSpecial   	= GUI_StringToAtom ("probSpecial");
  atomProbVirus     	= GUI_StringToAtom ("probVirus");
  atomRandomLevels  	= GUI_StringToAtom ("randomLevels");
  atomRandomPlayers 	= GUI_StringToAtom ("randomPlayers");
  atomRandomSeed 	= GUI_StringToAtom ("randomSeed");
  atomRatedGame 	= GUI_StringToAtom ("ratedGame"); // XBCC
  atomRange         	= GUI_StringToAtom ("range");
  atomRecLives          = GUI_StringToAtom ("reclives");
  atomRecordDemo     	= GUI_StringToAtom ("recordDemo");
  atomRecorded     	= GUI_StringToAtom ("recorded");
  atomRemote     	= GUI_StringToAtom ("remote");
  atomRemoteGame     	= GUI_StringToAtom ("remoteGame");
  atomRemotePlayer     	= GUI_StringToAtom ("remotePlayer");
  atomResults           = GUI_StringToAtom ("results");
  atomReviveExtra       = GUI_StringToAtom ("reviveExtra");
  atomReviveVirus   	= GUI_StringToAtom ("reviveVirus");
  atomRight             = GUI_StringToAtom ("right");
  atomRightKeyboard     = GUI_StringToAtom ("rightKeyboard");
  atomScrambleDel     	= GUI_StringToAtom ("scrambleDel");
  atomScrambleDraw     	= GUI_StringToAtom ("scrambleDraw");
  atomSelect        	= GUI_StringToAtom ("select");
  atomServer        	= GUI_StringToAtom ("server");
  atomShape         	= GUI_StringToAtom ("shape");
  atomShrink        	= GUI_StringToAtom ("shrink");
  atomShuffle        	= GUI_StringToAtom ("shuffle");
  atomSlowFlame         = GUI_StringToAtom ("slowMotionBurst");
  atomSmall             = GUI_StringToAtom ("small");
  atomSound             = GUI_StringToAtom ("sound");
  atomSpecial      	= GUI_StringToAtom ("special");
  atomSpecialBombs  	= GUI_StringToAtom ("specialBombs");
  atomStatusBg          = GUI_StringToAtom ("statusBg");
  atomStatusFg          = GUI_StringToAtom ("statusFg");
  atomStatusLed         = GUI_StringToAtom ("statusLed");
  atomStereo            = GUI_StringToAtom ("stereo");
  /* AbsInt begin */
  atomBeep              = GUI_StringToAtom ("beep");
  /* AbsInt end */
  atomTeamMode         	= GUI_StringToAtom ("teamMode");
  atomTime          	= GUI_StringToAtom ("time");
  atomTimeRatings       = GUI_StringToAtom ("ratings");
  atomTitleBg           = GUI_StringToAtom ("titleBg");
  atomTitleFg           = GUI_StringToAtom ("titleFg");
  atomTurnStepKeyboard 	= GUI_StringToAtom ("turnStepKeyboard");
  atomTurnStepJoystick 	= GUI_StringToAtom ("turnStepJoystick");
  atomTop          	= GUI_StringToAtom ("top");
  atomTotal          	= GUI_StringToAtom ("total");
  atomType          	= GUI_StringToAtom ("type");
  atomUseStopKey    	= GUI_StringToAtom ("useStopKey");
  atomVersionMajor      = GUI_StringToAtom ("major");
  atomVersionMinor      = GUI_StringToAtom ("minor");
  atomVersionPatch      = GUI_StringToAtom ("patch");
  atomWallClick    	= GUI_StringToAtom ("wallClick");
  atomWin32         	= GUI_StringToAtom ("win32");
  atomWinner         	= GUI_StringToAtom ("winner");
  atomWins          	= GUI_StringToAtom ("wins");
  atomX11               = GUI_StringToAtom ("x11");
  atomXblast            = GUI_StringToAtom ("xblast");
  atomXBCCRating        = GUI_StringToAtom ("XBCCRating");       // XBCC
  atomXBCCGamesPlayed   = GUI_StringToAtom ("XBCCGamesPlayed");
  atomXBCCRealWins      = GUI_StringToAtom ("XBCCRealWins");
  atomXBCCRelativeWins  = GUI_StringToAtom ("XBCCRelativeWins");
  atomXBCCTimeUpdate    = GUI_StringToAtom ("XBCCTimeUpdate");
  atomXBCCTimeRegister  = GUI_StringToAtom ("XBCCTimeRegister");


  /* arrays */
  for (i = 0; i < MAX_BLOCK; i ++) {
    atomArrayBlock00[i] = GUI_FormatToAtom ("block%02d", i);
  }
  for (i = 0; i < MAX_HOSTS; i ++) {
    atomArrayHost0[i] = GUI_FormatToAtom ("host%01d", i); 
  }
  for (i = 0; i <= MAX_PLAYER; i ++) {
    atomArrayControl0[i] = GUI_FormatToAtom ("control%1d", i);
    atomArrayPos0[i]     = GUI_FormatToAtom ("pos%1d",     i);
    atomArrayPlayer0[i]  = GUI_FormatToAtom ("player%1d",  i);
    atomArrayTeam0[i]    = GUI_FormatToAtom ("team%1d",    i); 
  }
  for (i = 0; i < MAZE_W * MAZE_H; i ++) {
    atomArrayPos000[i] = GUI_FormatToAtom ("pos%03d", i);
  }
  for (i = 0; i < MAZE_H; i ++) {
    atomArrayRow00[i] = GUI_FormatToAtom ("row%02d", i);
  }
} /* InitDefaultAtoms */

/*
 * end of file atom.c
 */
