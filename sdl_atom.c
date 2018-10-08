/*
 * x11_atom.h - atoms (aka XrmQuark) for faster database handling
 *
 * $Id: sdl_atom.c 112466 2009-07-06 08:37:37Z ingmar $
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

#include "sdl_common.h"

#define MAX_ATOMS 5000

typedef struct
{
	XBAtom atom;
	const char *name;
} AtomsTableStr;

static AtomsTableStr atomsTable[MAX_ATOMS];
static int atomsCount;

static int
AtomsTableCompare (const void *a, const void *b)
{
/*   printf("comparing %s with %s\n", ((AtomsTableStr *)a)->name, ((AtomsTableStr *)b)->name); */
	return strcmp (((const AtomsTableStr *) a)->name, ((const AtomsTableStr *) b)->name);
}

/*
 * Initialize Atoms
 */
XBBool
GUI_InitAtoms (void)
{

/*   atomsTable = calloc( MAX_ATOMS, sizeof(AtomsTableStr *) ); */
/*   if(atomsTable == NULL) { */
/*     return XBFalse; */
/*   } */
	atomsCount = 0;
	return XBTrue;
}								/* InitAtoms */

/*
 * conversion string to atom
 */
XBAtom
GUI_StringToAtom (const char *string)
{

	AtomsTableStr key;
	XBAtom atom;
	AtomsTableStr **atom_ptr;
	assert (NULL != string);

	key.name = string;

/*   printf("bsearch(%s) cnt = %d\n", string, atomsCount); */
	atom_ptr = bsearch (&key, atomsTable, atomsCount, sizeof (AtomsTableStr), AtomsTableCompare);
	if (atom_ptr) {
		atom = ((AtomsTableStr *) atom_ptr)->atom;
	}
	else {
/*     printf("failed!!!!!!\n"); */
		atom = atomsCount + 1;
		key.atom = atom;
		atomsTable[atomsCount].atom = atom;
		atomsTable[atomsCount].name = strdup (string);
		atomsCount++;
		qsort (atomsTable, atomsCount, sizeof (AtomsTableStr), AtomsTableCompare);
	}

	return atom;
}								/* GUI_StringToAtom */

/*
 * formatted string to atom
 */
XBAtom
GUI_FormatToAtom (const char *fmt, ...)
{
	XBAtom atom;
	char tmp[256];
	va_list argList;

	assert (NULL != fmt);
	/* formatting */
	va_start (argList, fmt);
	vsprintf (tmp, fmt, argList);
	va_end (argList);
	/* conversion */
	atom = GUI_StringToAtom (tmp);
	return atom;
}								/* GUI_FormatToAtom */

/*
 * convert int to atom
 */
XBAtom
GUI_IntToAtom (int value)
{
	return GUI_FormatToAtom ("%d", value);
}								/* GUI_IntToAtom */

/*
 * conversion atom to string
 */
const char *
GUI_AtomToString (XBAtom atom)
{
	int i;
	for (i = 0; i < atomsCount; i++) {
		if (atomsTable[i].atom == atom) {
			return atomsTable[i].name;
		}
	}
	return NULL;

/*   return atomsTable[atom]; */
}								/* GUI_StringToAtom */

/*
 * convert atom int or -1 
 */
int
GUI_AtomToInt (XBAtom atom)
{

	const char *s;
	int value;
	s = GUI_AtomToString (atom);
	if (NULL == s) {
		return -1;
	}
	if (1 != sscanf (s, "%d", &value)) {
		return -1;
	}
	return value;

}								/* GUI_AtomToInt */

/*
 * end of file x11_atom.c
 */
