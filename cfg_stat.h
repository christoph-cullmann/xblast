/*
 * file cfg_stat.h - level and game statistics
 *
 * $Id: cfg_stat.h,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
 *
 */
#ifndef _CFG_STAT_H
#define _CFG_STAT_H

#include "event.h"

/*
 * type definitions
 */
typedef struct {
  XBAtom      atom;
  const char *name;
  int         numWon;
  int         numTotal;
  double      scoreTotal;
  double      percent;
  double      average;
} XBStatData;

/*
 * global prototypes
 */
extern void LoadStatConfig (void);
extern void SaveStatConfig (void);
extern void FinishStatConfig (void);

extern void StoreLevelStat (XBAtom level, XBAtom player, XBBool won, double points); 

extern XBStatData *CreateLevelStat (XBAtom level, size_t *num);
extern XBStatData *CreatePlayerSingleStat (XBAtom level, size_t *num);
extern XBStatData *CreatePlayerTotalStat (size_t *num);
extern XBStatData *CreateLevelTotalStat (size_t *num);
extern XBStatData *CreateLevelSingleStat (XBAtom level, size_t *num);

#endif
/*
 * end of file cfg_stat.h
 */
