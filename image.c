/*
 * file image.c - maige filenames and sizes
 *
 * $Id: image.c,v 1.6 2004/06/26 03:33:27 iskywalker Exp $
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
#include "image.h"

#include "geom.h"
#include "util.h"

#include "gui.h"

/*
 * local types
 */
typedef struct _shape_list ShapeList;
struct _shape_list {
  char      *name;
  XBBool     valid;
  ShapeList *next;
};

/*
 * global variables
 */

/* search paths */
#ifdef WMS
#define GAME_DATADIR "."
#endif

const char *imgPathBlock  = GAME_DATADIR"/image/block";
const char *imgPathExpl   = GAME_DATADIR"/image/explosion";
const char *imgPathMisc   = GAME_DATADIR"/image/misc";
const char *imgPathScore  = GAME_DATADIR"/image/score";
const char *imgPathSprite = GAME_DATADIR"/image/sprite";

/* background pixmaps */
const char *imgFileTitle  = "title";
const char *imgFileTextBg = "text_bg" ;
const char *imgFileTextFg = "text_fg" ;
/* score board leds */
const char *imgFileScoreLed[] = {
  "led_off" ,
  "led_on" ,
};
/* score board tiles */
const char *imgFileScoreTile[] = {
  "tile_void" ,
  "text_left" ,
  "text_middle" ,
  "text_right" ,
};
/* score player stats */
const char *imgFileScorePlayer[] = {
  "player_dead",
  "player_sick",
  "player",
  "player_abort",
  "player_sick_abort",

};
/* bomb images */
const char *imgFileBomb[MAX_BOMBS][MAX_BOMB_ANIME] = {
  { 
    "bomb_0", "bomb_1", "bomb_2", "bomb_3", "bomb_4", "bomb_5", "bomb_6", "bomb_7",
    "bomb_8", "bomb_9", "bomb_10", "bomb_11", "bomb_12", "bomb_13", "bomb_14", "bomb_15",
    "bomb_x",
  },  
  {
    "mini_0", "mini_1", "mini_2", "mini_3", "mini_4", "mini_5", "mini_6", "mini_7",
    "mini_8", "mini_9", "mini_10", "mini_11", "mini_12", "mini_13", "mini_14", "mini_15",
    "mini_x",
  },
};
/* explosions image */
const char * imgFileExpl[MAX_EXPLOSION] = {
  "expl00",
  "expl01",
  "expl02",
  "expl03",
  "expl04",
  "expl05",
  "expl06",
  "expl07",
  "expl08",
  "expl09",
  "expl0a",
  "expl0b",
  "expl0c",
  "expl0d",
  "expl0e",
  "expl0f",
};

/*
 * offset and size data for sprites
 */
const BMRectangle imgRectSprite[MAX_ANIME] = {
  /* down */
  { 0, 3*BASE_Y, 8*BASE_X, 13*BASE_Y}, 
  { 0, 3*BASE_Y, 8*BASE_X, 13*BASE_Y}, 
  { 0, 3*BASE_Y, 8*BASE_X, 13*BASE_Y}, 
  { 0, 3*BASE_Y, 8*BASE_X, 13*BASE_Y}, 
  { 0, 3*BASE_Y, 8*BASE_X, 13*BASE_Y}, 
  /* up */
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  /* right */
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y}, 
  /* left */
  { 0, 4*BASE_Y, 8*BASE_X, 12*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 12*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 12*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 12*BASE_Y}, 
  { 0, 4*BASE_Y, 8*BASE_X, 12*BASE_Y}, 
  /* damaged */
  { 0,        (4-1)*BASE_Y, 8*BASE_X, 12*BASE_Y },
  { 1*BASE_X,     6*BASE_Y, 8*BASE_X, 10*BASE_Y },
  { 0,        (7+1)*BASE_Y, 8*BASE_X,  8*BASE_Y },
  {-1*BASE_X,     6*BASE_Y, 8*BASE_X, 10*BASE_Y },
  /* looser */
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y },
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y },
  { 0, 4*BASE_Y, 8*BASE_X, 11*BASE_Y },
  /* winner */
  { 0, 2*BASE_Y, 8*BASE_X, 13*BASE_Y },
  { 0, 2*BASE_Y, 8*BASE_X, 13*BASE_Y },
  { 0, 3*BASE_Y, 8*BASE_X, 12*BASE_Y },
  /* big winner */ 
  { -4*BASE_X, (-8 + 5)*BASE_Y, 16*BASE_X, 25*BASE_Y },
  /* skeleton */
  { 0, 5*BASE_Y, 8*BASE_X, 10*BASE_Y },
  { 0, 5*BASE_Y, 8*BASE_X, 11*BASE_Y },
  { 0, 5*BASE_Y, 8*BASE_X,  9*BASE_Y },
  { 0, 5*BASE_Y, 8*BASE_X, 11*BASE_Y },
  /* morphed eyes */
  { 0, 7*BASE_Y, 8*BASE_X, 8*BASE_Y },
  { 0, 5*BASE_Y, 8*BASE_X, 10*BASE_Y },/* zombie */
};
/* epm file formats */
static const char *imgFileSpriteEpm[MAX_ANIME_EPM] = {
  /* looking down */
  "%s_D_S", "%s_D_0", "%s_D_1", "%s_D_2", "%s_D_3",
  /* looking up */
  "%s_U_S", "%s_U_0", "%s_U_1", "%s_U_2", "%s_U_3",
  /* looking right */
  "%s_R_S", "%s_R_0", "%s_R_1", "%s_R_2", "%s_R_3",
  /* looking left */
  "%s_L_S", "%s_L_0", "%s_L_1", "%s_L_2", "%s_L_3",
  /* damaged player */
  "%s_DD", "%s_DL", "%s_DU", "%s_DR",
  /* loosing player */
  "%s_L", "%s_L1", "%s_L2", 
  /* winning player */
  "%s_W", "%s_W2", "%s_W3",
  /* big winning player */
  "%s_B",
};

const char *imgFileSpritePpm[MAX_ANIME_PPM] = {
  /* skeleton */
  "skel_D", "skel_L", "skel_U", "skel_R",
  /* morphed player (eyes only) */
  "morph","zombie",
};

/* icon sprites */
const char *imgFileIcon[MAX_ICON_SPRITES] = {
  /* 6 color sprites */
  "icon_color", "icon_color", "icon_color", "icon_color", "icon_color", "icon_color",
  /* other icons */
  "icon_led_on", "icon_led_off", "icon_abort", "icon_default", 
  /* team icons */
  "icon_led_off", "icon_led_on", "icon_led_on", "icon_led_on",
  /* new absint icons */
  "icon_target", "icon_loser"
};
const BMRectangle imgRectIcon[MAX_ICON_SPRITES] = {
  /* 6 color sprite */
  {   BASE_X,     BASE_Y, 6*BASE_X, 6*BASE_Y, },
  {   BASE_X,     BASE_Y, 6*BASE_X, 6*BASE_Y, },
  {   BASE_X,     BASE_Y, 6*BASE_X, 6*BASE_Y, },
  {   BASE_X,     BASE_Y, 6*BASE_X, 6*BASE_Y, },
  {   BASE_X,     BASE_Y, 6*BASE_X, 6*BASE_Y, },
  {   BASE_X,     BASE_Y, 6*BASE_X, 6*BASE_Y, },
  /* other icons */
  { 5*BASE_X/2, 3*BASE_Y, 3*BASE_X, 3*BASE_Y, },
  { 5*BASE_X/2, 3*BASE_Y, 3*BASE_X, 3*BASE_Y, },
  {   BASE_X,   2*BASE_Y, 6*BASE_X, 5*BASE_Y, },
  {   BASE_X,   2*BASE_Y, 6*BASE_X, 5*BASE_Y, },
  {        0,          0,        0,        0, },
  {        0,          0,        0,        0, },
  {        0,          0,        0,        0, },
  {        0,          0,        0,        0, },
  { 5*BASE_X/2, 3*BASE_Y, 3*BASE_X, 3*BASE_Y, },
  { 5*BASE_X/2, 3*BASE_Y, 3*BASE_X, 3*BASE_Y, },
};

/*
 * local variables
 */

/* shapes */
static int     numShapes  = 0;
static XBAtom *shapeTable = NULL;

/*
 * create list with possible shapes
 */
static ShapeList *
CreateShapeList (const XBDir *epmList)
{
  const char  *s;
  int         len;
  const XBDir *ptr;
  ShapeList   *item;
  ShapeList   *list = NULL;

 for (ptr = epmList; ptr != NULL; ptr = ptr->next) {
    s = strstr (ptr->name, "_B");
    if (NULL != s) {
      len  = s - ptr->name;
      /* found big player sprite */
      item = calloc (1, sizeof (ShapeList));
      assert (item != NULL);
      item->valid = XBTrue;
      item->name  = calloc (len + 1, sizeof (char));
      strncpy (item->name, ptr->name, len);
      /* add to list */
      item->next = list;
      list       = item;
    }
  }
  return list;
} /* CreateShapeList */

/*
 * check for invalid items in shape list
 */
static void
CheckShapeList (ShapeList *list, const XBDir *epmList)
{
  int          i;
  const XBDir *ptr;
  ShapeList   *item;
  char         fileName[80];
  XBBool       valid;

  for (item = list; item != NULL; item = item->next) {
    for (i = 0; i < MAX_ANIME_EPM; i ++) { 
      sprintf (fileName, imgFileSpriteEpm[i], item->name);
      valid    = XBFalse;
      for (ptr = epmList; ptr != NULL; ptr = ptr->next) {
	if (0 == strcmp (fileName, ptr->name) ) {
	  valid = XBTrue;
	  break;
	}
      }
      if (! valid) {
	item->valid = XBFalse;
	break;
      }
    }
  }
} /* CheckShapeList */

/*
 *
 */
static XBAtom *
CreateShapeArray (const ShapeList *list, int *pNum)
{
  int              i;
  XBAtom          *table;
  const ShapeList *item;

  assert (pNum != NULL);

  *pNum = 0;
  for (item = list; item != NULL; item = item->next) {
    if (item->valid) {
      *pNum = *pNum + 1;
    }
  }
  if (0 == *pNum) {
    return NULL;
  }
  table = calloc (*pNum, sizeof (XBAtom) );
  assert (table != NULL);
  i = 0;
#ifdef DEBUG
  fprintf (stderr, "found shape:");
#endif
  for (item = list; item != NULL; item = item->next) {
    if (item->valid) {
#ifdef DEBUG
      fputc (' ', stderr);
      fputs (item->name, stderr);
#endif
      table[i] = GUI_StringToAtom (item->name);
    }
    i ++;
  }  
#ifdef DEBUG
  fputc ('\n', stderr);
#endif
  return table;
} /* CreateShapeArray */

/*
 * Delete shape list
 */
static void
DeleteShapeList (ShapeList *list)
{
  ShapeList  *item, *itemNext;

  for (item = list; item != NULL; item = itemNext) {
    itemNext = item->next;
    free (item->name);
    free (item);
  }  
} /* DeleteShapeList */

/*
 * inititalize valid player shapes
 */
static XBAtom *
InitShapes (int *pNum)
{
  XBDir      *epmList;
  XBDir      *pbmList;
  ShapeList  *list = NULL;
  XBAtom     *table;
#ifdef DEBUG
  Dbg_StartClock ();
#endif

  epmList = CreateFileList (GAME_DATADIR"/image/sprite", "epm");
  pbmList = CreateFileList (GAME_DATADIR"/image/sprite", "pbm");
  /* find possible shapes */
  list = CreateShapeList (epmList);
  /* check them */
  CheckShapeList (list, epmList);
  CheckShapeList (list, pbmList);
  /* create array and delete list */
  table = CreateShapeArray (list, pNum);
  /* delete list */
  DeleteShapeList (list);
  /* delete file list */
  DeleteFileList (epmList);
  DeleteFileList (pbmList);
#ifdef DEBUG
  fprintf (stderr, "load shapes: %lu msec\n", Dbg_FinishClock ());
#endif
  return table;
} /* InitShapes */

/*
 * convert shape name to existing shape
 */
static const char *
ShapeToName (XBAtom shape)
{
  int i;

  if (NULL == shapeTable) {
    shapeTable = InitShapes (&numShapes);
  }
  assert (shapeTable != NULL);
  for (i = 0; i < numShapes; i ++) {
    if (shape == shapeTable[i]) {
      return GUI_AtomToString (shape);
    }
  }
  return "normal";
} /* NameToShape */

/*
 * create name for sprite 
 */
const char *
ImgFileSpriteEpm (XBAtom shape, int anime)
{
  static char tmp[80];

  assert (shape != ATOM_INVALID);
  assert (anime >= 0);
  assert (anime < MAX_ANIME_EPM);
  sprintf (tmp, imgFileSpriteEpm[anime], ShapeToName (shape));
  return tmp;
} /* ImgFileSpriteEpm */

/*
 * list all shapes
 */
const XBAtom *
GetShapeList (int *pNum)
{
  if (NULL == shapeTable) {
    shapeTable = InitShapes (&numShapes);
  }
  assert (shapeTable != NULL);
  *pNum = numShapes;
  return shapeTable;
} /* GetShapeList */

/*
 * delete shape list
 */
void
ClearShapeList (void)
{
  if (NULL != shapeTable) {
    free (shapeTable);
    shapeTable = NULL;
    numShapes  = 0;
  }
} /* ClearShapeList */

/*
 * end of file x11c_data.c
 */

