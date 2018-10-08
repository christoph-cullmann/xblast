/*
 * x11_atom.h - atoms (aka XrmQuark) for faster database handling
 *
 * $Id: x11_atom.c,v 1.5 2006/02/09 21:21:25 fzago Exp $
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
#include "x11_common.h"

/*
 * Initialize Atoms
 */
XBBool
GUI_InitAtoms (void)
{
	XrmInitialize ();
	return XBTrue;
}								/* InitAtoms */

/*
 * conversion string to atom
 */
XBAtom
GUI_StringToAtom (const char *string)
{
	XBAtom atom;

	assert (NULL != string);
	atom = XrmStringToQuark (string);
#ifdef DEBUG_ATOM
	fprintf (stderr, "atom = %lu\n", atom);
#endif
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
	atom = XrmStringToQuark (tmp);
#ifdef DEBUG_ATOM
	fprintf (stderr, "atom = %lu\n", atom);
#endif
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
	return XrmQuarkToString (atom);
}								/* GUI_StringToAtom */

/*
 * convert atom int or -1 
 */
int
GUI_AtomToInt (XBAtom atom)
{
	int value;

	if (1 != sscanf (XrmQuarkToString (atom), "%d", &value)) {
		return -1;
	}
	return value;
}								/* GUI_AtomToInt */

/*
 * end of file x11_atom.c
 */
