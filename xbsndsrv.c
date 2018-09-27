/*
 * Program XBLAST V2.5.15 or higher
 * (C) by Oliver Vogel (e-mail: vogel@ikp.uni-koeln.de)
 * March 21nd, 1997
 * started August 1993
 *
 * File: xbsndsrv.c 
 * sound server and sound processing
 *
 * Author: Norbert Nicolay, e-mail: nicolay@ikp.uni-koeln.de
 *         July 30th 1996
 *
 * $Id: xbsndsrv.c,v 1.14 2005/01/05 17:12:00 alfie Exp $
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licences as published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Publis License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#if defined(XBLAST_SOUND)

#define _SOUND_C_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else
#include <linux/soundcard.h>
#endif
#include <signal.h>
#include "snd.h"

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

/* 
 * play at least 5 sounds simultaneously regardless of machine power
 */
#define DEFAULT_MAX_SIM_SOUNDS 10

#define ABS_MAX_SIM_SOUNDS    250
#define MAX_CPU_LOAD_ADJUST    10

/* client/server commands */
#define SND_LOAD_SOUND      0
#define SND_PLAY_SOUND      1
#define SND_STOP_SOUND      2
#define SND_UNLOAD_SOUND    3

/* arguments to the commands */
#define STOP_ALL_SOUNDS     0

/* values to be used in acknowledge pipe */
#define SND_ACK_OK      0
#define SND_ACK_ERROR   1

static const char rcs_id[] = "$Id: xbsndsrv.c,v 1.14 2005/01/05 17:12:00 alfie Exp $";

typedef unsigned char u8;
typedef short s16;

typedef struct _xbsound {
  int              id;
  int              repeat;
  int              mono;
  int              position;
  u8              *samples;
  int              length;
  struct _xbsound *next;
} XBSOUND;

static int max_sim_sounds  = DEFAULT_MAX_SIM_SOUNDS;
static int next_free_sound = 0;
static int sounds_playing = 0;
static XBSOUND *sound_table;
static volatile int calibration_ready;

/*
 * sound sample table
 */
static struct _sound_name {
  int          sound_id;     /* the sound's id to refer to it */
  const char  *name;         /* raw samples data file name */
  u8          *samples;      /* pointer to samples memory */
  int          length;       /* length in samples of the sound */
  int          repeat;       /* repeat flag to play sound endlessly */
  int          mono;         /* mono flag indicating mono sounds */
} sound_name[] = {
  {SND_BAD,      "xb_bad.raw", NULL, 0, FALSE, FALSE},  /* got a skull */
  {SND_DROP,     "xb_drop.raw", NULL, 0, FALSE, TRUE},  /* dropped a bomb */
  {SND_NEWBOMB,  "xbnbmb.raw", NULL, 0, FALSE, TRUE},   /* got an extra bomb */
  {SND_NEWKICK,  "xbnkick.raw", NULL, 0, FALSE, TRUE},  /* got kick extra */
  {SND_NEWPUMP,  "xbnpmp.raw", NULL, 0, FALSE, TRUE},   /* got pump extra */
  {SND_NEWRC,    "xbnrc.raw", NULL, 0, FALSE, TRUE},    /* got rem. control */
  {SND_MOREFIRE, "xbfire.raw", NULL, 0, FALSE, TRUE},   /* got more range */
  {SND_DEAD,     "xb_dead.raw", NULL, 0, FALSE, FALSE}, /* player died */
  {SND_EXPL,     "xb_expl.raw", NULL, 0, FALSE, TRUE},  /* normal explosion */
  {SND_KICK,     "xb_kick.raw", NULL, 0, FALSE, TRUE},  /* kick a bomb */
  {SND_PUMP,     "xb_pump.raw", NULL, 0, FALSE, TRUE},  /* pump a bomb */
  {SND_OUCH,     "xb_ouch.raw", NULL, 0, FALSE, FALSE}, /* player lost life */
  {SND_INTRO,    "xb_intro.raw", NULL, 0, FALSE, FALSE},/* intro fanfare */
  {SND_APPL,     "xb_appl.raw", NULL, 0, FALSE, FALSE}, /* applause */
  {SND_APPL2,     "xb_app2.raw", NULL, 0, FALSE, FALSE}, /* applause */
  {SND_BUTT,     "xb_butt.raw", NULL, 0, FALSE, TRUE},  /* triggered button */
  {SND_SHOOT,    "xb_shoot.raw", NULL, 0, FALSE, FALSE},/* using rem. ctrl. */
  {SND_INVIS,    "xb_nvis.raw", NULL, 0, FALSE, FALSE}, /* player invisible */
  {SND_INVINC,   "xb_nvnc.raw", NULL, 0, FALSE, FALSE}, /* player invincible */
  {SND_NEWTELE,  "xbntel.raw", NULL, 0, FALSE, TRUE},   /* player got telep. */
  {SND_TELE,     "xbtele.raw", NULL, 0, FALSE, TRUE},   /* player uses tele. */
  {SND_INJ,      "xbinj.raw", NULL, 0, FALSE, FALSE},   /* player got junkie */
  {SND_MINIBOMB, "xbmbmb.raw", NULL, 0, FALSE, TRUE},   /* small bomb expl. */
  {SND_WON,      "xb_won.raw", NULL, 0, FALSE, FALSE},  /* player won */
  {SND_HAUNT,    "xb_haunt.raw", NULL, 0, FALSE, FALSE},/* haunting bomb */
  {SND_SPIRAL,   "xb_spir.raw", NULL, 0, FALSE, TRUE},  /* spiral shrinking */
  {SND_SPBOMB,   "xb_spbmb.raw", NULL, 0, FALSE, TRUE}, /* got special bomb */
  {SND_SLIDE,    "xbslide.raw", NULL, 0, FALSE, TRUE},  /* bomb slide sound */
  {SND_FINALE,   "xbfin.raw", NULL, 0, FALSE, FALSE},   /* final fanfare */
  {SND_WARN,     "xb_warn.raw", NULL, 0, FALSE, FALSE}, /* shrink warn sound */
  {SND_STUN,     "xb_stun.raw", NULL, 0, FALSE, FALSE}, /* player stun sound */
  {SND_WHIRL,    "xb_whrl.raw", NULL, 0, TRUE, FALSE},  /* intro whirl */
  {SND_COMPOUND, "xb_cmpnd.raw", NULL, 0, FALSE, FALSE},/* compound shrink */
  {SND_TELE1,    "xbtele1.raw", NULL, 0, FALSE, TRUE},  /* teleport start */
  {SND_TELE2,    "xbtele2.raw", NULL, 0, FALSE, TRUE},  /* teleport end */
  {SND_HOLY,     "xbholy.raw", NULL, 0, FALSE, FALSE},  /* holy grail extra */
  {SND_ENCLOAK,  "xbcloak.raw", NULL, 0, FALSE, TRUE},  /* encloak sound */
  {SND_DECLOAK,  "xbdcloak.raw", NULL, 0, FALSE, TRUE}, /* decloak sound */
  {SND_FAST,     "xbfast.raw", NULL, 0, FALSE, TRUE},   /* speed up extra */
  {SND_SLOW,     "xbslow.raw", NULL, 0, FALSE, TRUE},   /* slow down extra */
  {SND_SLAY,     "xbslay.raw", NULL, 0, FALSE, TRUE},   /* slay extra */
  {SND_LIFE,     "xblife.raw", NULL, 0, FALSE, TRUE},   /* extra life */
  {SND_NEWCLOAK, "xbcloakx.raw", NULL, 0, FALSE, TRUE}, /* new cloak extra */
  {SND_BOMBMORPH,"xb_bombmorph.raw", NULL, 0, FALSE, TRUE }, /* bomb morph */
  {SND_STEP1,     "xbstep1.raw", NULL, 0,  FALSE, TRUE },  /* Backgr. song #1 */
  {SND_STEP2,     "xbstep2.raw", NULL, 0, FALSE, TRUE },    /* Backgr. song #2 */
  {SND_STEP3,     "xbstep3.raw", NULL, 0, FALSE, TRUE },    /* Backgr. song #3 */
  {SND_STEP4,     "xbstep4.raw", NULL, 0, FALSE, TRUE },   /* Backgr. song #4 */
  {SND_STEP5,     "xbstep5.raw", NULL, 0, FALSE, TRUE },    /* Backgr. song #5 */
  {SND_STEP6,     "xbstep6.raw", NULL, 0, FALSE, TRUE },    /* Backgr. song #6 */
  {SND_SNG1,     "xbsng1.raw", NULL, 0, TRUE, FALSE},   /* Backgr. song #1 */
  {SND_SNG2,     "xbsng2.raw", NULL, 0, TRUE, FALSE},   /* Backgr. song #2 */
  {SND_SNG3,     "xbsng3.raw", NULL, 0, TRUE, FALSE},   /* Backgr. song #3 */
  {SND_SNG4,     "xbsng4.raw", NULL, 0, TRUE, FALSE},   /* Backgr. song #4 */
  {SND_SNG5,     "xbsng5.raw", NULL, 0, TRUE, FALSE},   /* Backgr. song #5 */
  {SND_SNG6,     "xbsng6.raw", NULL, 0, TRUE, FALSE},   /* Backgr. song #6 */
  {SND_MAX,     NULL, NULL, 0}              
};

#define SUBSIZE          2048
#define FRAGSIZE         0x0004000a

#define SOUND_DEVICE "/dev/dsp"
#define SAMPLE_RATE     22050
#define SAMPLE_CHANNELS     1
#define SAMPLE_SIZE         8

static s16 sumbuff[SUBSIZE];
static u8  playbuff[SUBSIZE];

static int mono_mode = FALSE;

static int fragsize =        FRAGSIZE;
static int sample_rate =     SAMPLE_RATE;
static int sample_channels = SAMPLE_CHANNELS;
static int sample_size =     SAMPLE_SIZE;

/*
 * outcomment the following line to suppress server statistics
 */
#define SERVER_STATISTICS

#if defined(SERVER_STATISTICS)
static double total_samples  = 0.0;
static int    total_played   = 0;
static int    total_skipped  = 0;
static int    total_loaded   = 0;
static int    total_unloaded = 0;
#endif

/*
 * Abort signal handler
 */
static void
server_abort()
{
#if defined(SERVER_STATISTICS)
  fprintf(stderr, "XBlast sound server statistics:\n");
  fprintf(stderr, "\tloaded %d sounds,\n", total_loaded);
  fprintf(stderr, "\tfreed %d sounds,\n", total_unloaded);
  fprintf(stderr, "\tplayed %d sounds,\n", total_played);
  fprintf(stderr, "\tskipped %d sounds,\n", total_skipped);
  fprintf(stderr, "\tprocessed %10.3f Mega samples on sound device.\n", 
	 total_samples / 1000000.0);
#endif
  fprintf(stderr, "XBlast sound server terminated.\n");
  fflush(stderr);
  exit(0);
}

/*
 * alarm signal handler for sound server calibration
 */
static void
calibration_stop()
{
  calibration_ready = TRUE;
}

/*
 * initialize sound device
 */
#if defined(__STDC__)
static void
init_dsp(int dsp)
#else
static void
init_dsp(dsp)
int dsp;
#endif
{
  if (ioctl(dsp, SNDCTL_DSP_SETFRAGMENT, &fragsize) < 0)
    {
      fprintf(stderr, "XBlast sound server: could not set fragment size %8x on sound device\n", fragsize);
    }
  if (ioctl(dsp, SNDCTL_DSP_STEREO, &sample_channels) < 0)
    {
      fprintf(stderr, "XBlast sound server: could not set %d sample channels on sound device\n", sample_channels);
    }
  if (ioctl(dsp, SNDCTL_DSP_SPEED, &sample_rate) < 0)
    {
      fprintf(stderr, "XBlast sound server: could not set sample rate %d on sound device\n", sample_rate);
    }
  if (ioctl(dsp, SNDCTL_DSP_SETFMT, &sample_size) < 0)
    {
      fprintf(stderr, "XBlast sound server: could not set sample size %d on sound device\n", sample_size);
    }
  if (ioctl(dsp, SNDCTL_DSP_GETBLKSIZE, &fragsize) < 0)
    {
      fprintf(stderr, "XBlast sound server: could not get block size of sound device\n");
    }
}


/*
 * resync sound device 
 */
#if defined(__STDC__)
static void 
resync(int dsp)
#else
static void
resync(dsp)
int dsp;
#endif
{
  /* clear sample sum buffer */
  register int i;
  register u8 *s = playbuff;
  
  for (i = 0; i < SUBSIZE; i++)
    {
      *s++ = 128;
    }
  /* resync sound device to correct any channel flipping */
  write(dsp, playbuff, SUBSIZE);
  write(dsp, playbuff, SUBSIZE);
  write(dsp, playbuff, SUBSIZE);
  (void) ioctl(dsp, SNDCTL_DSP_SYNC, NULL);
}

/*
 * load sound samples into server memory
 */
#if defined(__STDC__)
static int
server_load_sound(int number)
#else
static int
server_load_sound(number)
int number;
#endif
{
  char fname[1000];
  int i, f;
  static char *path_list[3] = {
    NULL,
    GAME_DATADIR"/",
    ".",
  };

  /* check environment for xblast search path */
  path_list[0] = getenv("XBLASTDIR");

  for (i=0; i<3; i++) 
    {
      if (path_list[i] != NULL) 
	{
	  sprintf(fname, "%s/%s/%s", path_list[i], "sounds", 
		  sound_name[number].name);
	  //	  fprintf(stderr," opening %s \n",fname);
	  if ((f = open(fname, O_RDONLY)) >= 0)
	    {
	      int sound_size;
	      u8 *sb;
	      struct stat snd_stat;

#ifdef DEBUG
	      fprintf(stderr, "Opened file \"%s\".\n", fname);
#endif	      
	      (void) fstat(f, &snd_stat);
	      sound_size = snd_stat.st_size / sizeof(u8);
	      if (sound_name[number].samples != NULL)
		{
		  free(sound_name[number].samples);
		  sound_name[number].samples = NULL;
		  sound_name[number].length  = 0;
		}
	      
	      if ((sb = (u8 *) malloc(sound_size * sizeof(u8))) == NULL)
		{
		  close(f);
		  return(-1);
		}
	      else
		{
		  read(f, sb, sound_size * sizeof(u8));
		  sound_name[number].samples = sb;
		  sound_name[number].length = sound_size;
		  close(f);
#if defined(SERVER_STATISTICS)
		  total_loaded++;
#endif
		  /*
		   * convert stereo samples to mono if running in mono mode 
		   */
		  if (mono_mode == TRUE && sound_name[number].mono == FALSE)
		    {
		      int i;
		      u8 *m, *s;
		      s16 sum;
		      
		      m = s = sound_name[number].samples;
		      
		      sound_name[number].length >>= 1;
		      for (i = 0; i < sound_name[number].length; i++)
			{
			  sum = *s + *(s + 1);
			  *m++ = sum >> 1;
			  s += 2;
			}
		    }
		  return(0);
		}
	    }
	}
    }
  fprintf(stderr, "could not open sound data file %s\n", 
	  sound_name[number].name);
  return(-1);
}

/*
 * free sample memory of a given sound
 */
#if defined(__STDC__)
static void
server_unload_sound(int id)
#else
static void
server_unload_sound(id)
int id;
#endif
{
  if (sound_name[id].samples != NULL)
    {
      free(sound_name[id].samples);
      sound_name[id].samples = NULL;
      sound_name[id].length  = 0;
#if defined(SERVER_STATISTICS)
      total_unloaded++;
#endif
    }
}

int RunningInstances(void){
  FILE *ptr;
  static char userPath[1024];
  char tmp[1024];
  unsigned int pid;

  char *home;
  /* set private xblast path */
  home = getenv ("HOME");
  if (NULL == home) {
    strcpy (userPath, "./user/xbsndsrv.pid");
  } else {
    sprintf (userPath, "%s/.xblast_tnt/xbsndsrv.pid", home);
  }
  ptr=fopen(userPath ,"r+");
  if(ptr==NULL){
    ptr=fopen(userPath ,"w+");
    sprintf(tmp,"%u",getpid());
    fwrite(tmp,1,strlen(tmp),ptr);
    fclose(ptr);
    return 0;

  }
  fscanf(ptr,"%u",&pid);
  fprintf(stderr," %i \n",pid);
  if(kill(pid,0)==-1 || pid<=0)
    {
      fseek(ptr,0,SEEK_SET);
      sprintf(tmp,"%u",getpid());
      fwrite(tmp,1,strlen(tmp),ptr);
      fclose(ptr);
      return 0;
    }
  else{
    return 1;
  }
    


}


/*
 * main function
 */
#if defined(__STDC__)
int
main(int argc, char **argv)
#else
int
main(argc, argv)
int argc;
char **argv;
#endif
{
  int dsp;
  int do_sync = TRUE;
  int did_sync = FALSE;
  int ack_val;


  if(RunningInstances()){
    fprintf(stderr,"xbsndsrv already running\n");
    return 1;
  }
  /*
   * open and prepare sound device
   */
  if ((dsp = open(SOUND_DEVICE, O_WRONLY)) < 0)
    {
      fprintf(stderr, "XBlast sound server: could not open sound device %s\n",
	      SOUND_DEVICE);
      ack_val = SND_ACK_ERROR;
      write(1, &ack_val, sizeof(ack_val));
      exit(-1);
    }
  ack_val = SND_ACK_OK;
  write(1, &ack_val, sizeof(ack_val));

  while (--argc > 0)
    {
      ++argv;
      if (!strcmp("-mono", *argv))
	{
	  mono_mode = TRUE;
	}
      else
	{
	  fprintf(stderr, "XBlast sound server: unknown option %s ignored\n",
		  *argv);
	}
    }

  if (mono_mode == TRUE)
    {
      sample_channels = 0;
    }

  init_dsp(dsp);
  /*
   * install server abort signal handler
   */
  signal(SIGINT, server_abort);

  fprintf(stderr, "XBlast sound server $Revision: 1.14 $ running in %s mode.\n",
	  (mono_mode == TRUE) ? "mono" : "stereo");


  /*
   * calibrate sound server to CPU power
   */
  signal(SIGALRM, calibration_stop);
  {
    unsigned long l = 0;
    int d1, d2, d3, d4, d5;
    calibration_ready = FALSE;
    alarm(1);
    while (calibration_ready == FALSE)
      {
	l++;
	/* do something similar to the "place a sample" loop */
	d5 = d2++ - 128;
	d1 = (d2 * (d3 + 1)) >> 4;
	d4 = (d2 * (d3 - MAX_SOUND_POSITION + 1)) >> 4;
	d2 += d1;
	d3 += d4;
      }
    max_sim_sounds = l / (SAMPLE_RATE * MAX_CPU_LOAD_ADJUST);
    l = d4 + d5; /* hope to fake C optimizer */

    /* calc. max playable sounds to something reasonable... */
    if (max_sim_sounds > ABS_MAX_SIM_SOUNDS)
      {
	max_sim_sounds = ABS_MAX_SIM_SOUNDS;
      }
    else if (max_sim_sounds < DEFAULT_MAX_SIM_SOUNDS)
      {
	max_sim_sounds = DEFAULT_MAX_SIM_SOUNDS;
      }
  }
   
  /*
   * allocate memory for playing sounds table
   */
  fprintf(stderr, "Xblast Sound Server: playing at most %d sounds simultaneously\n", max_sim_sounds);
  fflush(stderr);
  if ((sound_table = (XBSOUND *) malloc(max_sim_sounds * sizeof(XBSOUND))) 
      == NULL)
    {
      fprintf(stderr, "XBlast Sound Server: not enough memory to allocate sound table\n");
      exit(-1);
    }
  else
    {
      /* initialze sound table */
      XBSOUND *s = sound_table;
      int i;
      for (i = 0; i < max_sim_sounds; i++, s++)
	{
	  s->length = 0;
	}
    }

  /*
   * loop forever (or SIGINT)
   */
  while (1)
    {
      /* clear sample sum buffer */
      {
	register int i;
	register s16 *s = sumbuff;

	for (i = 0; i < SUBSIZE; i++)
	  {
	    *s++ = 128;
	  }
      }

      if (sounds_playing <= 0)
	{
	  /* no sound to play, may sync */
	  do_sync = TRUE;
	}
      else
	{
	  do_sync = FALSE;
	  did_sync = FALSE;
	}

      /* sum samples in sumup buffer */
      if (mono_mode == TRUE)
	{
	  /*
	   * process sounds in mono mode
	   */
	  XBSOUND *xs = sound_table;
	  int as;

	  for (as = 0; as < max_sim_sounds; as++)
	    {
	      if (xs->length > 0)
		{
		  int i;
		  register s16 *s = sumbuff;
		  
		  for (i = 0; i < SUBSIZE && xs->length > 0; i++, xs->length--)
		    {
		      *s++ += ((s16) *xs->samples++) - 128;
		    }

#if defined(SERVER_STATISTICS)
		  total_samples += (double) i;
#endif
		  /* repeat a sound if this is required */
		  if (xs->length <= 0)
		    {
		      if (xs->repeat == TRUE)
			{
			  int id = xs->id;
			  xs->length  = sound_name[id].length;
			  xs->samples = sound_name[id].samples;
			}
		      else
			{
			  next_free_sound = as;
			  sounds_playing--;
			}
		    }
		}
	      xs++;
	    }
	      
	  /* correct clipping */
	  {
	    register int i;
	    register s16 *s = sumbuff;
	    for (i = 0; i < SUBSIZE; i++)
	      {
		if (*s > 255)
		  {
		    *s = 255;
		  }
		else if (*s < 0)
		  {
		    *s = 0;
		  }
		s++;
	      }
	  }
      
	  /* copy sum buffer to playback buffer and play it */
	  {
	    register u8 *d = playbuff;
	    register s16 *s = sumbuff;
	    register int i;
	    
	    for (i = 0; i < SUBSIZE; i++)
	      {
		*d++ = (u8) *s++;
	      }
	    
	    /* play buffer */
	    write(dsp, playbuff, SUBSIZE);
	  }
	}
      else
	{
	  /*
	   * process sounds in stereo  mode
	   */
	  XBSOUND *xs = sound_table;
	  int as;

	  for (as = 0; as < max_sim_sounds; as++)
	    {
	      if (xs->length > 0)
		{
		  int i;
		  register s16 *s = sumbuff;
	      
		  for (i = 0; i < SUBSIZE && xs->length > 0; i++, xs->length--)
		    {
		      if (xs->mono == TRUE)
			{
			  /* calc. position of mono sounds and add to sumup buffer */
			  int pos    = xs->position;
			  s16 sample = ((s16) *xs->samples++) - 128;
			  s16 sr, sl;
		      
			  sr = (sample*(pos+1)) >> 4;
			  sl = (sample*(pos-MAX_SOUND_POSITION+1)) >> 4;
			  *s++ += sl;
			  *s++ += sr;
			  i++;
			}
		      else
			{
			  *s++ += ((s16) *xs->samples++) - 128;
			}
		    }
#if defined(SERVER_STATISTICS)
		  total_samples += (double) i;
#endif
		  /* repeat a sound if this is required */
		  if (xs->length <= 0)
		    {
		      if (xs->repeat == TRUE)
			{
			  int id = xs->id;
			  xs->length  = sound_name[id].length;
			  xs->samples = sound_name[id].samples;
			}
		      else
			{
			  sounds_playing--;
			  next_free_sound = as;
			}
		    }
		}
	      xs++;
	    }
	  
	  /* correct clipping */
	  {
	    register int i;
	    register s16 *s = sumbuff;
	    for (i = 0; i < SUBSIZE; i++)
	      {
		if (*s > 255)
		  {
		    *s = 255;
		  }
		else if (*s < 0)
		  {
		    *s = 0;
		  }
		s++;
	      }
	  }
      
	  /* copy sum buffer to playback buffer and play it */
	  {
	    register u8 *d = playbuff;
	    register s16 *s = sumbuff;
	    register int i;
	    
	    for (i = 0; i < SUBSIZE; i++)
	      {
		*d++ = (u8) *s++;
	      }
	    
	    /* play buffer */
	    write(dsp, playbuff, SUBSIZE);
	  }
	}
#if 1
      /* resync sound device to correct any channel flipping */
      if (do_sync == TRUE && did_sync == FALSE)
	{
	  (void) ioctl(dsp, SNDCTL_DSP_SYNC, NULL);
	  did_sync = TRUE;
	}
#endif

      /* check for new commands in input pipe */
      {
	int command_buff[1024];
	int *cmd;
	int n;
	int ret;
	struct timeval tv;
	fd_set rs;

	while(1)
	  {
	    
	    tv.tv_sec = tv.tv_usec = 0;
	    FD_ZERO(&rs);
	    FD_SET(0, &rs);
	    
	    if (select(1, &rs, NULL, NULL, &tv) > 0 && FD_ISSET(0, &rs))
	      {
		n = read(0, command_buff, 8);
		if ((n == 0) || (n < 0 && errno != EINTR && errno != EAGAIN)) {
		  fprintf(stderr, "Parent was killed, bailing out ...\n");
		  exit(0);
		}
		cmd = command_buff;
		/* there are commands in the pipe */
		while (n > 0)
		  {
		    if (*cmd == SND_PLAY_SOUND)
		      {
			int id = *++cmd;
			
			if (sounds_playing < max_sim_sounds)
			  {
			    XBSOUND *xs = sound_table + next_free_sound;
			    /* append new sound to play to play list */
#if defined(SERVER_STATISTICS)
			    total_played++;
#endif
			    while (xs->length > 0)
			      {
				next_free_sound++;
				if (next_free_sound >= max_sim_sounds)
				  {
				    next_free_sound = 0;
				  }
				xs = sound_table + next_free_sound;
			      }
			    xs->id        = id & 0xffff;
			    xs->position  = (id >> 16) & 0xffff;
			    id           &= 0xffff;
			    xs->repeat    = sound_name[id].repeat;
			    xs->mono      = sound_name[id].mono;
			    xs->samples   = sound_name[id].samples;
			    xs->length    = sound_name[id].length;
			    xs ->next     = NULL;
			    sounds_playing++;
			  }
			else
			  {
			    total_skipped++;
			  }
		      }
		    else if (*cmd == SND_STOP_SOUND)
		      {
			int stop_id = *++cmd;
			
			if (stop_id == 0 || stop_id == STOP_ALL_SOUNDS)
			  {
			    XBSOUND *s = sound_table;
			    int i;
			    for (i = 0; i < max_sim_sounds; i++, s++)
			      {
				s->length = 0;
				s->repeat = 0;
			      }
			    sounds_playing = 0;
			    resync(dsp);
			  }
			else
			  {
			    XBSOUND *s = sound_table;
			    int i;
			    for (i = 0; i < max_sim_sounds; i++, s++)
			      {
				if (s->id == stop_id)
				  {
				    s->length = 0;
				    s->repeat = 0;
				    sounds_playing--;
				  }
			      }
			  }
			write(1, cmd, 1);
		      }
		    else if (*cmd == SND_LOAD_SOUND)
		      {
			server_load_sound(*++cmd);
			write(1, cmd, 1);
		      }
		    else if (*cmd == SND_UNLOAD_SOUND)
		      {
			server_unload_sound(*++cmd);
			write(1, cmd, 1);
		      }
		    n -= 2 * sizeof(int);
		    cmd++;
		  }
	      }
	    else
	      {
		break;
	      }
	  }
      }
    }
}

#endif
