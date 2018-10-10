/*
 * file snd.h - sound library for xblast
 *
 * $Id: snd.h,v 1.5 2004/08/12 21:20:27 iskywalker Exp $
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

#ifndef XBLAST_SND_H
#define XBLAST_SND_H

#include "cfg_xblast.h"

/*
 * type definitions
 */

/* sound samples */
typedef enum {
  SND_BAD,
  SND_DROP,
  SND_NEWBOMB,
  SND_NEWKICK,
  SND_NEWPUMP,
  SND_NEWRC,
  SND_MOREFIRE,
  SND_DEAD,
  SND_EXPL,
  SND_KICK,
  SND_PUMP,
  SND_OUCH,
  SND_INTRO,
  SND_APPL,
  SND_APPL2,
  SND_BUTT,
  SND_SHOOT,
  SND_INVIS,
  SND_INVINC,
  SND_NEWTELE,
  SND_TELE,
  SND_INJ,
  SND_MINIBOMB,
  SND_WON,
  SND_HAUNT,
  SND_SPIRAL,
  SND_SPBOMB,
  SND_SLIDE,
  SND_FINALE,
  SND_WARN,
  SND_STUN,
  SND_WHIRL,
  SND_COMPOUND,
  SND_TELE1,
  SND_TELE2,
  SND_HOLY,
  SND_ENCLOAK,
  SND_DECLOAK,
  SND_FAST,
  SND_SLOW,
  SND_SLAY,
  SND_LIFE,
  SND_NEWCLOAK,
  SND_BOMBMORPH,
  SND_STEP1,
  SND_STEP2,
  SND_STEP3,
  SND_STEP4,
  SND_STEP5,
  SND_STEP6,
  SND_SNG1,
  SND_SNG2,
  SND_SNG3,
  SND_SNG4,
  SND_SNG5,
  SND_SNG6,
  /*---*/
  SND_MAX
} SND_Id;

/* 
 * Sound positions in the stereo panorama range from 0 (most left) to 
 * 16 (most right) with 8 as middle position.
 */
#define MAX_SOUND_POSITION     15
#define SOUND_MIDDLE_POSITION  7


/* 
 * function protoypes 
 */
extern XBBool SND_Init (const CFGSoundSetup *);
extern XBBool SND_Stop (SND_Id id);
extern XBBool SND_Play (SND_Id id, int position);
extern XBBool SND_Load (SND_Id id);
extern XBBool SND_Unload (SND_Id id);
extern void SND_Flush (void);
extern void SND_Finish (void);

#endif

/*
 * end of file snd.h
 */
