/*
 * file cfg_demo.c - configuration data for recorded demos
 *
 * $Id: cfg_demo.c,v 1.11 2006/02/24 21:29:16 fzago Exp $
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

/* current demo */
DBRoot *dbDemo = NULL;

/*
 * local variables
 */

/* list of all demos */
static DBRoot *dbList = NULL;
/* action keys for current demo */
static DBSection *frameSection = NULL;

/**********************
 * managing all demos *
 **********************/

/*
 * initialize current demo and list
 */
void
LoadDemoConfig (void)
{
	/* create demo database for current demo */
	dbDemo = DB_Create (DT_Demo, atomDemo);
	assert (NULL != dbDemo);
	/* load list of recorded demos */
	dbList = DB_Create (DT_Config, atomDemo);
	assert (NULL != dbDemo);
	Dbg_Config ("loading list of demos\n");
	if (DB_LoadDir (dbList, "demo", "dem", DT_Demo, atomTime, atomInfo, NULL, XBFalse)) {
		SaveDemoConfig ();
	}
	else {
		Dbg_Config ("no changes found\n");
	}
}								/* LoadDemoConfig */

/*
 * store current list of demos in config
 */
void
SaveDemoConfig (void)
{
	/* list of demos database */
	assert (dbList != NULL);
	if (DB_Changed (dbList)) {
		Dbg_Config ("saving list of demos\n");
		DB_Store (dbList);
	}
}								/* SaveDemoConfig */

/*
 * clear current demo and list
 */
void
FinishDemoConfig (void)
{
	/* remove single demo database */
	assert (dbDemo != NULL);
	DB_Delete (dbDemo);
	dbDemo = NULL;
	/* save and remove list of demos database */
	SaveDemoConfig ();
	DB_Delete (dbList);
	dbList = NULL;
	Dbg_Config ("demo config cleared\n");
}								/* FinishDemoConfig */

/*
 * compare to demo entries by time
 */
static int
CompareDemoEntries (const void *a, const void *b)
{
	assert (NULL != a);
	assert (NULL != b);
	return ((const CFGDemoEntry *) b)->time - ((const CFGDemoEntry *) a)->time;
}								/* CompareDemoEntries */

/*
 * create list with all demos available
 */
CFGDemoEntry *
CreateDemoList (size_t * num)
{
	CFGDemoEntry *list;
	size_t i, j;
	XBAtom atom;
	const DBSection *section;

	/* sanity check */
	assert (NULL != dbList);
	assert (NULL != num);
	/* alloc data */
	*num = DB_NumSections (dbList);
	if (0 == *num) {
		return NULL;
	}
	list = calloc (*num, sizeof (CFGDemoEntry));
	/* build list */
	for (i = 0, j = 0; i < *num; i++) {
		atom = DB_IndexSection (dbList, i);
		section = DB_GetSection (dbList, atom);
		assert (NULL != section);
		list[j].atom = atom;
		(void)DB_GetEntryAtom (section, atomLevel, &list[j].level);
		if (NULL == GetLevelNameByAtom (list[j].level)) {
			continue;
		}
		(void)DB_GetEntryInt (section, atomNumPlayers, &list[j].numPlayers);
		(void)DB_GetEntryTime (section, atomRecorded, &list[j].time);
		j++;
	}
	*num = j;
	/* sort it */
	qsort (list, *num, sizeof (CFGDemoEntry), CompareDemoEntries);
	return list;
}								/* CreateDemoList */

/*****************************
 * current demo from/to file *
 *****************************/

/*
 * load current demo from file
 */
XBBool
LoadDemoFromFile (XBAtom atom)
{
	/* clear current demo */
	assert (NULL != dbDemo);
	DB_Delete (dbDemo);
	/* create new demo */
	dbDemo = DB_Create (DT_Demo, atom);
	assert (NULL != dbDemo);
	/* load and return success */
	return DB_Load (dbDemo);
}								/* LoadDemoFromFile */

/*
 * store current demo in file
 */
void
SaveCurrentDemo (const char *filename)
{
	/* set file name for output */
	DB_SetAtom (dbDemo, GUI_StringToAtom (filename));
	/* single demo */
	assert (NULL != dbDemo);
	DB_Store (dbDemo);
	/* TODO: add file to demoList !!! */
}								/* SaveCurrentDemo */

/***********************
 * current demo config *
 ***********************/

/*
 * store config data for a demo in current database
 */
void
StoreDemoConfig (const CFGDemoInfo * cfgDemo)
{
	DBSection *section;
	/* sanity checks */
	assert (NULL != cfgDemo);
	assert (NULL != dbDemo);
	/* create empty section */
	DB_DeleteSection (dbDemo, atomInfo);
	section = DB_CreateSection (dbDemo, atomInfo);
	assert (NULL != section);
	/* store data */
	(void)DB_CreateEntryInt (section, atomNumPlayers, cfgDemo->numPlayers);
	(void)DB_CreateEntryInt (section, atomNumFrames, cfgDemo->numFrames);
	(void)DB_CreateEntryAtom (section, atomLevel, cfgDemo->level);
	(void)DB_CreateEntryInt (section, atomRandomSeed, cfgDemo->randomSeed);
	(void)DB_CreateEntryTime (section, atomRecorded, cfgDemo->time);
	(void)DB_CreateEntryInt (section, atomLives, cfgDemo->numLives);
	(void)DB_CreateEntryInt (section, atomFrameRate, cfgDemo->frameRate);
	(void)DB_CreateEntryBool (section, atomRandomPlayers, cfgDemo->randomPlayers);
	(void)DB_CreateEntryInt (section, atomWinner, cfgDemo->winner);
}								/* StoreDemoConfig */

/*
 * get config data for current demo from database
 */
XBBool
RetrieveDemoConfig (CFGDemoInfo * cfgDemo)
{
	const DBSection *section;
	/* sanity checks */
	assert (NULL != cfgDemo);
	assert (NULL != dbDemo);
	/* find section */
	if (NULL == (section = DB_GetSection (dbDemo, atomInfo))) {
		return XBFalse;
	}
	/* get data */
	if (!DB_GetEntryInt (section, atomNumPlayers, &cfgDemo->numPlayers) ||
		!DB_GetEntryInt (section, atomNumFrames, &cfgDemo->numFrames) ||
		!DB_GetEntryAtom (section, atomLevel, &cfgDemo->level) ||
		!DB_GetEntryInt (section, atomRandomSeed, &cfgDemo->randomSeed) ||
		!DB_GetEntryTime (section, atomRecorded, &cfgDemo->time) ||
		!DB_GetEntryInt (section, atomLives, &cfgDemo->numLives) ||
		!DB_GetEntryInt (section, atomFrameRate, &cfgDemo->frameRate) ||
		!DB_GetEntryBool (section, atomRandomPlayers, &cfgDemo->randomPlayers)) {
		return XBFalse;
	}
	return XBTrue;
}								/* RetrieveDemoConfig */

/****************************
 * current demo: frame data *
 ****************************/

/*
 * store data for single frame in current demo database
 */
void
StoreDemoFrame (int gameTime, int numPlayers, const unsigned char *buf)
{
	int i;
	size_t len = 0;
	char tmp[3 * MAX_PLAYER + 1] = "";
	/* create frame section if needed */
	if (NULL == frameSection) {
		assert (NULL != dbDemo);
		frameSection = DB_CreateSection (dbDemo, atomFrames);
		assert (NULL != frameSection);
	}
	/* create the entry */
	for (i = 0; i < numPlayers; i++) {
		len += sprintf (tmp + len, "%02X ", (unsigned)buf[i]);
	}
	(void)DB_CreateEntryString (frameSection, GUI_IntToAtom (gameTime), tmp);
}								/* StoreDemoFrame */

/*
 * copy frames of current demo into a buffer
 */
XBBool
RetrieveDemoFrames (int numPlayers, unsigned char (*buffer)[MAX_PLAYER])
{
	const DBSection *section;
	size_t i, j, num;
	int frame;
	const char *s;
	unsigned value;
	/* sanity check */
	assert (numPlayers <= MAX_PLAYER);
	assert (buffer != NULL);
	assert (dbDemo != NULL);
	/* find frames section */
	if (NULL == (section = DB_GetSection (dbDemo, atomFrames))) {
		return XBFalse;
	}
	/* get number of entries */
	num = DB_NumEntries (section);
	/* loop through entries */
	for (i = 0; i < num; i++) {
		/* get indexed entry */
		XBAtom atom = DB_IndexEntry (section, i);
		assert (ATOM_INVALID != atom);
		/* check for valid game time */
		frame = GUI_AtomToInt (atom);
		if (frame < 0 || frame >= GAME_TIME) {
			return XBFalse;
		}
		/* get action string */
		if (!DB_GetEntryString (section, atom, &s)) {
			return XBFalse;
		}
		/* validate length */
		if (strlen (s) < 2 * numPlayers) {
			return XBFalse;
		}
		/* try to extract keys for each player */
		for (j = 0; j < numPlayers; j++) {
			if (1 != sscanf (s + 3 * j, "%x", &value)) {
				return XBFalse;
			}
			/* store in buffer */
			buffer[frame][j] = (unsigned char)value;
		}
	}
	return XBTrue;
}								/* RetrieveDemoFrames */

/*
 * clear frame section of current database
 */
void
DeleteDemoFrames (void)
{
	assert (NULL != dbDemo);
	DB_DeleteSection (dbDemo, atomFrames);
	frameSection = NULL;
}								/* ClearDemoFrames */

/*
 * end of file cfg_demo.c
 */
