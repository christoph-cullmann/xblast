/*
 * file dat_rating.h - level and game statistics
 *
 * $Id: dat_rating.h,v 1.3 2004/05/14 10:00:33 alfie Exp $
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 * Added by Koen De Raedt for central support
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
#ifndef _DAT_RATING_H
#define _DAT_RATING_H

#include "event.h"
#include "cfg_player.h"

#define MAX_INFO 15

/*
 * global prototypes
 */
typedef struct {
  XBAtom      atom;
  const char *name;
  int         numWon;
  int         numTotal;
  double      score;
  double      percent;
  int         rank;
} XBCentralData;

typedef struct {
  const char *name;
  const char *value;
} XBCentralInfo;

extern XBCentralData *CreateCentralStat (size_t *num);
extern XBCentralInfo *CreateCentralInfo (XBAtom atom, XBCentralData data);

#endif
/*
 * end of file dat_rating.h
 */
