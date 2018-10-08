/*
 * cfg_player.c - player configuration data
 *
 * $Id: cfg_player.c,v 1.15 2006/06/12 10:51:48 fzago Exp $
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
 * global variables
 */
static const char *defaultShape[NUM_DEFAULT_PLAYERS] = {
	"normal", "normal", "normal", "fat", "girl", "tall",
};
static CFGPlayerGraphics defaultPlayerGraphics[NUM_DEFAULT_PLAYERS] = {
	{
	 ATOM_INVALID,
	 COLOR_TURQUOISE, COLOR_LIGHT_SALMON, COLOR_WHITE, COLOR_GRAY_75,
	 COLOR_GRAY_25, COLOR_ROYAL_BLUE, COLOR_WHITE,
	 },
	{
	 ATOM_INVALID,
	 COLOR_MIDNIGHT_BLUE, COLOR_LIGHT_SALMON, COLOR_NAVY_BLUE, COLOR_RED,
	 COLOR_ROYAL_BLUE, COLOR_GOLD, COLOR_WHITE,
	 },
	{
	 ATOM_INVALID,
	 COLOR_RED, COLOR_LIGHT_SALMON, COLOR_RED, COLOR_FOREST_GREEN,
	 COLOR_INDIAN_RED, COLOR_DARK_SEA_GREEN, COLOR_WHITE,
	 },
	{
	 ATOM_INVALID,
	 COLOR_YELLOW, COLOR_LIGHT_SALMON, COLOR_SPRING_GREEN, COLOR_ORANGE_RED,
	 COLOR_LIGHT_YELLOW, COLOR_ROYAL_BLUE, COLOR_WHITE,
	 },
	{
	 ATOM_INVALID,
	 COLOR_DEEP_PINK, COLOR_LIGHT_SALMON, COLOR_ORCHID, COLOR_SPRING_GREEN,
	 COLOR_ROYAL_BLUE, COLOR_DEEP_PINK, COLOR_WHITE,
	 },
	{
	 ATOM_INVALID,
	 COLOR_BLACK, COLOR_TAN, COLOR_BLACK, COLOR_ORANGE,
	 COLOR_BLACK, COLOR_ORANGE, COLOR_WHITE,
	 },
};

static const char *administratorShape = "normal";
static CFGPlayerGraphics administratorPlayerGraphics = {
	ATOM_INVALID,
	COLOR_TURQUOISE, COLOR_LIGHT_SALMON, COLOR_WHITE, COLOR_GRAY_75,
	COLOR_GRAY_25, COLOR_ROYAL_BLUE, COLOR_WHITE,
};

/*
 * local variables
 */
static DBRoot *dbLocal = NULL;
static DBRoot *dbRemote = NULL;

static DBRoot *dbCentral = NULL;	// XBCC central database
XBBool isCentral;

/* default values for new or empty configs */
static CFGPlayerGraphics newGfx = {
	ATOM_INVALID,
	COLOR_WHITE, COLOR_LIGHT_SALMON, COLOR_WHITE,
	COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
};
static CFGPlayerMessages newMsg = {
	NULL, NULL, NULL, NULL, NULL, NULL,
};
static CFGPlayerMisc newMisc = {
	XBTrue, 0, 4,
};
static CFGPlayerID newID = {
	-1, NULL
};

/* names for default players */
static const char *defaultName[NUM_DEFAULT_PLAYERS] = {
	"Olli",
	"Norbert",
	"Rodi",
	"Harald",
	"Alex",
	"Olaf",
};

/* XBCC  values for administrator config */
static CFGPlayerMessages administratorMsg = {
	NULL, NULL, NULL, NULL, NULL, "Welcome",
};
static CFGPlayerMisc administratorMisc = {
	XBTrue, 0, 4,
};
static CFGPlayerID administratorID = {
	0, "adminpass",
};
static CFGPlayerRating newRating = {
	0.0, 0, 0, 0, 0, 0,
};
static const char *administratorName = "Administrator";

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
	case CT_Demo:
		return dbDemo;
	case CT_Central:
		return dbCentral;
	default:
		return NULL;
	}
}								/* GetDB */

/*
 *
 */
const CFGPlayerGraphics *
DefaultPlayerGraphics (size_t i)
{
	assert (i < NUM_DEFAULT_PLAYERS);
	if (ATOM_INVALID == defaultPlayerGraphics[i].shape) {
		defaultPlayerGraphics[i].shape = GUI_StringToAtom (defaultShape[i]);
	}
	return defaultPlayerGraphics + i;
}								/* DefaultPlayerGraphics */

/*
 * XBCC
 */
static const CFGPlayerGraphics *
AdministratorPlayerGraphics (void)
{
	if (ATOM_INVALID == administratorPlayerGraphics.shape) {
		administratorPlayerGraphics.shape = GUI_StringToAtom (administratorShape);
	}
	return &administratorPlayerGraphics;
}								/* AdministratorPlayerGraphics */

/*
 *
 */
static void
StoreAnyPlayerName (DBRoot * db, XBAtom atom, const char *name)
{
	DBSection *section;

	assert (name != NULL);
	assert (ATOM_INVALID != atom);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	/* graphics */
	DB_CreateEntryString (section, atomName, name);
}								/* StorePlayerName */

/*
 *
 */
static void
StorePlayerName (CFGType cfgType, XBAtom atom, const char *name)
{
	StoreAnyPlayerName (GetDB (cfgType), atom, name);
}								/* StoreLocalPlayerName */

/*
 *
 */
static void
StoreAnyPlayerGraphics (DBRoot * db, XBAtom atom, const CFGPlayerGraphics * gfx)
{
	DBSection *section;

	assert (db != NULL);
	assert (gfx != NULL);
	assert (ATOM_INVALID != atom);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	/* graphics */
	(void)DB_CreateEntryAtom (section, atomShape, gfx->shape);
	(void)DB_CreateEntryColor (section, atomHelmet, gfx->helmet);
	(void)DB_CreateEntryColor (section, atomFace, gfx->face);
	(void)DB_CreateEntryColor (section, atomBody, gfx->body);
	(void)DB_CreateEntryColor (section, atomArmsLegs, gfx->armsLegs);
	(void)DB_CreateEntryColor (section, atomHandsFeet, gfx->handsFeet);
	(void)DB_CreateEntryColor (section, atomBackpack, gfx->backpack);
}								/* StorePlayerGraphics */

/*
 *
 */
void
StorePlayerGraphics (CFGType cfgType, XBAtom atom, const CFGPlayerGraphics * gfx)
{
	StoreAnyPlayerGraphics (GetDB (cfgType), atom, gfx);
}								/* StoreLocalPlayerGraphics */

/*
 *
 */
static XBBool
RetrieveAnyPlayerGraphics (const DBRoot * db, XBAtom atom, XBColor teamColor,
						   CFGPlayerGraphics * gfx)
{
	const DBSection *section;

	assert (db != NULL);
	assert (gfx != NULL);
	/* set to defaults */
	*gfx = newGfx;
	/* find section for player */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	(void)DB_GetEntryAtom (section, atomShape, &gfx->shape);
	(void)DB_GetEntryColor (section, atomHelmet, &gfx->helmet);
	(void)DB_GetEntryColor (section, atomFace, &gfx->face);
	(void)DB_GetEntryColor (section, atomBody, &gfx->body);
	(void)DB_GetEntryColor (section, atomArmsLegs, &gfx->armsLegs);
	(void)DB_GetEntryColor (section, atomHandsFeet, &gfx->handsFeet);
	(void)DB_GetEntryColor (section, atomBackpack, &gfx->backpack);
	/* set white color */
	gfx->white = COLOR_WHITE;
	/* save original colors */
	gfx->bodySave = gfx->body;
	gfx->handsFeetSave = gfx->handsFeet;
	/* set team colors */
	if (teamColor != COLOR_INVALID) {
		/* KOEN */
		/* gfx->helmet    = teamColor; */
		gfx->body = teamColor;
		gfx->handsFeet = teamColor;
	}
	/* that's all */
	return XBTrue;
}								/* RetrievePlayerGraphics */

/*
 * retrieve indexed player graphics
 */
XBBool
RetrievePlayerGraphics (CFGType cfgType, XBAtom atom, XBColor teamColor, CFGPlayerGraphics * gfx)
{
	return RetrieveAnyPlayerGraphics (GetDB (cfgType), atom, teamColor, gfx);
}								/* RetrievePlayerGraphics */

/*
 * store a signle message
 */
static void
StoreMessage (DBSection * section, XBAtom atom, const char *text)
{
	if (NULL != text) {
		DB_CreateEntryString (section, atom, text);
	}
	else {
		DB_DeleteEntry (section, atom);
	}
}								/* StoreMessage */

/*
 *
 */
static void
StoreAnyPlayerMessages (DBRoot * db, XBAtom atom, const CFGPlayerMessages * msg)
{
	DBSection *section;

	assert (db != NULL);
	assert (ATOM_INVALID != atom);
	assert (msg != NULL);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	/* messages */
	StoreMessage (section, atomMsgWinLevel, msg->msgWinLevel);
	StoreMessage (section, atomMsgWinGame, msg->msgWinGame);
	StoreMessage (section, atomMsgLoseLife, msg->msgLoseLife);
	StoreMessage (section, atomMsgLoseLevel, msg->msgLoseLevel);
	StoreMessage (section, atomMsgGloat, msg->msgGloat);
	StoreMessage (section, atomMsgLaola, msg->msgLaola);
	StoreMessage (section, atomMsgLoser, msg->msgLoser);
	StoreMessage (section, atomMsgWelcome, msg->msgWelcome);
}								/* StorePlayerMessages */

/*
 *
 */
void
StorePlayerMessages (CFGType cfgType, XBAtom atom, const CFGPlayerMessages * msg)
{
	StoreAnyPlayerMessages (GetDB (cfgType), atom, msg);
}								/* StoreLocalPlayerMessages */

/*
 *
 */
static XBBool
RetrieveAnyPlayerMessages (const DBRoot * db, XBAtom atom, CFGPlayerMessages * msg)
{
	const DBSection *section;

	assert (db != NULL);
	assert (msg != NULL);
	/* set to defaults */
	*msg = newMsg;
	/* find section for player */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	(void)DB_GetEntryString (section, atomMsgWinLevel, &msg->msgWinLevel);
	(void)DB_GetEntryString (section, atomMsgWinGame, &msg->msgWinGame);
	(void)DB_GetEntryString (section, atomMsgLoseLife, &msg->msgLoseLife);
	(void)DB_GetEntryString (section, atomMsgLoseLevel, &msg->msgLoseLevel);
	(void)DB_GetEntryString (section, atomMsgGloat, &msg->msgGloat);
	(void)DB_GetEntryString (section, atomMsgLaola, &msg->msgLaola);
	(void)DB_GetEntryString (section, atomMsgLoser, &msg->msgLoser);
	(void)DB_GetEntryString (section, atomMsgWelcome, &msg->msgWelcome);
	/* that's all */
	return XBTrue;
}								/* RetrievePlayerMessages */

/*
 * retrieve messages for local players
 */
XBBool
RetrievePlayerMessages (CFGType cfgType, XBAtom atom, CFGPlayerMessages * msg)
{
	return RetrieveAnyPlayerMessages (GetDB (cfgType), atom, msg);
}								/* RetrievePlayerMessages */

/*
 * store misc player options
 */
static void
StoreAnyPlayerMisc (DBRoot * db, XBAtom atom, const CFGPlayerMisc * misc)
{
	DBSection *section;

	assert (db != NULL);
	assert (misc != NULL);
	assert (ATOM_INVALID != atom);
	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	/* store data */
	DB_CreateEntryBool (section, atomUseStopKey, misc->useStopKey);
	DB_CreateEntryInt (section, atomTurnStepKeyboard, misc->turnStepKeyboard);
	DB_CreateEntryInt (section, atomTurnStepJoystick, misc->turnStepJoystick);
}								/* StorePlayerMisc */

/*
 * store misc player options
 */
void
StorePlayerMisc (CFGType cfgType, XBAtom atom, const CFGPlayerMisc * misc)
{
	StoreAnyPlayerMisc (GetDB (cfgType), atom, misc);
}								/* StoreLocalPlayerMisc */

/*
 * retrive misc player data
 */
static XBBool
RetrieveAnyPlayerMisc (const DBRoot * db, XBAtom atom, CFGPlayerMisc * misc)
{
	const DBSection *section;

	assert (db != NULL);
	assert (misc != NULL);
	/* set default values */
	*misc = newMisc;
	/* find section for player */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* store data */
	(void)DB_GetEntryBool (section, atomUseStopKey, &misc->useStopKey);
	(void)DB_GetEntryInt (section, atomTurnStepKeyboard, &misc->turnStepKeyboard);
	(void)DB_GetEntryInt (section, atomTurnStepJoystick, &misc->turnStepJoystick);
	return XBTrue;
}								/* RetrievePlayerMisc */

/*
 * retrieve misc player data
 */
XBBool
RetrievePlayerMisc (CFGType cfgType, XBAtom atom, CFGPlayerMisc * misc)
{
	return RetrieveAnyPlayerMisc (GetDB (cfgType), atom, misc);
}								/* RetrieveLocalPlayerMisc */

/*
 * save player config at once
 */
void
StorePlayer (CFGType cfgType, XBAtom atom, const CFGPlayer * cfg)
{
	assert (NULL != cfg);

	StorePlayerGraphics (cfgType, atom, &cfg->graphics);
	StorePlayerMessages (cfgType, atom, &cfg->messages);
	StorePlayerMisc (cfgType, atom, &cfg->misc);
	StorePlayerID (cfgType, atom, &cfg->id);
	StorePlayerName (cfgType, atom, cfg->name);
}								/* StorePlayer */

/*
 * load player config at once
 */
XBBool
RetrievePlayer (CFGType cfgType, XBAtom atom, XBColor color, CFGPlayer * cfg)
{
	XBBool result = XBTrue;

	assert (NULL != cfg);

	if (!RetrievePlayerGraphics (cfgType, atom, color, &cfg->graphics)) {
		result = XBFalse;
	}
	if (!RetrievePlayerMessages (cfgType, atom, &cfg->messages)) {
		result = XBFalse;
	}
	if (!RetrievePlayerMisc (cfgType, atom, &cfg->misc)) {
		result = XBFalse;
	}
	if (!RetrievePlayerID (cfgType, atom, &cfg->id)) {
		result = XBFalse;
	}
	cfg->name = GetPlayerName (cfgType, atom);
	return result;
}								/* RetrievePlayer */

/*
 * load player config at once
 */
XBBool
RetrievePlayerEx (CFGType cfgType, XBAtom atom, CFGPlayerEx * cfg)
{
	XBBool result = XBTrue;

	assert (NULL != cfg);

	if (!RetrievePlayerGraphics (cfgType, atom, COLOR_INVALID, &cfg->graphics)) {
		result = XBFalse;
	}
	if (!RetrievePlayerMessages (cfgType, atom, &cfg->messages)) {
		result = XBFalse;
	}
	if (!RetrievePlayerMisc (cfgType, atom, &cfg->misc)) {
		result = XBFalse;
	}
	if (!RetrievePlayerID (cfgType, atom, &cfg->id)) {
		result = XBFalse;
	}
	if (!RetrievePlayerRating (cfgType, atom, &cfg->rating)) {
		result = XBFalse;
	}
	cfg->name = GetPlayerName (cfgType, atom);
	return result;
}								/* RetrievePlayerEx */

/*
 * XBCC save player EX config at once
 */
void
StorePlayerEx (CFGType cfgType, XBAtom atom, const CFGPlayerEx * cfg)
{
	assert (NULL != cfg);

	StorePlayerGraphics (cfgType, atom, &cfg->graphics);
	StorePlayerMessages (cfgType, atom, &cfg->messages);
	StorePlayerMisc (cfgType, atom, &cfg->misc);
	StorePlayerID (cfgType, atom, &cfg->id);
	StorePlayerRating (cfgType, atom, &cfg->rating);
	StorePlayerName (cfgType, atom, cfg->name);
}								/* StorePlayerEx */

/*
 * init and load player config
 */
void
LoadPlayerConfig (void)
{
	int i;
	XBAtom atom;

	/* set default values */
	newGfx.shape = GUI_StringToAtom ("normal");
	/* initialize remote databse */
	dbRemote = DB_Create (DT_Config, atomRemotePlayer);
	assert (dbRemote != NULL);
	/* initialiaze config database */
	dbLocal = DB_Create (DT_Config, atomPlayer);
	assert (dbLocal != NULL);
	Dbg_Config ("loading local player configs\n");
	if (DB_Load (dbLocal)) {
		return;
	}
	Dbg_Config ("failed to load player config, setting defaults\n");
	/* set some useful default values */
	for (i = 0; i < NUM_DEFAULT_PLAYERS; i++) {
		atom = GUI_StringToAtom (defaultName[i]);
		StorePlayerName (CT_Local, atom, defaultName[i]);
		StorePlayerGraphics (CT_Local, atom, DefaultPlayerGraphics (i));
		StorePlayerMisc (CT_Local, atom, &newMisc);
		StorePlayerMessages (CT_Local, atom, &newMsg);
		StorePlayerID (CT_Local, atom, &newID);
	}
	DB_Store (dbLocal);
}								/* InitPlayerConfig */

/*
 * finish player config
 */
void
SavePlayerConfig (void)
{
	assert (dbLocal != NULL);
	if (DB_Changed (dbLocal)) {
		Dbg_Config ("saving local player configs\n");
		DB_Store (dbLocal);
	}
#ifdef DEBUG
	assert (dbRemote != NULL);
	if (DB_Changed (dbRemote)) {
		Dbg_Config ("saving remote player configs\n");
		DB_Store (dbRemote);
	}
#endif
}								/* SavePlayerConfig */

/*
 *
 */
void
FinishPlayerConfig (void)
{
	if (NULL != dbLocal) {
		DB_Delete (dbLocal);
		dbLocal = NULL;
	}
	if (NULL != dbRemote) {
		DB_Delete (dbRemote);
		dbRemote = NULL;
	}
	Dbg_Config ("player configs cleared\n");
}								/* FinishPlayerConfig */

/*
 * XBCC init and load player central statistics
 */
void
LoadPlayerCentral (XBBool amCentral)
{
	XBAtom atom;
	isCentral = amCentral;

	/* set default values */
	newGfx.shape = GUI_StringToAtom ("normal");
	if (amCentral) {
		/* initialiaze config database */
		dbCentral = DB_Create (DT_Central, atomCentralLocal);
	}
	else {
		/* initialize remote databse */
		dbCentral = DB_Create (DT_Central, atomCentralRemote);
	}
	assert (dbCentral != NULL);
	if (DB_Load (dbCentral)) {
		return;
	}
	/* set administrator defaults */
	if (amCentral) {
		atom = GUI_IntToAtom (administratorID.PID);
		StorePlayerName (CT_Central, atom, administratorName);
		StorePlayerGraphics (CT_Central, atom, AdministratorPlayerGraphics ());
		StorePlayerMisc (CT_Central, atom, &administratorMisc);
		StorePlayerMessages (CT_Central, atom, &administratorMsg);
		StorePlayerID (CT_Central, atom, &administratorID);
		StorePlayerRating (CT_Central, atom, &newRating);
	}
	DB_Store (dbCentral);
}								/* InitPlayerConfig */

/*
 * finish player config
 */
void
SavePlayerCentral (void)
{
	assert (dbCentral != NULL);
	if (DB_Changed (dbCentral)) {
		DB_Store (dbCentral);
	}
}								/* SavePlayerConfig */

/*
 *
 */
void
FinishPlayerCentral (void)
{
	if (NULL != dbCentral) {
		DB_Delete (dbCentral);
		dbCentral = NULL;
	}
}								/* FinishPlayerCentral */

void
RemoveAllPlayers (CFGType cfgType)
{
	DB_DeleteAll (GetDB (cfgType));
}

/*
 * get number of configs stored
 */
int
GetNumPlayerConfigs (CFGType cfgType)
{
	return DB_NumSections (GetDB (cfgType));
}								/* GetNumPlayerConfigs */

/*
 * index atoms for player config
 */
XBAtom
GetPlayerAtom (CFGType cfgType, int i)
{
	return DB_IndexSection (GetDB (cfgType), i);
}								/* GetPlayerConfigName */

/*
 * get name of player
 */
static const char *
GetAnyPlayerName (DBRoot * db, XBAtom atom)
{
	const DBSection *section;
	const char *s;

	assert (NULL != db);
  /*---*/
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return NULL;
	}
	while (!DB_GetEntryString (section, atomName, &s)) {
		const char *name = GUI_AtomToString (atom);
		DB_CreateEntryString (DB_CreateSection (db, atom), atomName, name);
	}
	return s;
}								/* GetPlayerName */

/*
 * get name of player
 */
const char *
GetPlayerName (CFGType cfgType, XBAtom atom)
{
	return GetAnyPlayerName (GetDB (cfgType), atom);
}								/* GetLocalPlayerName */

/*
 * search player names for newplayer name
 */
int
FindDoubleName (CFGType cfgType, XBAtom newplayer)
{
	XBAtom atom;
	const char *cmp;
	int j;
	cmp = GetPlayerName (cfgType, newplayer);
	for (j = 0; j < GetNumPlayerConfigs (cfgType); j++) {
		atom = GetPlayerAtom (cfgType, j);
		if (atom != newplayer) {
			if (0 == strcmp (cmp, GetPlayerName (cfgType, atom))) {
				return (j);
			}
		}
	}
	return (-1);
}								/* FindDoubleName */

/*
 * XBCC
 */
static void
StoreAnyPlayerID (DBRoot * db, XBAtom atom, const CFGPlayerID * id)
{
	DBSection *section;

	assert (db != NULL);
	assert (id != NULL);
	assert (ATOM_INVALID != atom);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	/* graphics */
	StoreMessage (section, atomPass, id->pass);
	(void)DB_CreateEntryInt (section, atomPID, id->PID);
}								/* StorePlayerGraphics */

/*
 *
 */
void
StorePlayerID (CFGType cfgType, XBAtom atom, const CFGPlayerID * id)
{
	StoreAnyPlayerID (GetDB (cfgType), atom, id);
}								/* StoreLocalPlayerGraphics */

/*
 *
 */
static XBBool
RetrieveAnyPlayerID (const DBRoot * db, XBAtom atom, CFGPlayerID * id)
{
	const DBSection *section;

	assert (db != NULL);
	assert (id != NULL);
	/* set to defaults */
	*id = newID;
	/* find section for player */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* set default PID to invalid */
	id->PID = -1;
	(void)DB_GetEntryString (section, atomPass, &id->pass);
	(void)DB_GetEntryInt (section, atomPID, &id->PID);
	/* that's all */
	return XBTrue;
}								/* RetrievePlayerMessages */

/*
 * retrieve messages for local players
 */
XBBool
RetrievePlayerID (CFGType cfgType, XBAtom atom, CFGPlayerID * id)
{
	return RetrieveAnyPlayerID (GetDB (cfgType), atom, id);
}								/* RetrievePlayerMessages */

/*
 * XBCC rating
 */
static void
StoreAnyPlayerRating (DBRoot * db, XBAtom atom, const CFGPlayerRating * rating)
{
	DBSection *section;

	assert (db != NULL);
	assert (rating != NULL);
	assert (ATOM_INVALID != atom);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	/* graphics */
	(void)DB_CreateEntryFloat (section, atomXBCCRating, rating->rating);
	(void)DB_CreateEntryInt (section, atomXBCCGamesPlayed, rating->gamesPlayed);
	(void)DB_CreateEntryInt (section, atomXBCCRealWins, rating->realWins);
	(void)DB_CreateEntryInt (section, atomXBCCRelativeWins, rating->relativeWins);
	(void)DB_CreateEntryTime (section, atomXBCCTimeUpdate, rating->timeUpdate);
	(void)DB_CreateEntryTime (section, atomXBCCTimeRegister, rating->timeRegister);
}								/* StorePlayerRating */

/*
 *
 */
void
StorePlayerRating (CFGType cfgType, XBAtom atom, const CFGPlayerRating * rating)
{
	StoreAnyPlayerRating (GetDB (cfgType), atom, rating);
}								/* StoreLocalPlayerRating */

/*
 *
 */
static XBBool
RetrieveAnyPlayerRating (const DBRoot * db, XBAtom atom, CFGPlayerRating * rating)
{
	const DBSection *section;

	assert (db != NULL);
	assert (rating != NULL);
	/* set to defaults */
	*rating = newRating;
	/* find section for player */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	(void)DB_GetEntryFloat (section, atomXBCCRating, &rating->rating);
	(void)DB_GetEntryInt (section, atomXBCCGamesPlayed, &rating->gamesPlayed);
	(void)DB_GetEntryInt (section, atomXBCCRealWins, &rating->realWins);
	(void)DB_GetEntryInt (section, atomXBCCRelativeWins, &rating->relativeWins);
	(void)DB_GetEntryTime (section, atomXBCCTimeUpdate, &rating->timeUpdate);
	(void)DB_GetEntryTime (section, atomXBCCTimeRegister, &rating->timeRegister);
	/* that's all */
	return XBTrue;
}								/* RetrievePlayerRating */

/*
 * retrieve messages for local players
 */
XBBool
RetrievePlayerRating (CFGType cfgType, XBAtom atom, CFGPlayerRating * rating)
{
	return RetrieveAnyPlayerRating (GetDB (cfgType), atom, rating);
}								/* RetrievePlayerRating */

/* XBCC */

/*
 * delete a new player
 */
void
DeletePlayerConfig (CFGType cfgType, XBAtom atom)
{
	DB_DeleteSection (GetDB (cfgType), atom);
}								/* DeletePlayerConfig */

/*
 * create new player config
 */
static XBAtom
CreatePlayerConfig (CFGType cfgType, const char *name, const CFGPlayerGraphics * cfgGfx,
					const CFGPlayerMessages * cfgMsg, const CFGPlayerMisc * cfgMisc,
					const CFGPlayerID * cfgID)
{
	XBAtom atom;

	/* sanity checks */
	assert (NULL != name);
	assert (NULL != cfgGfx);
	assert (NULL != cfgMsg);
	assert (NULL != cfgMisc);
	assert (NULL != cfgID);
	/* convert name to atom */
	atom = GUI_StringToAtom (name);
	assert (ATOM_INVALID != atom);
	/* look for config with the same name */
	if (NULL != GetPlayerName (cfgType, atom) || strlen (name) == 0) {
		return ATOM_INVALID;
	}
	/* create new dataset */
	StorePlayerName (cfgType, atom, name);
	StorePlayerGraphics (cfgType, atom, cfgGfx);
	StorePlayerMessages (cfgType, atom, cfgMsg);
	StorePlayerMisc (cfgType, atom, cfgMisc);
	StorePlayerID (cfgType, atom, cfgID);
	/* that's all */
	return atom;
}								/* CopyPlayerConfig */

/*
 * create a new player
 */
XBAtom
CreateNewPlayerConfig (CFGType cfgType, const char *name)
{
	CFGPlayerGraphics cfgGfx = newGfx;

	/* use random colors */
	cfgGfx.helmet = RandomColor ();
	cfgGfx.body = RandomColor ();
	cfgGfx.handsFeet = RandomColor ();
	cfgGfx.armsLegs = RandomColor ();
	cfgGfx.backpack = RandomColor ();

	return CreatePlayerConfig (cfgType, name, &cfgGfx, &newMsg, &newMisc, &newID);
}								/* CreatePlayerConfig */

/*
 * rename existing player
 */
XBAtom
RenamePlayerConfig (CFGType cfgType, XBAtom atom, const char *name)
{
	CFGPlayer cfgPlayer;
	XBAtom newAtom;

	/* sanity check */
	assert (atom != ATOM_INVALID);
	assert (name != NULL);
	/* retrieve existing player configs */
	(void)RetrievePlayer (cfgType, atom, COLOR_INVALID, &cfgPlayer);
	/* create new player with it */
	newAtom =
		CreatePlayerConfig (cfgType, name, &cfgPlayer.graphics, &cfgPlayer.messages,
							&cfgPlayer.misc, &cfgPlayer.id);
	if (newAtom != atom) {
		if (newAtom != ATOM_INVALID) {
			/* if successful, delete old entry */
			DB_DeleteSection (dbLocal, atom);
		}
		return newAtom;
	}
	return atom;
}								/* RenameLocalPlayerConfig */

/*
 * compare to graphics sets
 */
XBBool
ComparePlayerGraphics (const CFGPlayerGraphics * a, const CFGPlayerGraphics * b)
{
	assert (a != NULL);
	assert (b != NULL);
	return (a->shape == b->shape &&
			a->helmet == b->helmet &&
			a->face == b->face &&
			a->body == b->body &&
			a->handsFeet == b->handsFeet &&
			a->armsLegs == b->armsLegs && a->backpack == b->backpack);
}								/* ComparePlayerGraphics */

/*
 * put player config into telegram send queue
 */
static XBBool
SendAnyPlayerConfig (const DBRoot * db, XBSndQueue * queue, XBTeleCOT cot, XBTeleIOB iob,
					 XBAtom atom, XBBool toCentral)
{
	const DBSection *section;
	int i, k, l;
	size_t len;
	XBTelegram *tele;
	char tmp[256];
	char pass[256];

	assert (db != NULL);
	assert (queue != NULL);
	/* get section with player data */
	section = DB_GetSection (db, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* now print and send data */
	i = 0;
	l = sprintf (pass, "%s", GUI_AtomToString (atomPass));	// XBCC
	while (0 < (len = DB_PrintEntry (tmp, section, i))) {
		k = strncmp (pass, tmp, l) == 0;
		if ((k && toCentral) || !k) {	// XBCC only send password to central
			tele = Net_CreateTelegram (cot, XBT_ID_PlayerConfig, iob, tmp, len + 1);
			assert (tele != NULL);
			Net_SendTelegram (queue, tele);
		}
		i++;
	}
	/* no data means end of section */
	tele = Net_CreateTelegram (cot, XBT_ID_PlayerConfig, iob, NULL, 0);
	assert (tele != NULL);
	Net_SendTelegram (queue, tele);
	return XBTrue;
}								/* SendAnyPlayerConfig */

/*
 * put player config into telegram send queue
 */
XBBool
SendPlayerConfig (CFGType cfgType, XBSndQueue * queue, XBTeleCOT cot, XBTeleIOB iob, XBAtom atom,
				  XBBool toCentral)
{
	return SendAnyPlayerConfig (GetDB (cfgType), queue, cot, iob, atom, toCentral);
}								/* SendLocalPlayerConfig */

/*
 * add entry line to player config
 */
void
AddToPlayerConfig (CFGType cfgType, XBAtom atom, const char *line)
{
	DBRoot *db;
	DBSection *section;

	/* sanity check */
	assert (ATOM_INVALID != atom);
	/* get database */
	db = GetDB (cfgType);
	assert (NULL != db);
	/* create new player section */
	section = DB_CreateSection (db, atom);
	assert (NULL != section);
	/* add line */
	(void)DB_ParseEntry (section, line);
}								/* AddToPlayerConfig */

/*
 * XBST Store game results
 */
static void
StoreAnyGameResult (DBRoot * db, XBAtom atom, int k, int *regPl, int *PID, int *Score)	// XBST
{
	DBSection *section;
	int i, j;

	assert (db != NULL);
	assert (PID != NULL);
	assert (Score != NULL);
	assert (ATOM_INVALID != atom);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	for (i = 0; i < k; i++) {
		j = regPl[i];
		(void)DB_CreateEntryGameResult (section, GUI_IntToAtom (PID[j]), abs (Score[j]));
	}
}								/* StorePlayerRating */

/*
 * store game result in database
 */
void
StoreGameResult (CFGType cfgType, XBAtom atom, int k, int *regPl, int *PID, int *Score)
{								// XBST
	StoreAnyGameResult (GetDB (cfgType), atom, k, regPl, PID, Score);
}								/* StoreGameResult */

/*
 * store game result, append to file and delete in database
 */
void
AppendGameResult (CFGType cfgType, XBAtom fname, XBAtom atom, int k, int *regPl, int *PID,
				  int *Score)
{								/*  XBST */
	DBRoot *res;
	res = DB_Create (cfgType, fname);
	assert (res != NULL);
	StoreAnyGameResult (res, atom, k, regPl, PID, Score);
	DB_Append (res);
	DB_Delete (res);
}								/* AppendGameResult */

static void
StoreAnyTimePlayerRating (DBRoot * db, XBAtom atom, int k, int *regPl, int *PID, float *rating)	// XBST
{
	DBSection *section;
	int i, j;

	assert (db != NULL);
	assert (PID != NULL);
	assert (rating != NULL);
	assert (ATOM_INVALID != atom);

	section = DB_CreateSection (db, atom);
	assert (section != NULL);
	for (i = 0; i < k; i++) {
		j = regPl[i];
		(void)DB_CreateEntryFloat (section, GUI_IntToAtom (PID[j]), rating[j]);
	}
}								/* StorePlayerRating */

/*
 * store timed player rating to database
 */
void
StoreTimePlayerRating (CFGType cfgType, XBAtom atom, int k, int *regPl, int *PID, float *rating)
{								// XBST
	StoreAnyTimePlayerRating (GetDB (cfgType), atom, k, regPl, PID, rating);
}								/* StoreGameResult */

/*
 * store timed player rating, append to file and delete in database
 */
void
AppendTimePlayerRating (CFGType cfgType, XBAtom fname, XBAtom atom, int k, int *regPl, int *PID,
						float *rating)
{								/*  XBST */
	DBRoot *res;
	res = DB_Create (cfgType, fname);
	assert (res != NULL);
	StoreAnyTimePlayerRating (res, atom, k, regPl, PID, rating);
	DB_Append (res);
	DB_Delete (res);
}								/* StoreGameResult */

/*
 * end of file cfg_player.c
 */
