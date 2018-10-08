/*
 * file demo.h - recording and playback of xblast games
 *
 * $Id: demo.h,v 1.7 2006/02/09 21:21:23 fzago Exp $
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
#ifndef _DEMO_H
#define _DEMO_H

/*
 * type definitions
 */

/*
 * global prototypes
 */

/* initialize recording */
extern void DemoInitGame (CFGType cfgType, const CFGGame * cfgGame);
extern void DemoInitLevel (XBAtom level);
/* game recording */
extern void DemoRecordFrame (int gameTime, const PlayerAction * pa);
/* finish recording */
extern void DemoFinishLevel (int gameTime, int winner, const char *type);
extern void DemoFinishGame (void);
/* get game config for playback */
extern XBBool DemoPlaybackConfig (CFGGame *);
/* get level name for playback */
extern XBAtom DemoPlaybackLevel (void);
/* load actions for playback */
extern XBBool DemoPlaybackStart (void);
/* get action for playback */
extern XBBool DemoPlaybackFrame (int gameTime, PlayerAction * pa);

#endif
/*
 * end of file demo.h
 */
