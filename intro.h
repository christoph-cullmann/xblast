/*
 * file intro.h - intro and inbetween screens
 *
 * $Id: intro.h,v 1.6 2006/02/09 21:21:24 fzago Exp $
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
#ifndef XBLAST_INTRO_H
#define XBLAST_INTRO_H

/*
 * global prototypes
 */
extern void DoFade (XBFadeMode fadeMode, int maxLine);
extern void DoIntro (void);
extern XBBool InitPlayerSprites (int numPlayers, const CFGPlayer *);
extern void InitScoreBoard (int numPlayers, int numWins);
extern void InitWinner (int numPlayers);
extern XBBool LevelIntro (int numPlayers, const DBRoot * level, int timeOut);
extern XBBool LevelEnd (int numPlayers, int lastTeam, const char *msg, int timeOut);
extern XBBool ShowScoreBoard (int lastTeam, int maxNumWins, int numPlayers, BMPlayer * ps,
							  int timeOut);
extern void ShowWinner (int lastTeam, int numPlayers, BMPlayer * ps);

#endif
/*
 * end of file intro.h
 */
