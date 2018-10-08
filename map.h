/*
 * file map.h - managing the level tile map
 *
 * $Id: map.h,v 1.12 2006/02/09 21:21:24 fzago Exp $
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
#ifndef XBLAST_MAP_H
#define XBLAST_MAP_H

/*
 * type definitions
 */

/* tiles used in game */
typedef enum
{
	BTEvil = -2,				/* virtual tile: free space with bomb */
	BTBackground = -1,			/* virtual tile: don't draw this block */
	BTFree = 0,					/* free tile */
	BTBurned,					/* burned, free tile */
	BTBlock,					/* solid wall */
	BTBlockRise,				/* solid wall (rising) */
	BTExtra,					/* extra block */
	BTExtraOpen,				/* extras block (cracked/opening) */
	BTBomb,						/* bomb extra */
	BTRange,					/* range extra */
	BTSick,						/* illness extra */
	BTSpecial,					/* special extra */
	BTVoid,						/* empty space for decoration */
	BTNUM
} BMMapTile;

/* graphics definition for single tile */
typedef struct
{
	const char *name;
	XBColor fg;
	XBColor bg;
	XBColor add;
} BMBlockTile;

/* graphics definition for tile */
typedef BMBlockTile XBScoreGraphics[MAX_BLOCK];

/* layout for score board */
typedef BMMapTile XBScoreMap[MAZE_W][MAZE_H];

/*
 * prototypes
 */
extern XBBool ParseLevelGraphics (const DBSection * section, DBSection * warn);
extern void ConfigLevelGraphics (const DBSection * section);
extern XBBool ParseLevelMap (const DBSection * section, DBSection * warn);
extern void ConfigLevelMap (const DBSection * section);
extern void ConfigScoreGraphics (const XBScoreGraphics data);
extern void ConfigScoreMap (const XBScoreMap data);
extern void FinishLevelGraphics (void);
extern void DrawMaze (void);
extern void SetRedrawRectangles (void);
extern void MarkMaze (int x1, int y1, int x2, int y2);
extern void MarkMazeTile (int x, int y);
extern void MarkMazeRect (int x, int y, int w, int h);
extern XBBool SpriteMarked (const Sprite * spr);
extern void ClearRedrawMap (void);
extern void UpdateMaze (void);
extern void UpdateExpl (void);
extern XBBool CheckMaze (int x, int y);
extern XBBool CheckMazeFree (int x, int y);
extern XBBool CheckMazeFree2 (int x, int y);
extern XBBool CheckMazeOpen (int x, int y);
extern XBBool CheckMazeWall (int x, int y);
extern XBBool CheckMazeSolid (int x, int y);
extern XBBool CheckMazeExtra (int x, int y);
extern XBBool CheckExplosion (int x, int y);
extern void SetBlockExtra (int x, int y, BMMapTile extra);
extern void SetMazeBlock (int x, int y, BMMapTile block);
extern void SetExplBlock (int x, int y, int value);
extern int GetExtra (int invincible, int x, int y);
extern XBBool DistribSpecial (void);
extern void DistributeExtras (int bombs, int range, int extras, int specials);
extern void BlastExtraBlock (int x, int y);
extern void CopyExplBlock (int x, int y, const int block[CHARH][CHARW]);
extern BMMapTile GetBlockExtra (int x, int y);
extern BMMapTile CheckBonuses (int x, int y);
extern BMMapTile CheckBonuses2 (int x, int y);
extern int CheckMazePhantomWall (int x, int y);
extern int CheckMazeGhost (int ghost, int x, int y);
extern BMMapTile GetMazeBlock (int x, int y);
extern void DeleteAllBombSprites (void);
extern void DeleteAllMapBombSprites (void);
extern void SetXBMapMode (XBBool mode);
extern XBBool GetXBMapMode (void);
#endif
/*
 * end of file map.h
 */
