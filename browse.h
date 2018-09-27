/*
 * file browse.h - datatype used in game browsing
 *
 * $Id: browse.h,v 1.3 2004/05/14 10:00:32 alfie Exp $
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
#ifndef XBLAST_BROWSE_H
#define XBLAST_BROWSE_H

#include "xblast.h"

/*
 * type definitions
 */
typedef enum {
  XBBT_None,  /* invalid data */
  XBBT_Query, /* query for games */
  XBBT_Reply, /* direct reply from game server */
  XBBT_NewGame, /* XBCC inform central of new game */
  XBBT_NewGameOK /* XBCC inform central of new game */
} XBBrowseTeleType;

typedef struct {
  XBBrowseTeleType type;
  unsigned char    serial; /* Serial of request */
} XBBrowseTeleAny;

typedef struct {
  XBBrowseTeleAny any;
} XBBrowseTeleQuery;

typedef struct {
  XBBrowseTeleAny  any;
  unsigned short   port;       /* port for game */
  unsigned char    version[3]; /* version numbers */
  char             game[48];   /* name of the game */
  char             host[32];   /* XBCC host address */
  unsigned char    numLives;   /* number of lives */
  unsigned char    numWins;    /* number of matches to win */
  unsigned char    frameRate;  /* frames per second */
} XBBrowseTeleReply;

typedef struct {
  XBBrowseTeleAny  any;
  unsigned short   port;       /* port for game */
  unsigned char    version[3]; /* version numbers */
  char             game[48];   /* name of the game */
  //char             host[32];   /* XBCC host address */
  int              gameID;     /* XBCC */
  unsigned char    numLives;   /* number of lives */
  unsigned char    numWins;    /* number of matches to win */
  unsigned char    frameRate;  /* frames per second */
} XBBrowseTeleNewGame;

typedef struct {
  XBBrowseTeleAny  any;
  int              gameID;       /* XBCC ID for game */
} XBBrowseTeleNewGameOK;

typedef union {
  XBBrowseTeleType    type;
  XBBrowseTeleAny     any;
  XBBrowseTeleQuery   query;
  XBBrowseTeleReply   reply;
  XBBrowseTeleNewGame newGame;
  XBBrowseTeleNewGameOK      newGameOK;
} XBBrowseTele;

/*
 * global prototypes
 */
extern XBBrowseTeleType BrowseTele_Parse (XBBrowseTele *, const unsigned char *, size_t len);
extern size_t           BrowseTele_Write (const XBBrowseTeleAny *, unsigned char *);

#endif
/*
 * end of file browse.h
 */
