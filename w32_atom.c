/*
 * Datei w32_atom.c - atoms for database searches
 *
 * $Id: w32_atom.c,v 1.4 2006/02/19 13:33:01 lodott Exp $
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
#include "gui.h"

#include "w32_common.h"

/*
 * local constants
 */
#define ATOM_STRING_MAX  256
#define NUM_ATOMS        768
#define NUM_STATIC         8

/*
 * setup local atoms table
 */
XBBool
GUI_InitAtoms (void)
{
	InitAtomTable (NUM_ATOMS);
	return XBTrue;
}								/* InitAtoms */

/*
 * convert a string to an atom
 */
XBAtom
GUI_FormatToAtom (const char *fmt, ...)
{
	XBAtom atom;
	char string[ATOM_STRING_MAX];
	va_list argList;
#ifdef DEBUG_ATOM
	static unsigned atomCount = 0;
#endif

	assert (fmt != NULL);
	/* format string */
	va_start (argList, fmt);
	vsprintf (string, fmt, argList);
	va_end (argList);
#ifdef DEBUG_ATOM
	if (0 == FindAtom (string)) {
		atomCount++;
		if ((atomCount & 0xF) == 0) {
			fprintf (stderr, "atomCount=%u\n", atomCount);
		}
	}
#endif
	atom = AddAtom (string);
	return atom;
}								/* GUI_StringToAtom */

/*
 * convert a string to an atom
 */
XBAtom
GUI_StringToAtom (const char *string)
{
#ifdef DEBUG_ATOM
	return GUI_FormatToAtom ("%s", string);
#else
	XBAtom atom;

	assert (string != NULL);
	atom = AddAtom (string);
	return atom;
#endif
}								/* GUI_StringToAtom */

/*
 * convert integer to atom 
 */
XBAtom
GUI_IntToAtom (int value)
{
	return GUI_FormatToAtom ("%d", value);
}								/* GUI_IntToAtom */

/*
 * return string to an atom (transient)
 */
const char *
GUI_AtomToString (XBAtom atom)
{
	static int i = 0;
	static char string[NUM_STATIC][ATOM_STRING_MAX];

	i++;
	if (i >= NUM_STATIC) {
		i = 0;
	}
	if (0 == GetAtomName ((unsigned short)atom, string[i], sizeof (string[i]))) {
		return NULL;
	}
	return string[i];
}								/* GUI_AtomToString */

/*
 * convert atom int or -1 
 */
int
GUI_AtomToInt (XBAtom atom)
{
	const char *s;
	int value;

	if (NULL == (s = GUI_AtomToString (atom))) {
		return -1;
	}
	if (1 != sscanf (s, "%d", &value)) {
		return -1;
	}
	return value;
}								/* GUI_AtomToInt */

/*
 * end of file w32_atom.c
 */
