/*
 * Program XBLAST V2.1.8 or higher
 * (C) by Oliver Vogel (e-mail: vogel@ikp.uni-koeln.de)
 * September 24th 1996
 * started August 1993
 *
 * Bot by Plantet Didier (e-mal: plantet@info.enserb.u-bordeaux.fr)
 * File: bot.h
 *       include file for bot.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licences as by published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Publis License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _BOT_H
#define _BOT_H

#define NB_DIR 5
#define STOP  0
#define UP    1
#define LEFT  2
#define DOWN  3
#define RIGHT 4

#define PRIORITY_CANT_WALK 0
#define PRIORITY_SURVIVE 1
#define PRIORITY_KILL_PLAYER 2
#define PRIORITY_GET_BONUSES 3
#define PRIORITY_BREAK_WALL 4
#define PRIORITY_RANDOM 5

#define NB_PRIORITY 6

extern void gestionBot (BMPlayer * player_stat, PlayerAction * player_action, int numero_bot,
						int num_player);
extern void SetBotTime (int game_time);

#endif
/*
 * end of file bot.h
 */
