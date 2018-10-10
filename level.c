/*
 * file level.c - Setting up level given in database
 *
 * $Id: level.c,v 1.10 2004/12/04 06:01:13 lodott Exp $
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
#include "level.h"
#include "atom.h"
#include "bomb.h"
#include "func.h"
#include "info.h"
#include "map.h"
#include "scramble.h"
#include "shrink.h"

static DBRoot *warnings = NULL;

/*
 * initialize level problem database
 */
void
InitLevelWarnings()
{
  if (warnings != NULL) {
    DB_DeleteAll(warnings);
  } else {
    warnings = DB_Create(DT_Level, GUI_StringToAtom("levelwarn!"));
    assert (warnings != NULL);
  }

} /* InitLevelWarnings */

/*
 * get number of warnings found in last parse
 */
unsigned
GetWarningCount()
{
  unsigned cnt = DB_NumAllEntries(warnings);
  Dbg_Out("warnings = %u\n", cnt);
  return cnt;
} /* GetWarningCount */

/*
 * parse a new level
 */
XBBool
ParseLevel (const DBRoot *level)
{
  DBSection *sec, *sec1, *sec2;
#ifdef DEBUG
  Dbg_StartClock ();
#endif
  Dbg_Level("start parsing\n");
  /* sanity check */
  assert (level != NULL);
  InitLevelWarnings();
  /* TODO: disallow random positions in some levels */

  /* create info section in warnings, then parse */
  Dbg_Level("parsing section [%s]\n", GUI_AtomToString(atomInfo));
  sec = DB_CreateSection(warnings,atomInfo);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  if (! ParseLevelInfo (DB_GetSection (level, atomInfo), sec) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }
  /* create player section in warning, then parse */
  sec = DB_CreateSection(warnings,atomPlayer);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  Dbg_Level("parsing section [%s]\n", GUI_AtomToString(atomPlayer));
  if (! ParseLevelPlayers (DB_GetSection (level, atomPlayer), GetGameModeInfo(), sec) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }

  /* setup shrink pattern */
  sec = DB_CreateSection(warnings,atomShrink);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  Dbg_Level("parsing section [%s]\n", GUI_AtomToString(atomShrink));
  if (! ParseLevelShrink (DB_GetSection (level, atomShrink), sec ) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }

  /* setup scrambling blocks */
  sec1 = DB_CreateSection(warnings,atomScrambleDraw);
  if (NULL == sec1) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  sec2 = DB_CreateSection(warnings,atomScrambleDel);
  if (NULL == sec2) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  Dbg_Level("parsing sections [%s] and [%s]\n",
	  GUI_AtomToString(atomScrambleDraw),
	  GUI_AtomToString(atomScrambleDel) );
  if (! ParseLevelScramble (DB_GetSection (level, atomScrambleDraw),
			     DB_GetSection (level, atomScrambleDel), sec1, sec2 ) ) {
    Dbg_Level("sections invalid!\n");
    return XBFalse;
  }

  /* setup function pointers */
  sec = DB_CreateSection(warnings,atomFunc);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  Dbg_Level("parsing section [%s]\n", GUI_AtomToString(atomFunc));
  if (! ParseLevelFunc (DB_GetSection (level, atomFunc), sec ) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }

  /* setup bombs */
  sec = DB_CreateSection(warnings,atomBombs);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  Dbg_Level("parsing section [%s]\n", GUI_AtomToString(atomBombs));
  if (! ParseLevelBombs (DB_GetSection (level, atomBombs), sec ) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }

  /* setup graphics */
  sec = DB_CreateSection(warnings,atomGraphics);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  if (! ParseLevelGraphics (DB_GetSection (level, atomGraphics), sec ) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }

  /* setup map layout */
  sec = DB_CreateSection(warnings,atomMap);
  if (NULL == sec) {
    Dbg_Level("failed to create warning section\n");
    return XBFalse;
  }
  if (! ParseLevelMap (DB_GetSection (level, atomMap), sec ) ) {
    Dbg_Level("section invalid!\n");
    return XBFalse;
  }

  Dbg_Level("parsed in %lu msec\n", Dbg_FinishClock ());
  /* show all warnings */
  if (GetWarningCount()) {
    DB_Dump(warnings);
  }
  return XBTrue;
} /* ParseLevel */

/*
 * Configure new level
 */
XBBool
ConfigLevel (const DBRoot *level)
{
  /* sanity check */
  assert (level != NULL);
  /* parse first */
  if (!ParseLevel(level)) {
    return XBFalse;
  }
#ifdef DEBUG
  Dbg_StartClock ();
#endif
  Dbg_Level("start configuring\n");
  ConfigLevelPlayers (DB_GetSection (level, atomPlayer), XBTrue, GetGameModeInfo());
  ConfigLevelShrink (DB_GetSection (level, atomShrink));
  ConfigLevelFunc (DB_GetSection (level, atomFunc) );
  ConfigLevelBombs (DB_GetSection (level, atomBombs) );
  ConfigLevelGraphics (DB_GetSection (level, atomGraphics) );
  ConfigLevelMap(DB_GetSection (level, atomMap) );
  Dbg_Level("configured in %lu msec\n", Dbg_FinishClock ());
  return XBTrue;
} /* ConfigLevel */

/*
 * clean up after level
 */
void
FinishLevel (void)
{
  /* shrinking */
  FinishLevelShrink ();
  /* graphics */
  FinishLevelGraphics ();
  /* explosions */
  DeleteAllExplosions();
} /* FinishLevel */

/*
 * get string of level
 */
static const char *
GetLevelString (const DBRoot *level, XBAtom atom)
{
  const char      *s;
  const DBSection *section;

  assert (level != NULL);
  section = DB_GetSection (level, atomInfo);
  if (NULL == section) {
    return NULL;
  }
  if (! DB_GetEntryString (section, atom, &s) ) {
    return NULL;
  }
  return s;
} /* GetLevelString */

/*
 * get name of level
 */
const char *
GetLevelName (const DBRoot *level)
{
  return GetLevelString (level, atomName);
} /* GetLevelName */

/*
 * get author of level
 */
const char *
GetLevelAuthor (const DBRoot *level)
{
  return GetLevelString (level, atomAuthor);
} /* GetLevelAuthor */

/*
 * get hint for level
 */
const char *
GetLevelHint (const DBRoot *level)
{
  return GetLevelString (level, atomHint);
} /* GetLevelHint */

/*
 * end of file level.c
 */
