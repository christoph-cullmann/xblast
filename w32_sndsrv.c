/*
 * file w32_sndsrv.c - sound library for xblast
 *
 * $Id: w32_sndsrv.c,v 1.4 2004/08/12 21:31:48 iskywalker Exp $
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
#include "w32_sndsrv.h"
#include "snd.h"

//#include "w32_mm.h"

#include "util.h"

/*
 * local macros
 */
#define WAVE_BUFFER_SIZE   2210
#define NUM_WAVE_HEADERS   4

/*
 * local types
 */
typedef struct {
  SND_Id         id;      /* the sound's id to refer to it */
  const char    *name;    /* raw samples data file name */
  char          *samples; /* pointer to samples memory */
  size_t         length;  /* length in samples of the sound */
  XBBool         repeat;  /* repeat flag to play sound endlessly */
  XBBool         mono;    /* mono flag indicating mono sounds */
} SoundInfo;

typedef struct _sound_play {
  struct _sound_play *next;
  SND_Id              id;     /* reference to effect */
  short               left;   /* left speaker volume */
  short               right;  /* right speaker volume */
  size_t              pos;    /* current data position */
} SoundItem;

typedef struct {
  HGLOBAL        hHdr;
  WAVEHDR       *pHdr;
  HGLOBAL        hBuf;
  char          *pBuf;
} HeaderInfo;

/*
 * local variables
 */
static HWAVEOUT    hWaveOut;
static SoundItem  *playList  = NULL;
static unsigned    idThread;
static XBBool      finished  = XBFalse;
static HeaderInfo  header[NUM_WAVE_HEADERS];
static short       mixBuffer[WAVE_BUFFER_SIZE];

/* wave formats for stereo or mono */
static WAVEFORMATEX waveFormatMono = {
  WAVE_FORMAT_PCM, 1, 22050, 22050, 1 , 8, 0
};
static WAVEFORMATEX waveFormatStereo = {
  WAVE_FORMAT_PCM, 2, 22050, 44100, 2 , 8, 0
};

/*
 * sound sample table
 */
static SoundInfo soundInfo[SND_MAX] = {
  {SND_BAD,      "xb_bad",       NULL, 0, XBFalse, XBFalse}, /* got a skull */
  {SND_DROP,     "xb_drop",      NULL, 0, XBFalse, XBTrue},  /* dropped a bomb */
  {SND_NEWBOMB,  "xbnbmb",       NULL, 0, XBFalse, XBTrue},  /* got an extra bomb */
  {SND_NEWKICK,  "xbnkick",      NULL, 0, XBFalse, XBTrue},  /* got kick extra */
  {SND_NEWPUMP,  "xbnpmp",       NULL, 0, XBFalse, XBTrue},  /* got pump extra */
  {SND_NEWRC,    "xbnrc",        NULL, 0, XBFalse, XBTrue},  /* got rem. control */
  {SND_MOREFIRE, "xbfire",       NULL, 0, XBFalse, XBTrue},  /* got more range */
  {SND_DEAD,     "xb_dead",      NULL, 0, XBFalse, XBFalse}, /* player died */
  {SND_EXPL,     "xb_expl",      NULL, 0, XBFalse, XBTrue},  /* normal explosion */
  {SND_KICK,     "xb_kick",      NULL, 0, XBFalse, XBTrue},  /* kick a bomb */
  {SND_PUMP,     "xb_pump",      NULL, 0, XBFalse, XBTrue},  /* pump a bomb */
  {SND_OUCH,     "xb_ouch",      NULL, 0, XBFalse, XBFalse}, /* player lost life */
  {SND_INTRO,    "xb_intro",     NULL, 0, XBFalse, XBFalse}, /* intro fanfare */
  {SND_APPL,     "xb_appl",      NULL, 0, XBFalse, XBFalse}, /* applause */
  {SND_APPL2,    "xb_app2",      NULL, 0, XBFalse, XBFalse}, /* applause */
  {SND_BUTT,     "xb_butt",      NULL, 0, XBFalse, XBTrue},  /* triggered button */
  {SND_SHOOT,    "xb_shoot",     NULL, 0, XBFalse, XBFalse}, /* using rem. ctrl. */
  {SND_INVIS,    "xb_nvis",      NULL, 0, XBFalse, XBFalse}, /* player invisible */
  {SND_INVINC,   "xb_nvnc",      NULL, 0, XBFalse, XBFalse}, /* player invincible */
  {SND_NEWTELE,  "xbntel",       NULL, 0, XBFalse, XBTrue},  /* player got telep. */
  {SND_TELE,     "xbtele",       NULL, 0, XBFalse, XBTrue},  /* player uses tele. */
  {SND_INJ,      "xbinj",        NULL, 0, XBFalse, XBFalse}, /* player got junkie */
  {SND_MINIBOMB, "xbmbmb",       NULL, 0, XBFalse, XBTrue},  /* small bomb expl. */
  {SND_WON,      "xb_won",       NULL, 0, XBFalse, XBFalse}, /* player won */
  {SND_HAUNT,    "xb_haunt",     NULL, 0, XBFalse, XBFalse}, /* haunting bomb */
  {SND_SPIRAL,   "xb_spir",      NULL, 0, XBFalse, XBTrue},  /* spiral shrinking */
  {SND_SPBOMB,   "xb_spbmb",     NULL, 0, XBFalse, XBTrue},  /* got special bomb */
  {SND_SLIDE,    "xbslide",      NULL, 0, XBFalse, XBTrue},  /* bomb slide sound */
  {SND_FINALE,   "xbfin",        NULL, 0, XBFalse, XBFalse}, /* final fanfare */
  {SND_WARN,     "xb_warn",      NULL, 0, XBFalse, XBFalse}, /* shrink warn sound */
  {SND_STUN,     "xb_stun",      NULL, 0, XBFalse, XBFalse}, /* player stun sound */
  {SND_WHIRL,    "xb_whrl",      NULL, 0, XBTrue,  XBFalse}, /* intro whirl */
  {SND_COMPOUND, "xb_cmpnd",     NULL, 0, XBFalse, XBFalse}, /* compound shrink */
  {SND_TELE1,    "xbtele1",      NULL, 0, XBFalse, XBTrue},  /* teleport start */
  {SND_TELE2,    "xbtele2",      NULL, 0, XBFalse, XBTrue},  /* teleport end */
  {SND_HOLY,     "xbholy",       NULL, 0, XBFalse, XBFalse}, /* holy grail extra */
  {SND_ENCLOAK,  "xbcloak",      NULL, 0, XBFalse, XBTrue},  /* encloak sound */
  {SND_DECLOAK,  "xbdcloak",     NULL, 0, XBFalse, XBTrue},  /* decloak sound */
  {SND_FAST,     "xbfast",       NULL, 0, XBFalse, XBTrue},  /* speed up extra */
  {SND_SLOW,     "xbslow",       NULL, 0, XBFalse, XBTrue},  /* slow down extra */
  {SND_SLAY,     "xbslay",       NULL, 0, XBFalse, XBTrue},  /* slay extra */
  {SND_LIFE,     "xblife",       NULL, 0, XBFalse, XBTrue},  /* extra life */
  {SND_NEWCLOAK, "xbcloakx",     NULL, 0, XBFalse, XBTrue},  /* new cloak extra */
  {SND_BOMBMORPH,"xb_bombmorph", NULL, 0, XBFalse, XBTrue},  /* bomb morph */
  {SND_STEP1,     "xbstep1", NULL, 0,  XBFalse, XBTrue },  /* steps #1 */
  {SND_STEP2,     "xbstep2", NULL, 0, XBFalse, XBTrue },    /* steps #2 */
  {SND_STEP3,     "xbstep3", NULL, 0, XBFalse, XBTrue },    /* steps #3 */
  {SND_STEP4,     "xbstep4", NULL, 0, XBFalse, XBTrue },   /* steps #4 */
  {SND_STEP5,     "xbstep5", NULL, 0, XBFalse, XBTrue },    /* steps #5 */
  {SND_STEP6,     "xbstep6", NULL, 0, XBFalse, XBTrue },    /* steps #6 */
  {SND_SNG1,     "xbsng1", NULL, 0, XBTrue, XBFalse},   /* Backgr. song #1 */
  {SND_SNG2,     "xbsng2", NULL, 0, XBTrue, XBFalse},   /* Backgr. song #2 */
  {SND_SNG3,     "xbsng3", NULL, 0, XBTrue, XBFalse},   /* Backgr. song #3 */
  {SND_SNG4,     "xbsng4", NULL, 0, XBTrue, XBFalse},   /* Backgr. song #4 */
  {SND_SNG5,     "xbsng5", NULL, 0, XBTrue, XBFalse},   /* Backgr. song #5 */
  {SND_SNG6,     "xbsng6", NULL, 0, XBTrue, XBFalse},   /* Backgr. song #6 */
};

/*
 * check waveout capabilities
 */
XBBool
SoundCheckWaveOut (XBBool stereo)
{
  WAVEFORMATEX *pFormat = stereo ? &waveFormatStereo : &waveFormatMono;

  if (0 == waveOutOpen (NULL, WAVE_MAPPER, pFormat, 0, 0, WAVE_FORMAT_QUERY) ) {
    return XBTrue;
  } else {
    return XBFalse;
  }
} /* CheckWaveOut */

/*
 * Callback for audio device
 */
void PASCAL _export
WaveProc (HWAVE hWave, UINT msg, unsigned inst, unsigned par1, unsigned par2)
{
  /*  assert (hWave == hWaveOut); */

  switch (msg) {
  case MM_WOM_DONE:
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_DONE, 0, par1);
    break;
  case MM_WOM_CLOSE:
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_CLOSE, 0, 0);
    break;
  }
} /* WaveProc */

/*
 * initialize waveform headers
 */
static XBBool
InitHeader (HeaderInfo *header)
{
  static XBBool first = XBTrue;
  assert (NULL != header);
  
  /* header */
  header->hHdr = GlobalAlloc (GMEM_MOVEABLE, sizeof (WAVEHDR));
  assert (header->hHdr != NULL);
  header->pHdr = GlobalLock (header->hHdr);
  assert (header->pHdr != NULL);
  memset (header->pHdr, 0, sizeof (WAVEHDR));
  /* data */
  header->hBuf = GlobalAlloc (GMEM_MOVEABLE, WAVE_BUFFER_SIZE);
  assert (header->hBuf != NULL);
  header->pBuf = GlobalLock (header->hBuf);
  assert (header->pBuf != NULL);
  memset (header->pBuf, 0x80, WAVE_BUFFER_SIZE);
  /* set header data */
  header->pHdr->lpData         = header->pBuf; 
  header->pHdr->dwBufferLength = WAVE_BUFFER_SIZE; 
  /* Prepare and Write header */
  if (0 != waveOutPrepareHeader (hWaveOut, header->pHdr, sizeof (WAVEHDR) ) ) {
    printf ("prepare header failed\n");
    return XBFalse;
  }
  if (0 != waveOutWrite (hWaveOut, header->pHdr, sizeof (WAVEHDR) ) ) {
    printf ("write failed\n");
    return XBFalse;
  }
  first = ! first;
  return XBTrue;
} /* InitHeader */

/*
 * 
 */
static void
FinishHeader (HeaderInfo *header)
{
  assert (header != NULL);
  
  if (NULL != header->hBuf) {
    GlobalUnlock (header->hBuf);
    GlobalFree (header->hBuf);
    header->hBuf = NULL;
    header->pBuf = NULL;
  }
  if (NULL != header->hHdr) {
    GlobalUnlock (header->hHdr);
    GlobalFree (header->hHdr);
    header->hHdr = NULL;
    header->pHdr = NULL;
  }
} /* FinishHeader */

/*
 * Add single sound to buffer
 */
XBBool
AddSound (short *buf, SoundItem *ptr)
{
  size_t  i;
  size_t  nBytes;
  size_t  totalBytes = 0;
  char   *src;

  assert (NULL != buf);
  assert (NULL != ptr);
  assert (SND_MAX >= ptr->id);
  assert (NULL != soundInfo[ptr->id].samples);

  if (soundInfo[ptr->id].mono) {
    /* 
     * 1 channel 8 bits source 
     */
    /* number of bytes to add */
    if (soundInfo[ptr->id].length - ptr->pos > WAVE_BUFFER_SIZE/2) {
      nBytes = WAVE_BUFFER_SIZE/2;
    } else {
      nBytes = soundInfo[ptr->id].length - ptr->pos;
    }
    /* add on both channels */
    src = soundInfo[ptr->id].samples + ptr->pos;
    for (i = 0; i < nBytes; i ++) {
      buf[2*i]   += ptr->left  * src[i];
      buf[2*i+1] += ptr->right * src[i];
    }
    ptr->pos += nBytes;
  } else {
    /* 
     * 2 channel 8 bits source 
     */
    do {
      /* number of bytes to add */
      if (soundInfo[ptr->id].length - ptr->pos > WAVE_BUFFER_SIZE - totalBytes) {
	nBytes = WAVE_BUFFER_SIZE - totalBytes;
      } else {
	nBytes = soundInfo[ptr->id].length - ptr->pos;
      }
      /* add on both channels */
      src = soundInfo[ptr->id].samples + ptr->pos;
      for (i = 0; i < nBytes; i ++) {
	buf[i + totalBytes] +=  ptr->left * src[i];
      }
      ptr->pos += nBytes;
      if (soundInfo[ptr->id].repeat) {
	totalBytes += nBytes;
	if (ptr->pos == soundInfo[ptr->id].length) {
	  ptr->pos = 0;
	}
      } else {
	totalBytes = WAVE_BUFFER_SIZE;
      }
    } while (totalBytes < WAVE_BUFFER_SIZE);
  }
  return (ptr->pos < soundInfo[ptr->id].length);
} /* AddSound */

/*
 * one buffer is finished do it again
 */
static void
SoundDone (const WAVEHDR *hdr)
{
  int i, j;
  SoundItem *list = NULL;
  SoundItem *play, *next;
  char  *dst;
  short *src;
  short  val;

  /* find header */
  for (i = 0; i < NUM_WAVE_HEADERS; i ++) {
    if (header[i].pHdr == hdr) {
      break;
    }
  }
  if (i >= NUM_WAVE_HEADERS) {
    fprintf (stderr, "failed to identify header\n");
    return;
  }
  /* clean up */
  if (0 != waveOutUnprepareHeader (hWaveOut, header[i].pHdr, sizeof (WAVEHDR) ) ) {
    return;
  }
  /* add sound effects */
  if (NULL == playList) {
    memset (header[i].pBuf, 0x80, WAVE_BUFFER_SIZE);
  } else {
    memset (mixBuffer, 0x00, 2*WAVE_BUFFER_SIZE);
    for (play = playList; play != NULL; play = next) {
      next = play->next;
      if (AddSound (mixBuffer, play) ) {
        /* continue with this sound */
        play->next = list;
        list       = play;
      } else {
        /* no longer play this sound */
        free (play);
      }
    }
    playList = list;
    /* copy to waveout buffer */
    src = mixBuffer;
    dst = header[i].pBuf;
    for (j = 0; j < WAVE_BUFFER_SIZE; j ++) {
      val = src[j] / 16 + 128;
      if (val < 0) {
        dst[j] = 0;
      } else if (val > 255) {
        dst[j] = (unsigned char) 255;
      } else {
        dst[j] = (unsigned char) val;
      }
    }
  }
  /* write it again  */
  if (! finished) {
    if (0 != waveOutPrepareHeader (hWaveOut, header[i].pHdr, sizeof (WAVEHDR) ) ) {
      printf ("prepare header failed\n");
      return;
    }
    if (0 != waveOutWrite (hWaveOut, header[i].pHdr, sizeof (WAVEHDR) ) ) {
      printf ("write failed\n");
      return;
    }
  }
} /* SoundDone */

/*
 * add this sound to play list
 */
static void
SoundPlay (SND_Id id, short left, short right)
{
  SoundItem *ptr;

  /* 1. search for same entry with offset 0 */
  for (ptr = playList; ptr != NULL; ptr = ptr->next) {
    if (ptr->id == id && ptr->pos == 0) {
      break;
    }
  }
  /* 2. if no one was found create new one */
  if (NULL == ptr) {
    ptr = calloc (1, sizeof (SoundItem));
    assert (NULL != ptr);
    ptr->id   = id;
    ptr->next = playList;
    playList  = ptr;
  }
  /* 3. add own amplitudes */
  ptr->left  += left;
  ptr->right += right;
} /* SoundPlay */

/*
 * remove this sound to play list
 */
static void
SoundStop (SND_Id id)
{
  SoundItem *list = NULL;
  SoundItem *ptr, *next;

  /* delete all sounds of given type */
  for (ptr = playList; ptr != NULL; ptr = next) {
    next = ptr->next;
    if (ptr->id == id) {
      free (ptr);
    } else {
      ptr->next = list;
      list      = ptr;
    }
  }
  playList = list;
} /* SoundStop */

/*
 * unload sound 
 */ 
static void
SoundLoad (SND_Id id, char *samples)
{
  assert (soundInfo[id].id == id);
  
  if (NULL != soundInfo[id].samples) {
    free (soundInfo[id].samples);
  }
  soundInfo[id].samples = samples;
} /* SoundUnload */

/*
 * shutdown wave output
 */
static void
SoundShutdown ()
{
  int i;

  finished = XBTrue;
  waveOutReset (hWaveOut);
  waveOutClose (hWaveOut);
  hWaveOut = NULL;

  for (i = 0; i < NUM_WAVE_HEADERS; i ++) {
    FinishHeader (header + i);
  }
} /* SoundShutdown */


/*
 * thread start function
 */
DWORD PASCAL _export 
SoundThreadStereo (void *par)
{
  int i;
  MSG msg;

  /* get own id */
  idThread = GetCurrentThreadId ();
  /* open device */
  if (0 != waveOutOpen (&hWaveOut, WAVE_MAPPER, &waveFormatStereo, (DWORD) WaveProc, 0, CALLBACK_FUNCTION) ) {
    fprintf (stderr, "failed to open waveout device\n");
    return 0;
  }
  /* initialize two wave headers */
  for (i = 0; i < NUM_WAVE_HEADERS; i ++) {
    if (! InitHeader (header + i) ) {
      fprintf (stderr, "failed to init waveout headers\n");
      return 0;
    }
  }
  /* wait for messages */
  while (GetMessage (&msg, NULL, MSG_XBLAST_SOUND_FIRST, MSG_XBLAST_SOUND_LAST)) {
    switch (msg.message) {
    case MSG_XBLAST_SOUND_DONE:
      SoundDone ((WAVEHDR *) msg.lParam);
      break;
    case MSG_XBLAST_SOUND_PLAY:
      SoundPlay ((SND_Id) msg.wParam, (short) LOWORD (msg.lParam), (short) HIWORD (msg.lParam) );
      break;
    case MSG_XBLAST_SOUND_STOP:
      SoundStop ((SND_Id) msg.wParam);
      break;
    case MSG_XBLAST_SOUND_LOAD:
      SoundLoad ((SND_Id) msg.wParam, (char *) msg.lParam);
      break;
    case MSG_XBLAST_SOUND_SHUTDOWN:
      SoundShutdown ();
      break;
    case MSG_XBLAST_SOUND_CLOSE:
      /* TODO clean up */
      hWaveOut = NULL;
      return 1;
    }
  }
  return 1;
} /* SoundThreadStereo */

/*
 * load a sound file from disk
 */
char *
SoundLoadFile (SND_Id id)
{
  assert (soundInfo[id].id == id);
  return ReadRawFile ("sounds", soundInfo[id].name, &soundInfo[id].length);
} /* SoundLoad */

/*
 * end of file w32_sndsrv.c
 */
