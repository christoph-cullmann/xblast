/*
 * file xblast.c - main routine
 *
 * $Id: xblast.c,v 1.13 2004/11/29 14:40:22 lodott Exp $
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

#include "atom.h"
#include "bad_words.h"
#include "cfg_main.h"
#include "demo.h"
#include "game_client.h"
#include "game_demo.h"
#include "game_local.h"
#include "game_server.h"
#include "image.h"
#include "info.h"
#include "intro.h"
#include "menu.h"
#include "random.h"
#include "socket.h"
#include "menu_game.h"

#if defined(XBLAST_SOUND)
#include "snd.h"
#endif

/*
 * going down gracefully
 */
static void
Finish (void)
{
  Dbg_Out ("Finish\n");
  /* shutdown sound */
#if defined(XBLAST_SOUND)
  SND_Finish ();
#endif
  /* shutdown network */
  Socket_Finish ();
  /* clean up some modules */
  ClearShapeList ();
  ClearInfo ();
  /* save and delete current configuration */
  SaveConfig ();
  FinishConfig ();
#ifdef DEBUG_ALLOC
  Dbg_FinishAlloc ();
#endif
} /* Finish */

/*
 * main routine
 */
#ifdef WMS
int WINAPI WinMain(
  HINSTANCE hInstance,      // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,          // pointer to command line
  int nCmdShow              // show state of window
)
#else
int
main (int argc, char *argv[])
#endif
{
#if defined(XBLAST_SOUND)
  CFGSoundSetup  soundSetup;
#endif
#ifndef WMS
  int i;
#endif
  XBPlayerHost   hostType;
  XBBool         autoCentral;
  XBBool         nsound;
  XBBool         check;

  autoCentral = XBFalse;
  nsound = XBFalse;
  check = XBFalse;
#ifndef WMS
  if(argc>1){
    i=1;
    while(i<argc){
      if (0==strcmp("-central",argv[i])) {
	autoCentral = XBTrue;
	i++;
	continue;
      } else if (0==strcmp("-ns",argv[i])) {
	nsound = XBTrue;
	i++;
	continue;
      } else if (0==strcmp("-check",argv[i])) {
	check = XBTrue;
	i++;
	continue;
      } else {
	break;
      }
    }
  }
#endif
  /* init new configuration */
  InitConfig ();
  /* initialize network */
  if (! Socket_Init ()) {
    return -1;
  }
  /* Init Sound support */
#if defined(XBLAST_SOUND)
  RetrieveSoundSetup (&soundSetup);
  if (!nsound) {
    SND_Init (&soundSetup);
  }
#endif

  init_bad_words();

  /* Initialize graphics engine */
#ifdef WMS
  if (! GUI_Init (0, NULL) ) {
#else
  if (! GUI_Init (argc, argv) ) {
#endif
    Finish ();
    return -1;
  }
  GUI_OnQuit (Finish);
  if (check && ! CheckConfig() ) {
    GUI_Finish();
    Finish();
    return -2;
  }
  /* init random number generator */
  SeedRandom (time (NULL));
  /* Call intro */
  DoIntro ();

  /* main loop until quit */
  while (XBPH_None != (hostType = DoMenu (autoCentral) ) ) {
    /* save current configurations */
    SaveConfig ();
    SetHostType(hostType);
    /* run selected game  */
    switch (hostType) {
    case XBPH_Local:  RunLocalGame ();          break;
    case XBPH_Demo:   RunDemoGame ();           break;
    case XBPH_Server: RunServerGame ();         break;
    default:          RunClientGame (hostType); break;
    }
  }
  /* close Display */
  GUI_Finish ();
  /* shutdown the rest */
  Finish ();
  /* that's all */
  return 0;
} /* main */

/*
 * end of file xblast.c
 */
