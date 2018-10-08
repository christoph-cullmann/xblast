/*
 * file xblast.c - main routine
 *
 * $Id: xblast.c,v 1.19 2006/02/15 22:30:32 fzago Exp $
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

/* AbsInt start */
XBBool trace_aborted;
FILE *trace_aborted_file;
/* AbsInt end */

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
	/* finish all remaining databases */
	DB_Finish ();
	/* finish level selection, if necessary */
	FinishLevelSelection ();
#ifdef DEBUG_ALLOC
	Dbg_FinishAlloc ();
#endif
}								/* Finish */

/*
 * main routine
 */
#ifdef WMS
int WINAPI
WinMain (HINSTANCE hInstance,	// handle to current instance
		 HINSTANCE hPrevInstance,	// handle to previous instance
		 LPSTR lpCmdLine,		// pointer to command line
		 int nCmdShow			// show state of window
	)
#else
int
main (int argc, char *argv[])
#endif
{
#if defined(XBLAST_SOUND)
	CFGSoundSetup soundSetup;
#endif
#ifndef WMS
	int i;
#endif
	XBPlayerHost hostType;
	XBBool autoCentral;
	XBBool nsound;
	XBBool check;

#ifdef ENABLE_NLS
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif

	autoCentral = XBFalse;
	nsound = XBFalse;
	check = XBFalse;
    /* AbsInt start */
    trace_aborted = XBFalse;
    /* AbsInt end */

/* AbsInt: initialize chat filter from text file */
#if defined(XBLAST_CHAT_FILTER)
    init_bad_words();
#endif
/* AbsInt end */

#ifndef WMS
	if (argc > 1) {
		i = 1;
		while (i < argc) {
			if (0 == strcmp ("-central", argv[i])) {
				autoCentral = XBTrue;
				i++;
				continue;
			}
			else if (0 == strcmp ("-ns", argv[i])) {
				nsound = XBTrue;
				i++;
				continue;
			}
			else if (0 == strcmp ("-check", argv[i])) {
				check = XBTrue;
				i++;
				continue;
		    }
			/* AbsInt start */
            else if (0 == strcmp("-trace-abort", argv[1])) {
                printf("activating trace-mode...\n");
                trace_aborted = XBTrue;
                trace_aborted_file = fopen(GAME_DATADIR "/aborted_levels.txt", "at");
                if (!trace_aborted_file) {
                    printf("Cannot open '%s' for writing...\n", GAME_DATADIR "/aborted_levels.txt");
                    exit(EXIT_FAILURE);
                }
                ++i;
                continue;
			}
            /* AbsInt end */
			else {
				break;
			}
		}
	}
#endif
	/* init new configuration */
	InitConfig ();
	/* initialize network */
	if (!Socket_Init ()) {
		return -1;
	}
	/* Init Sound support */
#if defined(XBLAST_SOUND)
	RetrieveSoundSetup (&soundSetup);
	if (!nsound) {
		SND_Init (&soundSetup);
	}
#endif

	/* Initialize graphics engine */
#ifdef WMS
	if (!GUI_Init (0, NULL)) {
#else
	if (!GUI_Init (argc, argv)) {
#endif
		Finish ();
		return -1;
	}
	GUI_OnQuit (Finish);
	if (check && !CheckConfig ()) {
		GUI_Finish ();
		Finish ();
		return -2;
	}
	/* init random number generator */
	SeedRandom (time (NULL));
	/* Call intro */
	DoIntro ();

	/* main loop until quit */
	while (XBPH_None != (hostType = DoMenu (autoCentral))) {
		/* save current configurations */
		SaveConfig ();
		SetHostType (hostType);
		/* run selected game  */
		switch (hostType) {
		case XBPH_Local:
			RunLocalGame ();
			break;
		case XBPH_Demo:
			RunDemoGame ();
			break;
		case XBPH_Server:
			RunServerGame ();
			break;
		default:
			RunClientGame (hostType);
			break;
		}
	}
    /* AbsInt start */
    /* Close the trace file if needed */
    if (trace_aborted)
        fclose(trace_aborted_file);
    /* AbsInt end */
	/* close Display */
	GUI_Finish ();
	/* shutdown the rest */
	Finish ();
	/* that's all */
	return 0;
}								/* main */

/*
 * end of file xblast.c
 */
