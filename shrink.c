/*
 * file shrink.c - level shrink and scrambling blocks
 *
 * $Id: shrink.c,v 1.14 2005/01/11 22:44:41 lodott Exp $
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
#include "shrink.h"

#include "atom.h"
#include "bomb.h"
#include "geom.h"
#include "info.h"
#include "map.h"
#include "snd.h"

/*
 * local constants
 */
#define SHRINK_WARN_OFFSET    55

/* 
 * shrink element structure 
 */

/* generic shrink data 
typedef struct {
  int time;
  BMMapTile block;
  short x,y;
} ShrinkGeneric;
*/
/* gd shrink data */
typedef struct {
  int x,y;
  int offset;
  int level;
} shri_data;

typedef int shri_xoff_data;

typedef struct {
  int offset;
  int level;
  int block;
} shri_style2;

typedef struct {
  int num;
  shri_style2 *styl;
} shri_style;

/* Scramble Structure */
typedef struct {
  int time;
  int num_blocks;
  BMPosition *blocks;
} ScrambleStruct; 



/*
 * local variables - retrieved from level data
 */
static int shrinkType;
static char* stVoidName = "No Shrink";

/*
 * other local variables
 */
static SND_Id shrinkSound     	     	= SND_SPIRAL;
static XBBool soundFlipflop    	     	= XBTrue;
static XBBool playShrink       	     	= XBTrue;
static ShrinkGeneric *shrink_ptr    	= NULL;
static ShrinkGeneric *shrink_ptr1    	= NULL;
static ShrinkGeneric *shrink_data   	= NULL;
static ShrinkGeneric shrink_data_none[] = {
  /* terminator */
  {2*GAME_TIME , 0, 0, 0},
};

/* conversion table */
static DBToInt shrinkTable[] = {
  { "circle",         ST_Circle },
  { "compound",       ST_Compound },
  { "compound2F",     ST_Compound2F },
  { "compoundExtra",  ST_CompoundExtra },
  { "compoundF",      ST_CompoundF },
  { "compoundSolid",  ST_CompoundSolid },
  { "constrictWave",  ST_ConstrictWave },
  { "diag",           ST_Diag },
  { "down",           ST_Down },
  { "downF",          ST_DownF },
  { "earlySpiral",    ST_EarlySpiral },
  { "horizontal",     ST_Horiz },
  { "istyCompound2F", ST_IstyCompound2F },
  { "istySpiral",     ST_IstySpiral3 },
  { "lazyCompoundF",  ST_LazyCompoundF },
  { "move",           ST_Move },
  { "outwardExtra",   ST_OutwardExtra },
  { "outwardSpiral",  ST_OutwardSpiral },
  { "quad",           ST_Quad },
  { "savageCompound", ST_SavageCompound },
  { "speedSpiral",    ST_SpeedSpiral },
  { "spiral",         ST_Spiral },
  { "spiral23",       ST_Spiral23 },
  { "spiral3",        ST_Spiral3 },
  { "spiral5",        ST_Spiral5 },
  { "spiralLego",     ST_SpiralLego },
  { "spiralPlus",     ST_SpiralPlus },
  { "void",           ST_Void },
  { NULL, -1 },
};
/* shrink info text */
static const char *shrinkName[NUM_ST] = {
  NULL,
  "Spiral shrinking at half time",
  "Fast spiral shrinking at half time",
  "Spiral shrinking at half time",
  "3 level spiral shrinking at half time",
  "Fast spiral shrinking at half time",
  "1 level spiral shrinking at three quarter of time",
  "Spiral shrinking just before half time",
  "Continuous compound shrinking",
  "Continuous compound shrinking",
  "2 level compound shrinking",
  "Lazy compound shrinking",
  "Continuous compound shrinking",
  "Double continous compound shrinking",
  "Compound shrinking with blastables",
  "Continuous downward shrinking",
  "Continuous downward shrinking",
  "Quad shrinking at half time",
  "3 level wave shrink at half time",
  "Anticlockwise outward spiralling shrink",
  "Early Horizontal shrink ",
  "Cirlcle shrink (rephrase)",
  "Moving shrink (rephrase)",
  "Isty spiral 3 (rephrase)",
  "Isty compound 2F (rephrase)",
  "Diagonal shrink (rephrase)",
  "Outward compound extra (rephrase)",
  "Spiral5 (rephrase)"
};

/*
 * include shrinkdata 
 */
#include "shrinkdat.h"

/* XBCC */
int 
getShrinkTimes(int p_time) {
  static ShrinkGeneric *s_ptr    	= NULL;
  if(p_time<0) {
    s_ptr=shrink_data;
  }
  if(s_ptr == NULL) {
    return 0;
  }
  while (p_time == s_ptr->time) {
    s_ptr++;
  }
  if(s_ptr->time == 2*GAME_TIME) {
    return 0;
  } else {
    return s_ptr->time;
  }
}

/*
 *
 */
static int
CmpShrink (const void *a, const void *b)
{
  return ( ((ShrinkGeneric *)a)->time - ((ShrinkGeneric *)b)->time);
} /* CmpShrink */

/*
 *
 */
static void
SortShrinkArray (ShrinkGeneric *data, int nelem)
{
  qsort(data, nelem, sizeof(ShrinkGeneric), CmpShrink);
} /* SortShrinkArray */

ShrinkGeneric *
GetShrinkPtr ()
{
  return shrink_ptr;
}

/*
 *
 */
void
DoShrink (int g_time)
{
  if (g_time == shrink_ptr->time - SHRINK_WARN_OFFSET) {
    SND_Play (SND_WARN, SOUND_MIDDLE_POSITION);
  }
  //  fprintf(stderr," time %i %s \n",g_time,GetShrinkName(shrinkType));
  while ( (g_time) == shrink_ptr->time ) {
    // fprintf(stderr," %i %i \n",shrink_ptr->x, shrink_ptr->y);
    /* set block */
    SetMazeBlock(shrink_ptr->x, shrink_ptr->y, shrink_ptr->block);

    if (playShrink) {
      if (soundFlipflop) {
	SND_Play (shrinkSound, (shrink_ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
	soundFlipflop = XBFalse;
	playShrink    = XBFalse;
      } else {
	soundFlipflop = XBTrue;
      }
    }
    
    /* for solid blocks kill players and delete bombs */
    if ( (shrink_ptr->block == BTVoid) ||(shrink_ptr->block == BTBlock) || (shrink_ptr->block == BTExtra) ) {
      KillPlayerAtGhost(shrink_ptr->block,shrink_ptr->x, shrink_ptr->y);
      DeleteBombAt(shrink_ptr->x,shrink_ptr->y);
    }
    shrink_ptr ++;
  }
  playShrink = XBTrue;
} /* DoShrink */

void
DoShrinkMapEdit (int g_time)
{
  shrink_ptr=shrink_ptr1;
  if (g_time == shrink_ptr->time - SHRINK_WARN_OFFSET) {
    SND_Play (SND_WARN, SOUND_MIDDLE_POSITION);
  }
  while ( (g_time) != shrink_ptr->time && shrink_ptr->time!=0&& shrink_ptr->time<g_time  ) {
       /* set block */
    SetMazeBlock(shrink_ptr->x, shrink_ptr->y, shrink_ptr->block);
    shrink_ptr ++;
  }
  playShrink = XBTrue;
} /* DoShrink */
/*
 *  Generic shrink function 
 */
static void
CreateGenericShrink (shri_data *data, shri_xoff_data *xoffdata, int startlevel, int endlevel, int inclevel,
		     int starttime, int levelinctime, int offsetinctime, XBBool flags, shri_style *style)
{
  int nelem;
  int i, st;
  ShrinkGeneric *dst;
  int acelm;

  int num_styles;
  int inclevel2;

  int st_offset;
  int st_level;
  int block;
  int offset;
  int xoff;
  int bigxoff;

  int reallevel;
  int dellevel;
  int levelstep;

#ifdef DEBUG_SHRINK
  fprintf (stderr, "*GDS* Call\n");
  fprintf (stderr, "startlevel: %3d endlevel: %3d inclevel: %3d starttime: %3d",
	   startlevel, endlevel, inclevel, starttime);
  fprintf (stderr, "levelinctime: %3d offsetinctime: %3d flags: %1d\n",
	   levelinctime, offsetinctime, flags);
#endif

  /* Calculate the space required */
  /* This is the maximum that could be used, without checking for levels etc. */
  num_styles = style->num;
  for (nelem=0; data[nelem].x != -1; nelem++) {   /* per style*/
  }
  nelem *= num_styles;     /* Total elements */
  nelem++; /* Plus a terminator */
#ifdef DEBUG_SHRINK
  fprintf (stderr, "*GDS* Numstyles = %d nelem = %d\n",num_styles,nelem);
#endif

#ifdef DEBUG_SHRINK
  fprintf (stderr, "A total of %d blocks are needed\n", nelem);
#endif
  
  /* alloc blocks memory*/
  assert (shrink_data == NULL);
  shrink_data = (ShrinkGeneric *) malloc(nelem*sizeof(ShrinkGeneric));
  assert (shrink_data != NULL);

  dst = shrink_data; /* Pointer to current entry */

  acelm = 0;
  /* To adjust xoff for start level */
  if (flags) {
    bigxoff = xoffdata[startlevel];
  } else {
    bigxoff = 0;
  }

  for (st=0; st<num_styles; st++) {
    st_offset = style->styl[st].offset;
    st_level = style->styl[st].level;
    block = style->styl[st].block;

    for (i=0; data[i].x != -1; i++) {
      offset = data[i].offset;
      reallevel = data[i].level - st_level;
      /* Check if in range */
      if ( ( (inclevel>0) && 
	     (reallevel >= startlevel) && 
	     (reallevel <= endlevel)) || 
	   ( (inclevel<0) && 
	     (reallevel <= startlevel) && 
	     (reallevel >= endlevel)) ) {
        dellevel = reallevel - startlevel;
        inclevel2 = inclevel;
        if ( (dellevel < 0) && 
	     (inclevel < 0) ) {
          dellevel = -dellevel;
          inclevel2 = inclevel2;
        }
        if ( (dellevel >= 0) && 
	     (inclevel >= 0) && 
	     ! (dellevel % inclevel) ) {
          levelstep = dellevel / inclevel;
          if (flags) {
            xoff        = xoffdata[reallevel];
	    shrinkSound = SND_SPIRAL;
          } else {
            xoff        = 0;
	    shrinkSound = SND_COMPOUND;
          }
          dst->time = starttime + (levelstep * levelinctime)
               + ((offset + xoff - bigxoff)* offsetinctime) + st_offset;
          dst->x = data[i].x;
          dst->y = data[i].y;
          dst->block = block;
#ifdef DEBUG_SHRINK
          fprintf (stderr, "*GDS* dst: %9d i: %3d st: %2d Time: %5d X: %3d Y: %3d Block: %2d\n",
               (int)dst, i, st, dst->time, dst->x, dst->y,
               dst->block);
#endif
          dst ++; /* Get ready for next one */
          acelm ++;
        }
      }
    }
  }
  dst->time = GAME_TIME*2;
  dst->x = 0;
  dst->y = 0;
  dst->block = 0;
  acelm++;
  SortShrinkArray (shrink_data, acelm);
  shrink_ptr = shrink_data;
  shrink_ptr1 = shrink_ptr;
} /* CreateShrinkGeneric */

/*
 *
 */
static void
CreateSpiralShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink (spiral_shri_data, spiral_xoff, startlevel, endlevel, 1, starttime, 0, speed, XBTrue, style);
} /* CreateSpiralShrink */

/*
 *
 */
static void
CreateQuadShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink (quad_shri_data, quad_xoff, startlevel, endlevel, 1, starttime, 0, speed, XBTrue, style);
} /* CreateQuadShrink */

/*
 *
 */
static void
CreateWaveShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(quad_shri_data, quad_xoff, startlevel, endlevel, 1, starttime, speed, speed, XBFalse, style);
} /* CreateWaveShrink */

static void
CreateDiagShrink (int startlevel, int endlevel, int inc, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(diag_shri_data, diag_xoff, startlevel, endlevel, inc, starttime, speed, 0, XBTrue, style);
} /* CreateDiagShrink */

/* EPFL */
static void CreateHorizShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(horiz_shri_data, horiz_xoff, startlevel, endlevel, 1, starttime, 0, speed, XBTrue, style);
}

static void CreateCircleShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(circle_data, circle_xoff, startlevel, endlevel, 1, starttime, 0, speed, XBTrue, style);
}

static void CreateMoveShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(move_data, move_xoff, startlevel, endlevel, 1, starttime, 0, speed, XBTrue, style);
}

/* EPFL */

/*
 *
 */
static void
CreateCompoundShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(spiral_shri_data, spiral_xoff, startlevel, endlevel, 1, starttime, speed, 0, XBFalse, style);
} /* CreateCompoundShrink */

/*
 *
 */
static void
CreateFancyCompoundShrink (int startlevel, int endlevel, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink (quad_shri_data, spiral_xoff, startlevel, endlevel, 1, starttime, speed, 2, XBFalse, style);
} /* CreateFancyCompoundShrink */

/*
 *
 */
static void
CreateVerticalShrink (int startlevel, int endlevel, int inc, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(vertical_shri_data, vertical_xoff, startlevel, endlevel, inc, starttime, speed, 0, XBFalse, style);
} /* CreateVerticalShrink */

static void
CreateFancyVerticalShrink (int startlevel, int endlevel, int inc, int starttime, int speed, shri_style *style)
{
  CreateGenericShrink(vertical_shri_data, vertical_xoff, startlevel, endlevel, inc, starttime, speed, 1, XBFalse, style);
} /* CreateFancyVerticalShrink */

/* Now the actual shrink functions */


/*
 * parse shrink data in level data
 */
XBBool
ParseLevelShrink (const DBSection *section, DBSection *warn) {
  /* check existence of section */
  if (NULL == section) {
    Dbg_Out("LEVEL: shrink section is missing!\n");
    DB_CreateEntryString(warn,atomMissing,"true");
    return XBFalse;
  }
  /* type has default */
  switch (DB_ConvertEntryInt (section, atomType, (int *) &shrinkType, shrinkTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomType));
    shrinkType = ST_Void;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning %s\n", DB_SectionEntryString(section, atomType));
    shrinkType = ST_Void;
    DB_CreateEntryString(warn,atomType,DB_IntToString(shrinkTable,shrinkType));
    break;
  default:
    break;
  }
  return XBTrue;
} /* ParseLevelShrink */

/*
 * configure selected shrink
 */
XBBool
ConfigLevelShrink (const DBSection *section)
{
  const char *s;

  assert (section != NULL);
  switch (shrinkType) {
  case ST_Void:           shrink_ptr = shrink_data_none;                                             	     break;
  case ST_Spiral:         CreateSpiralShrink (1, 2,  GAME_TIME/2, 4, &style_rise_2);     	 	     break;
  case ST_SpeedSpiral:    CreateSpiralShrink (1, 2,  GAME_TIME/2, 2, &style_rise_2);     	 	     break;
  case ST_SpiralPlus:     CreateSpiralShrink (0, 2,  GAME_TIME/2, 4, &style_rise_2_plus);	 	     break;
  case ST_Spiral3:        CreateSpiralShrink (1, 3,  GAME_TIME/2, 4, &style_rise_2);     	 	     break;
  case ST_Spiral23:       CreateSpiralShrink (2, 3,  GAME_TIME/2, 2, &style_rise_2_plus);	 	     break;
  case ST_SpiralLego:     CreateSpiralShrink (3, 3,3*GAME_TIME/4, 4, &style_rise_2_plus);	 	     break;
  case ST_EarlySpiral:    CreateSpiralShrink (1, 2,3*GAME_TIME/8, 4, &style_rise_2);     	 	     break;
  case ST_Compound:       CreateCompoundShrink (1, 5, GAME_TIME/6, GAME_TIME/6, &style_compound);            break;
  case ST_CompoundF:      CreateFancyCompoundShrink (1, 5, GAME_TIME/6, GAME_TIME/6, &style_compound);       break;
  case ST_Compound2F:     CreateFancyCompoundShrink (1, 2, GAME_TIME/2, GAME_TIME/6, &style_compound);       break;
  case ST_LazyCompoundF:  CreateFancyCompoundShrink (1, 3, GAME_TIME/3, GAME_TIME/3, &style_compound_solid); break;
  case ST_CompoundSolid:  CreateCompoundShrink (1, 5, GAME_TIME/6, GAME_TIME/6, &style_compound_solid);      break;
  case ST_SavageCompound: CreateCompoundShrink (2, 5, GAME_TIME/6, GAME_TIME/3, &style_savage_compound);     break;
  case ST_CompoundExtra:  CreateCompoundShrink (2, 5, GAME_TIME/6, GAME_TIME/6, &style_compound_extra);      break;
  case ST_Down:           CreateVerticalShrink (1, 11, 1, GAME_TIME/11, GAME_TIME/11, &style_compound);      break;
  case ST_DownF:          CreateFancyVerticalShrink (1, 11, 1, GAME_TIME/11, GAME_TIME/11, &style_compound); break;
  case ST_Quad:           CreateQuadShrink (1, 3, GAME_TIME/2, 16, &style_rise_2);                           break;
  case ST_ConstrictWave:  CreateWaveShrink (1, 3, GAME_TIME/2,  5, &style_rise_2_plus);                      break;
  case ST_OutwardSpiral:  CreateGenericShrink (spiral_shri_data, spiral_xoff, 2, 5, 1, 3*GAME_TIME/8,        
					       -100, -3, XBFalse, &style_rise_2);                            break;
  /* EPFL */
  case ST_Horiz:          CreateHorizShrink(1,5,GAME_TIME/8,20,&style_rise_2);                               break;
  case ST_Circle:         CreateCircleShrink(0,4,GAME_TIME/8,20,&style_rise_2);                              break;
  case ST_Move:           CreateMoveShrink(0,119,1,34,&style_move);                                          break;
  case ST_IstySpiral3:    CreateSpiralShrink(1,3,GAME_TIME/12,5,&style_rise_2);                              break;
  case ST_IstyCompound2F: CreateFancyCompoundShrink(2,5,GAME_TIME/2,GAME_TIME/4,&style_compound);            break;
  case ST_Diag:           CreateDiagShrink(1,23,1,GAME_TIME/23,GAME_TIME/23,&style_rise_2);                  break;
  case ST_OutwardExtra:   CreateCompoundShrink (1, 3, GAME_TIME/2, -GAME_TIME/6, &style_outward_compound_extra); break;
  case ST_Spiral5:        CreateSpiralShrink(1,5,GAME_TIME/4,5,&style_rise_2);                               break;
  /* EPFL */  
  default:                break;
  }
  /* make info entry */
  if (NULL != (s = shrinkName[shrinkType]) ) {
    AddLevelInfo (s);
  }
  shrink_ptr1 = shrink_ptr;
  return XBTrue;
} /* ConfigLevelShrink */

/*
 * cleaning up
 */
void
FinishLevelShrink (void)
{
  if (NULL != shrink_data) {
    free (shrink_data);
    shrink_data = NULL;
  }
} /* FinishShrink */
/*
 * return the name of the shrink type
 */
const char*
GetShrinkName(XBShrinkType type) {
  int i;
  if (type == ST_Void)
    return stVoidName;
  for (i = 0; i < NUM_ST; ++i)
    if (shrinkTable[i].value == type)
      return shrinkTable[i].key;
  return stVoidName;
}

/*
 * end of file shrink.c
 */
