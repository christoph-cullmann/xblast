/*
 * file w32_config.h - config data for win32 graphics engine
 *
 * Program XBLAST
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * $Id: w32_config.c,v 1.5 2006/02/19 13:33:01 lodott Exp $
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
#include "w32_config.h"
#include "gui.h"

#include "atom.h"
#include "geom.h"
#include "ini_file.h"

/*
 * local variables
 */
static DBRoot *dbGui = NULL;

/* default geometry data */
static const RECT defaultGeometry = {
	0, 0, PIXW, PIXH + SCOREH,
};

/*
 * load config
 */
void
GUI_LoadConfig (void)
{
	/* create empty database for gui data */
	dbGui = DB_Create (DT_Config, atomWin32);
	assert (dbGui != NULL);
	Dbg_Config ("loading w32 gui data\n");
	if (DB_Load (dbGui)) {
		return;
	}
	Dbg_Config ("failed to load w32 gui data\n");
	/* TODO: set default values */

	/* and save it */
	DB_Store (dbGui);
}								/* GUI_LoadConfig */

/*
 * save config if changed
 */
void
GUI_SaveConfig (void)
{
	assert (dbGui != NULL);
	if (DB_Changed (dbGui)) {
		Dbg_Config ("saving w32 gui data\n");
		DB_Store (dbGui);
	}
}								/* GUI_SaveConfig */

/*
 * finish config
 */
void
GUI_FinishConfig (void)
{
	assert (dbGui != NULL);
	DB_Delete (dbGui);
	dbGui = NULL;
	Dbg_Config ("w32 gui data cleared\n");
}								/* GUI_FinishConfig */

/*
 * get window position
 */
XBBool
RetrieveWindowRect (RECT * rect)
{
	const DBSection *section;

	assert (NULL != rect);
	assert (dbGui != NULL);
	/* set defaults */
	*rect = defaultGeometry;
	/* find section */
	if (NULL == (section = DB_GetSection (dbGui, atomGeometry))) {
		return XBFalse;
	}
	(void)DB_GetEntryInt (section, atomLeft, (int *)&rect->left);
	(void)DB_GetEntryInt (section, atomRight, (int *)&rect->right);
	(void)DB_GetEntryInt (section, atomTop, (int *)&rect->top);
	(void)DB_GetEntryInt (section, atomBottom, (int *)&rect->bottom);
	return XBTrue;
}								/* RetreiveWindowGeometry */

/*
 * store window position
 */
void
StoreWindowRect (const RECT * rect)
{
	DBSection *section;

	assert (NULL != rect);
	assert (dbGui != NULL);
	/* create section */
	section = DB_CreateSection (dbGui, atomGeometry);
	assert (NULL != section);
	DB_CreateEntryInt (section, atomLeft, rect->left);
	DB_CreateEntryInt (section, atomRight, rect->right);
	DB_CreateEntryInt (section, atomTop, rect->top);
	DB_CreateEntryInt (section, atomBottom, rect->bottom);
}								/* StoreWindowRect */

/*
 * end of file w32_config.h
 */
