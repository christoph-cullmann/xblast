/*
 * file mi_player.c - show animated player srpite in menus
 *
 * $Id: mi_player.c,v 1.9 2006/02/09 21:21:24 fzago Exp $
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

#include "xblast.h"

/*
 * local types
 */
/* (animated) player sprite */
typedef struct
{
	XBMenuItem item;			/* menu item id */
	int id;						/* animation id */
	Sprite *sprite;				/* player sprite */
	const CFGPlayerGraphics **ptrConfig;	/* external config pointer */
	const CFGPlayerGraphics *oldConfig;	/* last known value */
	XBColor oldTeam;			/* last know team color */
	XBRgbValue currentRgb;		/* last known rgb value, for config */
	XBRgbValue *pRgb;			/* external rgb value */
	XBAtom currentShape;		/* last known shape */
	unsigned long animeMask;	/* loaded animation sprites */
	int cntAnime;				/* next animation index, -1 for no animation */
	int numAnime;				/* abs: animation indices, sgn: single/loop */
	BMSpriteAnimation *anime;	/* animation phases */
} XBMenuPlayerItem;

/*
 * animate player graphics
 */
static void
AnimatePlayerItem (XBMenuItem * ptr)
{
	XBMenuPlayerItem *player = (XBMenuPlayerItem *) ptr;
	assert (player != NULL);
	if (player->cntAnime >= 0) {
		/* get next animation phase */
		int anime = player->anime[player->cntAnime++];
		/* load if not yet loaded */
		if (0 == (player->animeMask & (1 << anime))) {
			GUI_LoadPlayerSprite (player->id, anime, player->oldConfig);
			player->animeMask |= (1 << anime);
		}
		/* choose sprite */
		SetSpriteAnime (player->sprite, anime);
		if (player->numAnime > 0) {
			/* loop animation for positive num count */
			if (player->cntAnime >= player->numAnime) {
				player->cntAnime = 0;
			}
		}
		else {
			/* single animation for negative num ocunt */
			if (player->cntAnime >= -player->numAnime) {
				player->cntAnime = -1;
			}
		}
	}
}								/* AnimatePlayerItem */

/*
 *  polling a player item
 */
static void
MenuPlayerPoll (XBMenuItem * ptr)
{
	XBMenuPlayerItem *player = (XBMenuPlayerItem *) ptr;
	assert (player != NULL);
	/* check if player data defined at all */
	if (*(player->ptrConfig) == NULL) {
		SetSpriteMode (player->sprite, SPM_UNMAPPED);
		player->oldConfig = NULL;
		player->cntAnime = -1;
		return;
	}
	/* check if pointer to graphics data changed */
	if (*(player->ptrConfig) != player->oldConfig) {
		SetSpriteMode (player->sprite, SPM_UNMAPPED);
		player->oldConfig = *(player->ptrConfig);
		player->oldTeam = player->oldConfig->bodySave;
		player->animeMask = 0;
		player->cntAnime = 0;
		SetSpriteMode (player->sprite, SPM_MAPPED);
	}
	/* check if body color has changed */
	if (player->oldConfig->body != player->oldTeam) {
		SetSpriteMode (player->sprite, SPM_UNMAPPED);
		player->oldTeam = player->oldConfig->body;
		player->animeMask = 0;
		player->cntAnime = 0;
		if (player->oldTeam != COLOR_INVALID) {
			SetSpriteMode (player->sprite, SPM_MAPPED);
		}
	}
	AnimatePlayerItem (ptr);
}								/* MenuPlayerPoll */

/*
 *  polling a config player item
 */
static void
MenuConfigPlayerPoll (XBMenuItem * ptr)
{
	XBMenuPlayerItem *player = (XBMenuPlayerItem *) ptr;

	assert (player != NULL);
	/* check if rgb has changed */
	if (memcmp (player->pRgb, &player->currentRgb, sizeof (XBRgbValue)) != 0 ||
		player->oldConfig->shape != player->currentShape) {
		SetSpriteMode (player->sprite, SPM_UNMAPPED);
		player->currentRgb = *(player->pRgb);
		player->currentShape = player->oldConfig->shape;
		player->animeMask = 0;
		player->cntAnime = 0;
		SetSpriteMode (player->sprite, SPM_MAPPED);
	}
	AnimatePlayerItem (ptr);
}								/* MenuConfigPlayerPoll */

/*
 * create a player item
 */
XBMenuItem *
MenuCreatePlayer (int x, int y, int w, int id, const CFGPlayerGraphics ** ptrConfig,
				  int numAnime, BMSpriteAnimation * anime)
{
	XBMenuPlayerItem *player;

	assert (ptrConfig != NULL);
	assert (anime != NULL);
	/* create item */
	player = calloc (1, sizeof (*player));
	assert (player != NULL);
	MenuSetItem (&player->item, MIT_Player, x, y, w, 2 * CELL_H, NULL, NULL, NULL, MenuPlayerPoll);
	/* set player specific data */
	player->id = id;
	player->ptrConfig = ptrConfig;
	player->oldConfig = NULL;
	player->oldTeam = COLOR_INVALID;
	player->sprite =
		CreatePlayerSprite (id, (x + (CELL_W - w) / 2) * BASE_X, (y - 1) * BASE_Y, SpriteStopDown,
							SPM_UNMAPPED);
	player->cntAnime = -1;
	player->numAnime = numAnime;
	player->anime = anime;
	player->animeMask = 0;
	/* that's all */
	return &player->item;
}								/* MenuCreateMenuPlayer */

/*
 * create a config player item
 */
XBMenuItem *
MenuCreateConfigPlayer (int x, int y, int w, int id, const CFGPlayerGraphics ** ptrConfig,
						int numAnime, BMSpriteAnimation * anime, XBRgbValue * pRgb)
{
	XBMenuPlayerItem *player;

	assert (ptrConfig != NULL);
	assert (anime != NULL);
	/* create item */
	player = calloc (1, sizeof (*player));
	assert (player != NULL);
	MenuSetItem (&player->item, MIT_Player, x, y, w, 2 * CELL_H, NULL, NULL, NULL,
				 MenuConfigPlayerPoll);
	/* set player specific data */
	player->id = id;
	player->currentRgb = *pRgb;
	player->currentRgb.red += 1;
	player->pRgb = pRgb;
	player->currentShape = ATOM_INVALID;
	player->ptrConfig = ptrConfig;
	player->oldConfig = *ptrConfig;
	player->oldTeam = COLOR_INVALID;
	player->sprite =
		CreatePlayerSprite (id, (x + (CELL_W - w) / 2) * BASE_X, (y - 1) * BASE_Y, SpriteStopDown,
							SPM_UNMAPPED);
	player->cntAnime = -1;
	player->numAnime = numAnime;
	player->anime = anime;
	player->animeMask = 0;
	/* that's all */
	return &player->item;
}								/* MenuCreateConfigPlayer */

/*
 * delete a player
 */
void
MenuDeletePlayer (XBMenuItem * item)
{
	XBMenuPlayerItem *player = (XBMenuPlayerItem *) item;
	assert (player->sprite != NULL);
	DeleteSprite (player->sprite);
}								/* DeletePlayerItem */

/*
 * end of file mi_player.c
 */
