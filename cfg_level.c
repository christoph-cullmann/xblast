/*
 * cfg_level.c - managing level data
 *
 * $Id: cfg_level.c,v 1.14 2005/01/14 19:57:40 lodott Exp $
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

#include "cfg_level.h"
#include "level.h"
#include "atom.h"
#include "str_util.h"
#include "util.h"
#include "random.h"
#include "version.h"

/*
 * level info structure
 */

/*
 * local marcos
 */
#define MAX_SHUFFLE (2<<24)

#define IOB_INFO          0u
#define IOB_PLAYER        1u
#define IOB_SHRINK        2u
#define IOB_SCRAMBLE_DRAW 3u
#define IOB_SCRAMBLE_DEL  4u
#define IOB_FUNC          5u
#define IOB_BOMBS         6u
#define IOB_GRAPHICS      7u
#define IOB_MAP           8u
#define NUM_IOBS          9u
#define IOB_NAME          254u
#define IOB_INVALID       255u

/*
 * local types
 */
typedef struct {
  union {
    const char *name;  /* name to sort by */
    int         value; /* random number to sort by */
    time_t      lastplayed; /* last play date to sort by */
  }            key;
  XBAtom       level; /* level */
} LevelTable;
/* lookup table for shuffled levels */
typedef struct {
  XBAtom level; /* level */
} LevelShuffled;
/*
 * local variables
 */
static DBRoot        *dbLevel         = NULL;
static LevelTable    *sortedTable     = NULL;
static LevelTable    *shuffledTable   = NULL;
static LevelTable    *timeTable       = NULL; /* Better level randomness */
static const char    *gameModeString  = "R23456STDL";
static unsigned       gameMode        = 0;
static XBBool         randomOrder     = XBFalse;
static int            levelOrder      = 0;
static XBBool         allLevels       = XBTrue;
static int            indexSorted     = 0;
static int            indexShuffled   = 0;
static int            indexTime       = 0;

static DBRoot        *levelRemote     = NULL;
static DBRoot        *levelLocal      = NULL;

/******************
 * local routines *
 ******************/

/*
 * additional info for new levels files
 */
static void
InsertLevel (DBSection *section)
{
  /* new levels are automatically selected */
  DB_CreateEntryBool (section, atomSelect, XBTrue);
  /* set new value for level shuffling */
  (void) DB_CreateEntryTime(section, atomLastPlayed, 0);
} /* InsertLevel */

/*
 * get game mode string for current level, given section
 */
static unsigned
GetLevelGameMode (XBAtom atom)
{
  const DBSection *section;
  const char      *s;
  int              i;
  unsigned         gameMode = 0;
  if (NULL == (section = DB_GetSection (dbLevel, atom) ) ) {
    return 0;
  }
  if (! DB_GetEntryString (section, atomGameMode, &s) ) {
    return 0;
  }
  for (i = 0; s[i] != 0 && gameModeString[i] != 0; i ++) {
    if (s[i] == gameModeString[i]) {
      gameMode |= (1 << i);
    }
  }
  return gameMode;
} /* GetLevelGameMode */

/*
 * get a level/atom entry from current level
 */
static const char *
GetLevelStringByAtom (XBAtom level, XBAtom atom)
{
  const char      *s;
  const DBSection *section;
  assert (dbLevel != NULL);
  section = DB_GetSection (dbLevel, level);
  if (NULL == section) {
    return NULL;
  }
  if (! DB_GetEntryString (section, atom, &s) ) {
    return NULL;
  }
  return s;
} /* GetLevelString */

/*
 * get shuffle index for current level, section atom
 */
static XBBool
GetLevelShuffle (XBAtom level, int *pShuffle)
{
  const DBSection *section;
  /* sanity checks */
  assert (pShuffle != NULL);
  assert (dbLevel  != NULL);
  /* get entry for level */
  section = DB_GetSection (dbLevel, level);
  if (NULL == section) {
    return XBFalse;
  }
  if (! DB_GetEntryInt (section, atomShuffle, pShuffle) ) {
    return XBFalse;
  }
  return XBTrue;
} /* GetLevelShuffle */

/*
 * store shuffle index in current level, section atom
 */
static void
StoreLevelShuffle (XBAtom atom, int shuffle)
{
  DBSection *section;
  assert (NULL != dbLevel);
  section = DB_CreateSection (dbLevel, atom);
  assert (section != NULL);
  /* we only change data not stored in file */
  (void) DB_CreateEntryInt (section, atomShuffle, shuffle);
} /* StoreLevelShuffle */

/*
 * get time from current level, section level
 */
static XBBool
GetLevelTime (XBAtom level, time_t *lpTime)
{
  const DBSection *section;
  /* sanity checks */
  assert (lpTime != NULL);
  assert (dbLevel  != NULL);
  /* get entry for level */
  section = DB_GetSection (dbLevel, level);
  if (NULL == section) {
    return XBFalse;
  }
  if (! DB_GetEntryTime (section, atomLastPlayed, lpTime) ) {
    return XBFalse;
  }
  return XBTrue;
} /* GetLevelTime */

/*
 * store time in current level, section level
 */
static void
StoreLevelTime (XBAtom atom, int lpTime)
{
  DBSection *section;
  assert (NULL != dbLevel);
  section = DB_CreateSection (dbLevel, atom);
  assert (section != NULL);
  /* we only change data not stored in file */
  (void) DB_CreateEntryTime (section, atomLastPlayed, lpTime);
} /* StoreLevelTime */

/*
 * set flag in current level, given section/entry
 */
static void
SetLevelWasLast (XBAtom level, XBAtom entry, XBBool flag)
{
  DBSection *section;
  section = DB_CreateSection (dbLevel, level);
  assert (section != NULL);
  (void) DB_CreateEntryBool (section, entry, flag);
} /* SetWasLastLevel */

/*
 * get flag in current level, given section/entry
 */
static XBBool
GetLevelWasLast (XBAtom level, XBAtom entry)
{
  const DBSection *section;
  XBBool result;
  section = DB_GetSection (dbLevel, level);
  assert (section != NULL);
  if (! DB_GetEntryBool (section, entry, &result) ) {
    return XBFalse;
  }
  return result;
} /* GetWasLastLevel */

/*
 * compare level names by name
 */
static int
CompareSorted (const void *a, const void *b)
{
  return strcmp ( ( (LevelTable *) a)->key.name, ( (LevelTable *) b)->key.name);
} /* CompareSorted */

/*
 * compare levels by shuffle value
 */
static int
CompareShuffle (const void *a, const void *b)
{
  return ( (LevelTable *) a)->key.value - ( (LevelTable *) b)->key.value;
} /* CompareShuffle */

/*
 * compare levels by time
 */
static int
CompareTime (const void *a, const void *b)
{
  return ( (LevelTable *) a)->key.lastplayed - ( (LevelTable *) b)->key.lastplayed;
} /* CompareTime */

/*
 * return level index after sorting by name
 */
static int
GetLastIndexSorted (void)
{
  int              i,numLevels;
  XBAtom           atom;
  /* allocate table if necessary */
  numLevels = GetNumLevels ();
  if (NULL == sortedTable) {
    sortedTable = calloc (numLevels, sizeof (LevelTable) );
    assert (sortedTable != NULL);
  }
  /* enter level names */
  for (i = 0; i < numLevels; i ++) {
    atom = GetLevelAtom (i);
    sortedTable[i].level    = atom;
    sortedTable[i].key.name = GetLevelNameByAtom (atom);
  }
  /* sort the table */
  qsort (sortedTable, numLevels,  sizeof (LevelTable), CompareSorted);
  /* find first level marked as sorted*/
  for (i = 0; i < numLevels; i ++) {
    if (GetLevelWasLast (sortedTable[i].level, atomLevelSorted) ) {
      Dbg_Level ("last level (sorted)   = %s\n", GetLevelNameByAtom (sortedTable[i].level) );
      return i;
    }
  }
  return numLevels;
} /* GetLastIndexSorted */

/*
 * return level index after sorting by value
 */
static int
GetLastIndexShuffled (void)
{
  int i, miss, max, swap, save, numLevels;
  /* allocate shuffledTable if necessary */
  numLevels = GetNumLevels ();
  if (NULL == shuffledTable) {
    shuffledTable = calloc (numLevels, sizeof (LevelTable) );
    assert (shuffledTable != NULL);
  }
  /* build table, count missing values, get next value */
  miss = 0;
  max = 0;
  for (i = 0; i < numLevels; i ++) {
    shuffledTable[i].level = GetLevelAtom (i);
    assert (ATOM_INVALID != shuffledTable[i].level);
    if (!GetLevelShuffle (shuffledTable[i].level, &shuffledTable[i].key.value) ) {
      shuffledTable[i].key.value = -1;
      miss++;
    }
    if (shuffledTable[i].key.value > max) {
      max = shuffledTable[i].key.value;
    }
  }
  Dbg_Level("%u missing shuffle values, %u is current max\n", miss, max);
  /* assign new values to missing */
  for (i = 0; i < numLevels; i ++) {
    if (shuffledTable[i].key.value < 0) {
      shuffledTable[i].key.value = -max;
      max++;
    }
  }
  Dbg_Level("update max value\n", max);
  /* shuffle missing values */
  for (i=0; i<numLevels; i++) {
    if (shuffledTable[i].key.value < 0) {
      swap = OtherRandomNumber(numLevels);
      save = -shuffledTable[i].key.value;
      shuffledTable[i].key.value = abs(shuffledTable[swap].key.value);
      shuffledTable[swap].key.value = save;
    }
  }
  /* update shuffle values in database */
  for (i = 0; i < numLevels; i ++) {
    StoreLevelShuffle(shuffledTable[i].level, shuffledTable[i].key.value);
  }
  /* sort the table by value */
  qsort (shuffledTable, numLevels,  sizeof (LevelTable), CompareShuffle);
  /* get last level played from database */
  for (i = 0; i < numLevels; i ++) {
    if (GetLevelWasLast (shuffledTable[i].level, atomLevelShuffled) ) {
      Dbg_Level ("last level (shuffled) = %s\n", GetLevelNameByAtom (shuffledTable[i].level) );
      return i;
    }
  }
  return numLevels;
} /* GetLastIndexShuffled */

/*
 * return last level after sorting by time
 */
static int
GetLastIndexTime (void)
{
  int i, numLevels;
  /* allocate timeTable if necessary */
  numLevels = GetNumLevels ();
  if (NULL == timeTable) {
    timeTable = calloc (numLevels, sizeof (LevelTable) );
    assert (timeTable != NULL);
  }
  /* build table */
  for (i = 0; i < numLevels; i ++) {
    timeTable[i].level = GetLevelAtom (i);
    assert (ATOM_INVALID != timeTable[i].level);
    if (!GetLevelTime (timeTable[i].level, &timeTable[i].key.lastplayed) ) {
      timeTable[i].key.lastplayed = 0;
      StoreLevelTime(timeTable[i].level, timeTable[i].key.lastplayed);
    }
  }
  /* sort table */
  qsort (timeTable, numLevels,  sizeof (LevelTable), CompareTime);
#ifdef DEBUG_LEVELTIME
  for (i=0; i<numLevels; i++) {
    Dbg_Out("Lastplayed time of level %s = %s\n",GetLevelNameByAtom(timeTable[i].level),DateString(timeTable[i].key.lastplayed));
  }
#endif
  return numLevels;
} /* GetLastIndexTime */

/*
 * reset sorted level
 */
static void
ResortLevels (void)
{
  indexSorted = 0;
} /* ResortLevels */

/*
 * reset level time
 */
static void
ReTimeLevels (void)
{
  indexTime = 0;
} /* ReTimeLevels */

/*
 * recreate shuffle table
 */
static void
ReshuffleLevels (void)
{
  int i,j,k, numLevels;

  Dbg_Level ("reshuffling levels\n");
  numLevels = GetNumLevels ();
  for (i=0; i< numLevels; i++) {
    shuffledTable[i].key.value=i;
  }
  for (i = numLevels-1; i > 0 ; i--) {
    j=OtherRandomNumber(i+1);
    k=shuffledTable[i].key.value;
    shuffledTable[i].key.value=shuffledTable[j].key.value;
    shuffledTable[j].key.value=k;
  }
  for (i=0; i< numLevels; i++) {
    StoreLevelShuffle(shuffledTable[i].level, shuffledTable[i].key.value);
  }
  SaveLevelConfig();
  qsort (shuffledTable, numLevels,  sizeof (LevelTable), CompareShuffle);
  indexShuffled = 0;
} /* ReshuffleLevels */

/*
 * get next level from table
 */
static XBAtom
GetNextLevelFromTable (const LevelTable *table, int *pIndex, XBAtom entry, void (*pFunc) (void))
{
  int    numLevels;
  XBAtom atom;

  assert (pIndex != NULL);
  assert (pFunc  != NULL);
  numLevels = GetNumLevels ();
  /* unmark last level */
  if (*pIndex < numLevels) {
    SetLevelWasLast (table[*pIndex].level, entry, XBFalse);
  }
  /* find next level to play */
  do {
    *pIndex = *pIndex + 1;
    if (*pIndex >= numLevels) {
      (*pFunc) ();
      *pIndex=0;
    }
    assert (table != NULL);
    /* get level */
    atom = table[*pIndex].level;
    assert (ATOM_INVALID != atom);
    /* check game mode */
  } while ( gameMode != (gameMode & GetLevelGameMode (atom) ) ||
	    (! allLevels &&
	     ! GetLevelSelected (atom) ) );
  /* mark as last level */
  SetLevelWasLast (atom, entry, XBTrue);
  StoreLevelTime(timeTable[*pIndex].level, time(NULL));
  /* that's all */
  return atom;
} /* GetNextLevelFromTable */

/*
 * get next level from table, variant
 */
static XBAtom
GetNextLevelFromTable2 (LevelTable *table, int *pIndex, XBAtom entry, void (*pFunc) (void))
{
  int    numLevels;
  int    i,l,cnt;
  time_t t0,t1;
  XBAtom atom=ATOM_INVALID;

  assert (pIndex != NULL);
  assert (pFunc  != NULL);
  numLevels = GetNumLevels ();
  /* find time interval for selected levels matching gamemode*/
  t0=0;
  t1=0;
  cnt=0;
  for (i = 0; i < numLevels; i++) {
    atom = table[i].level;
    assert (ATOM_INVALID != atom);
    if ( gameMode == (gameMode & GetLevelGameMode (atom) ) && ( allLevels ||  GetLevelSelected (atom) ) ) {
      if (cnt==0) {
	t0=table[i].key.lastplayed;
	t1=t0;
      } else {
	if (table[i].key.lastplayed < t0) {
	  t0 = table[i].key.lastplayed;
	}
	if (table[i].key.lastplayed > t1) {
	  t1 = table[i].key.lastplayed;
	}
      }
      cnt++;
    }
  }
  Dbg_Level("There are %i selected levels between [%s",cnt,DateString(t0));
  Dbg_Level(", %s]\n",DateString(t1));
  assert (cnt > 0);
  /* look at oldest tenth of time interval */
  t1=t0+(t1-t0)/10;
  Dbg_Level("Finding those between [%s",DateString(t0));
  Dbg_Level(", %s]\n",DateString(t1));
  cnt=0;
  for (i = 0; i<numLevels; i++) {
    atom = table[i].level;
    assert (ATOM_INVALID != atom);
    if( gameMode == (gameMode & GetLevelGameMode (atom) ) && ( allLevels ||  GetLevelSelected (atom) ) ) {
      if (table[i].key.lastplayed <= t1) {
	cnt++;
      }
    }
  }
  Dbg_Level("There were %i levels found between [%s",cnt,DateString(t0));
  Dbg_Level(", %s]\n",DateString(t1));
  assert (cnt > 0);
  /* choose a random level of those */
  l=GameRandomNumber(cnt)+1;
  Dbg_Level("The %ith of %i levels was chosen\n",l,cnt);
  /* find it in table */
  i = 0;
  while ( (i<numLevels) && (l>0)) {
    atom = table[i].level;
    assert (ATOM_INVALID != atom);
    if ( gameMode == (gameMode & GetLevelGameMode (atom) ) && ( allLevels ||  GetLevelSelected (atom) ) && (table[i].key.lastplayed <= t1) ) {
      l--;
    }
    i++;
  }
  assert(l==0);
  /* update level time */
  *pIndex = i-1;
  t0=table[*pIndex].key.lastplayed;
  Dbg_Level("Lastplayed time of level %s to %s\n",GetLevelNameByAtom(timeTable[*pIndex].level),DateString(t0));
  t0=time(NULL);
  table[*pIndex].key.lastplayed=t0;
  Dbg_Level("Set lastplayed time of level %s to %s\n",GetLevelNameByAtom(timeTable[*pIndex].level),DateString(t0));
  StoreLevelTime(timeTable[*pIndex].level, timeTable[*pIndex].key.lastplayed);
  DB_Store(dbLevel);
  /* that's all */
  return atom;
} /* GetNextLevelFromTable2 */

#if 0
/*
 * convert level section to iob
 */
static XBTeleIOB
SectionToIOB (XBAtom atom)
{
  if (GUI_CompareAtoms (atom, atomInfo) ) {
    return IOB_INFO;
  } else if (GUI_CompareAtoms (atom, atomPlayer) ) {
    return IOB_PLAYER;
  } else if (GUI_CompareAtoms (atom, atomShrink) ) {
    return IOB_SHRINK;
  } else if (GUI_CompareAtoms (atom, atomScrambleDraw) ) {
    return IOB_SCRAMBLE_DRAW;
  } else if (GUI_CompareAtoms (atom, atomScrambleDel) ) {
    return IOB_SCRAMBLE_DEL;
  } else if (GUI_CompareAtoms (atom, atomFunc) ) {
    return IOB_FUNC;
  } else if (GUI_CompareAtoms (atom, atomBombs) ) {
    return IOB_BOMBS;
  } else if (GUI_CompareAtoms (atom, atomGraphics) ) {
    return IOB_GRAPHICS;
  } else if (GUI_CompareAtoms (atom, atomMap) ) {
    return IOB_MAP;
  }
  return IOB_INVALID;
} /* SectionToIOB */
#endif

/*
 * convert iob to level section
 */
static XBAtom
IOBToSection (XBTeleIOB iob)
{
  switch (iob) {
  case IOB_INFO:          return atomInfo;
  case IOB_PLAYER:        return atomPlayer;
  case IOB_SHRINK:        return atomShrink;
  case IOB_SCRAMBLE_DRAW: return atomScrambleDraw;
  case IOB_SCRAMBLE_DEL:  return atomScrambleDel;
  case IOB_FUNC:          return atomFunc;
  case IOB_BOMBS:         return atomBombs;
  case IOB_GRAPHICS:      return atomGraphics;
  case IOB_MAP:           return atomMap;
  default:                return ATOM_INVALID;
  }
} /* IOBToSection */

/*******************
 * shared routines *
 *******************/

/*
 * load all level info into database
 */
void
LoadLevelConfig (void)
{
#ifdef DEBUG
  Dbg_StartClock ();
#endif
  /* create and load level database */
  dbLevel = DB_Create (DT_Config, atomLevel);
  assert (dbLevel != NULL);
  if (DB_LoadDir (dbLevel, GAME_DATADIR"/level", "xal", DT_Level, atomTime, atomInfo, InsertLevel)) {
    DB_Store (dbLevel);
  }
  Dbg_Level ("init levels: %lu msec\n", Dbg_FinishClock ());
} /* LoadLevelConfig */

/*
 * save level info from database to files
 */
void
SaveLevelConfig (void)
{
  assert (dbLevel != NULL);
  if (DB_Changed (dbLevel) ) {
    DB_Store (dbLevel);
  }
} /* SaveLevelConfig */

/*
 * clear local level databases
 */
void
FinishLevelConfig (void)
{
  DB_Delete (dbLevel);
  if (NULL != sortedTable) {
    free (sortedTable);
    sortedTable = NULL;
  }
  if (NULL != shuffledTable) {
    free (shuffledTable);
    shuffledTable = NULL;
  }
  if (NULL != levelRemote) {
    DB_Delete (levelRemote);
    levelRemote = NULL;
  }
  if (NULL != levelLocal) {
    DB_Delete (levelLocal);
    levelLocal = NULL;
  }
} /* FinishLevelConfig */

/*
 * number of level info's in database
 */
int
GetNumLevels (void)
{
  return DB_NumSections (dbLevel);
} /* GetNumLevels */

/*
 * get atom for level section with given index
 */
XBAtom
GetLevelAtom (int index)
{
  return DB_IndexSection (dbLevel, index);
} /* GetLevelAtom */

/*
 * get name of level from level atom
 */
const char *
GetLevelNameByAtom (XBAtom level)
{
  return GetLevelStringByAtom (level, atomName);
} /* GetLevelName */

/*
 * return selection flag for current level, given section
 */
XBBool
GetLevelSelected (XBAtom level)
{
  XBBool selected;
  const DBSection *section;
  assert (dbLevel != NULL);
  section = DB_GetSection (dbLevel, level);
  if (NULL == section) {
    return XBFalse;
  }
  if (! DB_GetEntryBool (section, atomSelect, &selected) ) {
    return XBTrue;
  }
  return selected;
} /* GetLevelSelected */

/*
 * set selection flag for given level, given section
 */
void
StoreLevelSelected (XBAtom atom, XBBool select)
{
  DBSection *section;
  assert (NULL != dbLevel);
  section = DB_CreateSection (dbLevel, atom);
  assert (section != NULL);
  /* we only change data not stored in file */
  Dbg_Level("Level select %s = %d\n",GetLevelNameByAtom(atom),select);
  (void) DB_CreateEntryBool (section, atomSelect,  select);
} /* StoreLevelSelection */

/*
 * load an actual level from file
 */
const DBRoot *
LoadLevelFile (XBAtom atom)
{
#ifdef DEBUG
  Dbg_StartClock ();
#endif

  if (NULL != levelLocal) {
    DB_Delete (levelLocal);
  }
  levelLocal = DB_Create (DT_Level, atom);
  assert (levelLocal != NULL);
  if (! DB_Load (levelLocal) ) {
    DB_Delete (levelLocal);
    return NULL;
  }
#ifdef DEBUG
  Dbg_Out ("load level: %lu msec\n", Dbg_FinishClock ());
#endif
  return levelLocal;
} /* LoadLevelFile */

/*
 * store level setup locally
 */
XBBool
InitLevels (const CFGGame *cfgGame)
{
  int    i;
  int    numSelected;
  int    numLevels;
  XBAtom level;

  assert (NULL != cfgGame);
  /* level order */
  randomOrder = cfgGame->setup.randomLevels;
  levelOrder  = cfgGame->setup.levelOrder;
  allLevels   = cfgGame->setup.allLevels;
  /* game mode  selection */
  gameMode = 0;
  /* number of players */
  switch (cfgGame->players.num) {
#ifdef DEBUG
  case 1: gameMode |= GM_2_Player; break;
#endif
  case 2: gameMode |= GM_2_Player; break;
  case 3: gameMode |= GM_3_Player; break;
  case 4: gameMode |= GM_4_Player; break;
  case 5: gameMode |= GM_5_Player; break;
  case 6: gameMode |= GM_6_Player; break;
  }
  /* team mode */
  gameMode |= GM_Single;
  /* two players on any one host */
  for (i = 0; i+1 < cfgGame->players.num; i ++) {
    if (cfgGame->players.host[i] == cfgGame->players.host[i+1]) {
      gameMode |= GM_LR_Players;
    }
  }
  /* now count levels to play */
  numSelected = 0;
  numLevels   = GetNumLevels ();
  for (i = 0; i < numLevels; i ++) {
    /* get level data */
    level = GetLevelAtom (i);
    assert (ATOM_INVALID != level);
    /* check game mode */
    if (gameMode != (gameMode & GetLevelGameMode (level) ) ) {
      continue;
    }
    /* check selection */
    if (! allLevels && ! GetLevelSelected (level)) {
      continue;
    }
    numSelected ++;
  }
  Dbg_Level("%d levels selected\n", numSelected);

  /* last levels played ... */
  indexSorted   = GetLastIndexSorted ();
  indexShuffled = GetLastIndexShuffled ();
  indexTime     = GetLastIndexTime ();
  ReshuffleLevels ();

  return (numSelected != 0);
} /* InitLevels */

/*
 * get next level to play
 */
XBAtom
GetNextLevel (void)
{
  if (levelOrder==2) { // LRF
    Dbg_Level ("get next level: shuffled\n");
    return GetNextLevelFromTable (shuffledTable, &indexShuffled, atomLevelShuffled, ReshuffleLevels);
  } else if (levelOrder==1) {
    Dbg_Level ("get next level: alfabet\n");
    return GetNextLevelFromTable (sortedTable,   &indexSorted,   atomLevelSorted,   ResortLevels);
  } else {
    Dbg_Level ("get next level: time based\n");
    return GetNextLevelFromTable2 (timeTable,   &indexTime,   atomLastPlayed,  ReTimeLevels);
  }
} /* GetNextLevel */

/*
 * send level data to host
 */
void
SendLevelConfig (XBSndQueue *queue, XBTeleCOT cot, const DBRoot *level)
{
  XBTeleIOB        iob;
  int              j;
  const DBSection *section;
  XBTelegram      *tele;
  size_t           len;
  char             buffer[256];
  /* sanity check */
  assert (queue != NULL);
  assert (level != NULL);
  /* first send level atom (aka filename) */
  len  = sprintf (buffer, "%s", GUI_AtomToString (DB_Atom (level) ) );
  tele = Net_CreateTelegram (cot, XBT_ID_LevelConfig, IOB_NAME, buffer, len + 1);
  assert (tele != NULL);
  Net_SendTelegram (queue, tele);
  /* iterate sections */
  for (iob = 0; iob < NUM_IOBS; iob ++) {
    section = DB_GetSection (level, IOBToSection (iob));
    if (NULL != section) {
      /* iterate entries */
      j = 0;
      while (0 < (len = DB_PrintEntry (buffer, section, j) ) ) {
	tele = Net_CreateTelegram (cot, XBT_ID_LevelConfig, iob, buffer, len + 1);
	assert (tele != NULL);
	Net_SendTelegram (queue, tele);
	j ++;
      }
    }
  }
  /* no data means end of section */
  tele = Net_CreateTelegram (cot, XBT_ID_LevelConfig, iob, NULL, 0);
  assert (tele != NULL);
  Net_SendTelegram (queue, tele);
} /* SendLevelConfig */

/*
 * clear remote level config
 */
void
ClearRemoteLevelConfig (void)
{
  XBTeleIOB iob;

  if (NULL != levelRemote) {
    DB_Delete (levelRemote);
  }
  levelRemote = DB_Create (DT_Level, atomRemote);
  for (iob = 0; iob < NUM_IOBS; iob ++) {
    (void) DB_CreateSection (levelRemote, IOBToSection (iob));
  }
} /* ClearRemoteLevelConfig */

/*
 * add new line to remote level config
 */
void
AddToRemoteLevelConfig (unsigned iob, const char *data)
{
  XBAtom atom;
  DBSection *section;
  assert (levelRemote != NULL);
  if (IOB_NAME == iob) {
    /* level names received */
    DB_SetAtom (levelRemote, GUI_StringToAtom (data));
    Dbg_Out ("set remote level file to \"%s\"\n", data);
  } else {
    /* possible part of level section */
    atom = IOBToSection (iob);
    if (ATOM_INVALID != atom) {
      /* create new section */
      section = DB_CreateSection (levelRemote, atom);
      assert (NULL != section);
      /* add line */
      (void) DB_ParseEntry (section, data);
    }
  }
} /* AddToRemoteLevelConfig */

/*
 * check local level for version (server)
 */
XBBool
CheckLocalLevelConfig() {
  assert (levelLocal != NULL);
  return CheckAnyLevelConfig(levelLocal);
} /* CheckLocalLevelConfig */

/*
 * check remote level for version (client)
 */
XBBool
CheckRemoteLevelConfig() {
  assert (levelRemote != NULL);
  return CheckAnyLevelConfig(levelRemote);
} /* CheckRemoteLevelConfig */

/*
 * check if received level is okay
 * accept all levels without warnings
 * warnings:
 * - unknown features
 * - incompatible server/client versions for some feature
 */
XBBool
CheckAnyLevelConfig(const DBRoot *level) {
  if (!ParseLevel(level)) {
    return XBFalse;
  }
  if (GetWarningCount() > 0) {
    return XBFalse;
  }
  return XBTrue;
} /* CheckAnyLevelConfig */

/*
 * check all levels in database
 */
XBBool
CheckAllLevelConfigs()
{
  XBAtom atom;
  const DBRoot *lvl;
  indexTime     = GetLastIndexTime ();
  indexSorted = GetLastIndexSorted();
  indexSorted = 0;
  do {
    Dbg_Out("-------- %i ---------\n", indexSorted);
    atom = GetNextLevelFromTable (sortedTable,   &indexSorted,   atomLevelSorted,   ResortLevels);
    Dbg_Out("+++LEVEL+++ current = %s(%s)!\n", GUI_AtomToString(atom), GetLevelNameByAtom(atom) );
    lvl = LoadLevelFile(atom);
    if (! ParseLevel(lvl) ) {
      Dbg_Out("+++LEVEL+++ Parse failed!\n");
      DB_Dump(lvl);
      return XBFalse;
    }
  } while (indexSorted > 0);
  return XBTrue;
} /* CheckAllLevelConfigs */

/*
 * retrieve remote level config
 */
const DBRoot *
GetRemoteLevelConfig (void)
{
  return levelRemote;
} /* GetRemoteLevelConfig */

/*
 * end of file cfg_level.c
 */
