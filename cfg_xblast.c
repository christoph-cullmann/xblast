/*
 * file cfg_xblast.c - general xblast configurations
 *
 * $Id: cfg_xblast.c,v 1.4 2004/09/20 10:00:41 alfie Exp $
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
#include "cfg_xblast.h"

#include "atom.h"
#include "ini_file.h"

/*
 * local variables
 */
static DBRoot *db = NULL;
static const CFGSoundSetup defaultSoundSetup = {
  XBSM_None, XBTrue,
};
/* conversion tables for sound mode (alphanumeric sort by key) */
static DBToInt convSoundModeTable[NUM_XBSM+1] = {
  { "Beep",    XBSM_Beep },
  { "None",    XBSM_None },
  { "Waveout", XBSM_Waveout },
  { NULL,      NUM_XBSM },
};
static const CFGCentralSetup defaultCentralSetup = {
  "xblast.debian.net", 16160,
};

static char centralname[64];


/*
 * load config database
 */
void
LoadXBlastConfig (void)
{
  db = DB_Create (DT_Config, atomXblast);
  assert (db != NULL);
  if (DB_Load (db)) {
    return;
  }
  /* set defaults */
  StoreSoundSetup (&defaultSoundSetup);
  DB_Store (db);
} /* LoadXBlastConfig */

/*
 * save config database
 */
void
SaveXBlastConfig (void)
{
  assert (NULL != db);
  if (DB_Changed (db)) {
    DB_Store (db);
  }
} /* SaveXBlastConfig */

/*
 * clean up
 */
void
FinishXBlastConfig (void)
{
  if (NULL != db) {
    DB_Delete (db);
    db = NULL;
  }
} /* FinishXBlastConfig */

/*
 * store steup for sound
 */
void
StoreSoundSetup (const CFGSoundSetup *sound)
{
  DBSection  *section;
  const char *key;

  assert (NULL != sound);
  assert (NULL != db);
  /* create section for sound */
  section = DB_CreateSection (db, atomSound);
  assert (NULL != section);
  /* add setup */
  if (NULL != (key = DB_IntToString (convSoundModeTable, sound->mode) ) ) {
    DB_CreateEntryString (section, atomMode, key);
  } else {
    DB_DeleteEntry (section, atomMode);
  }
  DB_CreateEntryBool (section, atomStereo, sound->stereo);
  /* AbsInt begin */
  DB_CreateEntryBool (section, atomBeep, sound->beep);
  /* AbsInt end */
} /* StoreSoundSetup */

/*
 * get ound setup from config
 */
XBBool
RetrieveSoundSetup (CFGSoundSetup *sound)
{
  const DBSection *section;

  assert (NULL != db);
  assert (NULL != sound);
  /* set default value */
  *sound = defaultSoundSetup;
  /* find section for sound */
  if (NULL == (section = DB_GetSection (db, atomSound) ) ) {
    return XBFalse;
  }
  /* parse section */
  DB_ConvertEntryInt (section, atomMode, (int *) &sound->mode, convSoundModeTable);
  DB_GetEntryBool (section, atomStereo, &sound->stereo);
  /* AbsInt begin */
  DB_GetEntryBool (section, atomBeep, &sound->beep);
  /* AbsInt end */
  /* that's all */
  return XBTrue;
} /* RetrieveSoundSetup */


/*
 * store steup for central XBCC
 */
void
StoreCentralSetup (const CFGCentralSetup *central)
{
  DBSection  *section;

  assert (NULL != central);
  assert (NULL != db);
  /* create section for sound */
  section = DB_CreateSection (db, atomCentral);
  assert (NULL != section);
  /* add setup */
  DB_CreateEntryString (section, atomCentralJoinName, central->name);
  DB_CreateEntryInt    (section, atomCentralJoinPort, central->port);
} /* StoreCentralSetup */

/*
 * get ound setup from config
 */
XBBool
RetrieveCentralSetup (CFGCentralSetup *central)
{
  const DBSection *section;

  assert (NULL != db);
  assert (NULL != central);
  /* set default value */
  *central = defaultCentralSetup;
  /* find section for sound */
  if (NULL == (section = DB_GetSection (db, atomCentral) ) ) {
    return XBFalse;
  }
  /* parse section */
  DB_GetEntryString (section, atomCentralJoinName, &central->name);
  DB_GetEntryInt    (section, atomCentralJoinPort, &central->port);
  sprintf(centralname,"%s","129.125.51.148");
  printf("%s %s %i\n",central->name,centralname,strcmp(central->name,centralname));
  if(strcmp(central->name,centralname)==0) {
    sprintf(centralname,"%s","129.125.51.134");
    central->name=centralname;
  }
  /* that's all */
  return XBTrue;
} /* RetrieveSoundSetup */


/*
 * end of file cfg_xblast.c
 */
