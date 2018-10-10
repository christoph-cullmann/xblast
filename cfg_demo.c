/*
 * file cfg_demo.c - configuration data for recorded demos
 *
 * $Id: cfg_demo.c,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
#include "cfg_demo.h"

#include "cfg_level.h"
#include "atom.h"

/*
 * global variables
 */
DBRoot *dbDemo = NULL;

/*
 * local variables
 */
static DBRoot    *dbList       = NULL;
static DBSection *frameSection = NULL;

/*
 *
 */
void 
LoadDemoConfig (void)
{
  /* create single demo database */
  printf("loading demos\n");
  dbDemo = DB_Create (DT_Demo, atomDemo);
  assert (NULL != dbDemo);
  /* load list of recorded demos */
  dbList = DB_Create (DT_Config, atomDemo);
  assert (NULL != dbDemo);
  if (DB_LoadDir (dbList, "demo", "dem", DT_Demo, atomTime, atomInfo, NULL) ) {
    DB_Store (dbList);
  }
} /* LoadDemoConfig */

/*
 *
 */
void
StoreDemoConfig (const CFGDemoInfo *cfgDemo)
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
  (void) DB_CreateEntryInt  (section, atomNumPlayers, 	 cfgDemo->numPlayers);
  (void) DB_CreateEntryInt  (section, atomNumFrames,  	 cfgDemo->numFrames);
  (void) DB_CreateEntryAtom (section, atomLevel,      	 cfgDemo->level);
  (void) DB_CreateEntryInt  (section, atomRandomSeed, 	 cfgDemo->randomSeed);
  (void) DB_CreateEntryTime (section, atomRecorded,   	 cfgDemo->time);
  (void) DB_CreateEntryInt  (section, atomLives,   	 cfgDemo->numLives);
  (void) DB_CreateEntryInt  (section, atomFrameRate,   	 cfgDemo->frameRate);
  (void) DB_CreateEntryBool (section, atomRandomPlayers, cfgDemo->randomPlayers);
  (void) DB_CreateEntryInt  (section, atomWinner,   	 cfgDemo->winner);
} /* StoreDemoConfig */

/*
 * get demo config from database
 */
XBBool
RetrieveDemoConfig (CFGDemoInfo *cfgDemo)
{
  const DBSection *section;

  /* sanity checks */
  assert (NULL != cfgDemo);
  assert (NULL != dbDemo);
  /* find section */
  if (NULL == (section = DB_GetSection (dbDemo, atomInfo) ) ) {
    return XBFalse;
  }
  /* get data */
  if (! DB_GetEntryInt  (section, atomNumPlayers,    &cfgDemo->numPlayers) ||
      ! DB_GetEntryInt  (section, atomNumFrames,     &cfgDemo->numFrames) ||
      ! DB_GetEntryAtom (section, atomLevel,         &cfgDemo->level) ||
      ! DB_GetEntryInt  (section, atomRandomSeed,    &cfgDemo->randomSeed) ||
      ! DB_GetEntryTime (section, atomRecorded,      &cfgDemo->time) ||
      ! DB_GetEntryInt  (section, atomLives,         &cfgDemo->numLives) ||
      ! DB_GetEntryInt  (section, atomFrameRate,     &cfgDemo->frameRate) ||
      ! DB_GetEntryBool (section, atomRandomPlayers, &cfgDemo->randomPlayers) ) {
    return XBFalse;
  }
  return XBTrue;
} /* RetrieveDemoConfig */

/*
 * clear frames in single demo
 */
void
DeleteDemoFrames (void)
{
  assert (NULL != dbDemo);
  DB_DeleteSection (dbDemo, atomFrames);
  frameSection = NULL;
} /* ClearDemoFrames */

/*
 * store data for single frame in demo database
 */
void 
StoreDemoFrame (int gameTime, int numPlayers, const unsigned char *buf)
{
  int    i;
  size_t len = 0;
  char   tmp[3*MAX_PLAYER+1] = "";

  if (NULL == frameSection) {
    assert (NULL != dbDemo);
    frameSection = DB_CreateSection (dbDemo, atomFrames);
    assert (NULL != frameSection);
  }
  for (i = 0; i < numPlayers; i ++) {
    len += sprintf (tmp + len, "%02X ", (unsigned) buf[i]);
  }
  (void) DB_CreateEntryString (frameSection, GUI_IntToAtom (gameTime), tmp);
} /* StoreDemoFrame */

/*
 * store current demo
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
} /* SaveCurrentDemo */

/*
 *
 */ 
void
SaveDemoConfig (void)
{
  /* list of demos database */
  assert (dbList != NULL);
  if (DB_Changed (dbList) ) {
    DB_Store (dbList);
  }
} /* SaveDemoConfig */

/*
 *
 */
void
FinishDemoConfig (void)
{
  /* single demo database */
  assert (dbDemo != NULL);
  DB_Delete (dbDemo);
  dbDemo = NULL;
  /* list of demos database */
  assert (dbList != NULL);
  if (DB_Changed (dbList) ) {
    DB_Store (dbList);
  }
  DB_Delete (dbList);
  dbList = NULL;
} /* FinishDemoConfig */

/*
 * compare to demo entries by time 
 */ 
static int
CompareDemoEntries (const void *a, const void *b)
{
  assert (NULL != a);
  assert (NULL != b);

  return ((CFGDemoEntry *) b)->time - ((CFGDemoEntry *) a)->time;
} /* CompareDemoEntries */

/*
 * create list with all demos available
 */
CFGDemoEntry *
CreateDemoList (size_t *num)
{
  CFGDemoEntry 	  *list;
  size_t       	   i, j;
  XBAtom       	   atom;
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
  for (i = 0, j = 0; i < *num; i ++) {
    atom    = DB_IndexSection (dbList, i);
    section = DB_GetSection (dbList, atom);
    assert (NULL != section);
    list[j].atom = atom;
    (void) DB_GetEntryAtom (section, atomLevel,      &list[j].level);
    if (NULL == GetLevelNameByAtom (list[j].level)) {
      continue;
    }
    (void) DB_GetEntryInt  (section, atomNumPlayers, &list[j].numPlayers);
    (void) DB_GetEntryTime (section, atomRecorded,   &list[j].time);
    j ++;
  }
  *num = j;
  /* sort it */
  qsort (list, *num, sizeof (CFGDemoEntry), CompareDemoEntries);
  return list;
} /* CreateDemoList */

/*
 * load demo from file
 */
XBBool
LoadDemoFromFile (XBAtom atom)
{
  assert (NULL != dbDemo);
  DB_Delete (dbDemo);
  dbDemo = DB_Create (DT_Demo, atom);
  assert (NULL != dbDemo);
  return DB_Load (dbDemo); 
} /* LoadDemoFromFile */

/*
 * retrieve frames from demo
 */
XBBool
RetrieveDemoFrames (int numPlayers, unsigned char (*buffer)[MAX_PLAYER])
{
  const DBSection *section;
  size_t           i, j, num;
  int              frame;
  const char      *s;
  unsigned         value;

  /* sanity check */
  assert (numPlayers <= MAX_PLAYER);
  assert (buffer != NULL);
  assert (dbDemo != NULL);
  /* find frames section */
  if (NULL == (section = DB_GetSection (dbDemo, atomFrames) ) ) {
    return XBFalse;
  }
  num = DB_NumEntries (section);
  for (i = 0; i < num; i ++) {
    XBAtom atom = DB_IndexEntry (section, i);
    assert (ATOM_INVALID != atom);
    /* check for valid frame */
    frame = GUI_AtomToInt (atom);
    if (frame < 0 || frame >= GAME_TIME) {
      return XBFalse;
    }
    if (! DB_GetEntryString (section, atom, &s) ) {
      return XBFalse;
    }
    if (strlen (s) < 2*numPlayers) {
      return XBFalse;
    }
    for (j = 0; j < numPlayers; j ++) {
      if (1 != sscanf (s + 3*j, "%x", &value) ) {
	return XBFalse;
      }
      buffer[frame][j] = (unsigned char) value;
    }
  }
  return XBTrue;
} /* RetrieveDemoFrames */

/*
 * end of file cfg_demo.c
 */
