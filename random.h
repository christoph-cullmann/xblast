/*
 * file random.h - generator
 *
 * $Id: random.h,v 1.4 2006/02/09 18:31:45 fzago Exp $
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
#ifndef _RANDOM_H
#define _RANDOM_H

/*
 * global macros
 */

/* needed for debugging */
#ifdef DEBUG_RANDOM
#define GameRandomNumber(a) GameRandomNumber2(a, __FILE__, __LINE__)
#else
#define GameRandomNumber(a) GameRandomNumber1(a)
#endif

/*
 * global prototypes
 */
extern int GameRandomNumber1 (int maxVal);
extern int GameRandomNumber2 (int maxVal, const char *file, int line);
extern void SeedRandom (unsigned seed);
extern unsigned GetRandomSeed (void);
extern int OtherRandomNumber (int maxVal);

#endif
/*
 * end of file random.h
 */
