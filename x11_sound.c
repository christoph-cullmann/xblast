/*
 * file x11_sound.c - sound via xbsndsrv or bell
 *
 * $Id: x11_sound.c,v 1.10 2006/06/12 11:06:36 fzago Exp $
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
#include "x11_common.h"

/*
 * local constants
 */
#ifndef XBLAST_SOUND_SERVER
#define XBLAST_SOUND_SERVER "padsp", "aixbsndsrv"
#endif

/* client/server commands */
#define SND_LOAD_SOUND      0
#define SND_PLAY_SOUND      1
#define SND_STOP_SOUND      2
#define SND_UNLOAD_SOUND    3

/* values to be used in acknowledge pipe */
#define SND_ACK_OK      0
#define SND_ACK_ERROR   1

/*
 * local variables
 */
static XBBool isActive = XBFalse;
static XBBool boom = XBFalse;

static XBBool soundInitialized = XBFalse;

static int cmdPipe[2];			/* pipe i/o descriptors */
static int ackPipe[2];
static pid_t serverPid = -1;

static char *monoArgs[] = {
	XBLAST_SOUND_SERVER,
	"-mono",
	NULL,
};

static char *stereoArgs[] = {
	XBLAST_SOUND_SERVER,
	NULL,
};

/*
 *
 */
XBBool
SND_Init (const CFGSoundSetup * setup)
{
	char **serverArgs = NULL;

	assert (NULL != setup);
	/* mode selection */
	switch (setup->mode) {
		/* wave sound via xbsndsrv */
	case XBSM_Waveout:
		serverArgs = setup->stereo ? stereoArgs : monoArgs;
		isActive = XBTrue;
		break;
		/* out via X11-bell */
	case XBSM_Beep:
		isActive = XBTrue;
		return XBTrue;
		/* no sound at all */
	case XBSM_None:
		isActive = XBFalse;
		return XBTrue;
	default:
		return XBFalse;
	}
	/* create pipes for communication */
	if (pipe (cmdPipe) < 0) {
		fprintf (stderr, "could not create cmd pipe for sound communication\n");
		return XBFalse;
	}
	if (pipe (ackPipe) < 0) {
		fprintf (stderr, "could not create ack pipe for sound communication\n");
		close (cmdPipe[0]);
		close (cmdPipe[1]);
		return XBFalse;
	}

	if (0 == (serverPid = fork ())) {
		/* child process */
		close (cmdPipe[1]);
		close (0);
		(void)dup (cmdPipe[0]);
		close (ackPipe[0]);
		close (1);
		(void)dup (ackPipe[1]);

		if (execvp (serverArgs[0], serverArgs) < 0) {
			int ack_val = SND_ACK_ERROR;
			fprintf (stderr, "Could not exec sound server\n");
			fflush (stderr);
			write (1, &ack_val, sizeof (ack_val));
		}
		exit (0);
	}
	else if (serverPid > 0) {
		/* parent (client) */
		int ack_val;

		close (cmdPipe[0]);
		close (ackPipe[1]);

		read (ackPipe[0], &ack_val, sizeof (ack_val));
		if (ack_val == SND_ACK_OK) {
			soundInitialized = XBTrue;
			/* Exit handler to kill sound server on exit */
			RegisterSound (ackPipe[0]);
			atexit (SND_Finish);
			return XBTrue;
		}
		else {
			soundInitialized = XBFalse;
			return XBTrue;
		}
	}
	else {
		fprintf (stderr, "could not fork sound server\n");
		close (cmdPipe[0]);
		close (cmdPipe[1]);
		soundInitialized = XBFalse;
		return XBFalse;
	}
}								/* SND_Init */

/*
 *
 */
XBBool
SND_Stop (SND_Id id)
{
	if (soundInitialized) {
		int cmd[2];

		cmd[0] = SND_STOP_SOUND;
		cmd[1] = id;
		write (cmdPipe[1], cmd, sizeof (cmd));
		/* read (ackPipe[0], cmd, 1); */
	}
	return XBTrue;
}								/* SND_Stop */

/*
 *
 */
XBBool
SND_Play (SND_Id id, int position)
{
	if (soundInitialized) {
		/* Note: position argument is ignored for stereo sounds */
		int cmd[2];

		cmd[0] = SND_PLAY_SOUND;
		cmd[1] = id | (position << 16);
		write (cmdPipe[1], cmd, sizeof (cmd));
	}
	else {
		/* beep at next frame */
		if (SND_MINIBOMB == id || SND_EXPL == id) {
			boom = XBTrue;
		}
	}
	return XBTrue;
}								/* SND_Play */

/*
 *
 */
XBBool
SND_Load (SND_Id id)
{
	if (soundInitialized) {
		int cmd[2];

		cmd[0] = SND_LOAD_SOUND;
		cmd[1] = id;
		write (cmdPipe[1], cmd, sizeof (cmd));
		/* read (ackPipe[0], cmd, 1); */
	}
	return XBTrue;
}								/* SND_Load */

/*
 *
 */
XBBool
SND_Unload (SND_Id id)
{
	if (soundInitialized) {
		int cmd[2];

		cmd[0] = SND_UNLOAD_SOUND;
		cmd[1] = id;
		write (cmdPipe[1], cmd, sizeof (cmd));
		/* read (ackPipe[0], cmd, 1); */
	}
	return XBTrue;
}								/* SND_Unload */

/*
 *
 */
void
SND_Flush (void)
{
	if (isActive && boom) {
		boom = XBFalse;
		/* ring the bell */
		XBell (dpy, 80);
	}
}								/* SND_Flush */

/*
 *
 */
void
SND_Finish (void)
{
	int wait_status;

	if (soundInitialized && serverPid != -1) {
		kill (serverPid, SIGINT);
		wait (&wait_status);
		UnregisterSound (ackPipe[0]);
	}
	soundInitialized = XBFalse;
}								/* SND_Finish */

/*
 * beep once
 */
void
SND_Beep (void)
{
	XBell (dpy, 80);
}								/* SND_Beep */

/*
 *
 */
void
HandleSound (int fd)
{
	if (fd == ackPipe[0]) {
		int cmd[2];
		read (ackPipe[0], cmd, 1);
		/* TODO: ack auswerten */
		//   Dbg_Out ("sound ack\n");
	}
}								/* HandleSound */

/*
 * end of file x11_sound.c
 */
