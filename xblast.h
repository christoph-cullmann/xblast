/*
 * file xblast.h - common macros, constants ansd types
 *
 * $Id: xblast.h,v 1.16 2006/03/28 11:58:16 fzago Exp $
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

#ifdef XBLAST_XBLAST_H
#error
#endif
#define XBLAST_XBLAST_H

/* include config header first */
#ifdef HAVE_CONFIG_H
/* autoconf generated */
#include "config.h"
#else
/* non-autoconf generated, include target specific header */
#ifdef __MINGW32__
#include "config-mingw.h"
#endif
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef W32
#ifndef WMS
#include <windows.h>
#include <winsock2.h>
#endif
#endif

#if defined(__MINGW32__) || defined(WMS)
#include <windows.h>
#include <direct.h>
#endif

#ifndef __USE_W32_SOCKETS
#ifndef W32
#include <sys/socket.h>
#endif
#endif

#ifdef WIN32
#else
#include <dirent.h>
#endif

/*--------------------------------------------------------------------------*/

/*
 * macros
 */
#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) ( (a)>=(b) ? (a) : (b) )

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) ( (a)<=(b) ? (a) : (b) )

#ifdef ABS
#undef ABS
#endif
#define ABS(a)   ( (a)>=0 ? (a) : (-(a)) )

/* how may player are maximum */
#ifdef SMPF
#define MAX_PLAYER 16
#else
#define MAX_PLAYER 6
#endif

/* maximum mask bytes allowed, affects player maximum in network game */
#define MAX_MASK_BYTES     4

/* how many local players */
#define NUM_LOCAL_PLAYER 6
/* how many client can connect */
#define MAX_HOSTS  MAX_PLAYER

/* maximum allowed number of victories to win */
#define MAX_VICTORIES 9

/* dimension of of maze */
#define MAZE_W 15
#define MAZE_H 13
#define STAT_W 20
#ifdef SMPF
#define STAT_H 3
#else
#define STAT_H 2
#endif

/* number block tiles */
#define MAX_BLOCK     11
#define MENU_MAX_TILE 21
#define MAX_TILE      MAX(MAX_BLOCK,MENU_MAX_TILE)

/* number of bombs and their animations */
#define MAX_BOMBS 2
#define MAX_BOMB_ANIME 17

/* size of big letters */
#define CHARW 3
#define CHARH 5

#define MAX_EXPLOSION 16

#define TIME_STEP   48
#define DEAD_TIME    8
#define GAME_TIME   (60*TIME_STEP + DEAD_TIME)

																				 /* Added by Fouf on 09/14/99 23:55:18 *//* Written by Amilhastre */
#define BOMB_SEARCH_X  (2.5*BLOCK_WIDTH)
#define BOMB_SEARCH_Y  (2.5*BLOCK_HEIGHT)

/* length of a frame in ms */
#define FRAME_TIME    50

/*
 * fundamental types
 */
typedef enum
{
	XBFalse = 0, XBTrue
} XBBool;

typedef struct
{
	double x, y;
} BMPoint;

typedef struct
{
	int x, y;
	int w, h;
} BMRectangle;

/*
 *  position vector
 */
typedef struct
{
	short y, x;
} BMPosition;

/*
 *  PFV Pointer to void/int functions
 */
typedef void (*PFV) (void);
typedef int (*PFI) (void);
typedef BMRectangle *(*PFR) (void);

/*
 * player sprite animations phase
 */
typedef enum
{
	SpriteStopDown = 0,
	SpriteWalkDown0, SpriteWalkDown1, SpriteWalkDown2, SpriteWalkDown3,
	SpriteStopUp,
	SpriteWalkUp0, SpriteWalkUp1, SpriteWalkUp2, SpriteWalkUp3,
	SpriteStopRight,
	SpriteWalkRight0, SpriteWalkRight1, SpriteWalkRight2, SpriteWalkRight3,
	SpriteStopLeft,
	SpriteWalkLeft0, SpriteWalkLeft1, SpriteWalkLeft2, SpriteWalkLeft3,
	SpriteDamagedDown, SpriteDamagedLeft, SpriteDamagedUp, SpriteDamagedRight,
	SpriteLooser, SpriteLooser1, SpriteLooser2,
	SpriteWinner, SpriteWinner2, SpriteWinner3,
	SpriteBigWinner,
	SpriteDeadDown, SpriteDeadLeft, SpriteDeadUp, SpriteDeadRight,
	SpriteMorphed,
	SpriteZombie,
	MAX_ANIME
} BMSpriteAnimation;

#define MAX_ANIME_EPM ((int) SpriteDeadDown)
#define MAX_ANIME_PPM ((int) (MAX_ANIME - MAX_ANIME_EPM))

/*
 * player directions
 */
typedef enum
{
	GoStop = 0, GoUp, GoLeft, GoDown, GoRight, GoDefault, GoAll,
	MAX_DIR
} BMDirection;

/*
 * ???
 */
typedef enum
{
	SBVoid = 0,
	SBTextLeft, SBTextMid, SBTextRight,
	SBDead = 4,
	SBSick = 4 + MAX_PLAYER,
	SBPlayer = 4 + 2 * MAX_PLAYER,
	SBAbort = 4 + 3 * MAX_PLAYER,
	SBSickAbort = 4 + 4 * MAX_PLAYER,
	MAX_SCORE_TILES = 4 + 5 * MAX_PLAYER
} BMScoreTile;

/* AbsInt start */
extern XBBool trace_aborted;
extern FILE*  trace_aborted_file;
/* AbsInt end */

/*--------------------------------------------------------------------------*/

#ifdef WMS
#define GAME_DATADIR "."
#endif

/*--------------------------------------------------------------------------*/


#include "debug.h"
#include "socket.h"
#include "util.h"
#include "str_util.h"
#include "event.h"
#include "random.h"
#include "version.h"

#include "color.h"
#include "sprite.h"
#include "image.h"

#include "cfg_xblast.h"
#include "snd.h"

#include "dat_rating.h"
#include "net_tele.h"
#include "action.h"
#include "ini_file.h"

#include "cfg_main.h"
#include "cfg_game.h"
#include "cfg_player.h"
#include "cfg_stat.h"
#include "cfg_demo.h"
#include "cfg_level.h"
#include "cfg_control.h"

#include "gui.h"
#include "browse.h"
#include "chat.h"
#include "atom.h"

#include "net_dgram.h"
#include "net_socket.h"

#include "network.h"
#include "com.h"
#include "com_central.h"
#include "com_base.h"
#include "com_central.h"
#include "com_dgram.h"
#include "com_from_central.h"
#include "com_newgame.h"
#include "com_reply.h"
#include "com_to_central.h"
#include "com_to_server.h"
#include "com_browse.h"
#include "com_dg_client.h"
#include "com_dg_server.h"
#include "com_listen.h"
#include "com_query.h"
#include "com_stream.h"
#include "com_to_client.h"

#include "player.h"
#include "client.h"
#include "server.h"
#include "central.h"
#include "user.h"

#include "mi_tool.h"
#include "mi_base.h"
#include "mi_button.h"
#include "mi_color.h"
#include "mi_cyclic.h"
#include "mi_int.h"
#include "mi_label.h"
#include "mi_player.h"
#include "mi_string.h"
#include "mi_toggle.h"
#include "mi_combo.h"
#include "mi_host.h"
#include "mi_keysym.h"
#include "mi_map.h"
#include "mi_stat.h"
#include "mi_tag.h"

#include "menu.h"
#include "menu_level.h"
#include "menu_game.h"
#include "menu_layout.h"
#include "menu_extras.h"
#include "menu_player.h"
#include "menu_control.h"
#include "menu_network.h"
#include "menu_level.h"
#include "menu_edit.h"

#include "game_local.h"
#include "game_demo.h"
#include "game_server.h"
#include "game_client.h"

#include "map.h"
#include "level.h"
#include "geom.h"
#include "bomb.h"
#include "shrink.h"
#include "scramble.h"
#include "func.h"
#include "bot.h"

#include "demo.h"
#include "game.h"

#include "status.h"

#include "intro.h"
#include "introdat.h"

#include "info.h"

#include "timeval.h"

/* i18n */
#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#define _(String) String
#define N_(String) String
#define gettext(String) String
#endif
