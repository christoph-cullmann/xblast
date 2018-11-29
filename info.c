/*
 * file info.c - display info at level start
 *
 * $Id: info.c,v 1.7 2005/01/11 22:44:41 lodott Exp $
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
#include "info.h"
#include "atom.h"
#include "shrink.h"
#include "func.h"
#include "map.h"
#include "bomb.h"

/*
 * local constants
 */
#define MAX_INFO    7
#define INFO_LENGTH 256

/*
 * local variables
 */
static int    levelCount;
static int    extraCount;
static int    playerCount;
static char **levelInfo  = NULL;
static char **extraInfo  = NULL;
static char **playerInfo = NULL;
static unsigned gameMode;
static unsigned maxVictories = 3;

/*
 * allocate space for info entries
 */
static char **
AllocInfo (void)
{
  char **ptr;
  int i;

  ptr = (char **) calloc (MAX_INFO, sizeof(char *) );
  assert (NULL != ptr);
  for (i=0; i<MAX_INFO; i++) {
    ptr[i] = calloc (INFO_LENGTH, sizeof(char) );
    assert (NULL != ptr[i]);
  }
  return ptr;
} /* AllocInfo */

/*
 * free info memory
 */
static void
FreeInfo (char **ptr)
{
  int i;

  assert (ptr != NULL);
  for (i=0; i<MAX_INFO; i++) {
    assert (ptr[i] != NULL);
    free (ptr[i]);
  }
  free (ptr);
} /* FreeInfo */

/*
 * clear alloc info data
 */
void
ClearInfo (void)
{
  if (NULL != extraInfo) {
    FreeInfo (extraInfo);
    extraInfo = NULL;
  }
  if (NULL != levelInfo) {
    FreeInfo (levelInfo);
    levelInfo = NULL;
  }
  if (NULL != playerInfo) {
    FreeInfo (playerInfo);
    playerInfo = NULL;
  }
} /* ClearInfo */

/*
 * reset info strings
 */
void
ResetInfo (void)
{
  int i;

  /* alloc data if needed */
  if (NULL == extraInfo) {
    extraInfo = AllocInfo ();
    assert (NULL != extraInfo);
  }
  if (NULL == levelInfo) {
    levelInfo = AllocInfo ();
    assert (NULL != levelInfo);
  }
  if (NULL == playerInfo) {
    playerInfo = AllocInfo ();
    assert (NULL != playerInfo);
  }
  /* set first strings to zero length ones */
  for (i=0; i<MAX_INFO; i++) {
    levelInfo[i][0]  = '\0';
    extraInfo[i][0]  = '\0';
    playerInfo[i][0] = '\0';
  }
  levelCount  = 0;
  extraCount  = 0;
  playerCount = 0;
} /* ResetInfo */

/*
 * parse level info section
 */
XBBool
ParseLevelInfo (const DBSection *section, DBSection *warn)
{
  const char *g;
  unsigned i;
  static const char    *gameModeString  = "R23456STDL";
  /* reseting */
  ResetInfo ();
  /* check if section exists */
  if (NULL == section) {
    Dbg_Out("LEVEL: info section is missing!\n");
    DB_CreateEntryString(warn,atomMissing,"true");
    return XBFalse;
  }
  /* extract gameMode */
  gameMode = 0;
  /* game mode entry is required */
  if (! DB_GetEntryString (section, atomGameMode, &g) ) {
    Dbg_Out("LEVEL: game mode is missing!\n");
    DB_CreateEntryString(warn,atomGameMode,"missing!");
    return XBFalse;
  }
  /* convert to unsigned */
  for (i = 0; g[i] != 0 && gameModeString[i] != 0; i ++) {
    if (g[i] == gameModeString[i]) {
      gameMode |= (1 << i);
    }
  }
  Dbg_Level("gameMode = %s, converted to %u\n", g, gameMode);
  return XBTrue;
} /* ParseLevelInfo */

/*
 * add a generic info line
 */
static void
AddInfo (char **info, int *count, const char *fmt, va_list argList)
{
  /* sanity check */
  assert (count != NULL);
  assert (info  != NULL);
  /* do we have any space left */
  /* BUGFIX1 */
  if (*count >= MAX_INFO) {
    return;
  }
  /* 1BUGFIX */
  assert (info[*count] != NULL);
  assert (fmt != NULL);
  /* add text */
  (void) vsprintf (info[*count], fmt, argList);
  /* next line */
  *count = *count + 1;
} /* AddInfo */

/*
 * add player info
 */
void
AddPlayerInfo (const char *fmt, ...)
{
  va_list argList;

  va_start (argList, fmt);
  AddInfo (playerInfo, &playerCount, fmt, argList);
  va_end (argList);
} /* AddPlayerInfo */

/*
 * add level info
 */
void
AddLevelInfo (const char *fmt, ...)
{
  va_list argList;

  va_start (argList, fmt);
  AddInfo (levelInfo, &levelCount, fmt, argList);
  va_end (argList);
} /* AddLevelInfo */

/*
 * add extra info
 */
void
AddExtraInfo (const char *fmt, ...)
{
  va_list argList;

  va_start (argList, fmt);
  AddInfo (extraInfo, &extraCount, fmt, argList);
  va_end (argList);
} /* AddExtraInfo */

/*
 *
 */
const char **
GetPlayerInfo (int *pNum)
{
  assert (pNum != NULL);
  *pNum = playerCount;
  return (const char **) playerInfo;
} /* GetPlayerInfo */

/*
 *
 */
const char **
GetLevelInfo (int *pNum)
{
  assert (pNum != NULL);
  *pNum = levelCount;
  return (const char **) levelInfo;
} /* GetLevelInfo */

/*
 *
 */
const char **
GetExtraInfo (int *pNum)
{
  assert (pNum != NULL);
  *pNum = extraCount;
  return (const char **) extraInfo;
} /* GetExtraInfo */

/*
 * set number of needed victories
 */
void
SetMaxVictories(int nvictories)
{
  maxVictories = nvictories;
} /* SetMaxVictories */

/*
 * get number of needed victories
 */
unsigned
GetMaxVictories()
{
  return maxVictories;
} /* GetMaxVictories */

/*
 * get current gamemode
 */
unsigned
GetGameModeInfo()
{
  return gameMode;
} /* GetGameModeInfo */

/*
 * end of file info.c
 */
