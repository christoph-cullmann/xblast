/*
 * cfg_game.c - storing and loading game setups
 *
 * $Id: cfg_game.c,v 1.25 2006/02/24 21:29:16 fzago Exp $
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

#include "xblast.h"

/*
 * local variables
 */
static DBRoot *dbLocal;
static DBRoot *dbRemote;

static CFGGame defaultGame = {
	{
	 0,							/* ifreclives  */
	 1,							/* number of lives */
	 9,							/* number of victories */
	 20,						/* frames per seconed */
	 XBTrue,					/* select all levels */
	 XBTrue,					/* random level order */
	 XBTrue,					/* random player positions */
	 0,							/* level order */
	 8,							/* info wait time */
	 XBFalse,					/* record demo file */
	 XBTM_None,					/* no team game */
	 XBTrue,					/* XBCC not rated */
	 XBFalse,					/* bot */
	 XBFalse,					/* beep */
	 XBFalse,					/* Music */
	 2,							/* number of rec lives (of valid if 1. XBTrue) */
	 1,							/* mask bytes for dgram */
	 },
	{
	 0,							/* num of players */
	 {ATOM_INVALID,},			/* player name atom */
	 {XBPC_None,},				/* player control */
	 {XBPH_None,},				/* player host */
	 {XBPT_Invalid,},			/* player team */
	 {COLOR_INVALID,},			/* player team color */
	 {0,},						/* player PID */
	 {0,},						/* player id */
	 },
	{
	 "localhost",				/* hostname */
	 16168,						/* tcp-port */
	 XBFalse,					/* fixed udp-port */
	 XBTrue,					/* browse LAN */
	 XBTrue,					/* allow NAT for clients */
	 XBTrue,					/* connect to central */
	 XBFalse,					/* beep if new client */
	 0,							/* Music */
	 "unnamed",					/* name of the game */
	 },
	{
	 6,							/* max hosts */
	 6,							/* max players */
	 6,							/* num local players */
	 1,							/* max mask bytes */
	 {0, 0, 0},					/* version */
	 },
};

/* conversion table for player control (alphanumeric sort by key) */
static DBToInt convControlTable[XBPC_NUM + 1] = {
	{"Joystick1", XBPC_Joystick1,},
	{"Joystick2", XBPC_Joystick2,},
	{"Joystick3", XBPC_Joystick3,},
	{"Joystick4", XBPC_Joystick4,},
	{"LeftKeyboard", XBPC_LeftKeyboard,},
	{"None", XBPC_None,},
	{"RightKeyboard", XBPC_RightKeyboard,},
	/* terminator */
	{NULL, XBPC_NUM},
};

/* conversion table for player hosts (alphanumeric sort by key) */
static DBToInt convHostTable[XBPH_NUM + 1] = {
	{"Central", XBPH_Central,},
	{"Client1", XBPH_Client1,},
	{"Client2", XBPH_Client2,},
	{"Client3", XBPH_Client3,},
	{"Client4", XBPH_Client4,},
	{"Client5", XBPH_Client5,},
#ifdef SMPF
	{"Client6", XBPH_Client6,},
	{"Client7", XBPH_Client7,},
	{"Client8", XBPH_Client8,},
	{"Client9", XBPH_Client9,},
	{"Client10", XBPH_Client10,},
	{"Client11", XBPH_Client11,},
	{"Client12", XBPH_Client12,},
	{"Client13", XBPH_Client13,},
	{"Client14", XBPH_Client14,},
	{"Client15", XBPH_Client15,},
#endif
	{"Demo", XBPH_Demo,},
	{"Local", XBPH_Local,},
	{"None", XBPH_None,},
	{"Server", XBPH_Server,},
	/* terminator */
	{NULL, XBPH_NUM},
};

/* conversion table for player teams (alphanumeric sort by key) */
static DBToInt convTeamTable[XBPT_NUM + 1] = {
	{"Blue", XBPT_Blue,},
	{"Green", XBPT_Green,},
	{"None", XBPT_None,},
	{"Red", XBPT_Red,},
	/* terminator */
	{NULL, XBPT_NUM,},
};

/* conversion table for player teams (alphanumeric sort by key) */
static DBToInt convTeamModeTable[XBTM_NUM + 1] = {
	{"Hunt", XBTM_Hunt,},
	{"None", XBTM_None,},
	{"Team", XBTM_Team,},
	/* terminator */
	{NULL, XBTM_NUM,},
};

/*
 * convert type to database
 */
static DBRoot *
GetDB (CFGType cfgType)
{
	switch (cfgType) {
	case CT_Local:
		return dbLocal;
	case CT_Remote:
		return dbRemote;
		/* case CT_Demo:   return dbDemo; */
	default:
		return NULL;
	}
}								/* GetDB */

/*
 *
 */
static void
StoreAnyGameSetup (DBRoot * db, XBAtom atom, const CFGGameSetup * game)
{
	DBSection *section;
	const char *key;

	/* sanity check */
	assert (db != NULL);
	assert (ATOM_INVALID != atom);
	assert (NULL != game);
	/* create new game section */
	section = DB_CreateSection (db, atom);
	assert (NULL != section);
	/* other stats */
	DB_CreateEntryInt (section, atomLives, game->numLives);
	DB_CreateEntryInt (section, atomRecLives, game->recLives);
	DB_CreateEntryBool (section, atomIfRecLives, game->ifRecLives);
	DB_CreateEntryInt (section, atomWins, game->numWins);
	DB_CreateEntryInt (section, atomFrameRate, game->frameRate);
	DB_CreateEntryBool (section, atomAllLevels, game->allLevels);
	DB_CreateEntryBool (section, atomRandomLevels, game->randomLevels);
	DB_CreateEntryBool (section, atomRandomPlayers, game->randomPlayers);
	DB_CreateEntryBool (section, atomRecordDemo, game->recordDemo);
	DB_CreateEntryBool (section, atomRatedGame, game->rated);
	DB_CreateEntryBool (section, atomBot, game->bot);
	DB_CreateEntryBool (section, atomBeep, game->beep);
	DB_CreateEntryInt (section, atomMusic, game->Music);
	DB_CreateEntryInt (section, atomInfoTime, game->infoTime);	// LRF
	DB_CreateEntryInt (section, atomLevelOrder, game->levelOrder);
	DB_CreateEntryInt (section, atomMaskBytes, game->maskBytes);
	/* team mode */
	if (NULL != (key = DB_IntToString (convTeamModeTable, (int)game->teamMode))) {
		DB_CreateEntryString (section, atomTeamMode, key);
	}
	else {
		DB_DeleteEntry (section, atomTeamMode);
	}
}								/* StoreAnyGameSetup */

/*
 *
 */
void
StoreGameSetup (CFGType cfgType, XBAtom atom, const CFGGameSetup * game)
{
	StoreAnyGameSetup (GetDB (cfgType), atom, game);
}								/* StoreLocalGameSetup */

/*
 * store GamePlayers data from struct to any database
 */
static void
StoreAnyGamePlayers (DBRoot * db, XBAtom atom, const CFGGamePlayers * game)
{
	int i;
	DBSection *section;
	const char *keyControl;
	const char *keyHost;
	const char *keyTeam;

	/* sanity check */
	assert (db != NULL);
	assert (ATOM_INVALID != atom);
	assert (NULL != game);
	/* create new game section */
	section = DB_CreateSection (db, atom);
	assert (NULL != section);
	/* number of players */
	DB_CreateEntryInt (section, atomNumPlayers, game->num);
	/* players */
	for (i = 0; i < game->num; i++) {
		keyControl = DB_IntToString (convControlTable, (int)game->control[i]);
		keyHost = DB_IntToString (convHostTable, (int)game->host[i]);
		keyTeam = DB_IntToString (convTeamTable, (int)game->team[i]);
		if (NULL != keyHost && NULL != keyControl && NULL != keyTeam) {
			DB_CreateEntryString (section, atomArrayPlayer0[i], GUI_AtomToString (game->player[i]));
			DB_CreateEntryString (section, atomArrayControl0[i], keyControl);
			DB_CreateEntryString (section, atomArrayHost0[i], keyHost);
			DB_CreateEntryString (section, atomArrayTeam0[i], keyTeam);
		}
		else {
			DB_DeleteEntry (section, atomArrayPlayer0[i]);
			DB_DeleteEntry (section, atomArrayControl0[i]);
			DB_DeleteEntry (section, atomArrayHost0[i]);
			DB_DeleteEntry (section, atomArrayTeam0[i]);
		}
	}
	for (; i < MAX_PLAYER; i++) {
		DB_DeleteEntry (section, atomArrayPlayer0[i]);
		DB_DeleteEntry (section, atomArrayControl0[i]);
		DB_DeleteEntry (section, atomArrayHost0[i]);
		DB_DeleteEntry (section, atomArrayTeam0[i]);
	}
}								/* StoreAnyGamePlayers */

/*
 * copy GamePlayers data from struct to a config database
 */
void
StoreGamePlayers (CFGType cfgType, XBAtom atom, const CFGGamePlayers * game)
{
	StoreAnyGamePlayers (GetDB (cfgType), atom, game);
}								/* StoreGamePlayers */

/*
 *
 */
static void
StoreAnyGameHost (DBRoot * db, XBAtom atom, const CFGGameHost * host)
{
	DBSection *section;
	/* sanity check */
	assert (db != NULL);
	assert (ATOM_INVALID != atom);
	assert (NULL != host);
	/* create new game section */
	section = DB_CreateSection (db, atom);
	assert (NULL != section);
	/* other stats */
	if (NULL != host->name) {
		DB_CreateEntryString (section, atomHost, host->name);
	}
	else {
		DB_DeleteEntry (section, atomHost);
	}
	DB_CreateEntryInt (section, atomPort, host->port);
	DB_CreateEntryBool (section, atomFixedUdpPort, host->fixedUdpPort);
	DB_CreateEntryBool (section, atomBrowseLan, host->browseLan);
	DB_CreateEntryBool (section, atomAllowNat, host->allowNat);
	DB_CreateEntryBool (section, atomCentral, host->central);
	DB_CreateEntryBool (section, atomBeep, host->beep);
	if (NULL != host->game) {
		DB_CreateEntryString (section, atomGame, host->game);
	}
	else {
		DB_DeleteEntry (section, atomGame);
	}
}								/* StoreGameHost */

/*
 *
 */
void
StoreGameHost (CFGType cfgType, XBAtom atom, const CFGGameHost * host)
{
	StoreAnyGameHost (GetDB (cfgType), atom, host);
}								/* StoreGameHost */

/*
 * store version data in any database
 */
static void
StoreAnyGameVersion (DBRoot * db, XBAtom atom, const XBVersion * ver)
{
	DBSection *section;
	/* sanity check */
	assert (db != NULL);
	assert (ATOM_INVALID != atom);
	assert (NULL != ver);
	/* create new game section */
	section = DB_CreateSection (db, atom);
	assert (NULL != section);
	/* store version data */
	DB_CreateEntryInt (section, atomVersionMajor, (int)ver->major);
	DB_CreateEntryInt (section, atomVersionMinor, (int)ver->minor);
	DB_CreateEntryInt (section, atomVersionPatch, (int)ver->patch);
}								/* StoreAnyGameVersion */

/*
 * copy version data database directory
 */
void
StoreGameVersion (CFGType cfgType, XBAtom atom, const XBVersion * ver)
{
	StoreAnyGameVersion (GetDB (cfgType), atom, ver);
}								/* StoreGameVersion */

/*
 * store constants in any database
 */
static void
StoreAnyGameConst (DBRoot * db, XBAtom atom, const CFGGameConst * con)
{
	DBSection *section;
	/* sanity check */
	assert (db != NULL);
	assert (ATOM_INVALID != atom);
	assert (NULL != con);
	/* create new game section */
	section = DB_CreateSection (db, atom);
	assert (NULL != section);
	/* store version data */
	DB_CreateEntryInt (section, atomMaxHosts, con->maxhosts);
	DB_CreateEntryInt (section, atomMaxPlayers, con->maxplayers);
	DB_CreateEntryInt (section, atomMaxLocals, con->maxlocals);
	DB_CreateEntryInt (section, atomMaxMaskBytes, con->maxbytes);
	DB_CreateEntryInt (section, atomVersionMajor, (int)con->version.major);
	DB_CreateEntryInt (section, atomVersionMinor, (int)con->version.minor);
	DB_CreateEntryInt (section, atomVersionPatch, (int)con->version.patch);
}								/* StoreAnyGameConst */

/*
 * store constants in config database
 */
void
StoreGameConst (CFGType cfgType, XBAtom atom, const CFGGameConst * con)
{
	StoreAnyGameConst (GetDB (cfgType), atom, con);
}								/* StoreGameConst */

/*
 * store local constants in config database
 */
void
StoreGameConstLocal (CFGType cfgType, XBAtom atom)
{
	CFGGameConst con;
	con.maxhosts = MAX_HOSTS;
	con.maxplayers = MAX_PLAYER;
	con.maxlocals = NUM_LOCAL_PLAYER;
	con.maxbytes = MAX_MASK_BYTES;
	memcpy (&con.version, &Ver_Local, sizeof (XBVersion));
	StoreAnyGameConst (GetDB (cfgType), atom, &con);
}								/* StoreGameConstLocal */

/*
 * retrieve game setup from any database
 */
static XBBool
RetrieveAnyGameSetup (const DBRoot * db, XBAtom atom, CFGGameSetup * game)
{
	const DBSection *section;

	assert (NULL != db);
	assert (NULL != game);
	/* set defaults */
	memcpy (game, &defaultGame.setup, sizeof (CFGGameSetup));
	/* find according db section */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* copy values */
	(void)DB_GetEntryInt (section, atomLives, &game->numLives);
	(void)DB_GetEntryInt (section, atomRecLives, &game->recLives);
	(void)DB_GetEntryBool (section, atomIfRecLives, &game->ifRecLives);
	(void)DB_GetEntryInt (section, atomWins, &game->numWins);
	(void)DB_GetEntryInt (section, atomFrameRate, &game->frameRate);
	(void)DB_GetEntryBool (section, atomAllLevels, &game->allLevels);
	(void)DB_GetEntryBool (section, atomRandomLevels, &game->randomLevels);
	(void)DB_GetEntryBool (section, atomRandomPlayers, &game->randomPlayers);
	(void)DB_GetEntryBool (section, atomRecordDemo, &game->recordDemo);
	(void)DB_GetEntryBool (section, atomRatedGame, &game->rated);
	(void)DB_GetEntryInt (section, atomInfoTime, &game->infoTime);	// LRF
	(void)DB_GetEntryInt (section, atomLevelOrder, &game->levelOrder);
	(void)DB_GetEntryBool (section, atomBot, &game->bot);
	(void)DB_GetEntryBool (section, atomBeep, &game->beep);
	(void)DB_GetEntryInt (section, atomMusic, (int *)&game->Music);
	(void)DB_GetEntryInt (section, atomMaskBytes, &game->maskBytes);
	/* team mode */
	if (DCR_Okay !=
		DB_ConvertEntryInt (section, atomTeamMode, (int *)&game->teamMode, convTeamModeTable)) {
		game->teamMode = XBTM_None;
	}
	/* that's all */
	return XBTrue;
}								/* RetrieveGameConfig */

/*
 * retrieve game setup from config database
 */
XBBool
RetrieveGameSetup (CFGType cfgType, XBAtom atom, CFGGameSetup * game)
{
	return RetrieveAnyGameSetup (GetDB (cfgType), atom, game);
}								/* RetrieveGameSetup */

/*
 * fill default player data, call only once when accessing game players
 */
static void
DefaultPlayerData (void)
{
	static int init = 0;
	int p;
	if (init == 0) {
		for (p = 1; p < MAX_PLAYER; p++) {
			defaultGame.players.player[p] = defaultGame.players.player[0];
			defaultGame.players.control[p] = defaultGame.players.control[0];
			defaultGame.players.host[p] = defaultGame.players.host[0];
			defaultGame.players.team[p] = defaultGame.players.team[0];
			defaultGame.players.PID[p] = defaultGame.players.PID[0];
			defaultGame.players.playerID[p] = defaultGame.players.playerID[0];
		}
		init = 1;
	}
}								/* DefaultPlayerData */

/*
 * copy GamePlayers data from any database to struct
 */
static XBBool
RetrieveAnyGamePlayers (const DBRoot * db, XBAtom atom, CFGGamePlayers * game)
{
	int i;
	const DBSection *section;
	const char *s;
	assert (NULL != db);
	assert (NULL != game);
	/* set defaults */
	DefaultPlayerData ();
	memcpy (game, &defaultGame.players, sizeof (CFGGamePlayers));
	/* find according db section */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* get num players */
	if (!DB_GetEntryInt (section, atomNumPlayers, &game->num)) {
		game->num = 0;
	}
	/* check player max */
	if (game->num > MAX_PLAYER) {
		return XBFalse;
	}
	/* get players */
	for (i = 0; i < game->num; i++) {
		if (DB_GetEntryString (section, atomArrayPlayer0[i], &s)) {
			game->player[i] = GUI_StringToAtom (s);
		}
		else {
			game->player[i] = ATOM_INVALID;
		}
		if (DCR_Okay !=
			DB_ConvertEntryInt (section, atomArrayControl0[i], (int *)&game->control[i],
								convControlTable)) {
			game->control[i] = XBPC_None;
		}
		if (DCR_Okay !=
			DB_ConvertEntryInt (section, atomArrayHost0[i], (int *)&game->host[i], convHostTable)) {
			game->host[i] = XBPH_None;
		}
		if (DCR_Okay !=
			DB_ConvertEntryInt (section, atomArrayTeam0[i], (int *)&game->team[i], convTeamTable)) {
			game->team[i] = XBPT_Invalid;
		}
		switch (game->team[i]) {
		case XBPT_Red:
			game->teamColor[i] = COLOR_RED;
			break;
		case XBPT_Green:
			game->teamColor[i] = COLOR_GREEN;
			break;
		case XBPT_Blue:
			game->teamColor[i] = COLOR_BLUE;
			break;
		default:
			game->teamColor[i] = COLOR_INVALID;
			break;
		}
	}
	return XBTrue;
}								/* RetrieveGamePlayers */

/*
 * get local player selection
 */
XBBool
RetrieveGamePlayers (CFGType cfgType, XBAtom atom, CFGGamePlayers * game)
{
	return RetrieveAnyGamePlayers (GetDB (cfgType), atom, game);
}								/* RetrieveGamePlayers */

/*
 *
 */
static XBBool
RetrieveAnyGameHost (const DBRoot * db, XBAtom atom, CFGGameHost * game)
{
	const DBSection *section;
	const char *s;

	assert (NULL != db);
	assert (NULL != game);
	/* find according db section */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* get host data */
	memcpy (game, &defaultGame.host, sizeof (CFGGameHost));
	if (DB_GetEntryString (section, atomHost, &s)) {
		game->name = s;
	}
	(void)DB_GetEntryInt (section, atomPort, &game->port);
	(void)DB_GetEntryBool (section, atomFixedUdpPort, &game->fixedUdpPort);
	(void)DB_GetEntryBool (section, atomBrowseLan, &game->browseLan);
	(void)DB_GetEntryBool (section, atomAllowNat, &game->allowNat);
	(void)DB_GetEntryBool (section, atomCentral, &game->central);
	(void)DB_GetEntryString (section, atomGame, &game->game);
	(void)DB_GetEntryBool (section, atomBeep, &game->beep);
	return XBTrue;
}								/* RetrieveGameHost */

/*
 *
 */
XBBool
RetrieveGameHost (CFGType cfgType, XBAtom atom, CFGGameHost * game)
{
	return RetrieveAnyGameHost (GetDB (cfgType), atom, game);
}								/* RetrieveGameHost */

/*
 * get version data from any database
 */
static XBBool
RetrieveAnyGameVersion (const DBRoot * db, XBAtom atom, XBVersion * ver)
{
	const DBSection *section;
	int maj, min, pat;
	assert (NULL != ver);
	/* find according db section */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	Version_Clear (ver);
	/* now extract version data */
	if (!DB_GetEntryInt (section, atomVersionMajor, &maj)) {
		return XBFalse;
	}
	if (!DB_GetEntryInt (section, atomVersionMinor, &min)) {
		return XBFalse;
	}
	if (!DB_GetEntryInt (section, atomVersionPatch, &pat)) {
		return XBFalse;
	}
	ver->major = maj & 0xFF;
	ver->minor = min & 0xFF;
	ver->patch = pat & 0xFF;
	return XBTrue;
}								/* RetrieveAnyGameVersion */

/*
 * get version data from config database
 */
XBBool
RetrieveGameVersion (CFGType cfgType, XBAtom atom, XBVersion * ver)
{
	return RetrieveAnyGameVersion (GetDB (cfgType), atom, ver);
}								/* RetrieveGameVersion */

/*
 * get game constants any database
 */
static XBBool
RetrieveAnyGameConst (const DBRoot * db, XBAtom atom, CFGGameConst * con)
{
	const DBSection *section;
	int maj, min, pat;
	assert (NULL != con);
	/* set default */
	memcpy (con, &defaultGame.constants, sizeof (CFGGameConst));
	/* find according db section */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* extract constants */
	(void)DB_GetEntryInt (section, atomMaxHosts, &con->maxhosts);
	(void)DB_GetEntryInt (section, atomMaxPlayers, &con->maxplayers);
	(void)DB_GetEntryInt (section, atomMaxLocals, &con->maxlocals);
	(void)DB_GetEntryInt (section, atomMaxMaskBytes, &con->maxbytes);
	/* now extract version data */
	if (!DB_GetEntryInt (section, atomVersionMajor, &maj)) {
		return XBTrue;
	}
	if (!DB_GetEntryInt (section, atomVersionMinor, &min)) {
		return XBTrue;
	}
	if (!DB_GetEntryInt (section, atomVersionPatch, &pat)) {
		return XBTrue;
	}
	con->version.major = maj & 0xFF;
	con->version.minor = min & 0xFF;
	con->version.patch = pat & 0xFF;
	return XBTrue;
}								/* RetrieveAnyGameConst */

/*
 * get game constants from config database
 */
XBBool
RetrieveGameConst (CFGType cfgType, XBAtom atom, CFGGameConst * con)
{
	return RetrieveAnyGameConst (GetDB (cfgType), atom, con);
}								/* RetrieveGameConst */

/*
 * save game config at once
 */
void
StoreGame (CFGType cfgType, XBAtom atom, const CFGGame * cfg)
{
	assert (NULL != cfg);

	StoreGameSetup (cfgType, atom, &cfg->setup);
	StoreGamePlayers (cfgType, atom, &cfg->players);
	StoreGameHost (cfgType, atom, &cfg->host);
	StoreGameConst (cfgType, atom, &cfg->constants);
}								/* StoreGame */

/*
 * retrieve game config at once
 */
XBBool
RetrieveGame (CFGType cfgType, XBAtom atom, CFGGame * cfg)
{
	XBBool result = XBTrue;

	assert (NULL != cfg);

	if (!RetrieveGameSetup (cfgType, atom, &cfg->setup)) {
		result = XBFalse;
	}
	if (!RetrieveGamePlayers (cfgType, atom, &cfg->players)) {
		result = XBFalse;
	}
	if (!RetrieveGameHost (cfgType, atom, &cfg->host)) {
		result = XBFalse;
	}
	(void)RetrieveGameConst (cfgType, atom, &cfg->constants);
	return result;
}								/* RetrieveGame */

/*
 * load game config from file
 */
void
LoadGameConfig (void)
{
	/* remote database */
	dbRemote = DB_Create (DT_Config, atomRemoteGame);
	assert (dbRemote != NULL);
	/* local database */
	dbLocal = DB_Create (DT_Config, atomGame);
	assert (dbLocal != NULL);
	Dbg_Config ("loading local game config\n");
	if (DB_Load (dbLocal)) {
		return;
	}
	Dbg_Config ("failed to load local game config, setting defaults\n");
	/* set default values */
	DefaultPlayerData ();
	StoreGame (CT_Local, atomLocal, &defaultGame);
	StoreGame (CT_Local, atomServer, &defaultGame);
	StoreGame (CT_Local, atomClient, &defaultGame);
	/* and save it */
	DB_Store (dbLocal);
}								/* LoadGameConfig */

/*
 * save game config to file
 */
void
SaveGameConfig (void)
{
	assert (dbLocal != NULL);
	if (DB_Changed (dbLocal)) {
		Dbg_Config ("saving local game config\n");
		DB_Store (dbLocal);
	}
#ifdef DEBUG
	assert (dbRemote != NULL);
	if (DB_Changed (dbRemote)) {
		Dbg_Config ("saving remote game config\n");
		DB_Store (dbRemote);
	}
#endif
}								/* SaveGameConfig */

/*
 * remove game configs
 */
void
FinishGameConfig (void)
{
	if (NULL != dbLocal) {
		DB_Delete (dbLocal);
		dbLocal = NULL;
	}
	if (NULL != dbRemote) {
		DB_Delete (dbRemote);
		dbRemote = NULL;
	}
	Dbg_Config ("game config cleared!\n");
}								/* FinishGameConfig */

/*
 * put game config into telegram send queue
 */
XBBool
SendGameConfig (CFGType cfgType, XBSndQueue * queue, XBTeleCOT cot, XBTeleIOB iob, XBAtom atom)
{
	const DBSection *section;
	int i;
	size_t len;
	XBTelegram *tele;
	char tmp[256];

	assert (queue != NULL);
	/* get section with game data */
	section = DB_GetSection (GetDB (cfgType), atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* now print and send data */
	i = 0;
	while (0 < (len = DB_PrintEntry (tmp, section, i))) {
		tele = Net_CreateTelegram (cot, XBT_ID_GameConfig, iob, tmp, len + 1);
		assert (tele != NULL);
		Net_SendTelegram (queue, tele);
		i++;
	}
	/* no data means end of section */
	tele = Net_CreateTelegram (cot, XBT_ID_GameConfig, iob, NULL, 0);
	assert (tele != NULL);
	Net_SendTelegram (queue, tele);
	return XBTrue;
}								/* SendGameConfig */

/*
 * delete a game config
 */
void
DeleteGameConfig (CFGType cfgType, XBAtom atom)
{
	DB_DeleteSection (GetDB (cfgType), atom);
}								/* DeleteGameConfig */

/*
 * add entry line to game config
 */
void
AddToGameConfig (CFGType cfgType, XBAtom atom, const char *line)
{
	DBSection *section;

	/* sanity check */
	assert (ATOM_INVALID != atom);
	/* create new game section */
	section = DB_CreateSection (GetDB (cfgType), atom);
	assert (NULL != section);
	/* add line */
	(void)DB_ParseEntry (section, line);
}								/* AddToGameConfig */

/*
 * return hostname and port
 */
const char *
GetHostName (CFGType cfgType, XBAtom atom)
{
	CFGGameHost cfg;
	static char txt[256];

	/* sanity check */
	assert (ATOM_INVALID != atom);
	/* get host config */
	if (!RetrieveGameHost (cfgType, atom, &cfg)) {
		strcpy (txt, "unknown");
	}
	else {
		sprintf (txt, "%s:%d", cfg.name, cfg.port);
	}
	return txt;
}								/* GetRemoteHostName */

 /*
  * push a single ip into ip history database
  */
void
StoreIpHistory (CFGGameHost * host, XBAtom atom)
{
	DBSection *section;
	CFGGameHost hist[10];
	int i = 0;
	/* sanity check */
	assert (ATOM_INVALID != atom);
	assert (NULL != host);
	section = (DBSection *) DB_GetSection (GetDB (CT_Local), atom);
	/* get old history data */
	assert (NULL != section);
	RetrieveIpHistory (hist, atom);
	/* push back server data */
	for (i = 9; i > 0; i--) {
		hist[i].name = hist[i - 1].name;
		hist[i].port = hist[i - 1].port;
	}
	/* store latest host data */
	hist[0].port = host->port;
	hist[0].name = host->name;
	/* store back to database, backwards (important for pointer validity!) */
	for (i = 9; i >= 0; i--) {
		if (NULL != hist[i].name) {
			DB_CreateEntryString (section, atomGamehis[i], hist[i].name);
			DB_CreateEntryInt (section, atomPorthis[i], hist[i].port);
		}
		else {
			DB_DeleteEntry (section, atomGamehis[i]);
			DB_DeleteEntry (section, atomPorthis[i]);
		}
	}
	RetrieveIpHistory (hist, atom);
}								/* StoreIpHistory */

/*
 * retrieve all ip history data
 */
XBBool
RetrieveIpHistory (CFGGameHost host[10], XBAtom atom)
{
	const DBSection *section;
	int i = 0;
	section = DB_GetSection (GetDB (CT_Local), atom);
	/* check if there is section with ip history data */
	if (NULL == section) {
		return XBFalse;
	}
	/* retrieve data */
	for (i = 0; i < 10; i++) {
		/* first default data for entry */
		host[i].name = NULL;
		host[i].game = NULL;
		host[i].port = 0;
		/* now update from database */
		(void)DB_GetEntryString (section, atomGamehis[i], &host[i].name);
		(void)DB_GetEntryInt (section, atomPorthis[i], &host[i].port);
	}
	return XBTrue;
}								/* RetrieveIpHistory */

/*
 * end of file cfg_game.c
 */
