/*
 * file w32_sound.c - Win32 sound interface for xblast
 *
 * $Id: w32_sound.c,v 1.2 2004/05/14 10:00:36 alfie Exp $
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
#include "snd.h"

#include "w32_sndsrv.h"

/*
 * local variables
 */
/* thread for sound playback */
static HANDLE   hThread  = NULL;
static DWORD    idThread = 0;
/* volume tables for stereo sound */
static short leftVol[MAX_SOUND_POSITION] = {
  16, 16, 16, 16, 16, 16, 16,
  16,
  14, 12, 10,  8,  6,  4,  2,
};
static short rightVol[MAX_SOUND_POSITION] = {
  2,  4,  6,  8, 10, 12, 14,
  16,
  16, 16, 16, 16, 16, 16, 16,
};
static short beepMode = XBFalse;
static short doBeep   = XBFalse;

/* 
 * start thread for sound playback
 */
XBBool 
SND_Init (const CFGSoundSetup *setup)
{
  LPTHREAD_START_ROUTINE threadFunc = NULL;

  assert (NULL != setup);
  /* determine mode to use */
  switch (setup->mode) {
  case XBSM_Waveout: 
    /* TODO: program mono support */
    if (SoundCheckWaveOut (XBTrue)) {
      threadFunc = SoundThreadStereo;
    }
    beepMode = XBFalse;
    break;
  case XBSM_Beep:
    beepMode = XBTrue;
    break;
  case XBSM_None:
    beepMode = XBFalse;
    break;
  default:
    break;
  }
  if (NULL != threadFunc) {
    hThread = CreateThread (NULL, 0, threadFunc, NULL, 0, &idThread);
    if (NULL == hThread) {
      fprintf (stderr, "failed to start thread for sound\n");
      return XBFalse;
    }
    if (! SetThreadPriority (hThread, THREAD_PRIORITY_TIME_CRITICAL) ) {
      fprintf (stderr, "failed to set priority of thread\n");
    }
  }
  return XBTrue;
} /* SND_Init */

/*
 * stop to play given sound effect
 */
XBBool 
SND_Stop (SND_Id id)
{
  if (0 != idThread) {
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_STOP, id, 0);
  } 
  return XBTrue;
} /* SND_Stop */

/*
 * play a given sound
 */
XBBool 
SND_Play (SND_Id id, int pos)
{
  if (0 != idThread) {
    assert (pos >= 0);
    assert (pos < MAX_SOUND_POSITION);
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_PLAY, id, MAKELPARAM (leftVol[pos], rightVol[pos]) );
  } else if (beepMode) {
    if (id == SND_EXPL || id == SND_MINIBOMB) {
      doBeep = XBTrue;
    }
  } 
  return XBTrue;
} /* SND_Play */

/*
 * unload given sound
 */
XBBool 
SND_Unload (SND_Id id)
{
  if (0 != idThread) {
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_LOAD, id, 0);
  }
  return XBTrue;
} /* SND_Unload */

/*
 * flushing (not needed in win 32)
 */
void 
SND_Flush (void)
{
  if (beepMode && doBeep) {
    doBeep = XBFalse;
    MessageBeep (0xFFFFFFFF);
  }
} /* SND_Flush */

/*
 * close thread etc
 */
void 
SND_Finish (void)
{
  if (0 != idThread) {
    /* send terminate message to thread */
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_SHUTDOWN, 0, 0);
    /* wait for its termination */
    WaitForSingleObject (hThread, INFINITE);
    /* clean up */
    CloseHandle (hThread);
    /* reset values */
    idThread = 0;
    hThread  = NULL;
  }  
} /* SND_Finish */

/*
 *
 */
XBBool 
SND_Load (SND_Id id)
{
  if (0 != idThread) {
    PostThreadMessage (idThread, MSG_XBLAST_SOUND_LOAD, id, (DWORD) SoundLoadFile (id));
  }
  return XBTrue;
} /* SND_Load */

/*
 * end of file w32_sound.c
 */
