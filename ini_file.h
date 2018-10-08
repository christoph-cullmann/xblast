/*
 * file ini_file.h - parsing ini-file into a database
 *
 * $Id: ini_file.h,v 1.17 2006/04/11 16:44:00 fzago Exp $
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
#ifndef _INI_FILE_H
#define _INI_FILE_H

/*
 * type definitions
 */

/* struct for string to int conversions */
typedef struct
{
	const char *key;
	int value;
} DBToInt;

/* struct for string to data conversions */
typedef struct
{
	const char *key;
	void *value;
} DBToData;

/* type of database */
typedef enum
{
	DT_Config,
	DT_Level,
	DT_Demo,
	DT_Central,
	NUM_DT
} DBType;

/* conversion results */
typedef enum
{
	DCR_Okay = 0,
	DCR_NoSuchEntry,
	DCR_Failure,
} DBConversionResult;

/* declarations of db elements */
typedef struct _db_entry DBEntry;
typedef struct _db_section DBSection;
typedef struct _db_root DBRoot;

/* callback for DB_LoadDir */
typedef void (*DBLoadDirFunc) (DBSection *);

/*
 * prototypes
 */

/* databases */
extern DBRoot *DB_Create (DBType type, XBAtom atom);
extern const DBRoot *DB_Get (DBType type, XBAtom atom);
extern XBBool DB_Changed (const DBRoot * db);
extern XBAtom DB_Atom (const DBRoot *);
extern void DB_SetAtom (DBRoot *, XBAtom);
extern void DB_Delete (DBRoot * db);

/* sections */
extern DBSection *DB_CreateSection (DBRoot * db, XBAtom atom);
extern DBSection *DB_GetSection (const DBRoot * db, XBAtom atom);
extern XBAtom DB_SectionAtom (const DBSection * section);
extern int DB_NumSections (const DBRoot * db);
extern XBAtom DB_IndexSection (const DBRoot * db, int index);
extern void DB_DeleteAll (DBRoot * db);
extern void DB_DeleteSection (DBRoot * db, XBAtom atom);

/* entries */
extern XBBool DB_CreateEntryString (DBSection * section, XBAtom atom, const char *value);
extern XBBool DB_CreateEntryAtom (DBSection * section, XBAtom atom, XBAtom value);
extern XBBool DB_CreateEntryInt (DBSection * section, XBAtom atom, int value);
extern XBBool DB_CreateEntryColor (DBSection * section, XBAtom atom, XBColor value);
extern XBBool DB_CreateEntryBool (DBSection * section, XBAtom atom, XBBool value);
extern XBBool DB_CreateEntryTime (DBSection * section, XBAtom atom, time_t value);
extern XBBool DB_CreateEntryFloat (DBSection * section, XBAtom atom, double value);
extern XBBool DB_CreateEntryGameResult (DBSection * section, XBAtom atom, int score);
extern XBBool DB_CreateEntryPlayerRating (DBSection * section, XBAtom atom, int PID, float rating);
extern XBBool DB_CreateEntryPos (DBSection * section, XBAtom atom, BMPosition * pValue);
extern XBBool DB_ParseEntry (DBSection * db, const char *line);
extern XBBool DB_CopyEntry (DBRoot * db, const DBSection * section, XBAtom atom);

extern XBBool DB_GetEntryString (const DBSection * section, XBAtom atom, const char **pString);
extern XBBool DB_GetEntryAtom (const DBSection * section, XBAtom atom, XBAtom * pAtom);
extern XBBool DB_GetEntryInt (const DBSection * section, XBAtom atom, int *pValue);
extern XBBool DB_GetEntryColor (const DBSection * section, XBAtom atom, XBColor * pValue);
extern XBBool DB_GetEntryBool (const DBSection * section, XBAtom atom, XBBool * pValue);
extern XBBool DB_GetEntryTime (const DBSection * section, XBAtom atom, time_t * pValue);
extern XBBool DB_GetEntryFloat (const DBSection * section, XBAtom atom, double *pValue);
extern XBBool DB_GetEntryPos (const DBSection * section, XBAtom atom, BMPosition * pValue);

extern DBConversionResult DB_ConvertEntryInt (const DBSection * section, XBAtom atom, int *pValue,
											  const DBToInt * convTable);
extern DBConversionResult DB_ConvertEntryData (const DBSection * section, XBAtom atom,
											   void **pValue, const DBToData * convTable);
extern DBConversionResult DB_ConvertEntryFlags (const DBSection * section, XBAtom atom,
												unsigned *pValue, const DBToInt * convTable);

extern int DB_NumAllEntries (const DBRoot * db);
extern int DB_NumEntries (const DBSection * section);
extern XBAtom DB_IndexEntry (const DBSection * section, int index);
extern size_t DB_PrintEntry (char *buffer, const DBSection * section, int index);
extern char *DB_SectionEntryString (const DBSection * section, XBAtom atom);
extern void DB_DeleteEntry (DBSection * section, XBAtom atom);

/* conversion */
extern const char *DB_IntToString (const DBToInt * table, int value);
extern const char *DB_DataToString (const DBToData * table, void *pValue);

/* file operations */
extern XBBool DB_Store (DBRoot * db);
extern XBBool DB_Append (DBRoot * db);
extern XBBool DB_Load (DBRoot * db);
extern XBBool DB_Dump (const DBRoot * db);
extern XBBool DB_LoadDir (DBRoot * db, const char *path, const char *ext, DBType type, XBAtom mtime,
						  XBAtom section, DBLoadDirFunc func, XBBool rec);
extern int ReadMessageOfTheDay (int m, char *s, int *l);

/* clear all data */
extern void DB_Finish (void);

#endif
/*
 * end of file ini_file.h
 */
