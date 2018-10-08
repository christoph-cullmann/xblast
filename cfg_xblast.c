/*
 * file cfg_xblast.c - general xblast configurations
 *
 * $Id: cfg_xblast.c,v 1.9 2006/02/09 21:21:23 fzago Exp $
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

/* config database */
static DBRoot *db = NULL;

/* default sound */
static const CFGSoundSetup defaultSoundSetup = {
	XBSM_None, XBTrue,
};

/* conversion tables for sound mode (alphanumeric sort by key) */
static DBToInt convSoundModeTable[NUM_XBSM + 1] = {
	{"Beep", XBSM_Beep},
	{"None", XBSM_None},
	{"Waveout", XBSM_Waveout},
	{NULL, NUM_XBSM},
};

/* default video */
static const CFGVideoSetup defaultVideoSetup = {
	XBVM_Windowed,
};

/* conversion tables for sound mode (alphanumeric sort by key) */
static DBToInt convVideoModeTable[NUM_XBSM + 1] = {
	{"fullscreen", XBVM_Full},
	{"windowed", XBVM_Windowed},
	{NULL, NUM_XBSM},
};

/* default central address */
static const CFGCentralSetup defaultCentralSetup = {
	"xblast.debian.net", 16160,
};

/*
 * load xblast config database
 */
void
LoadXBlastConfig (void)
{
	/* create empty database */
	db = DB_Create (DT_Config, atomXblast);
	assert (db != NULL);
	/* load it */
	Dbg_Config ("loading xblast config\n");
	if (DB_Load (db)) {
		return;
	}
	Dbg_Config ("failed to load xblast config, setting defaults\n");
	/* set defaults if load failed */
	StoreSoundSetup (&defaultSoundSetup);
	DB_Store (db);
}								/* LoadXBlastConfig */

/*
 * save xblast config database to disk
 */
void
SaveXBlastConfig (void)
{
	assert (NULL != db);
	if (DB_Changed (db)) {
		Dbg_Config ("saving xblast config\n");
		DB_Store (db);
	}
}								/* SaveXBlastConfig */

/*
 * clear xblast config database
 */
void
FinishXBlastConfig (void)
{
	if (NULL != db) {
		DB_Delete (db);
		db = NULL;
	}
	Dbg_Config ("xblast config cleared\n");
}								/* FinishXBlastConfig */

/*
 * store setup for sound
 */
void
StoreSoundSetup (const CFGSoundSetup * sound)
{
	DBSection *section;
	const char *key;
	assert (NULL != sound);
	assert (NULL != db);
	/* create section for sound */
	section = DB_CreateSection (db, atomSound);
	assert (NULL != section);
	/* add setup */
	if (NULL != (key = DB_IntToString (convSoundModeTable, sound->mode))) {
		DB_CreateEntryString (section, atomMode, key);
	}
	else {
		DB_DeleteEntry (section, atomMode);
	}
	DB_CreateEntryBool (section, atomStereo, sound->stereo);
    /* AbsInt begin */
    DB_CreateEntryBool (section, atomBeep, sound->beep);
    /* AbsInt end */
}								/* StoreSoundSetup */

/*
 * get ound setup from config
 */
XBBool
RetrieveSoundSetup (CFGSoundSetup * sound)
{
	const DBSection *section;
	assert (NULL != db);
	assert (NULL != sound);
	/* set default value */
	*sound = defaultSoundSetup;
	/* find section for sound */
	if (NULL == (section = DB_GetSection (db, atomSound))) {
		return XBFalse;
	}
	/* parse section */
	DB_ConvertEntryInt (section, atomMode, (int *)&sound->mode, convSoundModeTable);
	DB_GetEntryBool (section, atomStereo, &sound->stereo);
    /* AbsInt begin */
    DB_GetEntryBool (section, atomBeep, &sound->beep);
    /* AbsInt end */
	/* that's all */
	return XBTrue;
}								/* RetrieveSoundSetup */

/*
 * store video setup
 */
void
StoreVideoSetup (const CFGVideoSetup * video)
{
	DBSection *section;
	const char *key;
	assert (video);
	assert (db);

	section = DB_CreateSection (db, atomVideo);
	assert (section);

	/* add setup */
	if ((key = DB_IntToString (convVideoModeTable, video->mode))) {
		DB_CreateEntryString (section, atomMode, key);
	}
	else {
		DB_DeleteEntry (section, atomMode);
	}
}

/*
 * get video setup from config
 */
XBBool
RetrieveVideoSetup (CFGVideoSetup * video)
{
	const DBSection *section;

	assert (db);
	assert (video);

	/* set default value */
	*video = defaultVideoSetup;

	if (NULL == (section = DB_GetSection (db, atomVideo))) {
		return XBFalse;
	}

	DB_ConvertEntryInt (section, atomMode, (int *)&video->mode, convVideoModeTable);

	return XBTrue;
}

/*
 * store central ceonnection parameters
 */
void
StoreCentralSetup (const CFGCentralSetup * central)
{
	DBSection *section;
	assert (NULL != central);
	assert (NULL != db);
	/* create section for central */
	section = DB_CreateSection (db, atomCentral);
	assert (NULL != section);
	/* store host and port */
	DB_CreateEntryString (section, atomCentralJoinName, central->name);
	DB_CreateEntryInt (section, atomCentralJoinPort, central->port);
}								/* StoreCentralSetup */

/*
 * retrieve central connection parameters
 */
XBBool
RetrieveCentralSetup (CFGCentralSetup * central)
{
	const DBSection *section;
	assert (NULL != db);
	assert (NULL != central);
	/* set default value */
	*central = defaultCentralSetup;
	/* find section for central */
	if (NULL == (section = DB_GetSection (db, atomCentral))) {
		return XBFalse;
	}
	/* get central host name and port */
	DB_GetEntryString (section, atomCentralJoinName, &central->name);
	DB_GetEntryInt (section, atomCentralJoinPort, &central->port);
	/* that's all */
	return XBTrue;
}								/* RetrieveCentralSetup */

/*
 * end of file cfg_xblast.c
 */
