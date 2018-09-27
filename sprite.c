/*
 * file sprite.c - handling sprites 
 *
 * $Id: sprite.c,v 1.7 2004/10/19 17:59:19 iskywalker Exp $
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
#include "sprite.h"

#include "image.h"
#include "map.h"
#include "gui.h"
#include "geom.h"
#include "map.h"

/*
 * local types
 */

/* function pointers for region and drawing */


/*
 * local variables
 */
AnySprite *spriteFirst = NULL;
AnySprite *spriteLast  = NULL;

/* 
 * mark sprite positon to redraw
 */
void
MarkMazeSprite (const Sprite *spr)
{
  const BMRectangle *r = (*spr->any.rect)(spr);
  assert (r != NULL);
  MarkMaze (r->x/BLOCK_WIDTH, r->y/BLOCK_HEIGHT, (r->x + r->w - 1)/BLOCK_WIDTH, (r->y + r->h - 1)/BLOCK_HEIGHT);
} /* MarkMazeSprite */

/*
 * local function: create sprite
 */
static Sprite *
CreateSprite (SpriteType type, DrawFunc draw, RectFunc rect, int x, int y, int ysort, int anime, int mode)
{
  AnySprite *ptr;
  AnySprite *other;
  /* alloc memory */
  if (NULL == (ptr = (AnySprite *) malloc(sizeof(Sprite) ) ) ) {
    return NULL;
  }

  /* init values */
  ptr->type  = type;
  ptr->dirty = XBTrue;
  ptr->draw  = draw;
  ptr->rect  = rect;
  ptr->x     = x;
  ptr->y     = y;
  ptr->ysort = ysort;
  ptr->anime = anime;
  ptr->mode  = mode;
  ptr->next  = NULL;
  ptr->prev  = NULL;

  /* store sprite in list */
  if (NULL == spriteFirst) {
    spriteFirst = spriteLast = ptr;
  } else {
    for (other=spriteFirst; other != NULL; other = other->next) {
      /* list is sorted by inccreasing (y+ysort) value */
      if ( (other->ysort + other->y) > (ptr->ysort + ptr->y) ) {
	if (NULL == other->prev) {
	  /* start of list */
	  spriteFirst = ptr;
	  ptr->prev = NULL;
	} else {
	  other->prev->next = ptr;
	  ptr->prev = other->prev;
	}
	ptr->next = other;
	other->prev = ptr;
	goto Finish;
      }
    }
    spriteLast->next = ptr;
    ptr->prev = spriteLast;
    spriteLast = ptr;
  }
 Finish:
  return (Sprite *)ptr;
} /* CreateSprite */
	       
/*
 * rectangle function for player sprite
 */
static const BMRectangle *
RectPlayerSprite (const Sprite *ptr)
{
  static BMRectangle result;

  assert (ptr != NULL);
  result.x = imgRectSprite[ptr->any.anime].x + ptr->any.x;
  result.y = imgRectSprite[ptr->any.anime].y + ptr->any.y;
  result.w = imgRectSprite[ptr->any.anime].w;
  result.h = imgRectSprite[ptr->any.anime].h;
   
  return &result;
} /* RectPlayerSprite */

/*
 * public function: CreatePlayerSprite
 */
Sprite *
CreatePlayerSprite (int player, int x, int y, unsigned anime, int mode)
{
  Sprite *ptr;

  ptr = CreateSprite (STPlayer, GUI_DrawPlayerSprite, RectPlayerSprite, x, y, BLOCK_HEIGHT, anime, mode);
  if (NULL == ptr) {
    return NULL;
  }
  ptr->player.player = player;
  /* mark maze to redrawn */
  if (ptr->any.mode == SPM_MAPPED) {
    MarkMazeSprite ((Sprite *) ptr);
  }
  return ptr;
} /* CreatePlayerSprite */

static const BMRectangle *
RectBombSprite (const Sprite *ptr)
{
  static BMRectangle result;

  assert (ptr != NULL);
  result.x = ptr->any.x;
  result.y = ptr->any.y;
  result.w = BLOCK_WIDTH;
  result.h = BLOCK_HEIGHT;
   
  return &result;
} /* GUI_RectBombSprite */

/*
 * public function: CreateBombSprite
 */
Sprite *
CreateBombSprite (int bomb, int x, int y, unsigned anime, int mode)
{
  Sprite *ptr;

  ptr = CreateSprite (STBomb, GUI_DrawBombSprite, RectBombSprite, x, y, BASE_Y/2, anime, mode);
  if (NULL == ptr) {
    return NULL;
  }
  ptr->bomb.bomb  = bomb;
  /* mark maze to redrawn */
  if (ptr->any.mode == SPM_MAPPED) {
    MarkMazeSprite ((Sprite *) ptr);
  }
  return ptr;
} /* CreateBombSprite */

/*
 *
 */
static const BMRectangle *
RectTextSprite (const Sprite *ptr)
{
  static BMRectangle result;

  assert (ptr != NULL);
  result.x = ptr->any.x;
  result.y = ptr->any.y;
  result.w = ptr->text.w;
  result.h = ptr->text.h;

  return &result;
} /* GUI_RectTextSprite */

/*
 * 
 */
Sprite *
CreateTextSprite (const char *text, int x, int y, int w, int h, unsigned anime, int mode)
{
  Sprite *ptr;

  ptr = CreateSprite (STText, GUI_DrawTextSprite, RectTextSprite, x, y, 0, anime, mode);
  if (NULL == ptr) {
    return NULL;
  }
  ptr->any.anime  = anime;
  ptr->text.text  = text;
  ptr->text.w     = w;
  ptr->text.h     = h;
  /* mark maze to redrawn */
  if (ptr->any.mode == SPM_MAPPED) {
    MarkMazeSprite ((Sprite *) ptr);
  }
  return ptr;
} /* CreateTextSprite */

/*
 *
 */
static const BMRectangle *
RectIconSprite (const Sprite *spr)
{
  static BMRectangle result;

  assert (spr != NULL);
  assert (spr->any.anime < MAX_ICON_SPRITES);
  
  result.x = imgRectIcon[spr->any.anime].x + spr->any.x;
  result.y = imgRectIcon[spr->any.anime].y + spr->any.y;
  result.w = imgRectIcon[spr->any.anime].w;
  result.h = imgRectIcon[spr->any.anime].h;

  return &result;
} /* RectColorSprite */

/*
 *
 */
Sprite *
CreateIconSprite (int x, int y, unsigned anime, int mode)
{
  Sprite *ptr;

  ptr = CreateSprite (STIcon, GUI_DrawIconSprite, RectIconSprite, x, y, 0, anime, mode);
  if (NULL == ptr) {
    return NULL;
  }
  /* mark maze to redrawn */
  if (ptr->any.mode == SPM_MAPPED) {
    MarkMazeSprite ((Sprite *) ptr);
  }
  return ptr;
} /* CreateIconSprite */

/*
 * public function delete_sprite
 */
void
DeleteSprite (Sprite *spr)
{

  /* check for non-dirty sprites to be redrawn */
  if (NULL == spr->any.prev) {
    spriteFirst = spr->any.next;
  } else {
    spr->any.prev->next = spr->any.next;
  }
  if (NULL == spr->any.next) {
    spriteLast=spr->any.prev;
  } else {
    spr->any.next->prev = spr->any.prev;
  }
  if (spr->any.mode & SPM_MAPPED) {
    /* mark tiles on position */
    MarkMazeSprite (spr);
  }
  free (spr);
} /* DeleteSprite */

/*
 * local function swap_sprite_prev
 */
static void 
SwapSpritePrev (AnySprite *ptr)
{
  AnySprite *prev;
  
  if (NULL == (prev = ptr->prev) ) {
    return;
  }

  /* references from outside */
  if (prev->prev != NULL) {
    prev->prev->next = ptr;
  } else {
    spriteFirst = ptr;
  }
  if (ptr->next != NULL) {
    ptr->next->prev = prev;
  } else {
    spriteLast = prev;
  }
  /* internal refs */
  prev->next = ptr->next;
  ptr->prev  = prev->prev;

  prev->prev = ptr;
  ptr->next  = prev;
} /* SwapSpritePrev */

/*
 * public function: move_sprite
 */
void
MoveSprite (Sprite *sprite, int x, int y)
{
  AnySprite *spr = (AnySprite *) sprite;
  AnySprite *ptr;

  if ( (spr->y == y) && (spr->x == x) ) {
    return;
  }

  if (spr->mode & SPM_MAPPED) {
    /* mark sprite as dirty */
    spr->dirty = XBTrue;
    /* mark tiles on old position */
    MarkMazeSprite ((Sprite *) spr);
  }

  spr->x = x;
  if (spr->y < y) {
    spr->y = y;
    /* sprite has move downwards */
    for (ptr = spr->next;
	 (ptr != NULL) && ( (ptr->y + ptr->ysort) < (spr->y + spr->ysort) );
	 ptr = spr->next) {
      SwapSpritePrev (ptr);
    }
  }  else if (spr->y > y) {
    spr->y = y;
    /* sprite has moved upwards */
    for (ptr = spr->prev;
	 (ptr != NULL) && ( (ptr->y + ptr->ysort) > (spr->y + spr->ysort) );
	 ptr = spr->prev) {
      SwapSpritePrev (spr);
    }
  }

  /* mark tiles on new position */
  if (spr->mode & SPM_MAPPED) {
    MarkMazeSprite ((Sprite *)spr);
  }
} /* MoveSprite */

/*
 * public function: set_sprite_mode
 */ 
void
SetSpriteMode (Sprite *sprite, int mode)
{
  if (mode == sprite->any.mode) {
    return;
  }
  MarkMazeSprite ((Sprite *)sprite);
  sprite->any.mode  = mode;
  sprite->any.dirty = XBTrue;
} /* SetSpriteMode */

/*
 * 
 */
void
SetSpriteText (Sprite *sprite, const char *text)
{
  assert (sprite != NULL);
  assert (sprite->type = STText);
  
  MarkMazeSprite ((Sprite *)sprite);
  sprite->text.text = text;
  sprite->any.dirty = XBTrue;
} /* SetSpriteText */
      
/*
 * 
 */ 
void
SetSpriteAnime (Sprite *sprite, unsigned anime)
{
  if (anime == sprite->any.anime) {
    return;
  }
  if (sprite->any.mode & SPM_MAPPED) {
    MarkMazeSprite ((Sprite *)sprite);
    sprite->any.dirty = XBTrue;
  }
  sprite->any.anime = anime;
  if (sprite->any.mode & SPM_MAPPED) {
    MarkMazeSprite ((Sprite *)sprite);
  }
} /* SetSpriteAnime */
    
/*
 * set color for an icon/xolor sprite
 */
void
SetSpriteColor (Sprite *sprite, XBColor color)
{
  assert (sprite != NULL);
  assert (sprite->type == STIcon);

  GUI_LoadIconSprite (sprite->any.anime, color);
  MarkMazeSprite (sprite);
} /* SetSpriteColor */
    
/*
 * local function: sprite_intersect
 */
static void 
SpriteIntersect (AnySprite *a, AnySprite *b) 
{
  BMRectangle rect_a, rect_b;
  int left, right, top, bottom;

  if ( (a->mode & SPM_MAPPED) && (b->mode & SPM_MAPPED) ) {
    rect_a = *(*a->rect)((Sprite *)a);
    rect_b = *(*b->rect)((Sprite *)b);
    
    left   = MAX(rect_a.x, rect_b.x);
    right  = MIN(rect_a.x + rect_a.w, rect_b.x + rect_b.w);
    
    if (left < right) {
      top    = MAX(rect_a.y, rect_b.y);
      bottom = MIN(rect_a.y + rect_a.h, rect_b.y + rect_b.h);
      if (top < bottom) {
	a->dirty = XBTrue;
	b->dirty = XBTrue;
	MarkMazeRect (left, top, right-left, bottom-top);
      }
    }
  }
} /* SpriteIntersect */
  
/*
 * 
 */
void
ShuffleAllSprites (void)
{
  AnySprite *ptr;

  /* switch sprites on same (y+ysort) */
  if (spriteFirst != NULL) {
    ptr = spriteFirst;
    while (ptr->next != NULL) {
      if ( (ptr->y + ptr->ysort) == (ptr->next->y + ptr->next->ysort) ) { 
	if ( (! ptr->dirty) && (! ptr->next->dirty) ) {
	  SpriteIntersect (ptr, ptr->next);
	}
	SwapSpritePrev (ptr->next);
      } else {
	ptr = ptr->next;
      }
    }  
  }
} /* ShuffleAllSprites */

void
DeleteAllBombSprites(void)
{
  AnySprite *ptr,*tmp;

  /* check for non-dirty sprites to be redrawn */
  for (ptr = spriteFirst; ptr != NULL;) {
      if(ptr->draw==GUI_DrawBombSprite){
	tmp=ptr->next;
	DeleteSprite((Sprite *)ptr);
	ptr=tmp;
      }
      else{
	ptr = ptr->next;
      }
  }
} 
/*
 * 
 */
void
MarkAllSprites (void)
{
  AnySprite *ptr;

  /* check for non-dirty sprites to be redrawn */
  for (ptr = spriteFirst; ptr != NULL; ptr = ptr->next) {
    if (! ptr->dirty) {
      ptr->dirty = SpriteMarked ((Sprite *)ptr);
    }
  }
} /* MarkAllSprites */

/*
 * public function draw all sprites
 */
void
DrawAllSprites (void)
{
  AnySprite *ptr;

  for (ptr = spriteFirst; ptr != NULL; ptr = ptr->next) {
    if(ptr->type==STBomb){
    }
    if ( (ptr->mode & SPM_MAPPED) && (ptr->dirty) ) {
      (*ptr->draw)((Sprite *)ptr);
    }
    ptr->dirty = XBFalse;
  }
} /* DrawAllSprites */

/*
 * get current region of sprite
 */
const BMRectangle *
SpriteRectangle (const Sprite *sprite)
{
  assert (sprite != NULL);
  assert (sprite->any.rect != NULL);
  return (*sprite->any.rect) (sprite);
} /* SpriteRectangle */

/*
 * get sorite animation phase
 */
int
SpriteAnime (const Sprite *sprite)
{
  assert (sprite != NULL);
  return sprite->any.anime;
} /* SpriteAnime */

/*
 * get bomb sprite bomb type (mini/normal)
 */
int
SpriteBomb (const Sprite *sprite)
{
  assert (sprite != NULL);
  assert (sprite->type == STBomb);
  return sprite->bomb.bomb;
} /* SpriteBomb */

/*
 * get player sprite id
 */
int
SpritePlayer (const Sprite *sprite)
{
  assert (sprite != NULL);
  assert (sprite->type == STPlayer);
  return sprite->player.player;
} /* SpritePlayer */

/*
 * get text sprite text to draw
 */
const char *
SpriteText (const Sprite *sprite)
{
  assert (sprite != NULL);
  assert (sprite->type == STText);
  return sprite->text.text;
} /* SpriteText */

/*
 * check if sprite is masked
 */
XBBool
SpriteIsMasked (const Sprite *sprite)
{
  assert (sprite != NULL);
  return (sprite->any.mode & SPM_MASKED) ? XBTrue : XBFalse;
} /* SpriteIsMasked */

/*
 * end of sprite.c
 */
