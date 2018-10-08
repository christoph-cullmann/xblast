/*
 * file mi_stat.c - menu item for displaying game table
 *
 * $Id: mi_stat.c,v 1.10 2006/02/13 21:34:18 fzago Exp $
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

/* text sprite flags */
#define FF_TABLE_FOCUS     (FF_Small | FF_Black | FF_Outlined )
#define FF_TABLE_NO_FOCUS  (FF_Small | FF_White )
#define FF_TABLE_INACTIVE  (FF_Small | FF_Black )

/*
 * local types
 */
typedef struct
{
	char *text;					/* text to be displayed */
	Sprite *sprite;				/* sprite for it */
	unsigned align;				/* text alignment */
} TableCell;

typedef struct _xb_menu_table_item XBMenuTableItem;

typedef void (*TablePollFunc) (XBMenuTableItem *, const void *);

struct _xb_menu_table_item
{
	XBMenuItem item;
	size_t numCells;
	TableCell *cell;
	MIC_button func;
	void *funcData;
	TablePollFunc pollFunc;
	const void **pollPtr;
	const void *pollData;
};

/*
 * determine current text flags
 */
static void
SetTextFlags (XBMenuTableItem * table)
{
	size_t i;
	unsigned flags;
	/* sanity check */
	assert (NULL != table);
	assert (NULL != table->cell);
	/* get new flags */
	if (table->item.flags & MIF_FOCUS) {
		flags = FF_TABLE_FOCUS;
	}
	else if (table->item.flags & MIF_DEACTIVATED) {
		flags = FF_TABLE_INACTIVE;
	}
	else {
		flags = FF_TABLE_NO_FOCUS;
	}
	/* change in all cells */
	for (i = 0; i < table->numCells; i++) {
		assert (table->cell[i].sprite != NULL);
		SetSpriteAnime (table->cell[i].sprite, table->cell[i].align | flags);
	}
}								/* SetTextFlags */

/*
 * table has changed activation
 */
void
MenuActivateTable (XBMenuItem * ptr, XBBool flag)
{
	SetTextFlags ((XBMenuTableItem *) ptr);
}								/* MenuActivateTable */

/*
 * a horizontal table receives the focus
 */
static void
MenuTableFocus (XBMenuItem * ptr, XBBool flag)
{
	SetTextFlags ((XBMenuTableItem *) ptr);
}								/* MenuHTableFocus */

/*
 * a table is selected
 */
static void
MenuTableSelect (XBMenuItem * ptr)
{
	XBMenuTableItem *table = (XBMenuTableItem *) ptr;
	assert (ptr != NULL);
	MenuExecFunc (table->func, table->funcData);
}								/* MenuTableSelect */

/*
 * table was selected by mouse
 */
static void
MenuTableMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (code == XBE_MOUSE_1) {
		MenuTableSelect (ptr);
	}
}								/* MenuTableMouse */

/*
 * poll function
 */
static void
MenuTablePoll (XBMenuItem * ptr)
{
	XBMenuTableItem *table = (XBMenuTableItem *) ptr;
	/* check if poll data has changed */
	if (table->pollPtr != NULL && *table->pollPtr != table->pollData) {
		assert (table->pollFunc != NULL);
		table->pollData = *table->pollPtr;
		(*table->pollFunc) (table, table->pollData);
	}
}								/* MenuTablePoll */

/*
 * genric constructor for table item
 */
static XBMenuTableItem *
CreateTableItem (int x, int y, int w, MIC_button func, void *funcData, TablePollFunc pollFunc,
				 const void **pollPtr, int num)
{
	/* create item */
	XBMenuTableItem *table = calloc (1, sizeof (XBMenuTableItem));
	assert (table != NULL);
	MenuSetItem (&table->item, MIT_Table, x, y, w, CELL_H, MenuTableFocus, MenuTableSelect,
				 MenuTableMouse, (NULL != pollFunc) ? MenuTablePoll : NULL);
	/* cell sprites */
	table->numCells = num;
	table->cell = calloc (num, sizeof (TableCell));
	assert (NULL != table->cell);
	/* callback */
	table->func = func;
	table->funcData = funcData;
	/* polling */
	table->pollFunc = pollFunc;
	table->pollPtr = pollPtr;
	table->pollData = NULL;
	/* graphics */
	MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	/* that's all */
	return table;
}								/* CreateTableItem */

/*
 * update table cell
 */
static void
UpdateTableCell (XBMenuTableItem * table, size_t cell, const char *fmt, ...)
{
	va_list argList;
	char tmp[512];
	/* sanity checks */
	assert (NULL != table);
	assert (NULL != table->cell);
	assert (cell < table->numCells);
	assert (NULL != table->cell[cell].text);
	assert (NULL != table->cell[cell].sprite);
	assert (NULL != fmt);
	/* format string */
	va_start (argList, fmt);
	vsprintf (tmp, fmt, argList);
	va_end (argList);
	/* set values */
	free (table->cell[cell].text);
	table->cell[cell].text = DupString (tmp);
	assert (NULL != table->cell[cell].text);
	SetSpriteText (table->cell[cell].sprite, table->cell[cell].text);
}								/* UpdateTableCell */

/*
 * set table cell
 */
static void
SetTableCell (XBMenuTableItem * table, size_t cell, unsigned align, int x, int w, const char *fmt,
			  ...)
{
	va_list argList;
	char tmp[256];

	/* sanity checks */
	assert (NULL != table);
	assert (NULL != table->cell);
	assert (cell < table->numCells);
	assert (NULL != fmt);
	assert (0 == (align & ~(FM_Align | FM_Boxed)));
	/* format string */
	va_start (argList, fmt);
	vsprintf (tmp, fmt, argList);
	va_end (argList);
	/* set values */
	table->cell[cell].text = DupString (tmp);
	table->cell[cell].align = align;
	table->cell[cell].sprite =
		CreateTextSprite (table->cell[cell].text, BASE_X * (table->item.x + x),
						  BASE_Y * table->item.y, BASE_X * w, (CELL_H / 2) * BASE_Y,
						  FF_TABLE_NO_FOCUS | align, SPM_MAPPED);
	/* checks */
	assert (NULL != table->cell[cell].text);
	assert (NULL != table->cell[cell].sprite);
}								/* SetTableCell */

/*
 * poll routine for game entries
 */
static void
PollGameEntry (XBMenuTableItem * table, const void *ptr)
{
	const XBNetworkGame *game = ptr;

	/* update cell texts */
	if (NULL != game) {
		UpdateTableCell (table, 0, "%s", game->game);
		UpdateTableCell (table, 1, "%s:%hu", game->host, game->port);
		UpdateTableCell (table, 2, "%d ms", game->ping);
		UpdateTableCell (table, 3, "%s", game->version);
		UpdateTableCell (table, 4, "%d", game->numLives);
		UpdateTableCell (table, 5, "%d", game->numWins);
		UpdateTableCell (table, 6, "%d", game->frameRate);
	}
	else {
		size_t i;
		for (i = 0; i < table->numCells; i++) {
			UpdateTableCell (table, i, "");
		}
	}
}								/* PollGameEntry */

/*
 * create a score stat entry
 */
XBMenuItem *
MenuCreateStatEntry (int x, int y, int w, const XBStatData * data, MIC_button func, void *funcData)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, func, funcData, NULL, NULL, 6);
	/* set cells ... */
	assert (data != NULL);
	if (data->name != NULL) {
		SetTableCell (table, 0, FF_Left, 1, (w - 3 * CELL_W - 2), data->name);
	}
	else {
		SetTableCell (table, 0, FF_Left, 1, (w - 3 * CELL_W - 2), GUI_AtomToString (data->atom));
	}
	SetTableCell (table, 1, FF_Right, w - 1 - 12 * CELL_W / 2, CELL_W, "%6.1f", data->scoreTotal);
	SetTableCell (table, 2, FF_Right, w - 1 - 10 * CELL_W / 2, 3 * CELL_W / 2, "%6.3f",
				  data->average);
	SetTableCell (table, 3, FF_Right, w - 1 - 7 * CELL_W / 2, CELL_W, "%d", data->numWon);
	SetTableCell (table, 4, FF_Right, w - 1 - 5 * CELL_W / 2, 3 * CELL_W / 2, "%5.2f%%",
				  data->percent);
	SetTableCell (table, 5, FF_Right, w - 1 - 2 * CELL_W / 2, CELL_W, "%d", data->numTotal);
	/* --- */
	return &table->item;
}								/* MenuCreateStatEntry */

/*
 * create header for stat entry table
 */
XBMenuItem *
MenuCreateStatHeader (int x, int y, int w, const char *title)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, NULL, NULL, NULL, NULL, 6);
	/* set cells ... */
	SetTableCell (table, 0, FF_Boxed | FF_Left, 1, w - 6 * CELL_W - 2, title);
	SetTableCell (table, 1, FF_Boxed | FF_Right, w - 1 - 12 * CELL_W / 2, CELL_W, N_("Score"));
	SetTableCell (table, 2, FF_Boxed | FF_Right, w - 1 - 10 * CELL_W / 2, 3 * CELL_W / 2,
				  N_("Avg. Score"));
	SetTableCell (table, 3, FF_Boxed | FF_Right, w - 1 - 7 * CELL_W / 2, CELL_W, N_("# Won"));
	SetTableCell (table, 4, FF_Boxed | FF_Right, w - 1 - 5 * CELL_W / 2, 3 * CELL_W / 2, N_("%% Won"));
	SetTableCell (table, 5, FF_Boxed | FF_Right, w - 1 - 2 * CELL_W / 2, CELL_W, N_("# Total"));
	/* --- */
	return &table->item;
}								/* MenuCreateStatHeader */

/*
 * create entry for rankings table
 */
XBMenuItem *
MenuCreateCentralEntry (int x, int y, int w, const XBCentralData * data, MIC_button func,
						void *funcData)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, func, funcData, NULL, NULL, 6);
	/* set cells ... */
	assert (data != NULL);
	if (data->name != NULL) {
		SetTableCell (table, 0, FF_Left, 1, (w - 3 * CELL_W - 2), data->name);
	}
	else {
		SetTableCell (table, 0, FF_Left, 1, (w - 3 * CELL_W - 2), GUI_AtomToString (data->atom));
	}
	SetTableCell (table, 1, FF_Right, w - 1 - 12 * CELL_W / 2, 3 * CELL_W / 2, "%6.1f",
				  data->score);
	SetTableCell (table, 2, FF_Right, w - 1 - 9 * CELL_W / 2, CELL_W, "%d", data->rank);
	SetTableCell (table, 3, FF_Right, w - 1 - 7 * CELL_W / 2, CELL_W, "%d", data->numWon);
	SetTableCell (table, 4, FF_Right, w - 1 - 5 * CELL_W / 2, 3 * CELL_W / 2, "%5.2f%%",
				  data->percent);
	SetTableCell (table, 5, FF_Right, w - 1 - 2 * CELL_W / 2, CELL_W, "%d", data->numTotal);
	/* --- */
	return &table->item;
}								/* MenuCreateCentralEntry */

/*
 * create header for rankings table
 */
XBMenuItem *
MenuCreateCentralHeader (int x, int y, int w, const char *title)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, NULL, NULL, NULL, NULL, 6);
	/* set cells ... */
	SetTableCell (table, 0, FF_Boxed | FF_Left, 1, w - 6 * CELL_W - 2, title);
	SetTableCell (table, 1, FF_Boxed | FF_Right, w - 1 - 12 * CELL_W / 2, 3 * CELL_W / 2, N_("Rating"));
	SetTableCell (table, 2, FF_Boxed | FF_Right, w - 1 - 9 * CELL_W / 2, CELL_W, N_("Rank"));
	SetTableCell (table, 3, FF_Boxed | FF_Right, w - 1 - 7 * CELL_W / 2, CELL_W, N_("# Won"));
	SetTableCell (table, 4, FF_Boxed | FF_Right, w - 1 - 5 * CELL_W / 2, 3 * CELL_W / 2, N_("%% Won"));
	SetTableCell (table, 5, FF_Boxed | FF_Right, w - 1 - 2 * CELL_W / 2, CELL_W, N_("# Total"));
	/* --- */
	return &table->item;
}								/* MenuCreateCentralHeader */

/*
 * create player info entry
 */
XBMenuItem *
MenuCreateInfoEntry (int x, int y, int w, const XBCentralInfo * data, MIC_button func,
					 void *funcData)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, func, funcData, NULL, NULL, 2);
	/* set cells ... */
	assert (data != NULL);
	if (data->name != NULL) {
		SetTableCell (table, 0, FF_Left, 1, (w - 3 * CELL_W - 2), data->name);
	}
	else {
		SetTableCell (table, 0, FF_Left, 1, (w - 3 * CELL_W - 2), "");
	}
	SetTableCell (table, 1, FF_Left, w - 1 - 12 * CELL_W / 2, 12 * CELL_W / 2, "%s", data->value);
	/* --- */
	return &table->item;
}								/* MenuCreateInfoEntry */

/*
 * create header for player info table
 */
XBMenuItem *
MenuCreateInfoHeader (int x, int y, int w, const char *title)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, NULL, NULL, NULL, NULL, 2);
	/* set cells ... */
	SetTableCell (table, 0, FF_Boxed | FF_Left, 1, w - 6 * CELL_W - 2, N_("Parameter"));
	SetTableCell (table, 1, FF_Boxed | FF_Left, w - 1 - 12 * CELL_W / 2, 12 * CELL_W / 2, N_("Value"));
	/* --- */
	return &table->item;
}								/* MenuCreateInfoHeader */

/*
 * create a demo game entry
 */
XBMenuItem *
MenuCreateDemoEntry (int x, int y, int w, const CFGDemoEntry * data, MIC_button func,
					 void *funcData)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, func, funcData, NULL, NULL, 3);
	/* set cells ... */
	assert (data != NULL);
	/* set cells */
	SetTableCell (table, 0, FF_Left, 1, 5 * CELL_W / 2, DateString (data->time));
	SetTableCell (table, 1, FF_Left, 1 + 5 * CELL_W / 2, (w - 4 * CELL_W - 2),
				  GetLevelNameByAtom (data->level));
	SetTableCell (table, 2, FF_Center, w - 1 - 3 * CELL_W / 2, 3 * CELL_W / 2, "%d",
				  data->numPlayers);
	/* --- */
	return &table->item;
}								/* MenuCreateDemoEntry */

/*
 * create header for demo game table
 */
XBMenuItem *
MenuCreateDemoHeader (int x, int y, int w)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, NULL, NULL, NULL, NULL, 3);
	/* set cells */
	SetTableCell (table, 0, FF_Boxed | FF_Left, 1, 5 * CELL_W / 2, N_("Date & Time"));
	SetTableCell (table, 1, FF_Boxed | FF_Left, 1 + 5 * CELL_W / 2, (w - 4 * CELL_W - 2), N_("Level"));
	SetTableCell (table, 2, FF_Boxed | FF_Center, w - 1 - 3 * CELL_W / 2, 3 * CELL_W / 2,
				  N_("#players"));
	/* --- */
	return &table->item;
}								/* MenuCreateDemoHeader */

/*
 * create entry for game data
 */
XBMenuItem *
MenuCreateGameEntry (int x, int y, int w, const XBNetworkGame ** game, MIC_button func)
{
	XBMenuTableItem *table =
		CreateTableItem (x, y, w, func, game, PollGameEntry, (const void **)game, 7);
	/* set cells */
	SetTableCell (table, 0, FF_Left, 1, w - 2 - 16 * CELL_W / 2, "");
	SetTableCell (table, 1, FF_Center, w - 1 - 16 * CELL_W / 2, 11 * CELL_W / 4, "");
	SetTableCell (table, 2, FF_Center, w - 1 - 21 * CELL_W / 4, 2 * CELL_W / 2, "");
	SetTableCell (table, 3, FF_Center, w - 1 - 17 * CELL_W / 4, 5 * CELL_W / 4, "");
	SetTableCell (table, 4, FF_Center, w - 1 - 6 * CELL_W / 2, 2 * CELL_W / 2, "");
	SetTableCell (table, 5, FF_Center, w - 1 - 4 * CELL_W / 2, 2 * CELL_W / 2, "");
	SetTableCell (table, 6, FF_Center, w - 1 - 2 * CELL_W / 2, 2 * CELL_W / 2, "");
	/* --- */
	return &table->item;
}								/* MenuCreateGameEntry */

/*
 * create header for game table
 */
XBMenuItem *
MenuCreateGameHeader (int x, int y, int w)
{
	XBMenuTableItem *table = CreateTableItem (x, y, w, NULL, NULL, NULL, NULL, 7);
	/* set cells */
	SetTableCell (table, 0, FF_Boxed | FF_Left, 1, w - 2 - 16 * CELL_W / 2, N_("Game"));
	SetTableCell (table, 1, FF_Boxed | FF_Center, w - 1 - 16 * CELL_W / 2, 11 * CELL_W / 4, N_("Host"));
	SetTableCell (table, 2, FF_Boxed | FF_Center, w - 1 - 21 * CELL_W / 4, 2 * CELL_W / 2, N_("Ping"));
	SetTableCell (table, 3, FF_Boxed | FF_Center, w - 1 - 17 * CELL_W / 4, 5 * CELL_W / 4,
				  N_("Version"));
	SetTableCell (table, 4, FF_Boxed | FF_Center, w - 1 - 6 * CELL_W / 2, 2 * CELL_W / 2, N_("#lives"));
	SetTableCell (table, 5, FF_Boxed | FF_Center, w - 1 - 4 * CELL_W / 2, 2 * CELL_W / 2, N_("#wins"));
	SetTableCell (table, 6, FF_Boxed | FF_Center, w - 1 - 2 * CELL_W / 2, 2 * CELL_W / 2, N_("FPS"));
	/* --- */
	return &table->item;
}								/* MenuCreateGameHeader */

/*
 * delete a table
 */
void
MenuDeleteTable (XBMenuItem * item)
{
	XBMenuTableItem *table = (XBMenuTableItem *) item;

	size_t i;
	/* sanity check */
	assert (NULL != table);
	assert (MIT_Table == table->item.type);
	assert (NULL != table->cell);
	/* clean up cells */
	for (i = 0; i < table->numCells; i++) {
		assert (NULL != table->cell[i].text);
		assert (NULL != table->cell[i].sprite);
		free (table->cell[i].text);
		DeleteSprite (table->cell[i].sprite);
	}
	free (table->cell);
}								/* MenuDeleteTable */

/*
 * end of file mi_stat.c
 */
