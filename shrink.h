/*
 * file shrink.h - level shrink and scrambling blocks
 *
 * $Id: shrink.h,v 1.7 2004/11/29 14:44:49 lodott Exp $
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

#ifndef _SHRINK_H
#define _SHRINK_H

#include "ini_file.h"
#include "map.h"

/*
 * prototypes
 */
extern XBBool ConfigLevelShrink (const DBSection *);
extern void FinishLevelShrink (void);
extern void DoShrink (int g_time);
extern void DoShrinkMapEdit (int g_time);
extern XBBool ParseLevelShrink (const DBSection *section, DBSection *) ;

/* generic shrink data */
typedef struct {
  int time;
  BMMapTile block;
  short x,y;
} ShrinkGeneric;
extern ShrinkGeneric *GetShrinkPtr ();
/* XBCC */
extern int getShrinkTimes(int p_time);
/*
 * type definitions
 */
typedef enum {
  ST_Void,
  ST_Spiral,
  ST_SpeedSpiral,
  ST_SpiralPlus,
  ST_Spiral3,
  ST_Spiral23,
  ST_SpiralLego,
  ST_EarlySpiral,
  ST_Compound,
  ST_CompoundF,
  ST_Compound2F,
  ST_LazyCompoundF,
  ST_CompoundSolid,
  ST_SavageCompound,
  ST_CompoundExtra,
  ST_Down,
  ST_DownF,
  ST_Quad,
  ST_ConstrictWave,
  ST_OutwardSpiral,
  /* no new shrink after this line */
  /* EPFL */
  ST_Horiz,
  ST_Circle,
  ST_Move,
  ST_IstySpiral3,
  ST_IstyCompound2F,
  ST_Diag,
  ST_OutwardExtra,
  ST_Spiral5,
  /* EPFL */
  NUM_ST
} XBShrinkType;

extern const char* GetShrinkName(XBShrinkType type);
#endif
/*
 * end of file shrink.h
 */








