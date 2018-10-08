/*
 * file cfg_stat.c - level and game statistics
 *
 * $Id: cfg_stat.c,v 1.9 2006/02/24 21:29:16 fzago Exp $
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
 * local types
 */
typedef struct
{
	int numWon;					/* number of games won */
	int numTotal;				/* number of games played */
	double numScore;			/* total numbr of points achieved */
} StatEntry;

/*
 * local variables
 */
static DBRoot *dbLevel = NULL;
static DBRoot *dbPlayer = NULL;

/*
 * init stat entry
 */
static void
SetStatEntry (StatEntry * stat, int numWon, int numTotal, double numScore)
{
	assert (NULL != stat);
	stat->numWon = numWon;
	stat->numTotal = numTotal;
	stat->numScore = numScore;
}								/* SetStatEntry */

/*
 * add two entry
 */
static void
AddToStatEntry (StatEntry * dst, const StatEntry * src)
{
	assert (NULL != dst);
	assert (NULL != src);

	dst->numWon += src->numWon;
	dst->numTotal += src->numTotal;
	dst->numScore += src->numScore;
}								/* AddToStatEntry */

/*
 * get and parse statitics entry
 */
static XBBool
GetStatEntry (const DBSection * section, XBAtom atom, StatEntry * stat)
{
	const char *s;

	assert (NULL != section);
	assert (ATOM_INVALID != atom);
	assert (NULL != stat);
	if (!DB_GetEntryString (section, atom, &s)) {
		return XBFalse;
	}
	if (3 != sscanf (s, "%d %d %lf", &stat->numWon, &stat->numTotal, &stat->numScore)) {
		return XBFalse;
	}
	return XBTrue;
}								/* GetStatEntry */

/*
 *
 */
static void
SetStatData (XBStatData * data, XBAtom atom, const char *name, const StatEntry * stat)
{
	assert (NULL != data);
	assert (NULL != stat);

	data->atom = atom;
	data->name = name;
	data->numWon = stat->numWon;
	data->numTotal = stat->numTotal;
	data->scoreTotal = stat->numScore;
	if (stat->numTotal) {
		data->percent = 100.0 * (double)stat->numWon / (double)stat->numTotal;
		data->average = stat->numScore / (double)stat->numTotal;
	}
	else {
		data->percent = 0.0;
		data->average = 0.0;
	}
}								/* SetStatData */

/*
 * create stat entry for existing secton
 */
static void
CreateStatEntry (DBRoot * db, XBAtom sAtom, XBAtom eAtom, const StatEntry * stat)
{
	char tmp[256];
	DBSection *section;

	/* sanity check */
	assert (NULL != db);
	assert (ATOM_INVALID != sAtom);
	assert (ATOM_INVALID != eAtom);
	assert (NULL != stat);
	/* create section */
	section = DB_CreateSection (db, sAtom);
	assert (NULL != section);
	/* create entry */
	sprintf (tmp, "%d %d %f", stat->numWon, stat->numTotal, stat->numScore);
	DB_CreateEntryString (section, eAtom, tmp);
}								/* CreateEntryString */

/*
 * load statistics data
 */
void
LoadStatConfig (void)
{
	size_t i, j;
	XBAtom player;
	XBAtom level;
	const DBSection *section;
	StatEntry stat;
	StatEntry sumStat;

	/* create empty database for level statistics */
	dbLevel = DB_Create (DT_Config, atomLevelStat);
	assert (dbLevel != NULL);
	/* load from file */
	Dbg_Config ("loading level statistics\n");
	if (!DB_Load (dbLevel)) {
		Dbg_Config ("no level statistics found!\n");
	}
	/* create empty database  for player statistics */
	dbPlayer = DB_Create (DT_Config, atomPlayerStat);
	assert (dbLevel != NULL);
	/* calculate player statistics from level statistics */
	i = 0;
	while (ATOM_INVALID != (level = DB_IndexSection (dbLevel, i++))) {
		/* load section for one level */
		section = DB_GetSection (dbLevel, level);
		assert (section != NULL);
		j = 0;
		while (ATOM_INVALID != (player = DB_IndexEntry (section, j++))) {
			if (GetStatEntry (section, player, &stat)) {
				CreateStatEntry (dbPlayer, player, level, &stat);
			}
		}
	}
	/* build totals */
	j = 0;
	while (ATOM_INVALID != (player = DB_IndexSection (dbPlayer, j++))) {
		SetStatEntry (&sumStat, 0, 0, 0);
		/* load section for one level */
		section = DB_GetSection (dbPlayer, player);
		assert (NULL != section);
		i = 0;
		while (ATOM_INVALID != (level = DB_IndexEntry (section, i++))) {
			if (GetStatEntry (section, level, &stat)) {
				AddToStatEntry (&sumStat, &stat);
			}
		}
		CreateStatEntry (dbPlayer, player, atomTotal, &sumStat);
	}
}								/* LoadStatConfig */

/*
 * save data to disk
 */
void
SaveStatConfig (void)
{
	assert (dbLevel != NULL);
	if (DB_Changed (dbLevel)) {
		DB_Store (dbLevel);
		Dbg_Config ("storing level statistics\n");
	}
#ifdef DEBUG
	assert (dbPlayer != NULL);
	DB_Store (dbPlayer);
	Dbg_Config ("storing player statistics\n");
#endif
}								/* SaveStatConfig */

/*
 * clean up
 */
void
FinishStatConfig (void)
{
	if (NULL != dbLevel) {
		DB_Delete (dbLevel);
	}
	if (NULL != dbPlayer) {
		DB_Delete (dbPlayer);
	}
	Dbg_Config ("statistics cleared\n");
}								/* FinishStatConfig */

/*
 * store level result for one player
 */
void
StoreLevelStat (XBAtom level, XBAtom player, XBBool won, double score)
{
	const DBSection *section;
	StatEntry stat;
	StatEntry result;

	/* set level result for player */
	SetStatEntry (&result, won ? 1 : 0, 1, score);

	/* level statistics */
	if (NULL == (section = DB_GetSection (dbLevel, level)) ||
		!GetStatEntry (section, player, &stat)) {
		SetStatEntry (&stat, 0, 0, 0);
	}
	/* new standings */
	AddToStatEntry (&stat, &result);
	/* store it */
	CreateStatEntry (dbLevel, level, player, &stat);

	/*player statistics ... */
	if (NULL == (section = DB_GetSection (dbPlayer, player)) ||
		!GetStatEntry (section, level, &stat)) {
		SetStatEntry (&stat, 0, 0, 0);
	}
	/* new standings */
	AddToStatEntry (&stat, &result);
	/* store it */
	CreateStatEntry (dbPlayer, player, level, &stat);
}								/* StoreLevelStat */

/*
 * compare stat-data by percent
 */
static int
CompareStatDataByPercent (const void *a, const void *b)
{
	const XBStatData *pA = a;
	const XBStatData *pB = b;

	assert (pA != NULL);
	assert (pB != NULL);
	if (pA->percent == pB->percent) {
		return pB->numTotal - pA->numTotal;
	}
	else if (pA->percent > pB->percent) {
		return -1;
	}
	else {
		return +1;
	}
}								/* CompareStatDataByPercent */

#ifdef UNNEEDED
/*
 * compare stat-data by percent
 */
static int
CompareStatDataByAverage (const void *a, const void *b)
{
	const XBStatData *pA = a;
	const XBStatData *pB = b;

	assert (pA != NULL);
	assert (pB != NULL);
	if (pA->average == pB->average) {
		return pB->numTotal - pA->numTotal;
	}
	else if (pA->average > pB->average) {
		return -1;
	}
	else {
		return +1;
	}
}								/* CompareStatDataByPercent */
#endif

/*
 * compare stat-data by names
 */
static int
CompareStatDataByName (const void *a, const void *b)
{
	const XBStatData *pA = a;
	const XBStatData *pB = b;
	int result;

	assert (pA != NULL);
	assert (pB != NULL);
	/* first try to compare by names */
	if (pA->name == NULL) {
		if (pB->name != NULL) {
			return -1;
		}
	}
	else {
		if (pB->name == NULL) {
			return 1;
		}
		else {
			if (0 != (result = strcmp (pA->name, pB->name))) {
				return result;
			}
		}
	}
	/* next by percentage */
	if (pA->percent == pB->percent) {
		return pB->numTotal - pA->numTotal;
	}
	else if (pA->percent > pB->percent) {
		return -1;
	}
	else {
		return +1;
	}
}								/* CompareStatDataByPercent */

/*
 * swap given element to front of table
 */
static XBBool
StatDataRise (XBStatData * table, size_t num, XBAtom atom)
{
	size_t i;
	XBStatData swap;

	assert (NULL != table);
	for (i = 0; i < num; i++) {
		if (table[i].atom == atom) {
			swap = table[i];
			table[i] = table[0];
			table[0] = swap;
			return XBTrue;
		}
	}
	return XBFalse;
}								/* StatDataRise */

/*
 * level atom to name
 */
static const char *
LevelAtomToName (XBAtom atom)
{
	if (atomTotal == atom) {
		return N_("All Levels");
	}
	else {
		return GetLevelNameByAtom (atom);
	}
}								/* LevelAtomToName */

/*
 * player atom to name
 */
static const char *
PlayerAtomToName (XBAtom atom)
{
	if (atom == atomDrawGame) {
		return N_("Draw Games");
	}
	else if (atom == atomOutOfTime) {
		return N_("Out of Time");
	}
	else {
		return NULL;
	}
}								/* PlayerAtomToName */

/*
 * create stat table a given player
 */
XBStatData *
CreatePlayerSingleStat (XBAtom player, size_t * pNum)
{
	const DBSection *section;
	XBStatData *table;
	size_t i, j;
	StatEntry stat;
	XBAtom level;

	assert (ATOM_INVALID != player);
	assert (NULL != pNum);

	/* get section for player */
	assert (dbPlayer != NULL);
	section = DB_GetSection (dbPlayer, player);
	assert (section != NULL);
	/* alloc data for table */
	*pNum = DB_NumEntries (section);
	table = calloc (*pNum, sizeof (XBStatData));
	assert (NULL != table);
	/* copy data */
	for (i = 0, j = 0; i < *pNum; i++) {
		level = DB_IndexEntry (section, i);
		if (GetStatEntry (section, level, &stat) && stat.numTotal > 0) {
			SetStatData (table + j, level, LevelAtomToName (level), &stat);
			j++;
		}
	}
	*pNum = j;
	if (*pNum == 0) {
		free (table);
		return NULL;
	}
	/* find total */
	StatDataRise (table, *pNum, atomTotal);
	/* sort by percent */
	qsort (table + 1, *pNum - 1, sizeof (XBStatData), CompareStatDataByPercent);
	/* that's all */
	return table;
}								/* CreatePlayerStat */

/*
 * create stat table with all players 
 */
XBStatData *
CreatePlayerTotalStat (size_t * pNum)
{
	XBStatData *table;
	size_t i, j;
	const DBSection *section;
	StatEntry stat;
	XBAtom player;

	assert (NULL != pNum);

	/* get number of players */
	assert (dbPlayer != NULL);
	*pNum = DB_NumSections (dbPlayer);
	if (0 == *pNum) {
		return NULL;
	}
	table = calloc (*pNum, sizeof (XBStatData));
	assert (table != NULL);
	/* insert players */
	for (i = 0, j = 0; i < *pNum; i++) {
		player = DB_IndexSection (dbPlayer, i);
		assert (ATOM_INVALID != player);
		section = DB_GetSection (dbPlayer, player);
		assert (NULL != section);
		if (GetStatEntry (section, atomTotal, &stat)) {
			SetStatData (table + j, player, PlayerAtomToName (player), &stat);
			j++;
		}
	}
	*pNum = j;
	if (*pNum == 0) {
		free (table);
		return NULL;
	}
	/* bring draw game und and out of time to the top */
	StatDataRise (table, *pNum, atomDrawGame);
	StatDataRise (table + 1, *pNum - 1, atomOutOfTime);
	/* sort by percent */
	qsort (table + 2, *pNum - 2, sizeof (XBStatData), CompareStatDataByPercent);
	return table;
}								/* CreatePlayerTotalStat */

/*
 * create stat table with all players 
 */
XBStatData *
CreateLevelTotalStat (size_t * pNum)
{
	XBStatData *table;
	size_t i, j;
	const DBSection *section;
	StatEntry stat;
	StatEntry statDrawGame;
	StatEntry statOutOfTime;
	XBAtom level;

	assert (NULL != pNum);

	/* get number of levels */
	assert (dbLevel != NULL);
	*pNum = DB_NumSections (dbLevel);
	if (0 == *pNum) {
		return NULL;
	}
	table = calloc (*pNum, sizeof (XBStatData));
	assert (table != NULL);
	/* insert levels */
	for (i = 0, j = 0; i < *pNum; i++) {
		/* init values */
		level = DB_IndexSection (dbLevel, i);
		assert (ATOM_INVALID != level);
		/* find section for level */
		section = DB_GetSection (dbLevel, level);
		assert (NULL != section);
		/* get draw games */
		if (!GetStatEntry (section, atomDrawGame, &statDrawGame)) {
			SetStatEntry (&statDrawGame, 0, 0, 0);
		}
		if (!GetStatEntry (section, atomDrawGame, &statOutOfTime)) {
			SetStatEntry (&statOutOfTime, 0, 0, 0);
		}
		SetStatEntry (&stat, statDrawGame.numTotal,
					  statDrawGame.numTotal - statDrawGame.numWon - statOutOfTime.numWon, 0.0);
		if (stat.numTotal > 0) {
			SetStatData (table + j, level, LevelAtomToName (level), &stat);
			j++;
		}
	}
	*pNum = j;
	if (*pNum == 0) {
		free (table);
		return NULL;
	}
	/* sort by percent */
	qsort (table, *pNum, sizeof (XBStatData), CompareStatDataByName);
	return table;
}								/* CreateLevelTotalStat */

/*
 * create stat table a given player
 */
XBStatData *
CreateLevelSingleStat (XBAtom level, size_t * pNum)
{
	const DBSection *section;
	XBStatData *table;
	size_t i, j;
	StatEntry stat;
	XBAtom player;

	assert (ATOM_INVALID != level);
	assert (NULL != pNum);

	/* get section for level */
	assert (dbLevel != NULL);
	section = DB_GetSection (dbLevel, level);
	assert (section != NULL);
	/* alloc data for table */
	*pNum = DB_NumEntries (section);
	table = calloc (*pNum, sizeof (XBStatData));
	assert (NULL != table);
	/* copy data */
	for (i = 0, j = 0; i < *pNum; i++) {
		player = DB_IndexEntry (section, i);
		if (GetStatEntry (section, player, &stat)) {
			SetStatData (table + j, player, PlayerAtomToName (player), &stat);
			j++;
		}
	}
	*pNum = j;
	if (*pNum == 0) {
		free (table);
		return NULL;
	}
	/* find draw games and out of time */
	StatDataRise (table, *pNum, atomDrawGame);
	StatDataRise (table + 1, *pNum - 1, atomOutOfTime);
	/* sort by percent */
	qsort (table + 2, *pNum - 2, sizeof (XBStatData), CompareStatDataByPercent);
	/* that's all */
	return table;
}								/* CreateLevelStat */

/*
 * end of file cfg_stat.c
 */
