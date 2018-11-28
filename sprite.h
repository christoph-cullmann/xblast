/*
 * file sprite.h - handling all sprites
 *
 * $Id: sprite.h,v 1.4 2004/08/21 16:04:24 iskywalker Exp $
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

#ifndef XBLAST_SPRITE_H
#define XBLAST_SPRITE_H

#include "xblast.h"
#include "color.h"

/*
 * some flags
 */
#define SPM_UNMAPPED  0
#define SPM_MAPPED   (1<<1)
#define SPM_MASKED   (1<<2)

#define TargetIconVOffset 10
#define MAX_COLOR_SPRITES 6

typedef enum {
  ISA_Color1,
  ISA_Color2,
  ISA_Color3,
  ISA_Color4,
  ISA_Color5,
  ISA_Color6,
  ISA_LedOff,
  ISA_LedOn,
  ISA_Abort,
  ISA_Default,
  ISA_TeamNone,
  ISA_TeamRed,
  ISA_TeamGreen,
  ISA_TeamBlue,
  ISA_Target,
  ISA_Loser,
  MAX_ICON_SPRITES
} IconSpriteAnimation;

typedef union _sprite Sprite;

/* type of sprite */
typedef enum {
  STNone = 0,
  STPlayer,
  STBomb,
  STText,
  STIcon
} SpriteType;

typedef void (*DrawFunc)(const Sprite *);
typedef const BMRectangle *(*RectFunc)(const Sprite *);
typedef struct _any_sprite AnySprite;
/* common basis */
struct _any_sprite {
  SpriteType type;
  XBBool     dirty;
  AnySprite *prev;
  AnySprite *next;
  DrawFunc   draw;
  RectFunc   rect;
  int        ysort;
  int        x;
  int        y;
  int        mode;
  unsigned   anime;
};  
/* players */
typedef struct {
  AnySprite   any;
  int         player;
} PlayerSprite;

/* bombs */
typedef struct {
  AnySprite   any;
  int         bomb;
} BombSprite;

/* text (for menus) */
typedef struct {
  AnySprite   any;
  int         w;
  int         h;
  const char *text;
} TextSprite;

/* icons for menus */
typedef struct {
  AnySprite   any;
} IconSprite;

union _sprite {
  int          type ;
  AnySprite    any;
  PlayerSprite player;
  BombSprite   bomb;
  TextSprite   text;
  IconSprite   icon;
};

  
/*
 * prototypes
 */
extern Sprite *CreatePlayerSprite (int p, int x, int y, unsigned a, int m);
extern Sprite *CreateBombSprite (int b, int x, int y, unsigned a, int m);
extern Sprite *CreateTextSprite (const char *t, int x, int y, int w, int h, unsigned a, int m);
extern Sprite *CreateIconSprite (int x, int y, unsigned a, int m);
extern void DeleteSprite (Sprite *spr);
extern void MoveSprite (Sprite *sprite, int x, int y);
extern void SetSpriteMode (Sprite *sprite, int mode);
extern void SetSpriteAnime (Sprite *sprite, unsigned anime);
extern void SetSpriteText (Sprite *sprite, const char *text);
extern void SetSpriteColor (Sprite *sprite, XBColor color);
extern void ShuffleAllSprites (void);
extern void MarkAllSprites (void);
extern void DrawAllSprites (void);
extern void DeleteAllBombSprites (void);
extern const BMRectangle *SpriteRectangle (const Sprite *);
extern int SpriteAnime (const Sprite *);
extern int SpriteBomb (const Sprite *);
extern int SpritePlayer (const Sprite *);
extern const char *SpriteText (const Sprite *);
extern XBBool SpriteIsMasked (const Sprite *);
extern  void MarkMazeSprite (const Sprite *spr);

#endif
/*
 * end fo file sprite.h
 */
