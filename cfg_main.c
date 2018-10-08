/*
 * cfg_main.c - managing configuration files 
 *
 * $Id: cfg_main.c,v 1.6 2006/02/09 21:21:23 fzago Exp $
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
 * load all configurations
 */
void
InitConfig (void)
{
	GUI_InitAtoms ();
	InitDefaultAtoms ();
	/* load databases */
	LoadXBlastConfig ();
	LoadDemoConfig ();
	LoadPlayerConfig ();
	LoadGameConfig ();
	LoadLevelConfig ();
	LoadControlConfig ();
	LoadStatConfig ();
	GUI_LoadConfig ();
}								/* InitConfig */

/*
 * store all configurations
 */
void
SaveConfig (void)
{
	SaveXBlastConfig ();
	SaveDemoConfig ();
	SavePlayerConfig ();
	SaveGameConfig ();
	SaveLevelConfig ();
	SaveControlConfig ();
	SaveStatConfig ();
	GUI_SaveConfig ();
}								/* FinishConfig */

/*
 * check configurations
 */
XBBool
CheckConfig (void)
{
	return CheckAllLevelConfigs ();
}								/* CheckConfig */

/*
 * store all configurations
 */
void
FinishConfig (void)
{
	FinishXBlastConfig ();
	FinishDemoConfig ();
	FinishPlayerConfig ();
	FinishGameConfig ();
	FinishLevelConfig ();
	FinishControlConfig ();
	FinishStatConfig ();
	GUI_FinishConfig ();
}								/* FinishConfig */

/*
 * end of file cfg_main.c
 */
