/*
 * file w32_keysym.c - create X11 like keysmbol names for virtual keys
 * 
 * $Id: sdl_keysym.c 112466 2009-07-06 08:37:37Z ingmar $
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

/*
 * local types
 */
typedef struct
{
	SDLKey code;
	const char *name;
} KeyNameTable;

typedef struct
{
	XBAtom atom;
	SDLKey code;
} AtomCodeTable;

/*
 * local variables
 */
static int numKeys = 0;
static AtomCodeTable *atomTable = NULL;
static AtomCodeTable *codeTable = NULL;
static KeyNameTable keyTable[] = {
	{SDLK_BACKSPACE, "BackSpace"},
	{SDLK_TAB, "Tab"},
	{SDLK_RETURN, "Return"},
	{KMOD_SHIFT, "Shift"},
	{KMOD_CTRL, "Control"},
	{SDLK_PAUSE, "Pause"},
	{SDLK_CAPSLOCK, "Caps_Lock"},
	{SDLK_ESCAPE, "Escape"},
	{SDLK_SPACE, "space"},
	{SDLK_PAGEUP, "Prior"},
	{SDLK_PAGEDOWN, "Next"},
	{SDLK_END, "End"},
	{SDLK_HOME, "Home"},
	{SDLK_LEFT, "Left"},
	{SDLK_UP, "Up"},
	{SDLK_RIGHT, "Right"},
	{SDLK_DOWN, "Down"},
	{SDLK_PRINT, "Print"},
	{SDLK_INSERT, "Insert"},
	{SDLK_DELETE, "Delete"},
	{SDLK_0, "0"},
	{SDLK_1, "1"},
	{SDLK_2, "2"},
	{SDLK_3, "3"},
	{SDLK_4, "4"},
	{SDLK_5, "5"},
	{SDLK_6, "6"},
	{SDLK_7, "7"},
	{SDLK_8, "8"},
	{SDLK_9, "9"},
	{SDLK_a, "A"},
	{SDLK_b, "B"},
	{SDLK_c, "C"},
	{SDLK_d, "D"},
	{SDLK_e, "E"},
	{SDLK_f, "F"},
	{SDLK_g, "G"},
	{SDLK_h, "H"},
	{SDLK_i, "I"},
	{SDLK_k, "J"},
	{SDLK_k, "K"},
	{SDLK_l, "L"},
	{SDLK_m, "M"},
	{SDLK_n, "N"},
	{SDLK_o, "O"},
	{SDLK_p, "P"},
	{SDLK_q, "Q"},
	{SDLK_r, "R"},
	{SDLK_s, "S"},
	{SDLK_t, "T"},
	{SDLK_u, "U"},
	{SDLK_v, "V"},
	{SDLK_w, "W"},
	{SDLK_x, "X"},
	{SDLK_y, "Y"},
	{SDLK_z, "Z"},
	{SDLK_a, "a"},
	{SDLK_b, "b"},
	{SDLK_c, "c"},
	{SDLK_d, "d"},
	{SDLK_e, "e"},
	{SDLK_f, "f"},
	{SDLK_g, "g"},
	{SDLK_h, "h"},
	{SDLK_i, "i"},
	{SDLK_k, "j"},
	{SDLK_k, "k"},
	{SDLK_l, "l"},
	{SDLK_m, "m"},
	{SDLK_n, "n"},
	{SDLK_o, "o"},
	{SDLK_p, "p"},
	{SDLK_q, "q"},
	{SDLK_r, "r"},
	{SDLK_s, "s"},
	{SDLK_t, "t"},
	{SDLK_u, "u"},
	{SDLK_v, "v"},
	{SDLK_w, "w"},
	{SDLK_x, "x"},
	{SDLK_y, "y"},
	{SDLK_z, "z"},
	{SDLK_KP0, "KP_0"},
	{SDLK_KP1, "KP_1"},
	{SDLK_KP2, "KP_2"},
	{SDLK_KP3, "KP_3"},
	{SDLK_KP4, "KP_4"},
	{SDLK_KP5, "KP_5"},
	{SDLK_KP6, "KP_6"},
	{SDLK_KP7, "KP_7"},
	{SDLK_KP8, "KP_8"},
	{SDLK_KP9, "KP_9"},
	{SDLK_KP_MULTIPLY, "KP_Multiply"},
	{SDLK_KP_PLUS, "KP_Add"},
	{SDLK_KP_MINUS, "KP_Subtract"},
	{SDLK_KP_PERIOD, "KP_Decimal"},
	{SDLK_KP_DIVIDE, "KP_Divide"},
	{SDLK_F1, "F1"},
	{SDLK_F2, "F2"},
	{SDLK_F3, "F3"},
	{SDLK_F4, "F4"},
	{SDLK_F5, "F5"},
	{SDLK_F6, "F6"},
	{SDLK_F7, "F7"},
	{SDLK_F8, "F8"},
	{SDLK_F9, "F9"},
	{SDLK_F10, "F10"},
	{SDLK_F11, "F11"},
	{SDLK_F12, "F12"},
	{SDLK_F13, "F13"},
	{SDLK_F14, "F14"},
	{SDLK_F15, "F15"},
	/* terminator */
	{0, NULL},
};

/*
 * compare by code
 */
static int
CompareByCode (const void *a, const void *b)
{
	return ((const AtomCodeTable *) a)->code - ((const AtomCodeTable *) b)->code;
}								/* CompareByCode */

/*
 * compare by atom
 */
static int
CompareByAtom (const void *a, const void *b)
{
	return ((const AtomCodeTable *) a)->atom - ((const AtomCodeTable *) b)->atom;
}								/* CompareByAtom */

/*
 * init keysyms
 */
XBBool
InitKeysym (void)
{
	int i;

	/* count keys */
	for (numKeys = 0; keyTable[numKeys].name != NULL; numKeys++)
		continue;
	/* alloc tables */
	atomTable = calloc (numKeys, sizeof (AtomCodeTable));
	assert (NULL != atomTable);
	codeTable = calloc (numKeys, sizeof (AtomCodeTable));
	assert (NULL != codeTable);
	/* fill tables */
	for (i = 0; i < numKeys; i++) {
		XBAtom atom = GUI_StringToAtom (keyTable[i].name);
		atomTable[i].code = codeTable[i].code = keyTable[i].code;
		atomTable[i].atom = codeTable[i].atom = atom;
	}
	/* sort tables */
	qsort (atomTable, numKeys, sizeof (AtomCodeTable), CompareByAtom);
	qsort (codeTable, numKeys, sizeof (AtomCodeTable), CompareByCode);
	/* that's all */
	return XBTrue;
}								/* InitKeysym */

/*
 *
 */
void
FinishKeysym (void)
{
	if (NULL != atomTable) {
		free (atomTable);
		atomTable = NULL;
	}
	if (NULL != codeTable) {
		free (codeTable);
		codeTable = NULL;
	}
}								/* FinishKeysym */

/*
 * convert string to keycode
 */
SDLKey
StringToVirtualKey (const char *name)
{
	AtomCodeTable key;
	AtomCodeTable *result;
	XBAtom atom;

	atom = GUI_StringToAtom (name);
	key.atom = atom;
	result = bsearch (&key, atomTable, numKeys, sizeof (AtomCodeTable), CompareByAtom);
	if (NULL == result) {
		return SDLK_UNKNOWN;
	}
	return result->code;
}								/* StringToVirtualKey */

/*
 * convert keycode to string
 */
XBAtom
VirtualKeyToAtom (SDLKey code)
{
	AtomCodeTable key;
	AtomCodeTable *result;
	XBAtom atom;

	key.code = code;
	result = bsearch (&key, codeTable, numKeys, sizeof (AtomCodeTable), CompareByCode);
	if (NULL == result) {
		return ATOM_INVALID;
	}
	atom = result->atom;
	return atom;
}								/* StringToVirtualKey */

/*
 * end of file w32_keysym.c
 */
