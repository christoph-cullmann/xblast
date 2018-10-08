/*
 * cfg_game.h - game configuration data
 *
 * $Id: cfg_game.h,v 1.18 2006/02/09 21:21:23 fzago Exp $
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

/* player controls */
typedef enum
{
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
typedef enum
{
	XBPH_None,
	XBPH_Demo,					/* host watches only */
	XBPH_Local,					/* local host, either local game or during network game setup */
	XBPH_Server,				/* server host, in network game only */
	XBPH_Client1,				/* numbered client host, in network game only */
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

/* player teams */
typedef enum
{
	XBPT_Invalid,
	XBPT_None,
	XBPT_Red,
	XBPT_Green,
	XBPT_Blue,
	/* no new elements after this line */
	XBPT_NUM
} XBPlayerTeam;

/* game player config  */
typedef struct
{
	int num;					/* number of players */
	XBAtom player[MAX_PLAYER];	/* section atoms for player graphics */
	XBPlayerControl control[MAX_PLAYER];
	XBPlayerHost host[MAX_PLAYER];
	XBPlayerTeam team[MAX_PLAYER];
	XBColor teamColor[MAX_PLAYER];
	int PID[MAX_PLAYER];		/* pid's for rated games */
	int playerID[MAX_PLAYER];	/* ??? */
} CFGGamePlayers;

/* team modes */
typedef enum
{
	XBTM_None,					/* normal game */
	XBTM_Team,					/* teams, assignment is random */
	XBTM_Hunt,					/* all on one (3 to 6 players) */
	/* no new elements after this line */
	XBTM_NUM
} XBTeamMode;

/* game parameters */
typedef struct
{
	XBBool ifRecLives;			/* should recommended lives for levels be used */
	int numLives;				/* standard number of lives in levels */
	int numWins;				/* wins necessary for game end */
	int frameRate;				/* frame rate */
	XBBool allLevels;			/* use all levels in game */
	XBBool randomLevels;		/* random level choice */
	XBBool randomPlayers;		/* random player positions */
	int levelOrder;				/* how to sort the levels */
	int infoTime;				/* time for level info */
	XBBool recordDemo;			/* should game be saved for demo view */
	XBTeamMode teamMode;		/* initial team mode */
	XBBool rated;				/* rated game or not */
	XBBool bot;					/* start in bot mode (all players) */
	XBBool beep;				/* beep for new connection, game start */
	SND_Id Music;				/* background music */
	int recLives;				/* default recommended lives count */
	int maskBytes;				/* action mask bytes to use */
} CFGGameSetup;

/* connection to host */
typedef struct
{
	const char *name;			/* host name */
	int port;					/* port number for tcp connection */
	XBBool fixedUdpPort;		/* use fixed udp ports in game */
	XBBool browseLan;			/* open socket for la broadcasts */
	XBBool allowNat;			/* allow clients behind NAT */
	XBBool central;				/* register game at central */
	XBBool beep;				/* beep for new clients */
	SND_Id Music;				/* background music */
	const char *game;			/* game name */
} CFGGameHost;

/* host constants */
typedef struct
{
	int maxhosts;				/* max number of hosts allowed */
	int maxplayers;				/* max number of total players allowed */
	int maxlocals;				/* max number of local players allowed */
	int maxbytes;				/* max number of mask bytes handled */
	XBVersion version;			/* version number */
} CFGGameConst;

/* all in one */
typedef struct
{
	CFGGameSetup setup;
	CFGGamePlayers players;
	CFGGameHost host;
	CFGGameConst constants;
} CFGGame;

/*
 * global prototypes
 */

/* handle all game configs */
extern void LoadGameConfig (void);
extern void SaveGameConfig (void);
extern void FinishGameConfig (void);

/* handle single game config */
extern void AddToGameConfig (CFGType, XBAtom atom, const char *text);
extern XBBool SendGameConfig (CFGType, XBSndQueue * sndQueue, XBTeleCOT cot, XBTeleIOB iob,
							  XBAtom atom);
extern void DeleteGameConfig (CFGType, XBAtom);

/* retrieve/store in a single game config */
extern XBBool RetrieveGame (CFGType, XBAtom atom, CFGGame *);
extern void StoreGame (CFGType, XBAtom atom, const CFGGame *);

extern XBBool RetrieveGameHost (CFGType, XBAtom atom, CFGGameHost *);
extern const char *GetHostName (CFGType, XBAtom);
extern void StoreGameHost (CFGType, XBAtom atom, const CFGGameHost *);

extern XBBool RetrieveGameSetup (CFGType, XBAtom atom, CFGGameSetup *);
extern void StoreGameSetup (CFGType, XBAtom atom, const CFGGameSetup *);

extern XBBool RetrieveGamePlayers (CFGType, XBAtom atom, CFGGamePlayers *);
extern void StoreGamePlayers (CFGType, XBAtom atom, const CFGGamePlayers *);

extern XBBool RetrieveGameVersion (CFGType cfgType, XBAtom atom, XBVersion * ver);
extern void StoreGameVersion (CFGType cfgType, XBAtom atom, const XBVersion * ver);

extern XBBool RetrieveGameConst (CFGType cfgType, XBAtom atom, CFGGameConst * con);
extern void StoreGameConst (CFGType cfgType, XBAtom atom, const CFGGameConst * con);
extern void StoreGameConstLocal (CFGType cfgType, XBAtom atom);

extern XBBool RetrieveIpHistory (CFGGameHost game[10], XBAtom atom);
extern void StoreIpHistory (CFGGameHost * host, XBAtom atom);

#endif
/*
 * end of file cfg_game.h
 */
