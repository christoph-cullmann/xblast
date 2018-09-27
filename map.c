/*
 * file map.c - handling the level tile map
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * $Id: map.c,v 1.26 2005/01/14 22:20:54 iskywalker Exp $
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

#include "map.h"

#include "atom.h"
#include "bomb.h"
#include "geom.h"
#include "gui.h"
#include "info.h"
#include "str_util.h"
#include "random.h"
#include "version.h"

/*
 * local types 
 */

/* extra distribution */
enum BMExtraDistribution {
  DEnone = 0, 
  DEsingle, 
  DEall, 
  DEspecial, 
  DEget, 
  DEdouble
} ;
/* one maze column */
typedef unsigned short col[MAZE_H];
/* extra probabilities */
typedef struct {
  int bomb;
  int range;
  int ill;
  int invinc;
  int evil;
} BMExtraProb;

/*
 * local variables
 */ 
static col  em1[MAZE_W];
static col  em2[MAZE_W];
static col *explMaze    = em1;
static col *oldExplMaze = em2;

static col  em3[MAZE_W]; // KOEN
static col *kexplMaze    = em3; // KOEN

static unsigned explDirty;
static unsigned explOldDirty;

static unsigned redrawMaze[MAZE_H];
static unsigned redrawStat[STAT_H+1];
static Sprite *bombSprites[MAZE_W][MAZE_H];

static BMMapTile maze[MAZE_W+1][MAZE_H+1];
static BMMapTile extra[MAZE_W][MAZE_H];
static Sprite *bombSprite[MAZE_W][MAZE_H];

static XBBool tilesInitialized = XBFalse;
static BMBlockTile tile[MAX_BLOCK];
/* Added by VVL (Chat) 12/11/99 */
static unsigned redrawChat;
static unsigned redrawTiles;
static XBBool mapMode=XBFalse;

static int distribExtras;

static BMExtraProb extraProb;

/* conversion table for level parsing */
static DBToInt distribExtraTable[] = {
  { "all",     (int) DEall },
  { "double",  (int) DEdouble },
  { "get",     (int) DEget },
  { "none",    (int) DEnone },
  { "single",  (int) DEsingle },
  { "special", (int) DEspecial },
  { NULL, -1 },
};

/*
 * compare to tile graphics
 */
static XBBool
CompareTile (const BMBlockTile *a, const BMBlockTile *b)
{
  return ( 0      == strcmp (a->name, b->name) &&
	   a->fg  == b->fg &&
	   a->bg  == b->bg &&
	   a->add == b->add);
} /* CompareTile */

void
SetXBMapMode(XBBool mode){
  mapMode=mode;
}

XBBool
GetXBMapMode(void){
  return mapMode;
}


/*
 * parse level data for graphics
 */
XBBool
ParseLevelGraphics (const DBSection *section, DBSection *warn)
{
  int         i;
  const char *s;
  char      **argv[MAX_BLOCK];
  int         argc;

  /* check if section exists */
  if (NULL == section) {
    Dbg_Out("LEVEL: graphics section is missing!\n");
    DB_CreateEntryString(warn,atomMissing,"true");
    return XBFalse;
  }
  /* set tile names to NULL if called for first time */
  if (! tilesInitialized) {
    for (i=0; i< MAX_BLOCK; i++) {
      tile[i].name = NULL;
    }
    tilesInitialized = XBTrue;
  }
  /* parse each entry */
  for (i=0; i< MAX_BLOCK; i++) {
    if (! DB_GetEntryString (section, atomArrayBlock00[i], &s) ) {
      Dbg_Out("LEVEL: critical failure, %s is missing\n", GUI_AtomToString(atomArrayBlock00[i]));
      DB_CreateEntryString(warn,atomArrayBlock00[i],"missing");
      return XBFalse;
    }
    argv[i] = SplitString (s, &argc);
    assert (NULL != argv[i]);
    if (NULL != tile[i].name) {
      free((void *) tile[i].name);
	  tile[i].name=NULL;
    }
    /* check block type */
    if (1 == argc) {
      /* simple rgb pixmap */
      tile[i].name = DupString(argv[i][0]);
      tile[i].fg   = COLOR_INVALID;
      tile[i].bg   = COLOR_INVALID;
      tile[i].add  = COLOR_INVALID;
    } else if (4 == argc) {
      /* layered ppm pixmap */
      tile[i].name = DupString(argv[i][0]);
      if (COLOR_INVALID == (tile[i].fg  = StringToColor (argv[i][1]) ) ||
	  COLOR_INVALID == (tile[i].bg  = StringToColor (argv[i][2]) ) ||
	  COLOR_INVALID == (tile[i].add = StringToColor (argv[i][3]) ) ) {
	free ((char *) argv[i]);
	Dbg_Out("LEVEL: critical failure, invalid value for %s\n", GUI_AtomToString(atomArrayBlock00[i]));
	DB_CreateEntryString(warn,atomArrayBlock00[i],"invalid");
	return XBFalse;
      }
    } else {
      free ((char *) argv[i]);
      Dbg_Out("LEVEL: critical failure, invalid value for %s\n", GUI_AtomToString(atomArrayBlock00[i]));
      DB_CreateEntryString(warn,atomArrayBlock00[i],"invalid");
      return XBFalse;
    }
  }
  return XBTrue;
}

/*
 * configure after parsing
 */
void
ConfigLevelGraphics (const DBSection *section)
{
  int i;
  assert (section != NULL);
  /* load blocks */
  Dbg_Out("checking graphics\n");
  for (i=0; i< MAX_BLOCK; i++) {
    if (tile[i].fg != COLOR_INVALID) {
      GUI_LoadBlockCch (i, tile[i].name, tile[i].fg, tile[i].bg, tile[i].add);
    } else {
      GUI_LoadBlockRgb (i, tile[i].name);
    }
  }
  GUI_InitExplosionBlocks ();
  /* set level info */
  if (CompareTile (tile + BTFree, tile + BTBlock) ) {
    AddLevelInfo ("Walls are invisible");
  }
  if (CompareTile (tile + BTBomb, tile + BTRange) &&
      CompareTile (tile + BTBomb, tile + BTSick) &&
      CompareTile (tile + BTBomb, tile + BTSpecial) ) {
    AddLevelInfo ("Extras all look the same");
  }
  if (CompareTile (tile + BTExtra, tile + BTBlock) ) {
    AddLevelInfo ("Some walls are blastable");
  }
  Dbg_Out("checked graphics\n");
  /* that's all */
} /* ConfigLevelGraphics */

/*
 * string for extra prob
 */
static const char *
ExtraProbString (int val)
{
  if (val <= 0) {
    return NULL;
  } else if (val <=4) {
    return "scarce";
  } else if (val <=8) {
    return "rare";
  } else if (val <=16) {
    return "uncommon";
  } else if (val <=32) {
    return "common";
  } else {
    return "plentiful";
  }
} /* ExtraProbName */

/* 
 * clean up graphics allocated by level
 */
void 
FinishLevelGraphics (void)
{
  int i;

  for (i = 0; i< MAX_TILE; i++) {
    GUI_FreeBlock (i);
  }
  GUI_FreeExplosionBlocks ();
} /* UnloadBlocks */

/*
 * parse level data
 */
XBBool
ParseLevelMap (const DBSection *section, DBSection *warn)
{
  int prob,slowFlame;
  int x, y;
  const char *s;

  /* check if section exists */
  if (NULL == section) {
    Dbg_Out("LEVEL: map section is missing!\n");
    DB_CreateEntryString(warn,atomMissing,"true");
    return XBFalse;
  }
  /* ExtraDistribution has default */
  switch (DB_ConvertEntryInt (section, atomExtraDistribution, (int *) &distribExtras, distribExtraTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomExtraDistribution));
    distribExtras = DEnone;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomExtraDistribution));
    distribExtras = DEnone;
    DB_CreateEntryString(warn,atomExtraDistribution,DB_IntToString(distribExtraTable,distribExtras));
    break;
  default:
    break;
  }
  /* ProbBomb has default */
  if (DB_GetEntryInt (section, atomProbBomb, &prob) ) {
    extraProb.bomb = 64 * prob / 100;
  } else {
    Dbg_Level("default for %s\n", DB_SectionEntryString(section,atomProbBomb));
    extraProb.bomb = 0;
  }
  /* ProbRange has default */
  if (DB_GetEntryInt (section, atomProbRange, &prob) ) {
    extraProb.range = extraProb.bomb + 64 * prob / 100;
  } else {
    Dbg_Level("default for %s\n", DB_SectionEntryString(section,atomProbRange));
    extraProb.range = extraProb.bomb;
  }
  /* ProbVirus has default */
  if (DB_GetEntryInt (section, atomProbVirus, &prob) ) {
    extraProb.ill = extraProb.range + 64 * prob / 100;
  } else {
    Dbg_Level("default for %s\n", DB_SectionEntryString(section,atomProbVirus));
    extraProb.ill = extraProb.range;
  }
  /* ProbSpecial has default */
  if (DB_GetEntryInt (section, atomProbSpecial, &prob) ) {
    extraProb.invinc = extraProb.ill + 64 * prob / 100;
  } else {
    Dbg_Level("default for %s\n", DB_SectionEntryString(section,atomProbSpecial));
    extraProb.invinc = extraProb.ill;
  }
  /* ProbHidden has default */
  if (DB_GetEntryInt (section, atomProbHidden, &prob) ) {
    extraProb.evil = extraProb.invinc + 64 * prob / 100;
  } else {
    Dbg_Level("default for %s\n", DB_SectionEntryString(section,atomProbHidden));
    extraProb.evil = extraProb.invinc;
  }
  if (Version_AtLeast(&Ver_2_10_1)) {
    Dbg_Out("slowMotionBurst!!\n");
    /* SlowMotionBurst has default, since 2.10.1 */
    if (DB_GetEntryInt (section,atomSlowFlame , &prob) ) {
      slowFlame =prob;
    } else {
      Dbg_Level("default for %s\n", DB_SectionEntryString(section,atomSlowFlame));
      slowFlame = 1;
    }
    SetSlowMotionBurst(slowFlame);
  }
  /* the layout */
  for (y = 0; y < MAZE_H; y ++) {
    if (! DB_GetEntryString (section, atomArrayRow00[y], &s) ) {
      Dbg_Out("LEVEL: critical failure, missing %s!\n", DB_SectionEntryString(section,atomArrayRow00[y]));
      DB_CreateEntryString(warn,atomArrayRow00[y],"missing");
      return XBFalse;
    }
    for (x = 0; x < MAZE_W; x ++) {
      switch (s[x]) {
      case '_':
      case 'B':
      case 'R':
      case 'X':
      case 'b':
      case 'r':
      case 's':
      case 'q':
      case 'v':
      case 'e':
      case 'V': break;
      default:
	Dbg_Out("LEVEL: critical failure, invalid entry %s!\n", DB_SectionEntryString(section,atomArrayRow00[y]));
	DB_CreateEntryString(warn,atomArrayRow00[y],"invalid");
	return XBFalse;
      }
    }
  }
  return XBTrue;
}

/*
 * configure level map layout
 */
void
ConfigLevelMap (const DBSection *section)
{
  int x,y;
  const char *s;
  XBBool okay;
  assert (section != NULL);
  /* set level info */
  if (NULL != (s = ExtraProbString (extraProb.bomb) ) ) {
    AddExtraInfo ("Bomb extras are %s", s);
  }
  if (NULL != (s = ExtraProbString (extraProb.range - extraProb.bomb) ) ) {
    AddExtraInfo ("Rang extras are %s", s);
  }
  if (NULL != (s = ExtraProbString (extraProb.ill - extraProb.range) ) ) {
    AddExtraInfo ("Infections are %s", s);
  }
  if (NULL != (s = ExtraProbString (extraProb.invinc - extraProb.ill) ) ) {
    AddExtraInfo ("Special extras are %s", s);
  }
  if (NULL != (s = ExtraProbString (extraProb.evil - extraProb.invinc) ) ) {
    AddExtraInfo ("Hidden bombs are %s", s);
  }
  /* the layout */
  for (y = 0; y < MAZE_H; y ++) {
    okay = DB_GetEntryString (section, atomArrayRow00[y], &s);
    assert (okay);
    for (x = 0; x < MAZE_W; x ++) {
      switch (s[x]) {
      case '_': SetMazeBlock (x, y, BTFree);       break;
      case 'B': SetMazeBlock (x, y, BTBlock);      break;
      case 'R': SetMazeBlock (x, y, BTBlockRise);  break;
      case 'X': SetMazeBlock (x, y, BTExtra);      break;
      case 'b': SetMazeBlock (x, y, BTBomb);       break;
      case 'r': SetMazeBlock (x, y, BTRange);      break;
      case 's': SetMazeBlock (x, y, BTSick);       break;
      case 'q': SetMazeBlock (x, y, BTSpecial);    break;
      case 'v': SetMazeBlock (x, y, BTVoid);       break;
      case 'e': SetMazeBlock (x, y, BTEvil);       break;
      case 'V': SetMazeBlock (x, y, BTBackground); break;
      default: break;
      }
      explMaze[x][y]      = 0;
      oldExplMaze[x][y]  = 0;
      redrawMaze[y]      &= ~(1<<x);
    }
  }
  /* that's all */
} /* ConfigLevelMap */

/*
 * public function: setup_graphics
 */
void
ConfigScoreGraphics (const XBScoreGraphics data)
{
  int i;

  for (i=0; i< MAX_BLOCK; i++) {
    if (NULL != data[i].name) {
      if (data[i].fg == COLOR_INVALID) {
	GUI_LoadBlockRgb (i, data[i].name);
      } else {
	GUI_LoadBlockCch (i, data[i].name, data[i].fg, data[i].bg, data[i].add);
      }
    } else {
      GUI_FreeBlock (i);
    }
  }
  GUI_InitExplosionBlocks ();
} /* ConfigScoreGraphics */

/*
 * public function setup_map
 */
void
ConfigScoreMap (const XBScoreMap data)
{
  int x, y;

  for (x = 0; x < MAZE_W; x++) {
    for (y = 0; y < MAZE_H; y++) {
      explMaze[x][y]      = 0;
      oldExplMaze[x][y]  = 0;
      redrawMaze[y]      &= ~(1<<x);
      SetMazeBlock (x, y, (BMMapTile) data[x][y]);
    }
  }
} /* ConfigScoreMap */

/* 
 *
 */
void 
DrawMaze (void)
{
  int x, y;

  ClearRedrawMap ();
  for (x = 0; x < MAZE_W; x ++) {
    for (y = 0; y < MAZE_H; y ++) {
      GUI_DrawBlock (x, y, maze[x][y]);
    }
  }
  /* if(mapMode){
  for (x = 0; x < MAZE_W; x ++) {
    GUI_DrawBlock (x, MAZE_H, maze[x][MAZE_H]);

  }


  }*/
  GUI_FlushBlocks ();
} /* DrawMaze */

/* 
 * create list of rectangles to be redrawn
 */
void 
SetRedrawRectangles (void)
{
  int x,y;

  for (y = 0; y < MAZE_H; y ++) {
    if ((explDirty | explOldDirty) & (1<<y)) {
      for (x = 0; x < MAZE_W; x ++) { 
	if (redrawMaze[y] & (1<<x)) { 
	  GUI_AddMazeRectangle (x, y);
	} else if (explMaze[x][y] != oldExplMaze[x][y])  {
	  GUI_AddMazeRectangle (x, y);
	  redrawMaze[y] |= (1<<x);
	}
      }
    } else if (redrawMaze[y]) {
      for (x = 0; x < MAZE_W; x ++) { 
	if (redrawMaze[y] & (1<<x)) { 
	  GUI_AddMazeRectangle (x, y);
	} 
      }
    }
  }
  
  for (y = 0; y < STAT_H; y ++) {
    if (redrawStat[y]) {
      for (x = 0; x < STAT_W; x++) { 
	if (redrawStat[y] & (1<<x)) {
	  GUI_AddStatRectangle (x, y);
	}
      }
    }
  }
  if(redrawTiles){
    for (x=0; x<STAT_W; x++) {
      if (redrawTiles & (1<<x)) {
	GUI_AddTilesRectangle(x, 0);
      }
    }
  }
  /* Added by VVL (Chat) 12/11/99 : Begin */
  if (redrawTiles) {
    for (x=0; x<STAT_W; x++) {
      if (redrawTiles & (1<<x)) {
	GUI_AddTilesRectangle(x, 0);
      }
    }
  }
  /* Added by VVL (Chat) 12/11/99 : Begin */
  if (redrawChat) {
    for (x=0; x<STAT_W; x++) {
      if (redrawChat & (1<<x)) {
	GUI_AddChatRectangle(x, STAT_H);
      }
    }
  }
  /* Added by VVL (Chat) 12/11/99 : End */
} /* SetRedrawRectangles */

/* 
 * mark region of tiles as to be redrawn
 */
void 
MarkMaze (int x1, int y1, int x2, int y2)
{
  int x, y;

#ifdef SMPF
  for (y = MAX (y1, 0); y <= MIN (y2, MAZE_H+3); y ++) {
#else
  for (y = MAX (y1, 0); y <= MIN (y2, MAZE_H+2); y ++) {
#endif
    if (y < MAZE_H) {
      for (x = MAX (x1, 0); x <= MIN (x2, MAZE_W-1); x ++) {
	redrawMaze[y] |= (1<<x);
      }
    } 
#ifdef SMPF
    /* Added by VVL (Chat) 12/11/99 : Begin */
    else if (y==MAZE_H+3) {
#else
    else if (y==MAZE_H+2) {
#endif
      for (x=MAX(x1,0); x<=MIN(x2,STAT_W-1); x++) {
	redrawChat |= (1<<x);
	//	redrawStat[y-MAZE_H] |= (1<<x);
      }
      if(MAZE_H==y){
      for (x=MAX(x1,0); x<=MIN(x2,STAT_W-1); x++) {
	redrawTiles |= (1<<x);
      }
      }

      /* Added by VVL (Chat) 12/11/99 : End */
    } else {/*
      if(mapMode){
	if (y==MAZE_H) {
	  fprintf(stderr,"mariking map mode \n");
	  for (x = MAX (x1, 0); x <= MIN (x2, STAT_W - 1); x++) {
	    redrawTiles |= (1<<x);
	  }
	}
      }
      else{*/
	for (x = MAX (x1, 0); x <= MIN (x2, STAT_W - 1); x++) {
	  redrawStat[y-MAZE_H] |= (1<<x);
	 }
	//}
    }
  }
} /* MarkMaze */

/* 
 * mark single tile to be redrawn
 */
void
MarkMazeTile (int x, int y)
{
  if (-1<y && y < MAZE_H) {
    redrawMaze[y] |= (1<<x);
  } else {
    redrawStat[y-MAZE_H] |= (1<<x);
  }
} /* MarkMazeTile */

/* 
 * mark rectangle (in pixel coordinates) to be redrawn 
 */
void
MarkMazeRect (int x, int y, int w, int h)
{
  if (y + h < PIXH) {
    MarkMaze (x/BLOCK_WIDTH, y/BLOCK_HEIGHT, (x+w-1)/BLOCK_WIDTH, (y+h-1)/BLOCK_HEIGHT);
  } else if (y > PIXH) {
    MarkMaze (x/STAT_WIDTH, y/BLOCK_HEIGHT, (x+w-1)/STAT_WIDTH, (y+h-1)/BLOCK_HEIGHT);
  } else {
    MarkMaze (x/BLOCK_WIDTH, y/BLOCK_HEIGHT, (x+w-1)/BLOCK_WIDTH, MAZE_H - 1);
    MarkMaze (x/STAT_WIDTH, MAZE_H, (x+w-1)/STAT_WIDTH, (y+h-1)/BLOCK_HEIGHT);
  }
} /* MarkMazeRect */

/* 
 *
 */
XBBool
SpriteMarked (const Sprite *spr) 
{
  int left, right, top, bottom;
  const BMRectangle *r;
  int      x, y;
  unsigned mask;

  /* get sprite drawing rectangle */
  r = SpriteRectangle (spr);
  /* get borders of rectangle */
  left   = (r->x)            / BLOCK_WIDTH;
  right  = (r->x + r->w - 1) / BLOCK_WIDTH;
  top    = (r->y)            / BLOCK_HEIGHT;
  bottom = (r->y + r->h - 1) / BLOCK_HEIGHT;

  /* set redraw x test-mask */
  mask = 0;
  for (x=left; x<=right; x++) {
    mask |= (1<<x);
  }
  /* check if any rectangle under sprite must be redrawn */
  for (y = top; y <= bottom; y ++) {
    /* check for blocks under sprite to be redrawn */
    if (redrawMaze[y] & mask) {
      return XBTrue;
    }
  }
  return XBFalse;
} /* SpriteMarked */

/*
 *
 */
void 
ClearRedrawMap (void)
{
  col *swap;

  swap        = oldExplMaze;
  oldExplMaze = explMaze;
  explMaze    = swap;

  explOldDirty = explDirty;
  explDirty    = 0;
  /* Added by VVL (Chat) 12/11/99 */
  redrawChat=0;
  redrawTiles=0;

  memset (explMaze, 0, MAZE_W*sizeof(col) );
  memset (redrawMaze, 0, sizeof(redrawMaze) );
  memset (redrawStat, 0, sizeof(redrawStat) );
} /* ClearRedrawMap */

/* 
 *
 */
void 
UpdateMaze (void)
{
  int x, y;
  /*if(mapMode){
    height=MAZE_H+1;
  }
  else{
    height=MAZE_H;
    }*/
     
  for (y = 0; y <MAZE_H ; y++) {
    if (redrawMaze[y]) {
      /* check if explosions are needed in this row */
      if (explDirty & (1<<y)) {
	for (x = 0; x < MAZE_W; x++) {
	  if (redrawMaze[y] & (1<<x)) {
	    if (explMaze[x][y] & 0x10) {
	      GUI_DrawExplosion (x, y, explMaze[x][y] & 0xf);
	    } else {
	      GUI_DrawBlock (x, y, (int) maze[x][y]);
	    }
	  }
	}
      } else {
	/* only blocks are needed */
	for (x = 0; x < MAZE_W; x ++) {
	  if (redrawMaze[y] & (1<<x)) {
	    GUI_DrawBlock (x, y, (int) maze[x][y]);
	  }
	}
      }
    }
  }
  GUI_FlushBlocks ();
} /* UpdateMaze */

/*
 *
 */
void 
UpdateExpl (void)
{
  int x,y;
  for (y = 0; y < MAZE_H; y ++) {
    if (redrawMaze[y]) {
      for (x = 0; x < MAZE_W; x ++) {
	if ( (redrawMaze[y] & (1<<x)) && 
	     (explMaze[x][y] & 0x10) ) {
	  GUI_DrawExplosionSprite (x, y, explMaze[x][y] & 0xf);
	}
      }
    }
  }
} /* UpdateExpl */

/*
 *
 */
XBBool 
CheckMaze (int x, int y)
{
  return (maze[x][y] == BTBlock || 
	  maze[x][y] == BTBlockRise || 
	  maze[x][y] == BTExtra || 
	  maze[x][y] == BTExtraOpen  || 
	  maze[x][y] == BTVoid || 
	  maze[x][y] < 0);

} /* CheckMaze */

/* 
 *
 */
XBBool 
CheckMazeFree (int x, int y)
{
  return (maze[x][y] == BTFree || 
	  maze[x][y] == BTBurned );
} /* CheckMazeFree */

/* 
 *
 */
XBBool 
CheckMazeFree2 (int x, int y)
{
  return (maze[x][y] == BTFree || 
	  maze[x][y] == BTVoid || // koen
	  maze[x][y] == BTBurned );
} /* CheckMazeFree */

/*
 *
 */
XBBool
CheckMazeWall (int x, int y)
{
  return (maze[x][y] == BTBlock || 
	  maze[x][y] == BTBlockRise);
} /* CheckMazeWall */

/* public function check_maze_phantom_wall */

 /* Written by Amilhastre */ int 
 /* Written by Amilhastre */ CheckMazePhantomWall (int x,
 /* Written by Amilhastre */ 		 int y)
 /* Written by Amilhastre */ {
 /* Written by Amilhastre */   return((maze[x][y]==BTBlock) || (maze[x][y]==BTBlockRise) || (maze[x][y]==BTVoid));
 /* Written by Amilhastre */ }
/* Skywalker go through walls*/
int   CheckMazeGhost (int ghost,int x,
  		 int y)
  {
    if(!ghost) { return CheckMaze(x,y); }
    return( (maze[x][y]==BTVoid )  				  
	    || (maze[x][y]<0)
	    || (x<=0)  
	    || (y<=0)
	    || (x>MAZE_H )
	    || (y>MAZE_W-4 )
				  );
  }
XBBool 
CheckMazeSolid (int x, int y)
{
  return (maze[x][y] == BTBlock || 
	  maze[x][y] == BTBlockRise || 
	  maze[x][y] == BTExtra || 
	  maze[x][y] == BTExtraOpen);
} /* CheckMazeSolid */

/*
 *
 */
XBBool 
CheckMazeOpen (int x, int y)
{
  return (maze[x][y] == BTFree ||
	  maze[x][y] == BTBurned ||
	  maze[x][y] == BTVoid);
} /* CheckMazeOpen */

/* 
 *
 */
XBBool 
CheckMazeExtra (int x, int y)
{
  return (maze[x][y] == BTExtra);
} /* CheckMazeExtra */

/*
 *
 */
XBBool
CheckExplosion (int x, int y)
{
  return (0 != explMaze[x][y]);
} /* CheckExplosion */


/*
 *
 */
void
SetExplBlock (int x, int y, int value)
{
  if((x>=0) && (y>=0) && (x<MAZE_W) && (y<MAZE_H)) {
    explMaze[x][y] |= value;
    kexplMaze[x][y] = !(explMaze[x][y]==20);  // KOEN
    explDirty      |= (1<<y);
  }
} /* SetExplBlock */

/*
 *
 */
void
SetBlockExtra (int x, int y, BMMapTile value)
{
  extra[x][y] = value;
} /* SetBlockExtra */

/*
 *
 */
BMMapTile
GetBlockExtra (int x,int y)
{
  return extra[x][y] ;
} /* SetBlockExtra */

BMMapTile
CheckBonuses (int x,int y )
 {
   return( (maze[x][y]==BTBomb)
 	 || (maze[x][y]==BTRange) 
 	 || (maze[x][y]==BTSpecial)
	   //  || (maze[x][y]==BTExtra)
	   );
 }
BMMapTile
CheckBonuses2 (int x,int y )
 {
   return(  (maze[x][y]==BTSpecial)
	   //  || (maze[x][y]==BTExtra)
	   );
 }

void 
DeleteAllMapBombSprites(void){
  int x,y;
  for (x = 0; x < MAZE_W; x++) {
    for (y = 0; y < MAZE_H; y++) {
      if(bombSprite[x][y]){
	DeleteAllBombSprites();
	bombSprite[x][y]=NULL;
      }
    }
  }

}
/* 
 * 
 */
void 
SetMazeBlock (int x, int y, BMMapTile block)
{
  int rnd;

#ifdef DEBUG_MAP
  Dbg_Out ("SetMazeBlock (%2d,%2d,%2d)\n", x, y, block);
#endif

  MarkMazeTile (x, y);

  switch (block) {
  case BTExtra:
    if(mapMode){
      if(bombSprite[x][y]!=NULL){
	DeleteSprite(bombSprite[x][y]);
	bombSprite[x][y]=NULL;
      }
    }
    rnd = GameRandomNumber (64);
    if (rnd < extraProb.bomb) {
      extra[x][y] = BTBomb;
    } else if (rnd < extraProb.range) {
      extra[x][y] = BTRange;
    } else if (rnd < extraProb.ill) {
      extra[x][y] = BTSick;
    } else if (rnd < extraProb.invinc) {
      extra[x][y] = BTSpecial;
    } else if (rnd < extraProb.evil) {
      extra[x][y] = BTEvil;
    } else {
      extra[x][y] = BTFree;
    }
    maze[x][y] = block;
    break;
  case BTEvil:

    if(mapMode){
      if(x==1 && y==13){
	maze[x][y]  = BTBlock;
      }
      else{

      maze[x][y]  = BTFree;

      }
      if(bombSprite[x][y]==NULL){
	bombSprite[x][y]= CreateBombSprite (BB_NORMAL, 
				      x*BLOCK_WIDTH, y*BLOCK_HEIGHT,
				      0, SPM_MAPPED);
	fprintf(stderr," creating sprite \n");
	GUI_FlushBlocks ();
      }
    
    }
    else{
    maze[x][y]  = BTFree;
    extra[x][y] = BTFree;
    NewEvilBomb (x, y);
    }
    break;
  case BTExtraOpen:
    if(mapMode){
      if(bombSprite[x][y]!=NULL){
	DeleteSprite(bombSprite[x][y]);
	bombSprite[x][y]=NULL;
      }
    }
    maze[x][y]  = block;
    break;
  default:
    if(mapMode){
      if(bombSprites[x][y]){
      DeleteSprite( bombSprites[x][y]);
      bombSprites[x][y]= NULL;
      }
    }
    maze[x][y]  = block;
    extra[x][y] = BTFree;
    break;
  }
} /* SetMazeBlock */
/* 
 * 
 */
BMMapTile 
GetMazeBlock (int x, int y){

  return  maze[x][y];

}
/*
 * get extra on position (x,y)
 */
int 
GetExtra (int invincible, int x, int y)
{
  int extraBlock;

  extraBlock = (maze[x][y] <= BTExtraOpen) ? 0 : maze[x][y];
  if ( (invincible>0)  && (extraBlock == BTSick)) {
    extraBlock = 0;
  }

  //  if((x==1) && (y==4)) { Dbg_Out(" +++++++++++ %i %i \n", maze[x][y], extraBlock);}
  if (extraBlock != 0) {
    if(maze[x][y]!=BTVoid) {
      SetMazeBlock (x, y, BTFree);
    }
  }

  /* check if special extras is distributed immediately */
  if ( (extraBlock == BTSpecial) &&
       ( (distribExtras == DEget) || (distribExtras == DEdouble) ) ) {
    DistributeExtras (0, 0, -1, 0);
  }

  return extraBlock ;
} /* GetExtra */

/* 
 * check if special extra are distributed
 */
XBBool
DistribSpecial (void)
{
  return (distribExtras == DEspecial);
} /* DistribSpecial */

/*
 *
 */
static int
PutExtras (int freeBlocks, BMPosition *distPos, BMMapTile tile, int numExtras)
{
  int i, where;
  BMPosition swap;

  for (i = 0; (freeBlocks > 0) && (i < numExtras); i++) {
    where = GameRandomNumber (freeBlocks);
    SetMazeBlock (distPos[where].x, distPos[where].y, tile);
    freeBlocks --;
    /* this position is used */
    if (where != freeBlocks) {
      swap                = distPos[where];
      distPos[where]      = distPos[freeBlocks];
      distPos[freeBlocks] = swap;
    }
  }
  return freeBlocks;
} /* PutExtras */

/* 
 *
 */
void 
DistributeExtras (int bombs, int range, int extras, int specials)
{
  int x, y;
  int i;
  int freeBlocks = 0;
  int numExtras  = 0;
  static unsigned   distExtra[MAZE_H];
  static BMPosition distPos[MAZE_W*MAZE_H];
    
  /* are there extras to distribute */
  if (bombs + range + extras + specials == 0)  {
    return;
  }
  /* Create Extra Distribution Map */
  memset (distExtra, 0, sizeof (distExtra));
  /* First check for free Blocks */
  for (x = 0; x < MAZE_W; x++) {
    for (y = 0; y < MAZE_H; y++) {
      if (maze[x][y] == BTBurned || 
	  maze[x][y] == BTFree) {
	distExtra[y] |= (1 << x);
	freeBlocks ++;
      }
    }
  }
  freeBlocks = CheckDistribExpl (distExtra, freeBlocks);
  if (freeBlocks <= 0) {
    return;
  }
  /* fill dist_koord array */
  i = 0;
  for (x = 0; x < MAZE_W; x ++) {
    for (y = 0; y < MAZE_H; y ++) {
      if (distExtra[y] & (1 << x)) {
	distPos[i].x = x;
	distPos[i].y = y;
	i ++;
      }
    }
  }
    
  /* Distribute special extras */
  /* use -1 for direct distribution on get extra */
  switch (distribExtras) {
  case DEnone:    numExtras = 0;                         break;
  case DEsingle:  numExtras = (extras > 0) ? 1 : 0;      break;
  case DEall:     numExtras = (extras > 0) ? extras : 0; break;
  case DEspecial: numExtras = (specials + 2)/3;          break;
  case DEget:     numExtras = (extras < 0) ? 1 : 0;      break;
  case DEdouble:  numExtras = (extras < 0) ? 2 : 0;      break;
  }
  freeBlocks = PutExtras (freeBlocks, distPos, BTSpecial, numExtras);
  if (freeBlocks <= 0) {
    return;
  }
  /* distribute range extras */
  freeBlocks = PutExtras (freeBlocks, distPos, BTRange, range);
  if (freeBlocks <= 0) {
    return;
  }
  /* distribute bombs */
  freeBlocks = PutExtras (freeBlocks, distPos, BTBomb, bombs);
  if (freeBlocks <= 0) {
    return;
  }
} /* DistributeExtras */

/* 
 *
 */
void
BlastExtraBlock (int x, int y)
{
  switch(maze[x][y]) {
    /* open extra block */
  case BTExtraOpen:
    SetMazeBlock (x, y, extra[x][y]);
    break;
    /* special extras are redistributed in DEget mode */
  case BTSpecial:
    if (DEget == distribExtras || 
	DEdouble == distribExtras ) {
      DistributeExtras (0, 0, -1, 0);
    }
    /* blast away extra and correct shadow */
  case BTSick:
  case BTBomb:
  case BTRange:
    SetMazeBlock (x, y, BTFree);
    break;
  default:
    break;
  }
} /* BlastExtraBlock */

/*
 *
 */
void 
CopyExplBlock (int x, int y, const int block[CHARH][CHARW])
{
  int xp, yp;

  for (xp = x; xp < x+CHARW; xp ++) {
    for (yp = y; yp < y+CHARH; yp ++) {
      explMaze[xp][yp] = block[yp-y][xp-x];
      explDirty       |= (1<<yp);
      redrawMaze[yp]  |= (1<<xp);
    }
  }
} /* CopyExplBlock */

/*
 * end of file map.c
 */






