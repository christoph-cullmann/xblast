/*
 * file x11_config.c - x11 specific configuration
 *
 * $Id: x11_config.c,v 1.7 2006/02/09 21:21:25 fzago Exp $
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
#include "x11_config.h"

/*
 * local variables
 */
static DBRoot *dbGui = NULL;

/* default font config */
static CFGFont defaultFontConfig = {
#ifdef MINI_XBLAST
	"-misc-fixed-medium-r-normal--7-70-75-75-c-50-iso8859-1",	/* small */
	"-misc-fixed-medium-r-normal--10-100-75-75-c-60-iso8859-1",	/* medium */
	"-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso8859-1",	/* large */
	//  "-*-helvetica-medium-r-*-*-8-*-*-*-*-*-iso8859-*",  /* small */
	//"-*-helvetica-medium-r-*-*-10-*-*-*-*-*-iso8859-*", /* medium */
	// "-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-*", /* large */
#else
	"-*-helvetica-bold-r-*-*-14-*-*-*-*-*-iso8859-*",	/* small */
	"-*-helvetica-bold-r-*-*-18-*-*-*-*-*-iso8859-*",	/* medium */
	"-*-helvetica-bold-r-*-*-24-*-*-*-*-*-iso8859-*",	/* large */
#endif
};

/* default colors */
static CFGColor defaultColorConfig = {
	COLOR_GRAY_75,
	COLOR_MIDNIGHT_BLUE,
	COLOR_BLACK,
	COLOR_MIDNIGHT_BLUE,
	COLOR_GOLD,
	COLOR_YELLOW,
	COLOR_LIGHT_GOLDENROD,
	COLOR_SADDLE_BROWN,
	COLOR_SPRING_GREEN,
};

/*
 * store font config
 */
static void
StoreFontConfig (const CFGFont * cfgFont)
{
	DBSection *section;

	/* sanity check */
	assert (NULL != cfgFont);
	/* create new section */
#ifdef MINI_XBLAST
	section = DB_CreateSection (dbGui, atomFontMini);
#else
	section = DB_CreateSection (dbGui, atomFont);
#endif
	assert (section != NULL);
	/* set values */
	DB_CreateEntryString (section, atomSmall, cfgFont->small);
	DB_CreateEntryString (section, atomMedium, cfgFont->medium);
	DB_CreateEntryString (section, atomLarge, cfgFont->large);
}								/* StoreFontConfig */

/*
 * retrieve font config
 */
const CFGFont *
GetFontConfig (void)
{
	const DBSection *section;
	static CFGFont cfgFont;

	memcpy (&cfgFont, &defaultFontConfig, sizeof (CFGFont));
	/* get database entrry */
#ifdef MINI_XBLAST
	section = DB_GetSection (dbGui, atomFontMini);
#else
	section = DB_GetSection (dbGui, atomFont);
#endif
	if (NULL != section) {
		DB_GetEntryString (section, atomSmall, &cfgFont.small);
		DB_GetEntryString (section, atomMedium, &cfgFont.medium);
		DB_GetEntryString (section, atomLarge, &cfgFont.large);
	}
	return &cfgFont;
}								/* GetFontConfig */

/*
 * store color settings
 */
static void
StoreColorConfig (const CFGColor * cfgColor)
{
	DBSection *section;

	/* sanity check */
	assert (NULL != cfgColor);
	/* create new section */
	section = DB_CreateSection (dbGui, atomColor);
	assert (section != NULL);
	/* set values */
	DB_CreateEntryColor (section, atomTitleFg, cfgColor->titleFg);
	DB_CreateEntryColor (section, atomTitleBg, cfgColor->titleBg);
	DB_CreateEntryColor (section, atomDarkText1, cfgColor->darkText1);
	DB_CreateEntryColor (section, atomDarkText2, cfgColor->darkText2);
	DB_CreateEntryColor (section, atomLightText1, cfgColor->lightText1);
	DB_CreateEntryColor (section, atomLightText2, cfgColor->lightText2);
	DB_CreateEntryColor (section, atomStatusFg, cfgColor->statusFg);
	DB_CreateEntryColor (section, atomStatusBg, cfgColor->statusBg);
	DB_CreateEntryColor (section, atomStatusLed, cfgColor->statusLed);
}								/* StoreColorConfig */

/*
 * get color settings
 */
const CFGColor *
GetColorConfig (void)
{
	const DBSection *section;
	static CFGColor cfgColor;

	memcpy (&cfgColor, &defaultColorConfig, sizeof (CFGColor));
	/* get database entrry */
	if (NULL != (section = DB_GetSection (dbGui, GUI_StringToAtom ("color")))) {
		DB_GetEntryColor (section, atomTitleFg, &cfgColor.titleFg);
		DB_GetEntryColor (section, atomTitleBg, &cfgColor.titleBg);
		DB_GetEntryColor (section, atomDarkText1, &cfgColor.darkText1);
		DB_GetEntryColor (section, atomDarkText2, &cfgColor.darkText2);
		DB_GetEntryColor (section, atomLightText1, &cfgColor.lightText1);
		DB_GetEntryColor (section, atomLightText2, &cfgColor.lightText2);
		DB_GetEntryColor (section, atomStatusFg, &cfgColor.statusFg);
		DB_GetEntryColor (section, atomStatusBg, &cfgColor.statusBg);
		DB_GetEntryColor (section, atomStatusLed, &cfgColor.statusLed);
	}
	return &cfgColor;
}								/* GetColorConfig */

/*
 * load gui database
 */
void
GUI_LoadConfig (void)
{
	/* create empty database for gui data */
	dbGui = DB_Create (DT_Config, atomX11);
	assert (dbGui != NULL);
	/* load from file */
	Dbg_Config ("loading x11 gui data\n");
	if (DB_Load (dbGui)) {
		return;
	}
	Dbg_Config ("failed to load x11 gui data, setting defaults\n");
	/* set default values */
	StoreFontConfig (&defaultFontConfig);
	StoreColorConfig (&defaultColorConfig);
	/* and save it */
	DB_Store (dbGui);
}								/* GUI_LoadConfig */

/*
 * store gui database
 */
void
GUI_SaveConfig (void)
{
	assert (dbGui != NULL);
	if (!DB_Changed (dbGui)) {
		Dbg_Config ("saving x11 gui data\n");
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
	Dbg_Config ("x11 gui data cleared\n");
}								/* GUI_FinishConfig */

/*
 * end of file x11_config.c
 */
