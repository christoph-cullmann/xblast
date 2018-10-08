/*
 * file x11_sound.c - sound via xbsndsrv or bell
 *
 * $Id: sdl_sound.c 112466 2009-07-06 08:37:37Z ingmar $
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

#include "SDL_mixer.h"

/*
 * local constants
 */

/* values to be used in acknowledge pipe */
#define SND_ACK_OK      0
#define SND_ACK_ERROR   1

typedef short s16;

static struct _sound_name
{
	int sound_id;				/* the sound's id to refer to it */
	const char *name;			/* raw samples data file name */
	uint8_t *samples;			/* pointer to samples memory */
	int length;					/* length in samples of the sound */
	int repeat;					/* repeat flag to play sound endlessly */
	int mono;					/* mono flag indicating mono sounds */
} sound_name[] = {
	{
	SND_BAD, "xb_bad.raw", NULL, 0, XBFalse, XBFalse},	/* got a skull */
	{
	SND_DROP, "xb_drop.raw", NULL, 0, XBFalse, XBTrue},	/* dropped a bomb */
	{
	SND_NEWBOMB, "xbnbmb.raw", NULL, 0, XBFalse, XBTrue},	/* got an extra bomb */
	{
	SND_NEWKICK, "xbnkick.raw", NULL, 0, XBFalse, XBTrue},	/* got kick extra */
	{
	SND_NEWPUMP, "xbnpmp.raw", NULL, 0, XBFalse, XBTrue},	/* got pump extra */
	{
	SND_NEWRC, "xbnrc.raw", NULL, 0, XBFalse, XBTrue},	/* got rem. control */
	{
	SND_MOREFIRE, "xbfire.raw", NULL, 0, XBFalse, XBTrue},	/* got more range */
	{
	SND_DEAD, "xb_dead.raw", NULL, 0, XBFalse, XBFalse},	/* player died */
	{
	SND_EXPL, "xb_expl.raw", NULL, 0, XBFalse, XBTrue},	/* normal explosion */
	{
	SND_KICK, "xb_kick.raw", NULL, 0, XBFalse, XBTrue},	/* kick a bomb */
	{
	SND_PUMP, "xb_pump.raw", NULL, 0, XBFalse, XBTrue},	/* pump a bomb */
	{
	SND_OUCH, "xb_ouch.raw", NULL, 0, XBFalse, XBFalse},	/* player lost life */
	{
	SND_INTRO, "xb_intro.raw", NULL, 0, XBFalse, XBFalse},	/* intro fanfare */
	{
	SND_APPL, "xb_appl.raw", NULL, 0, XBFalse, XBFalse},	/* applause */
	{
	SND_APPL2, "xb_app2.raw", NULL, 0, XBFalse, XBFalse},	/* applause */
	{
	SND_BUTT, "xb_butt.raw", NULL, 0, XBFalse, XBTrue},	/* triggered button */
	{
	SND_SHOOT, "xb_shoot.raw", NULL, 0, XBFalse, XBFalse},	/* using rem. ctrl. */
	{
	SND_INVIS, "xb_nvis.raw", NULL, 0, XBFalse, XBFalse},	/* player invisible */
	{
	SND_INVINC, "xb_nvnc.raw", NULL, 0, XBFalse, XBFalse},	/* player invincible */
	{
	SND_NEWTELE, "xbntel.raw", NULL, 0, XBFalse, XBTrue},	/* player got telep. */
	{
	SND_TELE, "xbtele.raw", NULL, 0, XBFalse, XBTrue},	/* player uses tele. */
	{
	SND_INJ, "xbinj.raw", NULL, 0, XBFalse, XBFalse},	/* player got junkie */
	{
	SND_MINIBOMB, "xbmbmb.raw", NULL, 0, XBFalse, XBTrue},	/* small bomb expl. */
	{
	SND_WON, "xb_won.raw", NULL, 0, XBFalse, XBFalse},	/* player won */
	{
	SND_HAUNT, "xb_haunt.raw", NULL, 0, XBFalse, XBFalse},	/* haunting bomb */
	{
	SND_SPIRAL, "xb_spir.raw", NULL, 0, XBFalse, XBTrue},	/* spiral shrinking */
	{
	SND_SPBOMB, "xb_spbmb.raw", NULL, 0, XBFalse, XBTrue},	/* got special bomb */
	{
	SND_SLIDE, "xbslide.raw", NULL, 0, XBFalse, XBTrue},	/* bomb slide sound */
	{
	SND_FINALE, "xbfin.raw", NULL, 0, XBFalse, XBFalse},	/* final fanfare */
	{
	SND_WARN, "xb_warn.raw", NULL, 0, XBFalse, XBFalse},	/* shrink warn sound */
	{
	SND_STUN, "xb_stun.raw", NULL, 0, XBFalse, XBFalse},	/* player stun sound */
	{
	SND_WHIRL, "xb_whrl.raw", NULL, 0, XBTrue, XBFalse},	/* intro whirl */
	{
	SND_COMPOUND, "xb_cmpnd.raw", NULL, 0, XBFalse, XBFalse},	/* compound shrink */
	{
	SND_TELE1, "xbtele1.raw", NULL, 0, XBFalse, XBTrue},	/* teleport start */
	{
	SND_TELE2, "xbtele2.raw", NULL, 0, XBFalse, XBTrue},	/* teleport end */
	{
	SND_HOLY, "xbholy.raw", NULL, 0, XBFalse, XBFalse},	/* holy grail extra */
	{
	SND_ENCLOAK, "xbcloak.raw", NULL, 0, XBFalse, XBTrue},	/* encloak sound */
	{
	SND_DECLOAK, "xbdcloak.raw", NULL, 0, XBFalse, XBTrue},	/* decloak sound */
	{
	SND_FAST, "xbfast.raw", NULL, 0, XBFalse, XBTrue},	/* speed up extra */
	{
	SND_SLOW, "xbslow.raw", NULL, 0, XBFalse, XBTrue},	/* slow down extra */
	{
	SND_SLAY, "xbslay.raw", NULL, 0, XBFalse, XBTrue},	/* slay extra */
	{
	SND_LIFE, "xblife.raw", NULL, 0, XBFalse, XBTrue},	/* extra life */
	{
	SND_NEWCLOAK, "xbcloakx.raw", NULL, 0, XBFalse, XBTrue},	/* new cloak extra */
	{
	SND_BOMBMORPH, "xb_bombmorph.raw", NULL, 0, XBFalse, XBTrue},	/* bomb morph */
	{
	SND_STEP1, "xbstep1.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #1 */
	{
	SND_STEP2, "xbstep2.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #2 */
	{
	SND_STEP3, "xbstep3.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #3 */
	{
	SND_STEP4, "xbstep4.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #4 */
	{
	SND_STEP5, "xbstep5.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #5 */
	{
	SND_STEP6, "xbstep6.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #6 */
	{
	SND_SNG1, "xbsng1.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #1 */
	{
	SND_SNG2, "xbsng2.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #2 */
	{
	SND_SNG3, "xbsng3.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #3 */
	{
	SND_SNG4, "xbsng4.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #4 */
	{
	SND_SNG5, "xbsng5.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #5 */
	{
	SND_SNG6, "xbsng6.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #6 */
	{
	SND_MAX, NULL, NULL, 0}
};

static Mix_Chunk *sound_chunk[SND_MAX];
static int sound_channel[SND_MAX];

/* SUBSIZE small so no delay for playing...
   maybe a better solution should be there, since it is 
   not nice sending all and dont using the buffer.
*/
#define SUBSIZE          128

#define SOUND_DEVICE "/dev/dsp"
#define SAMPLE_RATE     22050
/* SDL_Mixer doesn't allow one channel with
left and right, so we need 2 output channels
*/
#define SAMPLE_CHANNELS     2
#define SAMPLE_SIZE         AUDIO_U8

static int mono_mode = XBFalse;

/*
 * local variables
 */
static XBBool isActive = XBFalse;
static XBBool boom = XBFalse;

static XBBool soundInitialized = XBFalse;

/*
 * Open Audio 
 */
XBBool
SND_Init (const CFGSoundSetup * setup)
{
	int i;

	assert (NULL != setup);

	/* mode selection */
	switch (setup->mode) {

	case XBSM_Waveout:
		isActive = XBTrue;
		break;

	case XBSM_Beep:
		isActive = XBTrue;
		return XBTrue;

	case XBSM_None:
		/* no sound at all */
		isActive = XBFalse;
		return XBTrue;

	default:
		return XBFalse;
	}

	/* Initialize SDL_mixer stuff */

	/* init audio */
	if (Mix_OpenAudio (SAMPLE_RATE, SAMPLE_SIZE, SAMPLE_CHANNELS, SUBSIZE) != 0) {
		fprintf (stderr, "Error: Couldn't initializate audio\n"
				 "Possible reason: %s\n", Mix_GetError ());
		return XBFalse;
	}

	/* Add more mixing channels. The default of MIX_CHANNELS (8) is
	 * not enough when there is many bombs. Is the new number too big
	 * for slow machines? */
	Mix_AllocateChannels(20);

	/* reset sound stuff */
	for (i = 0; i < SND_MAX; i++) {
		sound_chunk[i] = NULL;
		sound_channel[i] = -1;
	}

	soundInitialized = XBTrue;

	return XBTrue;

}								/* SND_Init */

/*
 * Stop Audio
 */
XBBool
SND_Stop (SND_Id id)
{
	if (soundInitialized) {
		if (id == STOP_ALL_SOUNDS) {
			Mix_HaltChannel (-1);
		}
		else					// stop specific sound
		{
			if (sound_channel[id] != -1)
				Mix_HaltChannel (sound_channel[id]);
			sound_channel[id] = -1;
		}
	}
	return XBTrue;
}								/* SND_Stop */

/*
 * Play sound and set right position,
 */
XBBool
SND_Play (SND_Id id, int position)
{
	if (soundInitialized) {

		// do a stereo effect
		int col1 = 255 - ((position * 255) / MAX_SOUND_POSITION);
		int col2 = (position * 255) / MAX_SOUND_POSITION;

		/* When there is too many bombs on the screen,
		 * this function will fails. */
		sound_channel[id] = Mix_PlayChannel (-1, sound_chunk[id], 0);
		if (sound_channel[id] != -1) {
			/* Panning must be after the channel opened,
			 * since we dont know the channel before */
			if (!Mix_SetPanning (sound_channel[id], col2, col1)) {
				fprintf (stderr, "Mix_SetPanning(%d, %d, %d) failed!\n", sound_channel[id], col1, col2);
				fprintf (stderr, "Reason: [%s].\n", Mix_GetError ());
			}
		}
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
 * beep once
 */
void
SND_Beep (void)
{
	/* TODO 
	   XBell (dpy, 80);
	 */
}								/* SND_Beep */

/*
 * Load sound
 */
XBBool
SND_Load (SND_Id id)
{
	if (soundInitialized) {
		/* First load RAW into memory */
		char fname[1000];
		int i, f;
		static char *path_list[3] = {
			NULL,
			GAME_DATADIR "/",
			".",
		};

		/* check environment for xblast search path */
		path_list[0] = getenv ("XBLASTDIR");

		for (i = 0; i < 3; i++) {
			if (path_list[i] != NULL) {
				sprintf (fname, "%s/%s/%s", path_list[i], "sounds", sound_name[id].name);
				if ((f = open (fname, O_RDONLY)) >= 0) {
					int sound_size;
					uint8_t *sb, *sb1;
					struct stat snd_stat;

#ifdef DEBUG
					fprintf (stderr, "Opened file \"%s\".\n", fname);
#endif
					(void)fstat (f, &snd_stat);
					sound_size = snd_stat.st_size / sizeof (uint8_t);
					if (sound_name[id].samples != NULL) {
						free (sound_name[id].samples);
						sound_name[id].samples = NULL;
						sound_name[id].length = 0;
					}

					if ((sb = malloc (sound_size * sizeof (uint8_t))) == NULL) {
						close (f);
						return (-1);
					}
					else {
						read (f, sb, sound_size * sizeof (uint8_t));
						close (f);
#if defined(SERVER_STATISTICS)
						total_loaded++;
#endif
						/* make sound Stereo although mono... blame SDL_mixer
						   same problem as the 2 channels issue
						 */
						if (mono_mode != XBTrue && sound_name[id].mono == XBTrue) {
							if ((sb1 = malloc (2 * sound_size * sizeof (uint8_t))) == NULL) {
								free (sb);
								return (-1);
							}
							for (i = 0; i < sound_size; i++) {
								sb1[2*i] = sb1[2*i + 1] = sb[i];

							}
							/* we free sb afterwards so we need
							   to pass sb1 to sb */
							free (sb);
							sb = sb1;
							sound_size *= 2;
						}
						sound_name[id].samples = sb;
						sound_name[id].length = sound_size;
						/*
						 * convert stereo samples to mono if running in mono mode 
						 */

						if (mono_mode == XBTrue && sound_name[id].mono == XBFalse) {
							int i;
							uint8_t *m, *s;
							s16 sum;

							m = s = sound_name[id].samples;

							sound_name[id].length >>= 1;
							for (i = 0; i < sound_name[id].length; i++) {
								sum = *s + *(s + 1);
								*m++ = sum >> 1;
								s += 2;
							}
						}
						if (!(sound_chunk[id] = Mix_QuickLoad_RAW (sb, sound_name[id].length)))
							fprintf (stderr, "Warning: Could not open RAW from memory: %s\n",
									 Mix_GetError ());

						break;
					}
				}
				else {
					fprintf (stderr, "Couldnt open file \"%s\".\n", fname);
				}
			}
		}

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
		Mix_FreeChunk (sound_chunk[id]);
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
	}
}								/* SND_Flush */

/*
 *
 */
void
SND_Finish (void)
{
	Mix_CloseAudio ();

	soundInitialized = XBFalse;
}								/* SND_Finish */
