/*
 * file mi_player.c - show animated player srpite in menus
 *
 * $Id: mi_player.c,v 1.4 2004/10/15 22:08:56 lodott Exp $
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
#include "mi_player.h"

#include "gui.h"

/*
 * local types
 */
/* (animated) player sprite */
typedef struct {
  XBMenuItem                item;
  int              	    id;
  Sprite           	   *sprite;
  const CFGPlayerGraphics **ptrConfig;
  const CFGPlayerGraphics  *oldConfig;
  XBColor                   oldTeam;
  unsigned long     	    animeMask;
  int               	    cntAnime;
  int               	    numAnime;
  BMSpriteAnimation 	   *anime;
} XBMenuPlayerItem;

/*
 *  polling a player item
 */
static void
MenuPlayerPoll (XBMenuItem *ptr)
{
  XBMenuPlayerItem *player = (XBMenuPlayerItem *) ptr;

  assert (player != NULL);
  /* check is config has changed */
  if (*(player->ptrConfig) != player->oldConfig) {
    SetSpriteMode (player->sprite, SPM_UNMAPPED);
    player->oldConfig = *(player->ptrConfig);
    if (NULL != player->oldConfig) {
      SetSpriteMode (player->sprite, SPM_MAPPED);
      player->oldTeam   = player->oldConfig->bodySave;
      player->animeMask = 0;
      player->cntAnime  = 0;
    } else {
      player->cntAnime = -1;
    }
  } else if (player->oldConfig != NULL) {
    /* check if team color has changed */
    if ( player->oldConfig->body != player->oldTeam) {
      player->oldTeam = player->oldConfig->body;
      player->animeMask = 0;
      player->cntAnime  = 0;
    }
  }
  if (player->cntAnime >= 0) {
    int anime = player->anime[player->cntAnime ++];
    if (0 == (player->animeMask & (1 << anime) ) ) {
      GUI_LoadPlayerSprite (player->id, anime, player->oldConfig);
      player->animeMask |= (1 << anime);
    }
    SetSpriteAnime (player->sprite, anime);
    if (player->numAnime > 0) {
      if (player->cntAnime >= player->numAnime) {
	player->cntAnime = 0;
      }
    } else {
      if (player->cntAnime >= -player->numAnime) {
	player->cntAnime = -1;
      }
    }
  }
} /* MenuPlayerPoll */

/*
 *
 */
XBMenuItem *
MenuCreatePlayer (int x, int y, int w, int id, const CFGPlayerGraphics **ptrConfig, 
		  int numAnime, BMSpriteAnimation *anime)
{
  XBMenuPlayerItem *player;

  assert (ptrConfig != NULL);
  assert (anime != NULL);
  /* create item */
  player = calloc (1, sizeof (*player) );
  assert (player != NULL);
  MenuSetItem (&player->item, MIT_Player, x, y, w, 2*CELL_H, NULL, NULL, NULL, MenuPlayerPoll);
  /* set player specific data */
  player->id        = id;
  player->ptrConfig = ptrConfig;
  player->oldConfig = NULL;
  player->oldTeam   = COLOR_INVALID;
  player->sprite    = CreatePlayerSprite (id, (x + (CELL_W - w)/2 ) * BASE_X, (y - 1) * BASE_Y, SpriteStopDown, SPM_UNMAPPED);
  player->cntAnime  = -1;
  player->numAnime  = numAnime;
  player->anime     = anime;
  player->animeMask = 0;
  /* that's all */
  return &player->item;
} /* CreateMenuPlayer */

/*
 * delete a player
 */
void
MenuDeletePlayer (XBMenuItem *item)
{
  XBMenuPlayerItem *player = (XBMenuPlayerItem *) item;

  assert (player->sprite != NULL);
  DeleteSprite (player->sprite);
} /* DeletePlayerItem */

/*
 * end of file mi_player.c
 */
