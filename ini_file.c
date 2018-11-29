/*
 * file ini_file.h - parsing ini-file into a database
 *
 * $Id: ini_file.c,v 1.16 2005/01/06 18:28:10 lodott Exp $
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
#include "ini_file.h"
#include "str_util.h"
#include "util.h"

/*
 * local types
 */
/* single entry */
struct _db_entry {
  XBAtom            atom;   /* atom for entry key */
  char             *value;  /* string value of key */
  struct _db_entry *next;   /* next entry for this section */
};

/* one complete section */
struct _db_section {
  XBAtom              atom;    /* name atom of section -> [...] line in file*/
  XBBool              changed; /* flag for unsaved changes in section */
  DBEntry            *entry;   /* list of entries */
  struct _db_section *next;    /* next section in this database */
};

/* one database */
struct _db_root {
  DBType     type;     /* in which list is database stored -> dirname */
  XBAtom     atom;     /* name atom for database -> filename */
  XBBool     changed;  /* flag for changes */
  DBSection *section;  /* pointer to sections */
  DBRoot    *next;     /* next database of this type */
};

/*
 * local variables
 */

/* database lists */
static DBRoot *db_list[NUM_DT] = {
  NULL, NULL, NULL, NULL,
};
/* paths for database file */
static const char *db_path[NUM_DT] = {
  "config",
  GAME_DATADIR"/level",
  "demo",
  "central",
};
/* file extensions */
static const char *db_ext[NUM_DT] = {
  "cfg",
  "xal",
  "dem",
  "dat",
};

/*******************
 * local functions *
 *******************/

/* free memory of defined structures */

/*
 * free memory of an entry
 */
static void
FreeEntry (DBEntry *entry)
{
  assert (entry != NULL);
  assert (entry->value != NULL);
  free (entry->value);
  free (entry);
} /* FreeEntry */

/*
 * free memory of a section and all its entries
 */
static void
FreeSection (DBSection *section)
{
  DBEntry *entry;
  DBEntry *nextEntry;

  assert (NULL != section);
  /* clear all entries */
  for (entry = section->entry; entry != NULL; entry = nextEntry) {
    nextEntry = entry->next;
    FreeEntry (entry);
  }
  free (section);
} /* FreeSection */

/*
 * free memory of a given database
 */
static void
FreeRoot (DBRoot *db)
{
  DBSection *section;
  DBSection *nextSection;

  assert (db != NULL);
  /* clear all sections */
  for (section = db->section; section != NULL; section = nextSection) {
    nextSection = section->next;
    FreeSection (section);
  }
  free (db);
} /* DB_Delete */

/* parse input lines */

/*
 * parse section name from given line
 */
static const char *
ParseSection (const char *line)
{
  const char *left;
  char *dst;
  static char result[256];

  /* check left side */
  for (left = line; *left != 0 && isspace (*left); left ++) continue;
  if (*left != '[') {
    return NULL;
  }
  left ++;
  dst = result;
  while (*left != ']') {
    if ('\0' == *left) {
      /* premature end of line */
      return NULL;
    }
    *dst = *left;
    dst ++;
    left ++;
  }
  *dst = '\0';
  return result;
} /* ParseSection */

/*
 * parse key part from a given line
 */
static const char *
ParseEntryName (const char *line)
{
  const char *src;
  char *dst;
  static char result[256];

  src = line;
  dst = result;
  while (*src != '=') {
    if ('\0' == *src) {
      /* premature end of line */
      return NULL;
    }
    *dst = *src;
    dst ++;
    src ++;
  }
  *dst = '\0';
  return result;
} /* ParseEntryName */

/*
 * return pointer to a named entry in given section
 */
static const DBEntry *
GetEntry (const DBSection *section, XBAtom atom)
{
  const DBEntry *ptr;
  assert (section != NULL);
  /* find entry */
  for (ptr = section->entry; ptr != NULL; ptr = ptr->next) {
    if (ptr->atom == atom) {
      return ptr;
    }
  }
  return NULL;
} /* GetEntry */

/*
 * return if character is eol
 * External level files could also come from windows environment,
 * where '\r' can be considered an EOL char for ini-file purposes.
 */
static int
XB_iniEOL( char c )
{
  switch ( c ) {
  case '\n': /* FALLTHRU */
  case '\r':
    return 1;
  default:
    return 0;
  }
} /* XB_iniEOL */

/*
 * parse the entry value part of a line
 */
static const char *
ParseEntryValue (const char *line)
{
  const char *src;
  char *dst;
  static char result[256];
  dst = result;
  src = strchr (line, '=');
  if (NULL == src) {
    return NULL;
  }
  src ++;
  while (isspace (*src) ) {
    src ++;
  }
  while ((!XB_iniEOL(*src)) && (*src != '\0')) {
    *dst = *src;
    dst ++;
    src ++;
  }

  while (isspace (*(dst-1)) ) {
    --dst;
  }
  *dst = 0;
  return result;
} /* ParseEntryValue */

/* file operations */

/*
 * open file corresponding to given database
 */
static FILE  *
OpenDBFile (const DBRoot *db, const char *mode)
{
  char db_name[320];
  assert (NULL != db);
  assert (NULL != mode);
  /* add index for demos */
  sprintf (db_name, "%s", GUI_AtomToString (db->atom) );
  return FileOpen (db_path[db->type], db_name, db_ext[db->type], mode);
} /* OpenDBFile */

/*
 * check if file directory database must be updated
 */
static XBBool
CheckInsertFile (const DBRoot *db, const char *name, XBAtom timeAtom, time_t mtime)
{
  const DBSection *section;
  time_t           ltime;
  /* does file exist as section name in database */
  section = DB_GetSection (db, GUI_StringToAtom (name) );
  if (NULL == section) {
    return XBFalse;
  }
  /* has the file been modified */
  if (! DB_GetEntryTime (section, timeAtom, &ltime) || ltime != mtime) {
    return XBFalse;
  }
  /* file entry exists and is up-to-date */
  return XBTrue;
} /* CheckInsertFile */

/*
 * insert file into database
 */
static XBBool
InsertFile (DBRoot *db, DBType type, const char *name, XBAtom timeAtom, time_t mtime, XBAtom fromAtom, DBLoadDirFunc func)
{
  DBRoot          *dbFile      = NULL;
  DBSection       *toSection   = NULL;
  const DBSection *fromSection = NULL;
  const DBEntry   *entry       = NULL;
  XBAtom           toAtom      = GUI_StringToAtom (name);
  /* try to load file type/name.ext into a temp database */
  dbFile = DB_Create (type, toAtom);
  assert (dbFile != NULL);
  if (! DB_Load (dbFile) ) {
    goto Error;
  }
  /* try to find given section */
  fromSection = DB_GetSection (dbFile, fromAtom);
  if (NULL == fromSection) {
    goto Error;
  }
  /* create new section for file data */
  toSection = DB_CreateSection (db, toAtom);
  assert (NULL != toSection);
  /* create entry for file modification time */
  DB_CreateEntryTime (toSection, timeAtom, mtime);
  /* now copy section */
  for (entry = fromSection->entry; NULL != entry; entry = entry->next) {
    (void) DB_CreateEntryString (toSection, entry->atom, entry->value);
  }
  /* call user function if any */
  if (NULL != func) {
    (*func) (toSection);
  }
  /* clean up */
  DB_Delete (dbFile);
  return XBTrue;

 Error:
  if (NULL != dbFile) {
    DB_Delete (dbFile);
  }
  if (NULL != toSection) {
    DB_DeleteSection (db, toAtom);
  }
  return XBFalse;
} /* CheckInsertFile */

/*
 * create named section in given database and mark it changed
 */
static void
UpdateFile (DBRoot *db, const char *name)
{
  DBSection *section = DB_CreateSection (db, GUI_StringToAtom (name));
  assert (NULL != section);
  section->changed = XBTrue;
} /* UpdateFile */

/*
 * delete any unchanged sections
 */
static XBBool
DeleteUnchangedSections (DBRoot *db)
{
  DBSection *ptr;
  DBSection *next;
  XBBool     deleted = XBFalse;
  assert (NULL != db);
  /* delete first section, while unchanged */
  while (NULL != db->section &&
	 ! db->section->changed) {
    next = db->section->next;
    FreeSection (db->section);
    db->section = next;
    deleted = XBTrue;
#ifdef DEBUG
    putchar ('-');
#endif
  }
  /* now proceed with the rest */
  if (NULL != db->section) {
    for (ptr = db->section; ptr != NULL && ptr->next != NULL; ptr = ptr->next) {
      while (NULL != ptr->next &&
	     ! ptr->next->changed) {
	next = ptr->next->next;
	FreeSection (ptr->next);
	ptr->next = next;
	deleted = XBTrue;
#ifdef DEBUG
	putchar ('-');
#endif
      }
    }
  }
#ifdef DEBUG
  fflush (stdout);
#endif
  return deleted;
} /* DeleteUnchangedSections */

/*************************
 * get/set database info *
 *************************/

/*
 * create a new database of given type and name
 */
DBRoot *
DB_Create (DBType type, XBAtom atom)
{
  DBRoot *db;
  assert (ATOM_INVALID != atom);
  /* create new database */
  db = calloc (1, sizeof (DBRoot));
  assert (NULL != db);
  db->atom      = atom;
  db->type      = type;
  db->changed   = XBTrue;
  /* store in list */
  db->next      = db_list[type];
  db_list[type] = db;
  /* that´s all */
  return db;
} /* DB_Create */

/*
 * return pointer of an existing database
 */
const DBRoot *
DB_Get (DBType type, XBAtom atom)
{
  const DBRoot *db;
  assert (ATOM_INVALID != atom);
  for (db = db_list[type]; db != NULL; db = db->next) {
    if (db->atom == atom) {
      return db;
    }
  }
  return NULL;
} /* DB_Get */

/*
 * get name atom of database (filename)
 */
XBAtom
DB_Atom (const DBRoot *db)
{
  assert (NULL != db);
  return db->atom;
} /* DB_Changed */

/*
 * check if database has changed
 */
XBBool
DB_Changed (const DBRoot *db)
{
  const DBSection *section;
  if (db->changed) {
    return XBTrue;
  }
  for (section = db->section; section != NULL; section = section->next) {
    if (section->changed) {
      return XBTrue;
    }
  }
  return XBFalse;
} /* DB_Changed */

/*
 * set name atom of given database (filename)
 */
void
DB_SetAtom (DBRoot *db, XBAtom atom)
{
  db->atom    = atom;
  db->changed = XBTrue;
} /* DB_SetAtom */

/*
 * mark given database as unchanged
 */
static void
MarkUnchanged (DBRoot *db)
{
  DBSection *section;
  /* mark all as unchanged */
  db->changed = XBFalse;
  for (section = db->section; section != NULL; section = section->next) {
    section->changed = XBFalse;
  }
} /* MarkUnchanged */

/*
 * delete a given database
 */
void
DB_Delete (DBRoot *db)
{
  DBRoot *ptr;
  assert (db_list[db->type] != NULL);
  /* check if first element in list */
  if (db_list[db->type] == db) {
    db_list[db->type] = db_list[db->type]->next;
  } else {
    for (ptr = db_list[db->type]; ptr->next != NULL; ptr = ptr->next) {
      if (db == ptr->next) {
	ptr->next = db->next;
	break;
      }
    }
  }
  FreeRoot (db);
} /* DB_Delete */

/************************
 * get/set section info *
 ************************/

/*
 * create a new section in a given database
 */
DBSection *
DB_CreateSection (DBRoot *db, XBAtom atom)
{
  DBSection *section;

  /* create new data structure */
  assert (NULL != db);
  assert (ATOM_INVALID != atom);
  /* try to find section first */
  for (section = db->section; section != NULL; section = section->next) {
    if (section->atom == atom) {
      return section;
    }
  }
  /* create new section */
  db->changed = XBTrue;
  section = calloc (1, sizeof (DBSection));
  assert (NULL != section);
  section->atom    = atom;
  section->changed = XBTrue;
  /* store in database */
  section->next    = db->section;
  db->section      = section;
  /* that´s all */
  return section;
} /* DB_CreateSection */

/*
 * return pointer to a named section in given database
 */
const DBSection *
DB_GetSection (const DBRoot *db, XBAtom atom)
{
  const DBSection *ptr;

  assert (NULL != db);
  assert (ATOM_INVALID != atom);
  /* find section */
  for (ptr = db->section; ptr != NULL; ptr = ptr->next) {
    if (ptr->atom == atom) {
      return ptr;
    }
  }
  return NULL;
} /* DB_GetSection */

/*
 * get section atom
 */
XBAtom
DB_SectionAtom(const DBSection *section)
{
  assert (section != NULL);
  return section->atom;
} /* DB_SectionAtom*/

/*
 * get the name of the nth section in given database
 */
XBAtom
DB_IndexSection (const DBRoot *db, int index)
{
  const DBSection *ptr;
  int num = 0;
  assert (NULL != db);
  /* find section */
  for (ptr = db->section; ptr != NULL; ptr = ptr->next) {
    if (num == index) {
      return ptr->atom;
    }
    num ++;
  }
  return ATOM_INVALID;
} /* DB_IndexSection */

/*
 * number of sections in a given database
 */
int
DB_NumSections (const DBRoot *db)
{
  const DBSection *ptr;
  int num = 0;
  assert (NULL != db);
  /* find section */
  for (ptr = db->section; ptr != NULL; ptr = ptr->next) {
    num ++;
  }
  return num;
} /* DB_NumSections */

/*
 * delete a section in a given database
 */
void
DB_DeleteSection (DBRoot *db, XBAtom atom)
{
  DBSection *save, *ptr;
  assert (db != NULL);
  db->changed = XBTrue;
  /* do we have any sections ? */
  if (NULL == db->section) {
    return;
  }
  assert (ATOM_INVALID != atom);
  /* it is the first section ? */
  if (db->section->atom == atom) {
    save        = db->section;
    db->section = db->section->next;
    FreeSection (save);
  } else {
    for (ptr = db->section; ptr->next != NULL; ptr = ptr->next) {
      if (ptr->next->atom == atom) {
	save      = ptr->next;
	ptr->next = ptr->next->next;
	FreeSection (save);
	break;
      }
    }
  }
} /* DB_DeleteSection */

/*
 * delete all sections in a given database
 */
void
DB_DeleteAll (DBRoot *db)
{
  DBSection *save;
  assert (db != NULL);
  db->changed = XBTrue;
  /* do we have any sections ? */
  if (NULL == db->section) {
    return;
  }
  while (db->section != NULL) {
    save        = db->section;
    db->section = db->section->next;
    FreeSection (save);
  }
} /* DB_DeleteAll */

/*************************
 * get entry information *
 *************************/

/*
 * create a string entry in a given section
 */
XBBool
DB_CreateEntryString (DBSection *section, XBAtom atom, const char *value)
{
  DBEntry *entry;

  assert (ATOM_INVALID != atom);
  assert (NULL != section);
  /* mark as changed */
  section->changed = XBTrue;
  /* try to find entry first */
  for (entry = section->entry; entry != NULL; entry = entry->next) {
    if (entry->atom == atom) {
      if (NULL != entry->value) {
	free (entry->value);
      }
      entry->value = DupString (value);
      assert (NULL != entry->value);
      return XBTrue;
    }
  }
  /* create new data strutcure */
  entry = calloc (1, sizeof (DBEntry));
  assert (NULL != entry);
  entry->atom    = atom;
  entry->value   = DupString (value);
  assert (NULL != entry->value);
  /* store in database */
  entry->next    = section->entry;
  section->entry = entry;
  /* that's all */
  return XBTrue;
} /* DB_CreateEntryString */

/*
 * create an atom entry in a given section
 */
XBBool
DB_CreateEntryAtom (DBSection *section, XBAtom atom, XBAtom value)
{
  return DB_CreateEntryString (section, atom, GUI_AtomToString (value));
} /* DB_CreateEntryAtom */

/*
 * create an integer entry in a given section
 */
XBBool
DB_CreateEntryInt (DBSection *section, XBAtom atom, int value)
{
  char tmp[32];
  sprintf (tmp, "%d", value);
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntryInt */

/*
 * create a game result entry in a given section
 */
XBBool
DB_CreateEntryGameResult (DBSection *section, XBAtom atom, int score)
{
  char tmp[32];
  sprintf (tmp, "%d",score);
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntry */

/*
 * create a player rating entry in a given section
 */
XBBool
DB_CreateEntryPlayerRating(DBSection *section, XBAtom atom, int PID, float rating)
{
  char tmp[32];
  sprintf (tmp, "%d %f", PID, rating);
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntryPlayerRating */

/*
 * create a bool entry in given section
 */
XBBool
DB_CreateEntryBool (DBSection *section, XBAtom atom, XBBool value)
{
  return DB_CreateEntryString (section, atom, value ? "true" : "false");
} /* DB_CreateEntryBool */

/*
 * create a color entry in given section
 */
XBBool
DB_CreateEntryColor (DBSection *section, XBAtom atom, XBColor color)
{
  char tmp[32];
  sprintf (tmp, "#%02x%02x%02x", GET_RED (color) << 3, GET_GREEN (color) << 3, GET_BLUE (color) << 3);
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntryColor */

/*
 * create a time entry in given section
 */
XBBool
DB_CreateEntryTime (DBSection *section, XBAtom atom, time_t value)
{
  char tmp[64];
  size_t len;
  len = sprintf (tmp, "%lu ; %s", value, ctime (&value));
  /* cut trailing newline*/
  tmp[len - 1] = 0;
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntryTime */

/*
 * create a float entry in given section
 */
XBBool
DB_CreateEntryFloat (DBSection *section, XBAtom atom, double value)
{
  char tmp[32];
  sprintf (tmp, "%f", value);
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntryFloat */

/*
 * create a position entry in given section
 */
XBBool
DB_CreateEntryPos (DBSection *section, XBAtom atom,BMPosition *pValue )
{
  char tmp[64];
  sprintf (tmp, "%d %d",pValue->x, pValue->y);
  return DB_CreateEntryString (section, atom, tmp);
} /* DB_CreateEntryPos */

/*
 * create entry in given section, defined by given key=value line
 */
XBBool
DB_ParseEntry (DBSection *section, const char *line)
{
  const char *entryName;
  assert (NULL != section);
  assert (NULL != line);
  /* parse entry */
  if (NULL == (entryName = ParseEntryName (line) ) ) {
    return XBFalse;
  }
  return DB_CreateEntryString (section, GUI_StringToAtom (entryName), ParseEntryValue (line) );
} /* DB_ParseEntry */

/*
 * copy name section/entry from given section to given database
 */
XBBool
DB_CopyEntry (DBRoot *db, const DBSection *section, const XBAtom atom)
{
  DBSection *sec;
  const DBEntry *ptr;
  assert (NULL != db);
  assert (NULL != section);
  assert (NULL != db);
  /* try to create section in database */
  sec = DB_CreateSection(db, section->atom);
  if (NULL == sec) {
    return XBFalse;
  }
  /* try to get entry */
  ptr = GetEntry(section, atom);
  if (NULL == ptr) {
    return XBFalse;
  }
  /* now duplicate entry */
  return DB_CreateEntryString(sec, atom, ptr->value);
} /* DB_CopyEntry */

/*
 * get the name of the nth entry in given section
 */
XBAtom
DB_IndexEntry (const DBSection *section, int index)
{
  const DBEntry *ptr;
  int num = 0;
  assert (NULL != section);
  /* find section */
  for (ptr = section->entry; ptr != NULL; ptr = ptr->next) {
    if (num == index) {
      return ptr->atom;
    }
    num ++;
  }
  return ATOM_INVALID;
} /* DB_IndexSection */

/*
 * number of entries in given database
 */
int
DB_NumAllEntries (const DBRoot *db)
{
  const DBSection *sec;
  int num = 0;
  assert (NULL != db);
  for (sec = db->section; sec != NULL; sec = sec->next) {
    num += DB_NumEntries(sec);
  }
  return num;
} /* DB_NumAllEntries */

/*
 * number of entries in given section
 */
int
DB_NumEntries (const DBSection *section)
{
  const DBEntry *ptr;
  int num = 0;
  assert (NULL != section);
  /* find section */
  for (ptr = section->entry; ptr != NULL; ptr = ptr->next) {
    num ++;
  }
  return num;
} /* DB_NumEntries */

/*
 * get string value for named entry in given section
 */
XBBool
DB_GetEntryString (const DBSection *section, XBAtom atom, const char **pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (entry->value != NULL);
  *pValue = entry->value;
  return XBTrue;
} /* DB_GetEntryString */

/*
 * get atom value for named entry in given section
 */
XBBool
DB_GetEntryAtom (const DBSection *section, XBAtom atom, XBAtom *pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  *pValue = GUI_StringToAtom (entry->value);
  return (ATOM_INVALID != *pValue);
} /* DB_GetEntryAtom */

/*
 * get int value for named entry in given section
 */
XBBool
DB_GetEntryInt (const DBSection *section, XBAtom atom, int *pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  return (1 == sscanf (entry->value, "%d", pValue)) ? XBTrue : XBFalse;
} /* DB_GetEntryInt */

/*
 * get bool value for named entry in given section
 */
XBBool
DB_GetEntryBool (const DBSection *section, XBAtom atom, XBBool *pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  if (0 == strcmp (entry->value, "true") ) {
    *pValue = XBTrue;
  } else if (0 == strcmp (entry->value, "false") ) {
    *pValue = XBFalse;
  } else {
    return XBFalse;
  }
  return XBTrue;
} /* DB_GetEntryBool */

/*
 * get color value for named entry in given section
 */
XBBool
DB_GetEntryColor (const DBSection *section, XBAtom atom, XBColor *pValue)
{
  const DBEntry *entry;
  unsigned red, green, blue;
  char hash;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  if (4 != sscanf (entry->value, "%c%2X%2X%2X", &hash, &red, &green, &blue) ) {
    return XBFalse;
  }
  if (hash != '#') {
    return XBFalse;
  }
  *pValue = SET_COLOR (red >> 3, green >> 3, blue >> 3);
  return XBTrue;
} /* DB_GetEntryColor */

/*
 * get time value for named entry in given sectiong
 */
XBBool
DB_GetEntryTime (const DBSection *section, XBAtom atom, time_t *pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  return (1 == sscanf (entry->value, "%lu", pValue)) ? XBTrue : XBFalse;
} /* DB_GetEntryTime */

/*
 * get float value for named entry in given section
 */
XBBool
DB_GetEntryFloat (const DBSection *section, XBAtom atom, double *pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  return (1 == sscanf (entry->value, "%lf", pValue)) ? XBTrue : XBFalse;
} /* DB_GetEntryFloat */

/*
 * get position value for named entry in given section
 */
XBBool
DB_GetEntryPos (const DBSection *section, XBAtom atom, BMPosition *pValue)
{
  const DBEntry *entry;
  assert (pValue != NULL);
  entry = GetEntry (section, atom);
  if (NULL == entry) {
    return XBFalse;
  }
  assert (NULL != entry->value);
  return (2 == sscanf (entry->value, "%hd%hd", &pValue->x, &pValue->y)) ? XBTrue : XBFalse;
} /* DB_GetEntryPos */

/*
 * get string value for named entry in given section and convert to integer
 */
DBConversionResult
DB_ConvertEntryInt (const DBSection *section, XBAtom atom, int *pValue, const DBToInt *convTable)
{
  const DBEntry *entry;
  const DBToInt *ptr;
  int cmp;
  if (NULL == (entry = GetEntry (section, atom) ) ) {
    return DCR_NoSuchEntry;
  }
  assert (convTable != NULL);
  for (ptr = convTable; ptr->key != NULL; ptr ++) {
    cmp = strcmp (ptr->key, entry->value);
    if (0 == cmp) {
      /* match */
      assert (pValue != NULL);
      *pValue = ptr->value;
      return DCR_Okay;
    } else if (cmp > 0) {
      return DCR_Failure;
    }
  }
  return DCR_Failure;
} /* DB_ConvertEntryInt */

/*
 * get string value for named entry in given section and convert to a generic pointer
 */
DBConversionResult
DB_ConvertEntryData (const DBSection *section, XBAtom atom, void **pValue, const DBToData *convTable)
{
  const DBEntry *entry;
  const DBToData *ptr;
  int cmp;
  if (NULL == (entry = GetEntry (section, atom) ) ) {
    return DCR_NoSuchEntry;
  }
  assert (convTable != NULL);
  for (ptr = convTable; ptr->key != NULL; ptr ++) {
    cmp = strcmp (ptr->key, entry->value);
    if (0 == cmp) {
      /* match */
      assert (pValue != NULL);
      *pValue = ptr->value;
      return DCR_Okay;
    } else if (cmp > 0) {
      return DCR_Failure;
    }
  }
  return DCR_Failure;
} /* DB_ConvertEntryData */

/*
 * get string value for named entry in given section and convert to an entry flag number
 */
DBConversionResult
DB_ConvertEntryFlags (const DBSection *section, XBAtom atom, unsigned *pValue, const DBToInt *convTable)
{
  const DBEntry *entry;
  const DBToInt *ptr;
  int cmp;
  int i, argc;
  char **argv;
  if (NULL == (entry = GetEntry (section, atom) ) ) {
    return DCR_NoSuchEntry;
  }
  assert (convTable != NULL);
  assert (pValue != NULL);
  *pValue = 0;
  /* split value string */
  argv = SplitString (entry->value, &argc);
  assert (NULL != argv);
  /* parse list */
  for (i = 0; i < argc; i ++) {
    for (ptr = convTable; ptr->key != NULL; ptr ++) {
      cmp = strcmp (ptr->key, argv[i]);
      if (0 == cmp) {
	/* match */
	*pValue |= ptr->value;
	break;
      } else if (cmp > 0) {
	free ((char *) argv);
	return DCR_Failure;
      }
    }
  }
  free ((char *) argv);
  return DCR_Okay;
} /* DB_ConvertEntryFlag */

/*
 * create a string line for n-th entry in given section, return length
 */
size_t
DB_PrintEntry (char *buffer, const DBSection *section, int index)
{
  const DBEntry *ptr;
  int num = 0;
  assert (NULL != buffer);
  assert (NULL != section);
  /* find entry */
  for (ptr = section->entry; ptr != NULL; ptr = ptr->next) {
    if (num == index) {
      return sprintf (buffer, "%s=%s", GUI_AtomToString (ptr->atom), ptr->value);
    }
    num ++;
  }
  /* entry not found */
  buffer[0] = 0;
  return 0;
} /* DB_PrintEntry */

/*
 * add string to sized buffer, fill with x if added string does not fit
 */
static XBBool
AddStringToBuffer(char *buf, size_t len, const char *add)
{
  int i, nul,write;
  /* sanity checks */
  assert( NULL != buf);
  assert( NULL != add);
  /* find first null byte */
  nul = -1;
  for (i = 0; (i<len) && (nul<0); i++) {
    if ( 0 == *(buf+i) ) {
      nul = i;
    }
  }
  /* check if buffer is full */
  if (nul == len-1) {
    return XBFalse;
  }
  /* check if there is a null byte */
  if (nul<0) {
    Dbg_Out("Buffer has no null byte!\n");
    memset(buf+len-1,0,1);
    return XBFalse;
  }
  /* check if add string fits */
  write = strlen(add);
  if (nul+write < len) {
    memcpy(buf+nul,add,write+1);
    return XBTrue;
  }
  /* no, does not fit */
  memset(buf+nul,'x',len-nul-1);
  memset(buf+len-1,0,1);
  return XBFalse;
} /* AddStringToBuffer */

/*
 * return a string for entry in given section
 */
char *
DB_SectionEntryString (const DBSection *section, XBAtom atom)
{
  static char buffer[100];
  const char *tmp;
  const DBEntry *ptr;
  /* init buffer */
  memset(buffer,0,1);
  /* get section name */
  AddStringToBuffer(buffer,sizeof(buffer),"[");
  assert (NULL != section);
  tmp = GUI_AtomToString(section->atom);
  if (NULL == tmp) {
    tmp = "???";
  }
  AddStringToBuffer(buffer,sizeof(buffer),tmp);
  AddStringToBuffer(buffer,sizeof(buffer),"] ");
  /* get entry name */
  tmp = GUI_AtomToString(atom);
  if (NULL == tmp) {
    tmp = "???";
  }
  AddStringToBuffer(buffer,sizeof(buffer),tmp);
  AddStringToBuffer(buffer,sizeof(buffer),"=");
  /* find entry value */
  ptr = GetEntry(section, atom);
  if (NULL == ptr) {
    tmp = "<none>";
  } else {
    tmp = ptr->value;
  }
  AddStringToBuffer(buffer,sizeof(buffer),tmp);
  return buffer;
} /* DB_PrintEntry */

/*
 * delete an entry from section
 */
void
DB_DeleteEntry (DBSection *section, XBAtom atom)
{
  DBEntry *save, *ptr;
  assert (section != NULL);
  section->changed = XBTrue;
  /* do we have any entries ? */
  if (NULL == section->entry) {
    return;
  }
  assert (ATOM_INVALID != atom);
  /* it is the first entry ? */
  if (section->entry->atom == atom) {
    save           = section->entry;
    section->entry = section->entry->next;
    FreeEntry (save);
  } else {
    for (ptr = section->entry; ptr->next != NULL; ptr = ptr->next) {
      if (ptr->next->atom == atom) {
	save      = ptr->next;
	ptr->next = ptr->next->next;
	FreeEntry (save);
	break;
      }
    }
  }
} /* DB_DeleteEntry */

/************************
 * conversion functions *
 ************************/

/*
 * translate an integer value into its conversion string
 */
const char *
DB_IntToString (const DBToInt *table, int value)
{
  const DBToInt *ptr;
  assert (NULL != table);
  for (ptr = table; ptr->key != NULL; ptr ++) {
    if (ptr->value == value) {
      return ptr->key;
    }
  }
  return NULL;
} /* DB_IntToString */

/*
 * translate a data pointer into its conversion string
 */
const char *
DB_DataToString (const DBToData *table, void *pValue)
{
  const DBToData *ptr;
  assert (NULL != table);
  for (ptr = table; ptr->key != NULL; ptr ++) {
    if (ptr->value == pValue) {
      return ptr->key;
    }
  }
  return NULL;
} /* DB_DataToString */

/*******************
 * file operations *
 *******************/

/*
 * load given database from file
 */
XBBool
DB_Load (DBRoot *db)
{
  FILE       *fp;
  DBSection  *section;
  char        line[256];
  const char *sectionName;
  const char *entryName;
  assert (db != NULL);
  assert (ATOM_INVALID != db->atom);
  /* try to open file for reading */
  if (NULL == (fp = OpenDBFile (db, "r") ) ) {
    return XBFalse;
  }
  /* parse it */
  section = NULL;
  while (NULL != fgets (line, sizeof (line), fp) ) {
    if (NULL != (sectionName = ParseSection (line) ) ) {
      section = DB_CreateSection (db, GUI_StringToAtom (sectionName) );
    } else if (NULL != section &&
	       NULL != (entryName = ParseEntryName (line) ) ) {
      (void) DB_CreateEntryString (section, GUI_StringToAtom (entryName), ParseEntryValue (line) );
    }
  }
  /* mark all as unchanged */
  MarkUnchanged (db);
  /* that's all */
  fclose (fp);
  return XBTrue;
} /* DB_Load */

/*
 * store database in file
 */
XBBool
DB_Store (DBRoot *db)
{
  FILE            *fp;
  const DBSection *section;
  const DBEntry   *entry;
  /* try to open file for writing */
  if (NULL == (fp = OpenDBFile (db, "w") ) ) {
    return XBFalse;
  }
  /* now write it */
  for (section = db->section; section != NULL; section = section->next) {
    fprintf (fp, "[%s]\n", GUI_AtomToString (section->atom) );
    for (entry = section->entry; entry != NULL; entry = entry->next) {
      fprintf (fp, "%s=%s\n", GUI_AtomToString (entry->atom), entry->value);
    }
    fprintf (fp, "\n");
  }
  /* mark all as unchanged */
  MarkUnchanged (db);
  /* that's all */
  fclose (fp);
  return XBTrue;
} /* DB_Store */

/*
 * append database to file
 */
XBBool
DB_Append (DBRoot *db)
{
  FILE            *fp;
  const DBSection *section;
  const DBEntry   *entry;
  /* try to open for appending */
  if (NULL == (fp = OpenDBFile (db, "a") ) ) {
    return XBFalse;
  }
  /* now write it */
  for (section = db->section; section != NULL; section = section->next) {
    fprintf (fp, "[%s]\n", GUI_AtomToString (section->atom) );
    for (entry = section->entry; entry != NULL; entry = entry->next) {
      fprintf (fp, "%s=%s\n", GUI_AtomToString (entry->atom), entry->value);
    }
    fprintf (fp, "\n");
  }
  /* mark all as unchanged */
  MarkUnchanged (db);
  /* that's all */
  fclose (fp);
  return XBTrue;
} /* DB_Append */

/*
 * dump database to stderr
 */
XBBool
DB_Dump (const DBRoot *db)
{
  const DBSection *section;
  const DBEntry   *entry;
  /* now write it */
  fprintf(stderr,"---database dump----\n");
  for (section = db->section; section != NULL; section = section->next) {
    fprintf (stderr, "[%s]\n", GUI_AtomToString (section->atom) );
    for (entry = section->entry; entry != NULL; entry = entry->next) {
      fprintf (stderr, "%s=%s\n", GUI_AtomToString (entry->atom), entry->value);
    }
    fprintf (stderr, "\n");
  }
  fprintf(stderr,"---end of database dump---\n");
  /* that's all */
  return XBTrue;
} /* DB_Dump */

/*
 * load complete directory of files into database,
 */
XBBool
DB_LoadDir (DBRoot *db, const char *path, const char *ext, DBType type, XBAtom timeAtom, XBAtom section, DBLoadDirFunc func)
{
  XBBool  changed;
  XBDir  *fileList;
  XBDir  *ptr;
  /* sanity checks */
  assert (NULL != db);
  assert (NULL != path);
  assert (NULL != ext);
  /* load database from file */
  DB_Load (db);
  /* now compare with current directory contents */
  changed  = XBFalse;
  fileList = CreateFileList (path, ext);
  for (ptr = fileList; NULL != ptr; ptr = ptr->next) {
    if (! CheckInsertFile (db, ptr->name, timeAtom, ptr->mtime) ) {
      InsertFile (db, type, ptr->name, timeAtom, ptr->mtime, section, func);
      changed = XBTrue;
#ifdef DEBUG
      putchar ('*');
#endif
    } else {
      UpdateFile (db, ptr->name);
#ifdef DEBUG
      putchar ('.');
#endif
    }
#ifdef DEBUG
    fflush (stdout);
#endif
  }
  /* now delete "unchanged section", because they do not have any related files */
  if (DeleteUnchangedSections (db) ) {
    changed = XBTrue;
  }
  if (NULL != fileList) {
    DeleteFileList (fileList);
#ifdef DEBUG
    putchar ('\n');
#endif
  }
  return changed;
} /* DB_LoadDir */

/*
 * read message of the day, return line count
 */
int
ReadMessageOfTheDay(int m,char *s,int *l)
{
  FILE *fp;
  int i;
  char *c,*d;
  i=0;
  /* try to open the file */
  fp = FileOpen(db_path[3], "message", "txt", "r");
  if (NULL!=fp) {
    c=s;
    i=0;
    l[i]=0;
    while (1) {
      /* get character */
      d = fgets(c,m-l[i],fp);
      if (d == NULL) {
	break;
      }
      i++;
      l[i]=strlen(s)-1;
      c=s+strlen(s)-1;
    }
    l[i+1]=strlen(s);
  }
  return i;
} /* ReadmessageOfTheDay */

/*
 * end of file ini_file.c
 */
