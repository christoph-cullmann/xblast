/*
 * cfg_main.c - managing configuration files 
 *
 * $Id: cfg_main.c,v 1.4 2004/08/07 18:27:44 lodott Exp $
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
#include "cfg_main.h"

#include "atom.h"
#include "cfg_demo.h"
#include "cfg_player.h"
#include "cfg_level.h"
#include "cfg_game.h"
#include "cfg_control.h"
#include "cfg_stat.h"
#include "cfg_xblast.h"

#include "gui.h"

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
} /* InitConfig */

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
} /* FinishConfig */

/*
 * check configurations
 */
XBBool
CheckConfig (void)
{
  return CheckAllLevelConfigs();
} /* CheckConfig */

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
} /* FinishConfig */

/*
 * end of file cfg_main.c
 */
