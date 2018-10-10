/*
 * file w32_keysym.c - create X11 like keysmbol names for virtual keys
 * 
 * $Id: w32_keysym.c,v 1.2 2004/05/14 10:00:35 alfie Exp $
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
#include "w32_keysym.h"
#include "gui.h"

/*
 * local types
 */
typedef struct {
  UINT        code;
  const char *name;
} KeyNameTable;

typedef struct {
  ATOM atom;
  UINT code;
} AtomCodeTable;

/*
 * local variables
 */
static int numKeys              = 0;
static AtomCodeTable *atomTable = NULL;
static AtomCodeTable *codeTable = NULL;
static KeyNameTable keyTable[]  = {
  { VK_BACK,      "BackSpace" },
  { VK_TAB,       "Tab" },
  { VK_RETURN,    "Return" },
  { VK_SHIFT,     "Shift" },
  { VK_CONTROL,   "Control" },
  { VK_PAUSE,     "Pause" },
  { VK_CAPITAL,   "Caps_Lock" },
  { VK_ESCAPE,    "Escape" },
  { VK_SPACE,     "space" },
  { VK_PRIOR,     "Prior" },
  { VK_NEXT,      "Next" },
  { VK_END,       "End" },
  { VK_HOME,      "Home" },
  { VK_LEFT,      "Left" },
  { VK_UP,        "Up" },
  { VK_RIGHT,     "Right" },
  { VK_DOWN,      "Down" },
  { VK_PRINT,     "Print" },
  { VK_INSERT,    "Insert" },
  { VK_DELETE,    "Delete" },
  { '0', 	  "0" },
  { '1', 	  "1" },
  { '2', 	  "2" },
  { '3', 	  "3" },
  { '4', 	  "4" },
  { '5', 	  "5" },
  { '6', 	  "6" },
  { '7', 	  "7" },
  { '8', 	  "8" },
  { '9', 	  "9" },
  { 'A', 	  "A" },
  { 'B', 	  "B" },
  { 'C', 	  "C" },
  { 'D', 	  "D" },
  { 'E', 	  "E" },
  { 'F', 	  "F" },
  { 'G', 	  "G" },
  { 'H', 	  "H" },
  { 'I', 	  "I" },
  { 'J', 	  "J" },
  { 'K', 	  "K" },
  { 'L', 	  "L" },
  { 'M', 	  "M" },
  { 'N', 	  "N" },
  { 'O', 	  "O" },
  { 'P', 	  "P" },
  { 'Q', 	  "Q" },
  { 'R', 	  "R" },
  { 'S', 	  "S" },
  { 'T', 	  "T" },
  { 'U', 	  "U" },
  { 'V', 	  "V" },
  { 'W', 	  "W" },
  { 'X', 	  "X" },
  { 'Y', 	  "Y" },
  { 'Z', 	  "Z" },
  { VK_NUMPAD0,   "KP_0" },
  { VK_NUMPAD1,   "KP_1" },
  { VK_NUMPAD2,   "KP_2" },
  { VK_NUMPAD3,   "KP_3" },
  { VK_NUMPAD4,   "KP_4" },
  { VK_NUMPAD5,   "KP_5" },
  { VK_NUMPAD6,   "KP_6" },
  { VK_NUMPAD7,   "KP_7" },
  { VK_NUMPAD8,   "KP_8" },
  { VK_NUMPAD9,   "KP_9" },
  { VK_MULTIPLY,  "KP_Multiply" },
  { VK_ADD,       "KP_Add" },
  { VK_SUBTRACT,  "KP_Subtract" },
  { VK_DECIMAL,   "KP_Decimal" },
  { VK_DIVIDE,    "KP_Divide" },
  { VK_F1,        "F1" },
  { VK_F2,        "F2" },
  { VK_F3,        "F3" },
  { VK_F4,        "F4" },
  { VK_F5,        "F5" },
  { VK_F6,        "F6" },
  { VK_F7,        "F7" },
  { VK_F8,        "F8" },
  { VK_F9,        "F9" },
  { VK_F10,       "F10" },
  { VK_F11,       "F11" },
  { VK_F12,       "F12" },
  { VK_F13,       "F13" },
  { VK_F14,       "F14" },
  { VK_F15,       "F15" },
  { VK_F16,       "F16" },
  { VK_F17,       "F17" },
  { VK_F18,       "F18" },
  { VK_F19,       "F19" },
  { VK_F20,       "F20" },
  { VK_F21,       "F21" },
  { VK_F22,       "F22" },
  { VK_F23,       "F23" },
  { VK_F24,       "F24" },
  /* terminator */
  { 0,            NULL },
};

/*
 * compare by code
 */
static int
CompareByCode (const void *a, const void *b)
{
  return ((AtomCodeTable *) a)->code - ((AtomCodeTable *) b)->code;
} /* CompareByCode */

/*
 * compare by atom
 */
static int
CompareByAtom (const void *a, const void *b)
{
  return ((AtomCodeTable *) a)->atom - ((AtomCodeTable *) b)->atom;
} /* CompareByAtom */

/*
 * init keysyms
 */
XBBool
InitKeysym (void)
{
  int i;

  /* count keys */
  for (numKeys = 0; keyTable[numKeys].name != NULL; numKeys ++) continue;
  /* alloc tables */
  atomTable = calloc (numKeys, sizeof (AtomCodeTable));
  assert (NULL != atomTable);
  codeTable = calloc (numKeys, sizeof (AtomCodeTable));
  assert (NULL != codeTable);
  /* fill tables */
  for (i = 0; i < numKeys; i ++) {
    XBAtom atom = GUI_StringToAtom (keyTable[i].name);
    atomTable[i].code = codeTable[i].code = keyTable[i].code;
    atomTable[i].atom = codeTable[i].atom = atom;
  }
  /* sort tables */
  qsort (atomTable, numKeys, sizeof (AtomCodeTable), CompareByAtom);
  qsort (codeTable, numKeys, sizeof (AtomCodeTable), CompareByCode);
  /* that's all */
  return XBTrue;
} /* InitKeysym */

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
} /* FinishKeysym */

/*
 * convert string to keycode
 */ 
UINT
StringToVirtualKey (const char *name)
{
  AtomCodeTable key;
  AtomCodeTable *result;
  XBAtom atom;
  
  atom     = GUI_StringToAtom (name);
  key.atom = atom;
  result   = bsearch (&key, atomTable, numKeys, sizeof (AtomCodeTable), CompareByAtom);
  if (NULL == result) {
    return 0;
  }
  return result->code;
} /* StringToVirtualKey */

/*
 * convert keycode to string
 */ 
XBAtom
VirtualKeyToAtom (UINT code)
{
  AtomCodeTable key;
  AtomCodeTable *result;
  XBAtom atom;

  key.code = code;
  result   = bsearch (&key, codeTable, numKeys, sizeof (AtomCodeTable), CompareByCode);
  if (NULL == result) {
    return ATOM_INVALID;
  }
  atom = result->atom;
  return atom;
} /* StringToVirtualKey */

/*
 * end of file w32_keysym.c
 */
