/*
 * file w32_sndsrv.c - sound library for xblast
 *
 * $Id: w32_sndsrv.h,v 1.4 2006/02/09 21:21:25 fzago Exp $
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
#ifndef _W32_SNDSRV_H
#define _W32_SNDSRV_H

#include "w32_common.h"
#include "snd.h"


/*
 * global macros
 */
#define MSG_XBLAST_SOUND_DONE      WM_USER
#define MSG_XBLAST_SOUND_PLAY     (WM_USER+1)
#define MSG_XBLAST_SOUND_STOP     (WM_USER+2)
#define MSG_XBLAST_SOUND_LOAD     (WM_USER+3)
#define MSG_XBLAST_SOUND_SHUTDOWN (WM_USER+4)
#define MSG_XBLAST_SOUND_CLOSE    (WM_USER+5)

#define MSG_XBLAST_SOUND_FIRST     WM_USER
#define MSG_XBLAST_SOUND_LAST     (WM_USER+5)

/*
 * global prototypes
 */
extern XBBool SoundCheckWaveOut (XBBool stereo);
extern DWORD PASCAL _export SoundThreadStereo (void *);
extern char *SoundLoadFile (SND_Id id);

#endif
/*
 * end of file w32_sndsrv.h
 */
