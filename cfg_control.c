/*
 * cfg_control.c - keyboard and joystick configuration data
 * 
 * $Id: cfg_control.c,v 1.15 2006/03/28 11:41:19 fzago Exp $
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

/*
 * local macros
 */
#define NUM_GAME_KEYS     (10+10)	//chat
#define NUM_XBLAST_KEYS    3

/*
 * local variables
 */
static DBRoot *dbControl = NULL;

/* conversion control to event type */
const XBEventCode keyEventType[NUM_KEYB_CONTROLS] = {
	XBE_KEYB_1, XBE_KEYB_2,
};

/* menu key init table */
static CFGKeyTable menuKeyTable[] = {
	{"Tab", XBE_MENU, XBMK_NEXT},
	{"BackSpace", XBE_MENU, XBMK_PREV},
	{"Up", XBE_MENU, XBMK_UP},
	{"Down", XBE_MENU, XBMK_DOWN},
	{"Left", XBE_MENU, XBMK_LEFT},
	{"Right", XBE_MENU, XBMK_RIGHT},
	{"space", XBE_MENU, XBMK_SELECT},
	{"Return", XBE_MENU, XBMK_DEFAULT},
	{"Escape", XBE_MENU, XBMK_ABORT},
	/* end of data */
	{NULL, XBE_NONE, XBMK_NONE},
};

/* game control keys */
static CFGKeyTable xblastKeyTable[] = {
	{"Escape", XBE_XBLAST, XBXK_EXIT},
	{NULL, XBE_NONE, XBXK_NONE},
};

/* fixed chat control keys */
static CFGKeyTable chatKeyTable[] = {
	{"Escape", XBE_CHAT, XBCE_ESCAPE,},
	{"BackSpace", XBE_CHAT, XBCE_BACK},
	{"Return", XBE_CHAT, XBCE_ENTER},
	{NULL, XBE_NONE, XBCE_NONE},
};

/*
 * set default config keyboard config (left side)
 */
static void
SetDefaultLeftKeyboard (CFGControlKeyboard * cfg)
{
	assert (NULL != cfg);

	cfg->keyUp = GUI_StringToAtom ("T");
	cfg->keyLeft = GUI_StringToAtom ("F");
	cfg->keyDown = GUI_StringToAtom ("B");
	cfg->keyRight = GUI_StringToAtom ("H");
	cfg->keyStop = GUI_StringToAtom ("G");
	cfg->keyBomb = GUI_StringToAtom ("space");
	cfg->keySpecial = GUI_StringToAtom ("Tab");
	cfg->keyPause = GUI_StringToAtom ("P");
	cfg->keyAbort = GUI_StringToAtom ("A");
	cfg->keyAbortCancel = GUI_StringToAtom ("Z");
	/* Skywalker */
	cfg->keyLaola = GUI_StringToAtom ("1");
	cfg->keyLooser = GUI_StringToAtom ("2");
	cfg->keyBot = GUI_StringToAtom ("3");
	cfg->keyChatStart = GUI_StringToAtom ("4");
	cfg->keyChatSend = GUI_StringToAtom ("5");
	cfg->keyChatCancel = GUI_StringToAtom ("6");
	cfg->keyChatChangeReceiver = GUI_StringToAtom ("7");
	/* */
}								/* SetDefaultLeftKeyboard */

/*
 * set default config keyboard config (left side)
 */
static void
SetDefaultRightKeyboard (CFGControlKeyboard * cfg)
{
	assert (NULL != cfg);

	cfg->keyUp = GUI_StringToAtom ("KP_8");
	cfg->keyLeft = GUI_StringToAtom ("KP_4");
	cfg->keyDown = GUI_StringToAtom ("KP_2");
	cfg->keyRight = GUI_StringToAtom ("KP_6");
	cfg->keyStop = GUI_StringToAtom ("KP_5");
	cfg->keyBomb = GUI_StringToAtom ("KP_0");
	cfg->keySpecial = GUI_StringToAtom ("Return");
	cfg->keyPause = GUI_StringToAtom ("KP_Subtract");
	cfg->keyAbort = GUI_StringToAtom ("KP_Multiply");
	cfg->keyAbortCancel = GUI_StringToAtom ("KP_Divide");
	/* Skywalker */
	cfg->keyLaola = GUI_StringToAtom ("F3");
	cfg->keyLooser = GUI_StringToAtom ("F4");
	cfg->keyBot = GUI_StringToAtom ("F5");
	cfg->keyChatStart = GUI_StringToAtom ("F6");
	cfg->keyChatSend = GUI_StringToAtom ("F7");
	cfg->keyChatCancel = GUI_StringToAtom ("F8");
	cfg->keyChatChangeReceiver = GUI_StringToAtom ("F9");
	/* */
}								/* SetDefaultLeftKeyboard */

/*
 * get atom for control type
 */
static XBAtom
AtomType (XBEventCode type)
{
	switch (type) {
	case XBE_KEYB_1:
		return atomRightKeyboard;
	case XBE_KEYB_2:
		return atomLeftKeyboard;
	default:
		return ATOM_INVALID;
	}
}								/* AtomType */

/*
 * store single key entry
 */
static void
StoreEntry (DBSection * section, XBAtom atomEntry, XBAtom atomValue)
{
	if (ATOM_INVALID != atomValue) {
		DB_CreateEntryString (section, atomEntry, GUI_AtomToString (atomValue));
	}
	else {
		DB_DeleteEntry (section, atomEntry);
	}
}								/* StoreEntry */

/*
 * store control config
 */
void
StoreControlKeyboard (XBEventCode type, const CFGControlKeyboard * cfg)
{
	XBAtom atom;
	DBSection *section;

	/* determine section */
	atom = AtomType (type);
	if (ATOM_INVALID == atom) {
		return;
	}
	/* create section */
	section = DB_CreateSection (dbControl, atom);
	assert (section != NULL);
	/* store entries */
	StoreEntry (section, atomKeyUp, cfg->keyUp);
	StoreEntry (section, atomKeyLeft, cfg->keyLeft);
	StoreEntry (section, atomKeyDown, cfg->keyDown);
	StoreEntry (section, atomKeyRight, cfg->keyRight);
	StoreEntry (section, atomKeyStop, cfg->keyStop);
	StoreEntry (section, atomKeyBomb, cfg->keyBomb);
	StoreEntry (section, atomKeySpecial, cfg->keySpecial);
	StoreEntry (section, atomKeyPause, cfg->keyPause);
	StoreEntry (section, atomKeyAbort, cfg->keyAbort);
	StoreEntry (section, atomKeyAbortCancel, cfg->keyAbortCancel);
	/* Skywalker */
	StoreEntry (section, atomKeyLaola, cfg->keyLaola);
	StoreEntry (section, atomKeyLooser, cfg->keyLooser);
	StoreEntry (section, atomKeyBot, cfg->keyBot);
	StoreEntry (section, atomKeyChatSend, cfg->keyChatSend);
	StoreEntry (section, atomKeyChatStart, cfg->keyChatStart);
	StoreEntry (section, atomKeyChatCancel, cfg->keyChatCancel);
	StoreEntry (section, atomKeyChatChangeReceiver, cfg->keyChatChangeReceiver);
	/* */
}								/* StoreControlKeyboard */

/*
 * get single key entry
 */
static void
RetrieveEntry (const DBSection * section, XBAtom atomEntry, XBAtom * atomValue)
{
	const char *s;

	assert (atomValue != NULL);
	if (DB_GetEntryString (section, atomEntry, &s)) {
		*atomValue = GUI_StringToAtom (s);
	}
	else {
		*atomValue = ATOM_INVALID;
	}
}								/* RetrieveEntry */

/*
 * get control config from database
 */
XBBool
RetrieveControlKeyboard (XBEventCode type, CFGControlKeyboard * cfg)
{
	XBAtom atom;
	const DBSection *section;

	/* determine section */
	atom = AtomType (type);
	if (ATOM_INVALID == atom) {
		return XBFalse;
	}
	/* get section */
	section = DB_GetSection (dbControl, atom);
	if (NULL == section) {
		return XBFalse;
	}
	/* get values */
	RetrieveEntry (section, atomKeyUp, &cfg->keyUp);
	RetrieveEntry (section, atomKeyLeft, &cfg->keyLeft);
	RetrieveEntry (section, atomKeyDown, &cfg->keyDown);
	RetrieveEntry (section, atomKeyRight, &cfg->keyRight);
	RetrieveEntry (section, atomKeyStop, &cfg->keyStop);
	RetrieveEntry (section, atomKeyBomb, &cfg->keyBomb);
	RetrieveEntry (section, atomKeySpecial, &cfg->keySpecial);
	RetrieveEntry (section, atomKeyPause, &cfg->keyPause);
	RetrieveEntry (section, atomKeyAbort, &cfg->keyAbort);
	RetrieveEntry (section, atomKeyAbortCancel, &cfg->keyAbortCancel);
	/* Skywalker */
	RetrieveEntry (section, atomKeyLaola, &cfg->keyLaola);
	RetrieveEntry (section, atomKeyLooser, &cfg->keyLooser);
	RetrieveEntry (section, atomKeyBot, &cfg->keyBot);
	RetrieveEntry (section, atomKeyChatSend, &cfg->keyChatSend);
	RetrieveEntry (section, atomKeyChatStart, &cfg->keyChatStart);
	RetrieveEntry (section, atomKeyChatCancel, &cfg->keyChatCancel);
	RetrieveEntry (section, atomKeyChatChangeReceiver, &cfg->keyChatChangeReceiver);
	/* */
	return XBTrue;
}								/* RetrieveControlKeyboard */

/*
 * int and load controls config
 */
void
LoadControlConfig (void)
{
	CFGControlKeyboard cfgKeyboard;
	/* create empty database */
	dbControl = DB_Create (DT_Config, atomControl);
	assert (dbControl != NULL);
	/* load from file */
	Dbg_Config ("loading control configs\n");
	if (DB_Load (dbControl)) {
		return;
	}
	Dbg_Config ("failed to load control configs, settting defaults\n");
	/* set default values */
	SetDefaultRightKeyboard (&cfgKeyboard);
	StoreControlKeyboard (XBE_KEYB_1, &cfgKeyboard);
	SetDefaultLeftKeyboard (&cfgKeyboard);
	StoreControlKeyboard (XBE_KEYB_2, &cfgKeyboard);
	/* store it */
	DB_Store (dbControl);
}								/* LoadControlConfig */

/*
 * save control config
 */
void
SaveControlConfig (void)
{
	assert (dbControl != NULL);
	if (DB_Changed (dbControl)) {
		Dbg_Config ("storing control configs\n");
		DB_Store (dbControl);
	}
}								/* SaveControlConfig  */

/*
 * finish control config
 */
void
FinishControlConfig (void)
{
	DB_Delete (dbControl);
	dbControl = NULL;
	Dbg_Config ("control config cleared\n");
}								/* FinishControlConfig  */

/*
 * put entry into key table
 */
static int
PutEntry (CFGKeyTable * table, XBEventCode type, int key, const DBSection * section, XBAtom atom)
{
	const char *s;

	assert (table != NULL);
	if (DB_GetEntryString (section, atom, &s)) {
		table->keysym = s;
		table->eventCode = type;
		table->eventData = key;
		return 1;
	}
	return 0;
}								/* PutEntry */

/*
 * get key control table for gui
 */
const CFGKeyTable *
GetGameKeyPressTable (void)
{
	int i, num;
	XBEventCode type;
	const DBSection *section;
	static CFGKeyTable keyTable[2 * NUM_GAME_KEYS + NUM_XBLAST_KEYS + 1];

	/* clear old table */
	memset (keyTable, 0, sizeof (keyTable));
	/* game control keys */
	num = 0;
	for (i = 0; i < NUM_KEYB_CONTROLS; i++) {
		type = keyEventType[i];
		section = DB_GetSection (dbControl, AtomType (type));
		if (NULL != section) {
			num += PutEntry (keyTable + num, type, XBGK_GO_UP, section, atomKeyUp);
			num += PutEntry (keyTable + num, type, XBGK_GO_LEFT, section, atomKeyLeft);
			num += PutEntry (keyTable + num, type, XBGK_GO_DOWN, section, atomKeyDown);
			num += PutEntry (keyTable + num, type, XBGK_GO_RIGHT, section, atomKeyRight);
			num += PutEntry (keyTable + num, type, XBGK_STOP_ALL, section, atomKeyStop);
			num += PutEntry (keyTable + num, type, XBGK_BOMB, section, atomKeyBomb);
			num += PutEntry (keyTable + num, type, XBGK_SPECIAL, section, atomKeySpecial);
			num += PutEntry (keyTable + num, type, XBGK_PAUSE, section, atomKeyPause);
			num += PutEntry (keyTable + num, type, XBGK_ABORT, section, atomKeyAbort);
			num += PutEntry (keyTable + num, type, XBGK_ABORT_CANCEL, section, atomKeyAbortCancel);
/* Skywalker */
			num += PutEntry (keyTable + num, type, XBGK_LAOLA, section, atomKeyLaola);
			num += PutEntry (keyTable + num, type, XBGK_LOOSER, section, atomKeyLooser);
			num += PutEntry (keyTable + num, type, XBGK_BOT, section, atomKeyBot);
			/* */
		}
	}
	/* game control keys */
	memcpy (keyTable + num, xblastKeyTable, sizeof (xblastKeyTable));
	/* that's all */
	return keyTable;
}								/* GetGameKeyPressTable */

/*
 * get key control table for chat
 */
const CFGKeyTable *
GetChatKeyTable (void)
{
	int i, num;
	XBEventCode type;
	const DBSection *section;
	static CFGKeyTable keyTable[2 * NUM_GAME_KEYS + NUM_XBLAST_KEYS + 1];

	/* clear old table */
	memset (keyTable, 0, sizeof (keyTable));
	/* game control keys */
	num = 0;
	for (i = 0; i < NUM_KEYB_CONTROLS; i++) {
		type = keyEventType[i];
		section = DB_GetSection (dbControl, AtomType (type));
		if (NULL != section) {
			num += PutEntry (keyTable + num, type, XBCE_SEND, section, atomKeyChatSend);
			num += PutEntry (keyTable + num, type, XBCE_START, section, atomKeyChatStart);
			num += PutEntry (keyTable + num, type, XBCE_CANCEL, section, atomKeyChatCancel);
			num += PutEntry (keyTable + num, type, XBCE_CHANGE, section, atomKeyChatChangeReceiver);
		}
	}
	/* fixed chat keys */
	memcpy (keyTable + num, chatKeyTable, sizeof (chatKeyTable));
	/* that's all */
	return keyTable;
}								/* GetGameKeyPressTable */

/*
 * get key control table for gui
 */
const CFGKeyTable *
GetGameKeyReleaseTable (void)
{
	int i, num;
	XBEventCode type;
	const DBSection *section;
	static CFGKeyTable keyTable[2 * NUM_GAME_KEYS + 1];

	/* clear old table */
	memset (keyTable, 0, sizeof (keyTable));
	/* build new one */
	num = 0;
	for (i = 0; i < NUM_KEYB_CONTROLS; i++) {
		type = keyEventType[i];
		section = DB_GetSection (dbControl, AtomType (type));
		if (NULL != section) {
			num += PutEntry (keyTable + num, type, XBGK_STOP_UP, section, atomKeyUp);
			num += PutEntry (keyTable + num, type, XBGK_STOP_LEFT, section, atomKeyLeft);
			num += PutEntry (keyTable + num, type, XBGK_STOP_DOWN, section, atomKeyDown);
			num += PutEntry (keyTable + num, type, XBGK_STOP_RIGHT, section, atomKeyRight);
		}
	}
	/* that's all */
	return keyTable;
}								/* GetGameKeyReleaseTable */

/*
 * get menu key table for gui
 */
const CFGKeyTable *
GetMenuKeyTable (void)
{
	return menuKeyTable;
}								/* GetMenuKeyTable */

/*
 * end cfg_control.c
 */
