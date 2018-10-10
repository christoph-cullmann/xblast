/*
 * file bomb.c - bombs and explosions
 *
 * $Id: bomb.c,v 1.23 2005/01/12 15:48:22 lodott Exp $
 *
 * Program XBLAST 
 * (C) 1993-1999 by Oliver Vogel (e-mail: m.vogel@ndh.net)
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
#include "bomb.h"

#include "atom.h"
#include "func.h"
#include "geom.h"
#include "info.h"
#include "map.h"
#include "snd.h"
#include "random.h"

/*
 * macros
 */
#define BOMB_ERROR_PROB 32 // 23 apr 2002
#define BOMB_DELAY 5

/* haunt factors */
#define HAUNT_NONE 0
#define HAUNT_FAST 10
#define HAUNT_SLOW 35

/* nasty walls */
#define NASTY_INC GAME_TIME/12

/*
 * local types
 */

 char *bomb_name_choice[] = {
/*"Normal"        ->  */    "Normal",
/*"Napalm"        ->  */    "Napalm",
/*"Instant"       ->  */    "blastnow",
/*"Close"         ->  */     "close",
/*"Firecracker"   ->  */     "firecracker",
/*"Firecracker 2" ->  */     "firecracker2",
/*"Construction"  ->  */    "Construction",
/*"Three"         ->  */     "threebombs",
/*"Grenade"       ->  */    "Grenade",
/*"Triangle"      ->  */     "triangle",
/*"Destruction"   ->  */    "Destruction",
/*"Fungus"        ->  */    "Fungus",
/*"Renovation"    ->  */    "Renovation",
/*"Pyro"          ->  */    "Pyro",
/*"Instant Pyro"  ->  */     "pyro2",
/*"Random"        ->  */     "random",
/*   short     ->     */    "short",
/*"Diagonal"      ->  */    "Diagonal",
/*"Scissor"       ->  */    "Scissor",
/*"Anti Scissor"  ->  */    "Anti Scissor",
/*"Parallel"      ->  */    "Parallel", 
/*"Distance"      ->  */     "Distance", 
/*"Lucky"         ->  */     "Lucky", 
/*"Parasol"       ->  */    "Parasol", 
/*"Comb"          ->  */     "Comb", 
/*"Far Pyro"      ->  */     NULL,
/*"Nuclear"       ->  */     NULL, 
/*"Protection"    ->  */    "Protection",
/*                    */    "ringoffire",
/*"Mine"          ->  */    "Auto Mine",
/*"Short"         ->  */     NULL,
/*"Row"           ->  */     NULL,
/*"Column"        ->  */     NULL,
/*"Searching"     ->  */    "Searching",
/*"Ring Of Fire"  ->  */     NULL,
/*"Psycho"        ->  */     NULL,
/* */                        NULL,
/**/ };
int ChoiceDefaultBomb;

/* pointer to function with type specifix behaviour */
typedef XBBool (*XBSpecialBombFunc) (Explosion *);

/*
 * local prototypes
 */

/* "constructor" for a bomb */
static XBBool NewExplosion (BMPlayer *player, int x, int y, int range, XBBool remoteControlled, XBBool malfunction, 
			    BMBombType type, int typeExtr, BMDirection initialdir);
/* special bomb functions */
static XBBool SpecialBombNormal (Explosion *ptr);
static XBBool SpecialBombNapalm (Explosion *ptr);
static XBBool SpecialBombFirecracker (Explosion *ptr);
static XBBool SpecialBombConstruction (Explosion *ptr);
static XBBool SpecialBombThreebombs (Explosion *ptr);
static XBBool SpecialBombGrenade (Explosion *ptr);
static XBBool SpecialBombTrianglebombs (Explosion *ptr);
static XBBool SpecialBombDestruction (Explosion *ptr);
static XBBool SpecialBombFungus (Explosion *ptr);
static XBBool SpecialBombRenovation (Explosion *ptr);
static XBBool SpecialBombPyro (Explosion *ptr);

/* EPFL SHIT */
static XBBool SpecialBombDiagThree (Explosion *ptr);
static XBBool SpecialBombScissor (Explosion *ptr);       /* added by stn */
static XBBool SpecialBombScissor2 (Explosion *ptr);      /* added by stn */
static XBBool SpecialBombParallel (Explosion *ptr);      /* added by stn */
static XBBool SpecialBombDistance (Explosion *ptr);      /* added by stn */
static XBBool SpecialBombLucky (Explosion *ptr);         /* added by stn */
static XBBool SpecialBombParasol (Explosion *ptr);       /* added by stn */
static XBBool SpecialBombComb (Explosion *ptr);          /* added by stn */
static XBBool SpecialBombFarpyro (Explosion *ptr);       /* added by stn */
static XBBool SpecialBombNuclear (Explosion *ptr);       /* added by stn */
static XBBool SpecialBombProtectbombs (Explosion *ptr); /* Written by Amilhastre */
static XBBool SpecialBombRingofire (Explosion *ptr);   /* Added by x-bresse on 06.04.2000 */
static XBBool SpecialBombMine (Explosion *ptr);        /* Added by x-bresse on 06.04.2000 */
static XBBool SpecialBombRow (Explosion *ptr);    /* Added by x-bresse on 06.04.2000 */
static XBBool SpecialBombColumn (Explosion *ptr);  /* Added by x-bresse on 06.04.2000 */
static XBBool SpecialBombPsycho (Explosion *ptr);  /* Added by x-bresse on 06.04.2000 */
static XBBool SpecialBombChangeDirectionAtHalf (Explosion *ptr);  /* Added by Skywalker */

/* bomb click functions */
void BombClickNone (Explosion *bomb);
void BombClickInitial (Explosion *bomb);
void BombClickThru (Explosion *bomb);
void BombClickSnooker (Explosion *bomb);
void BombClickContact (Explosion *bomb);
void BombClickClockwise (Explosion *bomb);
void BombClickAnticlockwise (Explosion *bomb);
void BombClickRandomdir (Explosion *bomb);
void BombClickRebound (Explosion *bomb);
void BombClickSplit (Explosion *bomb);
static void SpreXDir (int lx, int ly, int range, int type, 
		      int type_extr, BMDirection dir);

/*
 * local variables
 */

/* array to locate bomb/explosion on map */
static Explosion *bombMaze[MAZE_W][MAZE_H];
/* list with all bombs and explosions */
static int numExpl = 0 ;
Explosion *explList;
static Explosion *explEnd;
int initialBombDir;
static int hauntFactor;
int curBombTime;
int defaultBMT;
static int  specialBMT, evilBMT;
static int slowMotionBurst=0;
static int nextNasty;
static int ceilNasty;
static int divNextNasty;
static int gentleNasty;
static int rangeNasty;
/* bomb click functions */
XBBombClickFunc doBombClick;
XBBombClickFunc doWallClick;
XBBombClickFunc doPlayerClick;
/* lookup table for special bomb function */
static XBSpecialBombFunc doSpecialBombFunction[NUM_BMT] = {
  SpecialBombNormal,
  SpecialBombNapalm,
  SpecialBombNormal,
  SpecialBombNormal,
  SpecialBombFirecracker,
  SpecialBombFirecracker,
  SpecialBombConstruction,
  SpecialBombThreebombs,
  SpecialBombGrenade,
  SpecialBombTrianglebombs,
  SpecialBombDestruction,
  SpecialBombFungus,
  SpecialBombRenovation,
  SpecialBombPyro,
  SpecialBombPyro,
  SpecialBombNormal,
  SpecialBombNormal,
  /* EPFL SHIT */  /* FUNCTION NAMES */
  SpecialBombDiagThree,
  SpecialBombScissor,
  SpecialBombScissor2,
  SpecialBombParallel,
  SpecialBombDistance,
  SpecialBombLucky,
  SpecialBombParasol,
  SpecialBombComb,
  SpecialBombFarpyro,
  SpecialBombNuclear,
  SpecialBombProtectbombs,
  SpecialBombRingofire,
  SpecialBombMine,
  SpecialBombRow,
  SpecialBombColumn,
  SpecialBombPsycho,
  SpecialBombNormal, /* search bomb */
  SpecialBombChangeDirectionAtHalf,  /** Skywalker **/
};
/* conversion table for bomb clicks */
static DBToData bombClickTable[] = {
  { "anticlockwise", (void *) BombClickAnticlockwise },
  { "clockwise",     (void *) BombClickClockwise },
  { "contact",       (void *) BombClickContact },
  { "initial",       (void *) BombClickInitial },
  { "none",          (void *) BombClickNone },
  { "null",          (void *) BombClickNone },
  { "randomdir",     (void *) BombClickRandomdir },
  { "rebound",       (void *) BombClickRebound },
  { "snooker",       (void *) BombClickSnooker },
  { "split",         (void *) BombClickSplit },
  { "thru",          (void *) BombClickThru },
  { NULL, NULL },
};
/* conversion table for direction */
static DBToInt bombDirTable[] = {
  { "down",  GoDown },
  { "left",  GoLeft },
  { "right", GoRight },
  { "up",    GoUp },
  { "stop",  GoStop },
  { NULL, -1 }
};
/* conversion table for fuse times */
static DBToInt fuseTimeTable[] = {
  { "long",  LONG_FUSE },
  { "short", SHORT_FUSE },
  { "normal", BOMB_TIME },
  { NULL,    -1 }
};

/* conversion table for bomb types */

static DBToInt bombTypeTable[] = { /* NAME IN LEVEL FILE ALFABETICAL ORDER !!! */
  { "blastnow",       BMTblastnow }, 
  { "changedirectionathalf", BMTchangedirectionathalf /* Skywalker */} , 
  { "close",          BMTclose }, 
  { "column",         BMTcolumn },
  { "comb",           BMTcomb },
  { "construction",   BMTconstruction }, 
  { "default",        BMTdefault },
  { "destruction",    BMTdestruction }, 
  { "diagional",      BMTdiagthreebombs }, /* EPFL */
  { "distance",       BMTdistance },
  { "farpyro2",       BMTfarpyro },
  { "firecracker",    BMTfirecracker }, 
  { "firecracker2",   BMTfirecracker2 }, 
  { "fungus",         BMTfungus }, 
  { "grenade",        BMTgrenade },
  { "lucky",          BMTlucky },
  { "mine",           BMTmine },
  { "napalm",         BMTnapalm },
  { "normal",         BMTnormal },
  { "nuclear",        BMTnuclear },
  { "parallel",       BMTparallel },
  { "parasol",        BMTparasol },
  { "protectbombs",   BMTprotectbombs },
  { "psycho",         BMTpsycho },
  { "pyro",           BMTpyro }, 
  { "pyro2",          BMTpyro2 }, 
  { "random",         BMTrandom }, 
  { "renovation",     BMTrenovation }, 
  { "ringofire",      BMTringofire },
  { "row",            BMTrow },
  { "scissor",        BMTscissor },
  { "scissor2",       BMTscissor2 },
  { "search",         BMTsearch },
  { "short",          BMTshort },
  { "snipe",          BMTsnipe },
  { "threebombs",     BMTthreebombs }, 
  { "trianglebombs",  BMTtrianglebombs }, 
  { NULL,             NUM_BMT },
};
/* bomb haunting */
static DBToInt hauntFactorTable[] = {
  { "fast", HAUNT_FAST },
  { "slow", HAUNT_SLOW },
  { "none", HAUNT_NONE },
  { NULL,   -1 },
};
/* bomb extra info */
static char *bombName[] = {
  NULL,
  "Napalm bomb",
  "Instant bomb",
  NULL,
  "Firecracker",
  NULL,
  "Construction bomb",
  "Three bomb",
  "Grenade",
  "Triangle bomb",
  "Destruction bomb",
  "Fungus bomb",
  "Renovation bomb",
  "Pyro bomb",
  NULL,
  "Random bomb",
  "Fast bomb",
  "Diagonal bomb", /* EPFL */
  "Scissor bomb >",
  "Scissor bomb <",
  "Parallel bomb",
  "Distance bomb",
  "Lucky bomb",
  "Parasol bomb",
  "Combo bomb",
  "Farpyro bomb",
  "Nuclear bomb",
  "Protection bomb",
  "Ring of Fire",
  "Mine",
  "Row",
  "Column",
  "Psycho",
};
/* bomb direction info */
static const char *bombDirInfo[MAX_DIR] = {
  NULL,
  "Bombs are going up",
  "Bombs are going left",
  "Bombs are falling down",
  "Bombs are going right",
  NULL,
};
static char* bmNormalName = "normal";
 
const char* GetBombName(BMBombType type){

  int i;
  
  if (type ==BMTnormal )
    return bmNormalName;
  for (i = 0; i <NUM_BMT ; ++i)
    if (bombTypeTable[i].value == type)
      return bombTypeTable[i].key;
  return bmNormalName;


}
void SetSlowMotionBurst(int flame){
  slowMotionBurst=flame;
}

/*
 * load bombs from level data
 */
XBBool
ParseLevelBombs (const DBSection *section, DBSection *warn)
{
  void *ptr;

  assert (NULL == explList);
  assert (NULL == explEnd);
  assert (0    == numExpl);
  /* check if section exists */
  if (NULL == section) {
    Dbg_Out("LEVEL: bomb section is missing!\n");
    DB_CreateEntryString(warn,atomMissing,"true");
    return XBFalse;
  }
  /* clear list */
  ptr = NULL;
  /* clear lookup table */
  memset (bombMaze, 0, sizeof (bombMaze) );
  /* BombClick has default */
  switch (DB_ConvertEntryData (section, atomBombClick, &ptr, bombClickTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomBombClick));
    doBombClick = BombClickNone;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomBombClick));
    doBombClick = BombClickNone;
    DB_CreateEntryString(warn,atomBombClick, DB_DataToString(bombClickTable, doBombClick));
    break;
  default:
    doBombClick = (XBBombClickFunc) ptr;
    break;
  }
  /* WallClick has default */
  switch (DB_ConvertEntryData (section, atomWallClick, &ptr, bombClickTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomWallClick));
    doWallClick = BombClickNone;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomWallClick));
    doWallClick = BombClickNone;
    DB_CreateEntryString(warn,atomWallClick, DB_DataToString(bombClickTable, doWallClick));
    break;
  default:
    doWallClick = (XBBombClickFunc) ptr;
    break;
  }
  /* PlayerClick has default */
  switch (DB_ConvertEntryData (section, atomPlayerClick, &ptr, bombClickTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomPlayerClick));
    doPlayerClick = BombClickNone;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomPlayerClick));
    doPlayerClick = BombClickNone;
    DB_CreateEntryString(warn,atomPlayerClick, DB_DataToString(bombClickTable, doPlayerClick));
    break;
  default:
    doPlayerClick = (XBBombClickFunc) ptr;
    break;
  }
  /* Direction has default */
  switch (DB_ConvertEntryInt (section, atomDirection, &initialBombDir, bombDirTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomDirection));
    initialBombDir = GoStop;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomDirection));
    initialBombDir = GoStop;
    DB_CreateEntryString(warn,atomDirection, DB_IntToString(bombDirTable, initialBombDir));
    break;
  default:
    break;
  }
  /* FuseTime has default */
  switch (DB_ConvertEntryInt (section, atomFuseTime, &curBombTime, fuseTimeTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomFuseTime));
    curBombTime = BOMB_TIME;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomFuseTime));
    curBombTime = BOMB_TIME;
    DB_CreateEntryString(warn,atomFuseTime, DB_IntToString(fuseTimeTable, curBombTime));
    break;
  default:
    break;
  }
  /* Haunt has default */
  switch (DB_ConvertEntryInt (section, atomHaunt, &hauntFactor, hauntFactorTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomHaunt));
    hauntFactor = HAUNT_NONE;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomHaunt));
    hauntFactor = HAUNT_NONE;
    DB_CreateEntryString(warn,atomHaunt, DB_IntToString(hauntFactorTable, hauntFactor));
    break;
  default:
    break;
  }
  /* Haunt has default */
  switch (DB_ConvertEntryInt (section, atomDefault, &defaultBMT, bombTypeTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomDefault));
    defaultBMT = BMTnormal;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomDefault));
    defaultBMT = BMTnormal;
    DB_CreateEntryString(warn,atomDefault, DB_IntToString(bombTypeTable, defaultBMT));
    break;
  default:
    break;
  }
  /* Special has default */
  switch (DB_ConvertEntryInt (section, atomSpecial, &specialBMT, bombTypeTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomSpecial));
    specialBMT = BMTnormal;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomSpecial));
    specialBMT = BMTnormal;
    DB_CreateEntryString(warn,atomSpecial, DB_IntToString(bombTypeTable, specialBMT));
    break;
  default:
    break;
  }
  /* Hidden has default */
  switch (DB_ConvertEntryInt (section, atomHidden, &evilBMT, bombTypeTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomHidden));
    evilBMT = BMTnormal;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomHidden));
    evilBMT = BMTnormal;
    DB_CreateEntryString(warn,atomHidden, DB_IntToString(bombTypeTable, evilBMT));
    break;
  default:
    break;
  }
  /* NastyCeil has default */
  if (! DB_GetEntryInt (section, atomNastyCeil, &ceilNasty)) {
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomNastyCeil));
    ceilNasty = 0;
  }
  /* NastyGentle/NastyRange have default */
  if (DB_GetEntryInt (section, atomNastyGentle, &gentleNasty) &&
      DB_GetEntryInt (section, atomNastyRange, &rangeNasty) ) {
    /* check for NextNasty */
    if (!DB_GetEntryInt (section, atomNextNasty, &divNextNasty) || (divNextNasty==0) ) {
      divNextNasty = 1;
    }
    nextNasty    = NASTY_INC/divNextNasty;;
    gentleNasty *= GAME_TIME;
  }  else {
    Dbg_Level("default nasty data\n");
    nextNasty   = GAME_TIME + NASTY_INC + 1;
    gentleNasty = 0;
    rangeNasty  = 0;
  }
  return XBTrue;
}

/* 
 * set level info text 
 */
void
ConfigLevelBombs (const DBSection *section)
{
  const char *s;

  /* bomb types */
  if (NULL != bombName[defaultBMT]) {
    AddLevelInfo ("%s as default", bombName[defaultBMT]);
  }
  if (0 != NumSpecialBombs () && 
      NULL != bombName[specialBMT]) {
    AddLevelInfo ("%d %ss", NumSpecialBombs (), bombName[specialBMT]);
  }
  if (HasSpecialBombs () &&
      NULL != bombName[specialBMT]) {
    AddLevelInfo ("%s as an extra", bombName[specialBMT]);
  }
  if (NULL != bombName[evilBMT]) {
    AddLevelInfo ("Hidden %ss", bombName[evilBMT]);
  }
  /* fuse time */
  switch (curBombTime) {
  case LONG_FUSE:  AddLevelInfo ("All bombs are long fused");  break;
  case SHORT_FUSE: AddLevelInfo ("All bombs are short fused"); break;
  default:         break;
  }
  /* direction */
  if (NULL != (s = bombDirInfo[initialBombDir]) ) {
    AddLevelInfo (s);
  }
  /* haunting */
  switch (hauntFactor) {
  case HAUNT_SLOW: AddLevelInfo ("All bombs are haunted"); break;
  case HAUNT_FAST: AddLevelInfo ("All bombs are haunted (and dangerous)"); break;
  default:         break;
  }
  /* bomb click */
  if (doBombClick == BombClickRebound) {
    AddLevelInfo ("Bombs rebound from others");
  } else if (doBombClick == BombClickContact) {
    AddLevelInfo ("Bombs explode on contact with others");
  } else if (doBombClick == BombClickClockwise) {
    AddLevelInfo ("Bombs turn clockwise on hitting others");
  } else if (doBombClick == BombClickAnticlockwise) {
    AddLevelInfo ("Bombs turn anticlockwise on hitting others");
  } else if (doBombClick == BombClickRandomdir) {
    AddLevelInfo ("Bombs bounce off randomly from others");
  } else if (doBombClick == BombClickSnooker) {
    AddLevelInfo ("Bombs are snooker bombs");
  } else if (doBombClick == BombClickSplit) {
    fprintf(stderr,"Bombs split whith contact off Bombs");
    AddLevelInfo ("Bombs split whith contact off Bombs");
  }
  /* wall click */
  if (doWallClick == BombClickRebound) {
    AddLevelInfo ("Bombs rebound off walls");
  } else if (doWallClick == BombClickContact) {
    AddLevelInfo ("Bombs explode on contact with walls");
  } else if (doWallClick == BombClickClockwise) {
    AddLevelInfo ("Bombs turn clockwise on hitting walls");
  } else if (doWallClick == BombClickAnticlockwise) {
    AddLevelInfo ("Bombs turn anticlockwise on hitting walls");
  } else if (doWallClick == BombClickRandomdir) {
    AddLevelInfo ("Bombs bounce off randomly of walls");
  } else if (doWallClick == BombClickSplit) {
    fprintf(stderr,"Bombs split whith contact off walls");
    AddLevelInfo ("Bombs split whith contact off Walls");
  }
  /* player click */
  if (doPlayerClick == BombClickThru) {
    AddLevelInfo ("Bombs stun players running through");
  } else if (doPlayerClick == BombClickContact) {
    AddLevelInfo ("Bombs explode on contact with players");
  } else if (doPlayerClick == BombClickRebound) {
    AddLevelInfo ("Bombs rebound off players");
  }  else if (doPlayerClick == BombClickSplit) {
    fprintf(stderr,"Bombs split whith contact off player");
    AddLevelInfo ("Bombs split whith contact off player");
  } 
  /* nasty walls */
  if (gentleNasty != 0) {
    AddLevelInfo ("The Walls launch bombs");
  }
  ChoiceDefaultBomb = defaultBMT;
  /* that's all */
} /* ConfigBombs */

/*
 *
 */
static void 
OneExplAt (int x, int y, int ra, int ri, BMBurnOut *burnOut)
{
  int i;

  assert (NULL != burnOut);
  /* right */
  if (0 == (*burnOut & BO_RIGHT) ) { 
    for (i = 0; (i <= ra) && CheckMazeOpen(x+i, y) ; i ++) {
      if (i >= ri) {
	if (i != ri && i != ra) {
	  SetExplBlock (x+i, y, 0x1a);
	} else if (i != ri) {
	  SetExplBlock (x+i, y, 0x18);
	} else if (i == ra) {
	  SetExplBlock (x+i, y, 0x10);
	} else {
	  SetExplBlock (x+i, y, 0x12);
	}
      }
    }
    if (i < ri) {
      *burnOut |= BO_RIGHT;
    }
    if ( (i <= ra) && (CheckMazeExtra (x+i, y)) ) {
      SetMazeBlock (x+i, y, BTExtraOpen);
    }
  }
  /* left */
  if (0 == (*burnOut & BO_LEFT) ) { 
    for (i = 0; (i <= ra) && CheckMazeOpen (x-i, y); i ++) {
      if (i >= ri) {
	if (i != ri && i != ra) {
	  SetExplBlock (x-i, y, 0x1a);
	} else if (i != ri) {
	  SetExplBlock (x-i, y, 0x12);
	} else if (i == ra) {
	  SetExplBlock (x-i, y, 0x10);
	} else {
	  SetExplBlock (x-i, y, 0x18);
	}
      }
    }
    if (i < ri) {
      *burnOut |= BO_LEFT;
    }
    if ( (i <= ra ) && (CheckMazeExtra(x-i, y)) ) {
      SetMazeBlock (x-i, y, BTExtraOpen);
    }
  }
  /* up */
  if (0 == (*burnOut & BO_UP) ) { 
    for (i = 0; (i <= ra) && CheckMazeOpen (x, y-i); i ++) {
      if (i >= ri) {
	if (i != ri && i != ra) {
	  SetExplBlock (x, y-i, 0x15);
	} else if (i != ri) {
	  SetExplBlock (x, y-i, 0x14);
	} else if (i == ra) {
	  SetExplBlock (x, y-i, 0x10);
	} else {
	  SetExplBlock (x, y-i, 0x11);
	}
      }
    }
    if (i < ri) {
      *burnOut |= BO_UP;
    }
    if ( (i <= ra) && CheckMazeExtra(x, y-i) ) {
      SetMazeBlock (x, y-i, BTExtraOpen);
    }
  }
  /* down */
  if (0 == (*burnOut & BO_DOWN) ) { 
    for (i = 0; (i <= ra) && CheckMazeOpen (x, y+i); i ++) {
      if (i >= ri) {
	if (i != ri && i != ra) {
	  SetExplBlock (x, y+i, 0x15);
	} else if (i != ri) {
	  SetExplBlock (x, y+i, 0x11);
	} else if (i == ra) {
	  SetExplBlock (x, y+i, 0x10);
	} else {
	  SetExplBlock (x, y+i, 0x14);
	}
      }
    }
    if (i < ri) {
      *burnOut |= BO_DOWN;
    }
    if ( (i <= ra) && CheckMazeExtra(x, y+i) ) {
      SetMazeBlock (x, y+i, BTExtraOpen);
    }
  }
} /* OneExplAt */

/*
 *
 */
static void 
DelExplosion (Explosion *ptr)
{
  Explosion *hilf;
  int i,x,y,r;

  assert (ptr != NULL);

  /* give bomb back to  player */
  if (ptr->player != NULL) {
    ptr->player->bombs++;
    if (ptr->isMorphed) {
      /* remorph player, if he was this bomb */
      ptr->player->morphed = 0;
      ptr->player->x       =  ptr->x      * BLOCK_WIDTH;
      ptr->player->y       = (ptr->y - 1) * BLOCK_HEIGHT;
      ptr->player->num_extras --;
    }
    /** Skywalker **/
    if (ptr->isSniping) {
      
      ptr->player->sniping = 0;
      ptr->player->num_extras --;
      //     ptr->player->x = ptr->x * BLOCK_WIDTH;
      //  ptr->player->y = (ptr->y-1) * BLOCK_HEIGHT;
    }
    /** **/
  }
  /* just for convenience */
  x = ptr->x;
  y = ptr->y;
  r = ptr->range;
  /* currect lookup map */
  bombMaze[x][y] = NULL;
  /* one less explosions to worry about */
  numExpl --;
  /* look right for blocks to blast */
  for (i = 0; i <= r; i ++) {
    if (! CheckMazeFree2 (x + i, y)) {
      BlastExtraBlock (x + i, y);
      break;
    }
  }
  /* look left for blocks to blast */
  for (i = 0; i <= r; i++) {
    if (! CheckMazeFree2 (x-i, y)) {
      BlastExtraBlock (x-i, y);
      break;
    }
  }
  /* look down for blocks to blast */
  for (i = 0; i <= r; i ++) {
    if (! CheckMazeFree2 (x, y + i)) {
      BlastExtraBlock (x, y + i);
      break;
    }
  }
  /* look up for blocks to blast */
  for (i = 0; i <= r; i ++) {
    if (! CheckMazeFree2 (x, y - i)) {
      BlastExtraBlock (x, y - i);
      break;
    }
  }
  /* delete bomb sprite */
  if (ptr->sprite != NULL) {
    DeleteSprite (ptr->sprite);
  }
  /* remove form list */
  if (ptr == explList) {
    explList = ptr->next;
    if (explList == NULL) {
      explEnd = NULL;
    }
  } else {
    for (hilf = explList; hilf->next != NULL; hilf = hilf->next) {
      if (hilf->next == ptr) {
	if (explEnd == ptr) {
	  explEnd = hilf;
	}
	hilf->next = hilf->next->next;
	break;
      }
    }
  }

  /* free memory */
    free (ptr);
} /* DelExplosion */

/*
 * delete all explosions in the game
 */
void
DeleteAllExplosions (void)
{
  Dbg_Out ("delete all explosions\n");
  /* just delete the first element as long as one exists */
  while (NULL != explList) {
    DelExplosion (explList);
  }
  assert (NULL == explList);
  assert (NULL == explEnd);
} /* DeleteAllExplosions */

/*------------------------------------------------------------------------*
 * 
 * Player, wall, and bomb click functions (Garth Denley) 
 *
 *------------------------------------------------------------------------*/

/*
 * no effect (bomb stops)
 */
 void
BombClickNone (Explosion *bomb)
{
  bomb->dir = GoStop;
  bomb->dx  = 0;
  bomb->dy  = 0;
} /* BombClickNone */

/*
 * bomb goes on with inital direction
 */
 void
BombClickInitial (Explosion *bomb)
{
  switch(initialBombDir) {
  case GoStop:
    bomb->dx = 0;
    bomb->dy = 0;
    break;
  case GoRight:
    if (CheckMazeFree (bomb->x+1, bomb->y)) {
      bomb->dx = BOMB_VX;
    }
    bomb->dy = 0;
    break;
  case GoLeft:
    if (CheckMazeFree (bomb->x-1, bomb->y)) {
      bomb->dx = -BOMB_VX;
    }
    bomb->dy = 0;
    break;
  case GoDown:
    bomb->dx = 0;
    if (CheckMazeFree (bomb->x, bomb->y+1)) {
      bomb->dy = BOMB_VY;
    }
    break;
  case GoUp:
    bomb->dx = 0;
    if (CheckMazeFree (bomb->x, bomb->y-1)) {
      bomb->dy = -BOMB_VY;
    }
    break;
  }
  bomb->dir = initialBombDir;
} /* BombClickInitial */

/* 
 * bomb goes thru 
 */
 void
BombClickThru (Explosion *bomb)
{
} /* BombClickThru */

/* 
 * snooker bombs  
 */
 void
BombClickSnooker (Explosion *bomb)
{
  int dir;

  dir = bomb-> dir;

  bomb->dir = GoStop;
  bomb->dx = 0;
  bomb->dy = 0;

  switch (dir) {
  case GoUp:    MoveBomb (bomb->x,   bomb->y-1, dir); break;
  case GoLeft:  MoveBomb (bomb->x-1, bomb->y,   dir); break;
  case GoDown:  MoveBomb (bomb->x,   bomb->y+1, dir); break;
  case GoRight: MoveBomb (bomb->x+1, bomb->y,   dir); break;
  }
} /* BombClickSnooker */

/* 
 * contact bombs 
*/
 void 
BombClickContact (Explosion *bomb)
{
  bomb->dir   = GoStop;
  bomb->dx    = 0;
  bomb->dy    = 0;
  bomb->count = 0;
} /* BombClickContact */

/* 
 * clockwise bombs 
 */
 void 
BombClickClockwise (Explosion *bomb)
{
  static BMDirection turnClockwise[MAX_DIR] = {
    GoStop, GoRight, GoUp, GoLeft, GoDown, GoDefault
  };
  
  bomb->dx  = 0;
  bomb->dy  = 0;
  bomb->dir = turnClockwise[bomb->dir];
} /* BombClickClockwise */

/* 
 * anticlockwise bombs 
 */
 void 
BombClickAnticlockwise (Explosion *bomb)
{
  static BMDirection turnAnticlockwise[MAX_DIR] = {
    GoStop, GoLeft, GoDown, GoRight, GoUp, GoDefault
  };

  bomb->dx  = 0;
  bomb->dy  = 0;
  bomb->dir = turnAnticlockwise[bomb->dir];
} /* BombClickAnticlockwise */

/* 
 * randomdir bombs 
 */
 void 
BombClickRandomdir (Explosion *bomb)
{
  bomb->dx  = 0;
  bomb->dy  = 0;
  bomb->dir = (BMDirection) (GameRandomNumber(4) + 1);
} /* BombClickRandomdir */

/*Sky*/
/* Added by Fouf on 09/02/99 22:46:25 */ 
/* Added by "Belgium Guys" */ 
/* 
 * BombClickRebound 
 */
void
BombClickSplit( Explosion *bomb)
{ 
  static BMDirection turnOpposite[MAX_DIR] = {
    GoStop, GoDown, GoRight, GoUp, GoLeft, GoDefault
  };
  static BMDirection turnClockwise[MAX_DIR] = {
    GoStop, GoRight, GoUp, GoLeft, GoDown, GoDefault
  };
  static BMDirection turnAnticlockwise[MAX_DIR] = {
    GoStop, GoLeft, GoDown, GoRight, GoUp, GoDefault
  };
  bomb->dir = turnOpposite[bomb->dir];
  SpreXDir(bomb->x+1,bomb->y+1,bomb->range,defaultBMT,0,turnClockwise[bomb->dir]);
  SpreXDir(bomb->x-1,bomb->y-1,bomb->range,defaultBMT,0,turnAnticlockwise[bomb->dir]);

}
/* BombClickRebound */
/*Sky*/
/* Added by Fouf on 09/02/99 22:46:25 */ /* Added by "Belgium Guys" */ 
/* 
 * rebound bombs 
 */
 void 
BombClickRebound (Explosion *bomb)
{
  static BMDirection turnOpposite[MAX_DIR] = {
    GoStop, GoDown, GoRight, GoUp, GoLeft, GoDefault
  };

  bomb->dir = turnOpposite[bomb->dir];
} /* BombClickRebound */

/* 
 * 
 */
static void 
HauntBomb (Explosion *ptr)
{
  assert (ptr != NULL);

  if (ptr->dir == GoStop) {
    if (! CheckPlayerNear (ptr->x, ptr->y) ) {
      switch (GameRandomNumber(4)) {
      case 0:
	ptr->dir = GoUp;
	ptr->dx  = 0;
	break;
      case 1:
	ptr->dir = GoDown;
	ptr->dx  = 0;
	break;
      case 2:
	ptr->dir = GoLeft;
	ptr->dy  = 0;
	break;
      case 3:
	ptr->dir = GoRight;
	ptr->dy  = 0;
	break;
      }
      SND_Play (SND_HAUNT, SOUND_MIDDLE_POSITION);
    }
  }
} /* HauntKick */

/*
 * move bomb one step upwards 
 */
static void
MoveBombUp (Explosion *ptr)
{
  int tt;
  assert (ptr != NULL);

  tt=(ptr->y -1 +MAZE_H) % MAZE_H; // 02-05-2002
  if  ( (ptr->dy == 0) && 
	! CheckMazeFree (ptr->x, tt) )  {
    (*doWallClick)(ptr);
  } else if ( (ptr->dy <= 0) && CheckBomb(ptr->x,tt) ) { // 02-05-2002
    (*doBombClick)(ptr);
  } else {
    ptr->dy -= BOMB_VY;
    if (ptr->dy <= -BLOCK_HEIGHT/2) {
      SND_Play (SND_SLIDE, (ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
      bombMaze[ptr->x][ptr->y] = NULL;
      ptr->dy += BLOCK_HEIGHT;
      ptr->y  -= 1;
      ptr->y  = tt;
      bombMaze[ptr->x][ptr->y] = ptr;
    }
  }
} /* MoveBombUp */

/*
 * move bomb one step downwards
 */
static void
MoveBombDown (Explosion *ptr)
{
  int tt;
  assert (ptr != NULL);

  tt=(ptr->y +1 +MAZE_H) % MAZE_H; // 02-05-2002
  if ( (ptr->dy == 0) && 
       ! CheckMazeFree(ptr->x, tt) ) {
    (*doWallClick)(ptr);
  } else if ( (ptr->dy >= 0) && CheckBomb(ptr->x,tt)) {
    (*doBombClick)(ptr);
  } else {
    ptr->dy += BOMB_VY;
    if (ptr->dy >= BLOCK_HEIGHT/2) {
      SND_Play (SND_SLIDE, (ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
      bombMaze[ptr->x][ptr->y] = NULL;
      ptr->dy -= BLOCK_HEIGHT;
      ptr->y  += 1;
      ptr->y  = tt;
      bombMaze[ptr->x][ptr->y] = ptr;
    }
  }
} /* MoveBombDown */

/* 
 * move bomb one step right
 */
static void
MoveBombRight (Explosion *ptr)
{
  int tt;
  assert (ptr != NULL);

  tt=(ptr->x +1 +MAZE_W) % MAZE_W; // 02-05-2002
  if ( (ptr->dx == 0) && 
       ! CheckMazeFree(tt, ptr->y) ) {
    (*doWallClick)(ptr);
  } else if ( (ptr->dx >= 0) && CheckBomb(tt,ptr->y)) {
    (*doBombClick)(ptr);
  } else {
    ptr->dx += BOMB_VX;
    if (ptr->dx >= BLOCK_WIDTH/2) {
      SND_Play (SND_SLIDE, (ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
      bombMaze[ptr->x][ptr->y] = NULL;
      ptr->dx -= BLOCK_WIDTH;
      ptr->x  += 1;
      ptr->x  = tt;
      bombMaze[ptr->x][ptr->y] = ptr;
    }
  }
} /* MoveBombRight */

/*
 * move bomb one step left
 */
void
MoveBombLeft (Explosion *ptr)
{
  int tt;
  assert (ptr != NULL);

  tt=(ptr->x -1 +MAZE_W) % MAZE_W; // 02-05-2002
  if ( (ptr->dx == 0) && 
       ! CheckMazeFree(tt, ptr->y) ) {
    (*doWallClick)(ptr);
  } else if ( (ptr->dx <= 0) && CheckBomb(tt,ptr->y)) {
    (*doBombClick)(ptr);
  } else {
    ptr->dx -= BOMB_VX;
    if (ptr->dx <= -BLOCK_WIDTH/2) {
      SND_Play (SND_SLIDE, (ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
      bombMaze[ptr->x][ptr->y] = NULL;
      ptr->dx += BLOCK_WIDTH;
      ptr->x  -= 1;
      ptr->x  = tt;
      bombMaze[ptr->x][ptr->y] = ptr;
    }
  }
} /* MoveBombLeft */

/*
 * work all bombs in list
 */
void 
DoBombs (BMPlayer *ps, int numPlayer) // void changed for search bombs
{
  Explosion *ptr;
  int bombToHaunt = - 1;
/* Added by Fouf on 09/14/99 23:55:18 */ /* Written by Amilhastre */   int tt, difX, difY , bwX, bwY;


  /* determine if any bomb is haunted */
  if (HAUNT_NONE != hauntFactor) {
    bombToHaunt = GameRandomNumber (hauntFactor);
    if (bombToHaunt > 6) {
      bombToHaunt = -1;
    }
  }

  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->count == 0) {
      /* Bomb is just exploding */
      if (NULL != ptr->sprite) {
	ptr->anime = MAX_BOMB_ANIME -1;
	SetSpriteAnime (ptr->sprite, ptr->anime);
	SetSpriteMode (ptr->sprite, SPM_MAPPED);
	MoveSprite (ptr->sprite, ptr->x*BLOCK_WIDTH, ptr->y*BLOCK_HEIGHT);
      }
    } else if (ptr->count < 0) {
      /* Bomb has not yet exploded */
      /* haunt bomb */
      if (HAUNT_NONE != hauntFactor) {
	if (0 == bombToHaunt) {
	  HauntBomb (ptr);
	}
	bombToHaunt --;
      } if (ptr->stop && (ptr->dx == 0) && (ptr->dy == 0)) {
	//	fprintf(stderr," stoping!!\n");
/* EPFL STOP ++ */     ptr->dir = GoStop;
 /* EPFL STOP ++ */	  ptr->stop = XBFalse;
/* EPFL STOP ++ */   }

/* Added by Fouf on 09/02/99 22:46:25 */ /* Added by "Belgium Guys" */
      if (ptr->jump == 1) { 	
	ptr->dir=GoStop; 
      }
/* Written by Amilhastre */       /* new dir for up-going or down-going search bomb */
/* Written by Amilhastre */  if (ptr->type == BMTsearch && ptr->dir != GoStop){
 /* Written by Amilhastre */ 	if (ptr->dir == GoUp || ptr->dir == GoDown){
 /* Written by Amilhastre */ 	  if (ptr->dy == 0) { 
 /* Written by Amilhastre */ 	    bwX = ptr->x*BLOCK_WIDTH ;
 /* Written by Amilhastre */ 	    bwY = ((ptr->y)-1)*BLOCK_HEIGHT;
 /* Written by Amilhastre */ 	    for (tt = 0 ; tt < numPlayer; tt++) {
 /* Written by Amilhastre */ 	      if ((ps[tt].lives)&&((ptr->player!=NULL)?(ps[tt].team != ptr->player->team):1)){
 /* Written by Amilhastre */ 		difX = bwX - ps[tt].x ;
 /* Written by Amilhastre */ 		if((ABS(difX) <= BOMB_SEARCH_X) && (ABS(bwY-ps[tt].y) < BLOCK_HEIGHT)){
 /* Written by Amilhastre */ 		  if (difX > 0 && (CheckMazeFree(ptr->x-1, ptr->y))) {ptr->dir = GoLeft;break;}
 /* Written by Amilhastre */ 		  if (difX < 0 && (CheckMazeFree(ptr->x+1, ptr->y))) {ptr->dir = GoRight;break;}
 /* Written by Amilhastre */ 		}
 /* Written by Amilhastre */ 	      }
 /* Written by Amilhastre */ 	    }
 /* Written by Amilhastre */ 	  }
 /* Written by Amilhastre */ 	}
 /* Written by Amilhastre */ 	/* new dir for left-going or right-going search bomb */
 /* Written by Amilhastre */ 	else if (ptr->dir == GoLeft || ptr->dir == GoRight){
 /* Written by Amilhastre */ 	  if (ptr->dx == 0) { 
 /* Written by Amilhastre */ 	    bwX = ptr->x*BLOCK_WIDTH ;
 /* Written by Amilhastre */ 	    bwY = ((ptr->y)-1)*BLOCK_HEIGHT;
 /* Written by Amilhastre */ 	    for (tt = 0 ; tt < numPlayer; tt++) {
 /* Written by Amilhastre */ 	      if ((ps[tt].lives)&&((ptr->player!=NULL)?(ps[tt].team != ptr->player->team):1)){
 /* Written by Amilhastre */ 		difY = bwY- ps[tt].y;
 /* Written by Amilhastre */ 		if ((ABS(difY) <= BOMB_SEARCH_Y) && (ABS(bwX-ps[tt].x) <  BLOCK_WIDTH)){
 /* Written by Amilhastre */ 		  if(difY > 0 && (CheckMazeFree(ptr->x, ptr->y-1))) {ptr->dir = GoUp;break;}
 /* Written by Amilhastre */ 		  if(difY < 0 && (CheckMazeFree(ptr->x, ptr->y+1))) {ptr->dir = GoDown;break;}
 /* Written by Amilhastre */ 		}
 /* Written by Amilhastre */ 	      }
 /* Written by Amilhastre */ 	    }
 /* Written by Amilhastre */ 	  }
 /* Written by Amilhastre */ 	}
 /* Written by Amilhastre */       }
 /* Written by Amilhastre */     


      /* old version before skywalker
      switch(ptr->dir) {
      case GoUp:    MoveBombUp (ptr); break;
      case GoDown:  MoveBombDown (ptr); break;
      case GoRight: MoveBombRight (ptr); break;
      case GoLeft:  MoveBombLeft (ptr); break;
      default:      break;
      }*/

      /** Skywalker **/
      if(ptr->isSniping==0){
	/** **/
	switch(ptr->dir) {
	case GoUp:    MoveBombUp (ptr);
/* Added by Fouf on 09/02/99 22:46:25 */ /* Added by "Belgium Guys" */ 
/* Added by Fouf on 09/02/99 22:46:25 */ /* Added by "Belgium Guys" */       if(ptr->jump>0) {
                 ptr->type=BMTshort;
                 if (ptr->dy <= -BLOCK_HEIGHT/2) {
                      bombMaze[ptr->x][ptr->y] = NULL;
                      ptr->dy += BLOCK_HEIGHT;
                      ptr->y -= 1;
                      bombMaze[ptr->x][ptr->y] = ptr;
                  }
 		ptr->dy -= BOMB_VY;
                 if(ptr->y<=1) { ptr->dy=0; }
                 ptr->jump--;
       } else {
        break;
        }
 break;
	case GoDown:  MoveBombDown (ptr);
        if(ptr->jump>0) {
         ptr->type=BMTshort;
          if (ptr->dy >= BLOCK_HEIGHT/2) {
 	  bombMaze[ptr->x][ptr->y] = NULL;
           ptr->dy -= BLOCK_HEIGHT;
           ptr->y += 1;
           bombMaze[ptr->x][ptr->y] = ptr;
           }
           ptr->dy += BOMB_VY;
           if(ptr->y>=MAZE_H-2) { ptr->dy=0; }
           ptr->jump--;
         } else {
        }
 break;
	case GoRight: MoveBombRight (ptr); 
         if(ptr->jump>0) {
 	       if (ptr->dx >= BLOCK_WIDTH/2) {
 		  bombMaze[ptr->x][ptr->y] = NULL;
                   ptr->dx -= BLOCK_WIDTH;
                   ptr->x += 1;
                   bombMaze[ptr->x][ptr->y] = ptr;
 		}
 		ptr->dx += BOMB_VX;
                 if(ptr->x>=MAZE_W-2) { ptr->dx=0; }
 		ptr->jump--;
         } else {
         }
break;
	case GoLeft:  MoveBombLeft (ptr);
        if(ptr->jump>0) {
          if (ptr->dx <= -BLOCK_WIDTH/2) {
             bombMaze[ptr->x][ptr->y] = NULL;
             ptr->dx += BLOCK_WIDTH;
             ptr->x -= 1;
             bombMaze[ptr->x][ptr->y] = ptr;
          }
          ptr->dx -= BOMB_VX;
          if(ptr->x<=1) { ptr->dx=0; }
 	 ptr->jump--;
        } else {  
         }
 break;
	default:      break;
	}
	/** Skywalker **/
      } else {
	if(NULL != ptr->player) {
	//	ptr->player->x = ptr->dx + ptr->x * BLOCK_WIDTH;
	//	ptr->player->y = ptr->dy + (ptr->y-1) * BLOCK_HEIGHT; 
	//	move_sprite (ptr->player->sprite, ptr->player->x, ptr->player->y + BASE_Y); 
	  switch(ptr->player->d_soll) {
	  case GoUp:
	    MoveBombUp(ptr);
	    break;
	  case GoDown:
	    MoveBombDown(ptr);
	    break;
	  case GoRight:
	    MoveBombRight (ptr);
	    break;
	  case GoLeft:
	    MoveBombLeft (ptr);
	    break;
	  default:
	    break;
	  }
	}
	else{/** **/
	  switch(ptr->dir) {
	  case GoUp:    MoveBombUp (ptr); break;
	  case GoDown:  MoveBombDown (ptr); break;
	  case GoRight: MoveBombRight (ptr); break;
	  case GoLeft:  MoveBombLeft (ptr); break;
	  default:      break;
	  }
	}
      }	


      /* move bomb sprite */
      if (NULL != ptr->sprite) {
	MoveSprite (ptr->sprite, ptr->x*BLOCK_WIDTH + ptr->dx, ptr->y*BLOCK_HEIGHT + ptr->dy);
      }
      /* if player is morphed */
      if (ptr->isMorphed && NULL != ptr->player) {
	ptr->player->x = ptr->dx + ptr->x * BLOCK_WIDTH;
	ptr->player->y = ptr->dy + (ptr->y-1) * BLOCK_HEIGHT;
	MoveSprite (ptr->player->sprite, ptr->player->x, ptr->player->y + BASE_Y);
      }
      /* check if bomb is on an explosion */
      if (CheckExplosion(ptr->x, ptr->y)) {
	ptr->count = 0;
      }
      /* handle bomb animation and blinking */
      if (ptr->sprite != NULL) {
	if (ptr->blink + ptr->count == 0) {
	  SetSpriteMode (ptr->sprite, SPM_MAPPED | SPM_MASKED);
	  ptr->blink = ptr->blink >> 1;
	} else {
	  SetSpriteMode (ptr->sprite, SPM_MAPPED);
	}
	if (ptr->count == ptr->nextAnime) {
	  ptr->anime ++;
	  ptr->nextAnime += curBombTime / (MAX_BOMB_ANIME - 1);
	  SetSpriteAnime (ptr->sprite, ptr->anime);
	}
      }
      /* Bomb malfunction, random or illness */
      if ( (ptr->count == -3) 
	   && (ptr->malfunction || (GameRandomNumber (BOMB_ERROR_PROB) == 0) ) ) {
	ptr->malfunction = 0;
	ptr->count = -BOMB_TIME *(2 + GameRandomNumber (BOMB_DELAY));
	ptr->blink = (BOMB_TIME >>1);
      }
    }
  }
} /* DoBombs */

int 
StopPlayersBombs (BMPlayer *ps)
{
  Explosion *ptr;
  int numberOfBombs = 0;

  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->count < 0) {
      /* draw a bomb */
      if ( ptr->player == ps ) {
	//	fprintf(stderr," stoping!!!\n");

	ptr->stop = XBTrue;
	numberOfBombs ++;
      }
    }
  }
  return(numberOfBombs);
}
/* 
 * ignite all bombs of one given player
 */
int 
IgnitePlayersBombs (BMPlayer *ps)
{
  Explosion *ptr;
  int numberOfBombs = 0;

  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->count < 0) {
      /* draw a bomb */
      if ( ptr->player == ps ) {
	ptr->count = 0;
	numberOfBombs ++;
      }
    }
  }
  return numberOfBombs;
} /* IgnitePlayersBombs */

/* 
 * ignite all bombs 
 */
int 
IgniteAllBombs (BMPlayer *ps)
{
  Explosion *ptr;
  int numberOfBombs = 0;

  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->count < 0) {
      ptr->count = 0;
      numberOfBombs ++;
    }
  }
  return numberOfBombs;
} /* IgniteAllBombs */

/*
 * ignite bombs on explosions
 */
void 
IgniteBombs (void)
{
  Explosion *ptr;

  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->count < 0) {
      /* draw a bomb */
      if (CheckExplosion(ptr->x, ptr->y)) {
	ptr->count = 0;
      }
    }
  }
} /* IgniteBombs */

/* 
 * handle all explosions in list
 */
void 
DoExplosions (void)
{
  Explosion *ptr, *nextPtr;
  register int hilf;

  ptr = explList;
  while (ptr != NULL) {
       
    nextPtr = ptr->next;
    /* check if bomb is exploding */
    if (ptr->count >= 0) {
      /* hide morphed player's eyes */
      if (ptr->count == 0 &&
	  ptr->isMorphed &&
	  ptr->player != NULL) {
	ptr->player->morphed = 3;
      }
      /* check if bomb has burned out */
      if ( (ptr->burnOut == BO_TOTAL) ||
	   (ptr->count  >= (2*ptr->range + 2) ) ) {
	DelExplosion(ptr);
	ptr = nextPtr;
	continue;
      } else {
	/* get exploding time */
	if ( (hilf = ptr->count) == 0) {
	
	  /* set any free tiel to burned */
	  if (CheckMazeFree (ptr->x, ptr->y) ) {
	    SetMazeBlock (ptr->x, ptr->y, BTBurned);
	  }
	  /* play according sound */
          if (ptr->range == 1 ||
              ptr->type == BMTfirecracker ||
              ptr->type == BMTfirecracker2) {
	    SND_Play (SND_MINIBOMB, (ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
	  } else {
	    SND_Play (SND_EXPL, (ptr->x * BLOCK_WIDTH) / (PIXW / MAX_SOUND_POSITION));
	  }
	}
	/* now do the explosion */
	OneExplAt (ptr->x, ptr->y, MIN(ptr->range,hilf), MAX(0,( (hilf) - ptr->range)), &(ptr->burnOut) );
      }
    }
    if ((*doSpecialBombFunction[ptr->type])(ptr)) {
      if (slowMotionBurst) {
	if( ptr->count>0){
	  ptr->count += ptr->countslower2 % slowMotionBurst==0;
	} else {
	  ptr->count++;
	}
	ptr->countslower+= ptr->countslower2%slowMotionBurst==0;
	ptr->countslower2++;
      } else {
	ptr->count++;
      }
      if ( 2 == ptr->count && 
	   NULL != ptr->sprite ) {
	DeleteSprite (ptr->sprite);
	ptr->sprite = NULL;
      }
    } else {
      /* bomb deleted you must set explicitly to NULL (dont ask me why) */
      /* if (doSpecialBombFunction[ptr->type]==SpecialBombDestruction||
         doSpecialBombFunction[ptr->type]==SpecialBombConstruction){	 
    }*/
      ptr=NULL;
    }
    if (ptr!=NULL) {
      ptr=ptr->next;
    } else {
       ptr = nextPtr;
    }
  }
} /* DoExplosions */

/*------------------------------------------------------------------------*
 * 
 * Special bomb code (Garth Denley) 
 *
 *------------------------------------------------------------------------*/

/* 
 * Used to spread an explosion out 
 */
static void 
SpreadExplosion (int lx, int ly, int range, int type, int typeExtr)
{
  if ( (lx < MAZE_W) && 
       (lx > -1) && 
       (ly < MAZE_H) && 
       (ly > -1) && 
       ! CheckMazeSolid (lx, ly) ) {
    NewExplosion (NULL, lx, ly, range, XBFalse, XBFalse, type, typeExtr, GoStop);
  }
} /* SpreadExplosion */


/* Added by Fouf on 09/02/99 22:46:25 */ /* Added by "Belgium Guys" */ 
static void
SpreXDir (int lx,
	  int ly,
	  int range,
	  int type,
	  int type_extr,
	  BMDirection dir)
 {
   if ( (lx < MAZE_W) && (lx > -1) && (ly < MAZE_H) && (ly > -1)
        && !CheckMazeSolid(lx, ly) ) {
     NewExplosion(NULL, lx, ly, range, XBFalse, XBFalse,
                   type, type_extr, dir);
   }
 }
 

/*
 *
 */
static void
MoveBlockFromTo (int sx, int sy, int dx, int dy)
{
  if ( (sx > 0) && (sx < MAZE_W-1) && 
       (sy > 0) && (sy < MAZE_H-1) && 
       (dx > 0) && (dx < MAZE_W-1) && 
       (dy > 0) && (dy < MAZE_H-1) && 
       CheckMazeWall(sx, sy) && 
       CheckMazeFree(dx, dy)) {
    KillPlayerAt (dx, dy);
    DeleteBombAt (dx, dy);
    SetMazeBlock (sx, sy, BTFree);
    SetMazeBlock (dx, dy, BTBlock);
  }
} /* MoveBlockFromTo */

/*
 * special bomb functions
 */

/*
 *
 */
static XBBool
SpecialBombNormal (Explosion *ptr)
{
  return XBTrue;
}

/* 
 * napalm bomb 
*/
static XBBool
SpecialBombNapalm (Explosion *ptr)
{
  int i;

  //  if (ptr->count == -1) {
  if (ptr->count == 0) {
    ptr->type = BMTnormal;
    for (i= -2; i<=2; i++) {
      SpreadExplosion (ptr->x+i, ptr->y, ptr->range/(ABS(i)+1), BMTblastnow, 0);
      SpreadExplosion (ptr->x, ptr->y+i, ptr->range/(ABS(i)+1), BMTblastnow, 0);
    }
  }
  return XBTrue;
} /* SpecialBombNapalm */

/* 
 * firecracker 
 */
static XBBool
SpecialBombFirecracker (Explosion *ptr)
{
  int i;
  int nasty;

  if (ptr->count >= 1) {
    if (ptr->type == BMTfirecracker && 
	GameRandomNumber (10) == 0) {
      ptr->typeExtr = -5;
    }
    nasty = ptr->typeExtr;
    for (i= -1; i<=1; i++) {
      if (nasty < 2 || 0 == GameRandomNumber (1 + nasty) ) {
	SpreadExplosion (ptr->x+i, ptr->y, 1, BMTfirecracker2, nasty + 1);
      }
      if (nasty < 2 || 0 == GameRandomNumber (1 + nasty) ) {
	SpreadExplosion (ptr->x, ptr->y+i, 1, BMTfirecracker2, nasty + 1);
      }
    }
    ptr->type = BMTnormal;
  }

  return XBTrue;
} /* SpecialBombFirecracker */

/* 
 * construction 
 */
static XBBool
SpecialBombConstruction (Explosion *ptr)
{
  int x, y;

  if (ptr->count == 1) {
    x = ptr->x;
    y = ptr->y;
    DelExplosion(ptr);
    if (! CheckPlayerNear (x,y) ) {
      SetMazeBlock (x, y, BTExtra);
      SetBlockExtra (x, y, BTFree);
    }
    return XBFalse;
  }
  return XBTrue;
} /* SpecialBombConstruction */

/*
 * fungus 
 */
static XBBool
SpecialBombFungus (Explosion *ptr)
{
  int i, x, y;

  x = ptr->x;
  y = ptr->y;
  if (ptr->count == (-curBombTime)*3/5) {
    for (i = -1; i<=1; i++) {
        SpreadExplosion(x+i,y,1,BMTfungus,0);
        SpreadExplosion(x,y+i,1,BMTfungus,0);
      }
  }
  return XBTrue;
} /* SpecialBombFungus */

/* 
 * threebombs 
 */
static XBBool
SpecialBombThreebombs (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    SpreadExplosion(ptr->x-2,ptr->y,ptr->range,defaultBMT,0);
    SpreadExplosion(ptr->x+2,ptr->y,ptr->range,defaultBMT,0);
  }
  return XBTrue;
} /* SpecialBombThreebombs */

/* 
 * grenade 
 */
static XBBool
SpecialBombGrenade (Explosion *ptr)
{
  int i, j;

  if (ptr->range > 0) {
    if (ptr->count == -1) {
      if (ptr->range == 1) {
	SpreadExplosion(ptr->x-1, ptr->y-1, 0, BMTblastnow, 0);
	SpreadExplosion(ptr->x+1, ptr->y-1, 0, BMTblastnow, 0);
	SpreadExplosion(ptr->x-1, ptr->y+1, 0, BMTblastnow, 0);
	SpreadExplosion(ptr->x+1, ptr->y+1, 0, BMTblastnow, 0);
      } else {
	for (i = -((ptr->range)-1); i<=((ptr->range)-1); i++) {
	  for (j = -((ptr->range)-1); j<=((ptr->range)-1); j++) {
	    SpreadExplosion (ptr->x+i, ptr->y+j, 1, BMTblastnow, 0);
	  }
	}
      }
    }
  }
  return XBTrue;
} /* SpecialBombGrenade */

/* 
 * trianglebombs 
*/
static XBBool
SpecialBombTrianglebombs (Explosion *ptr)
{
  if (ptr->count == -curBombTime + 2) {
    int i,j;
    
    i = GameRandomNumber (2) * 4 - 2;
    j = GameRandomNumber (2) * 4 - 2;
    SpreadExplosion (ptr->x+i, ptr->y,   ptr->range, BMTnormal, 0);
    SpreadExplosion (ptr->x,   ptr->y+j, ptr->range, BMTnormal, 0);
    ptr->type = BMTnormal;
  }
  return XBTrue;
} /* SpecialBombTrianglebombs */

/* desctruction */
static XBBool
SpecialBombDestruction (Explosion *ptr)
{
  int i, x, y;

  x = ptr->x;
  y = ptr->y;
  if (ptr->count == 1) {
    DelExplosion(ptr);
    for (i = -1; i<=1; i++) {
      if ( (x+i < (MAZE_W-1)) && 
	   CheckMaze(x+i,y) && 
	   (x+i > 0) ) {
	SetMazeBlock(x+i,y,BTFree);
      }
      if ( (y+i < (MAZE_H-1)) && 
	   CheckMaze(x,y+i) && 
	   (y+i > 0) ) {
	SetMazeBlock(x,y+i,BTFree);
      }
    }    
    return XBFalse;
  }
  return XBTrue;
} /* SpecialBombDestruction */

/* 
 * renovation 
*/
static XBBool
SpecialBombRenovation (Explosion *ptr)
{
  int x, y;

  x = ptr->x;
  y = ptr->y;
  if (ptr->count == 1) {
    MoveBlockFromTo (x-1, y,   x-2, y  );
    MoveBlockFromTo (x+1, y,   x+2, y  );
    MoveBlockFromTo (x,   y-1, x,   y-2);
    MoveBlockFromTo (x,   y+1, x,   y+2);
  }
  return XBTrue;
} /* SpecialBombdRenovation */

/* 
 * pyro 
*/
static XBBool
SpecialBombPyro (Explosion *ptr)
{
  int x, y, k;

  if (ptr->count == 1) {
    for (k=0;k<5;k++) {
      x = ptr->x + GameRandomNumber (3) - 1;
      y = ptr->y + GameRandomNumber (3) - 1;  
      if ( ! bombMaze[x][y] && 
	   CheckMazeFree (x,y) ) {
	SpreadExplosion (x, y, 1, BMTpyro2, 0);
	break;
      }
    }
  }
  return XBTrue;
} /* SpecialBombPyro */

/*
 *
 */
static XBBool 
NewExplosion (BMPlayer *player, int x, int y, int range, XBBool remote_controlled, XBBool malfunction, BMBombType type, 
	      int typeExtr, BMDirection initialdir)
{
  Explosion *newExpl;

  /* check if there is already a bomb in this tile */
  if (NULL != bombMaze[x][y]) {
    return XBFalse;
  }
  
  /* mark for redraw */
  MarkMazeTile (x,y);
  /* alloc data */
  newExpl = (Explosion *) calloc(1,sizeof(Explosion));
  /* put in lookup map */
  bombMaze[x][y] = newExpl;
  /* put in explosion list */
  numExpl ++;
  if (explList == NULL) {
    explList = newExpl;
  } else {
    explEnd->next = newExpl;
  }
  explEnd   = newExpl;
  /* set values */
  newExpl->next        = NULL;
  newExpl->player      = player;
  newExpl->x           = x;
  newExpl->y           = y;
  newExpl->dx          = 0;
  newExpl->dy          = 0;
  newExpl->malfunction = malfunction;
  newExpl->isMorphed   = (NULL != player) ? player->morphed : XBFalse;
  /** Skywalker **/
  if(NULL != player){
  newExpl->isSniping   =  player->sniping;
  if( player->sniping){
  newExpl->count=-GAME_TIME;
  }
  } else {
    newExpl->isSniping   = 0;
  }
  /** **/
  newExpl->dir         = (initialdir == GoDefault) ? initialBombDir : initialdir;
  /* set type */
  switch (type) {
  case BMTdefault: type = defaultBMT; break;
  case BMTspecial: type = specialBMT; break;
  case BMTevil:    type = evilBMT;    break;
  default:                            break;
  }
  newExpl->type = type;
  /* Random bomb ! */
  if (newExpl->type == BMTrandom) {
    switch (GameRandomNumber (5)) {
    case 0: newExpl->type = BMTnapalm;      break;
    case 1: newExpl->type = BMTfirecracker; break;
    case 2: newExpl->type = BMTgrenade;     break;
    case 3: newExpl->type = BMTfungus;      break;
    case 4: newExpl->type = BMTpyro;        break;
    }
  }
  /* extry type info */
  newExpl->typeExtr = typeExtr;
  /* these are "nasty bombs"  */
  if (type != BMTclose) {
    switch(newExpl->dir) {
    case GoDown:
      if(CheckMazeFree(newExpl->x,newExpl->y+1)) {
	newExpl->dy = BOMB_VY;
      }
      break;
    case GoUp:
      if(CheckMazeFree(newExpl->x,newExpl->y-1)) {
	newExpl->dy = -BOMB_VY;
      }
      break;
    case GoLeft:
      if(CheckMazeFree(newExpl->x-1,newExpl->y)) {
	newExpl->dx = -BOMB_VX;
      }
      break;
      
    case GoRight:
      if(CheckMazeFree(newExpl->x+1,newExpl->y)) {
	newExpl->dx = BOMB_VX;
      }
      break;

    default:
      break;
    }
  }
  /* set range according to type */
  switch (type) {
  case BMTfirecracker:
  case BMTfungus:
  case BMTpyro:     
    newExpl->range = 1; 
    break;
  case BMTconstruction:
    newExpl->range = 0;
    break;
  case BMTgrenade:
    newExpl->range = range / 2;
    break;
  default:
    newExpl->range = range;
  }
  /* bomb counter */
    newExpl->countslower = -curBombTime;
  if ( (type == BMTblastnow) || 
       (type == BMTfirecracker2) || 
       (type == BMTpyro2) ) {
    /* bombs which explode immediately */
    newExpl->count = 0;
    newExpl->countslower = 0;
  } else if (remote_controlled) {
    /* remote controled bombs */
    newExpl->count     = -GAME_TIME;
    newExpl->anime     = MAX_BOMB_ANIME-2;
    newExpl->nextAnime = 1;
  } else if (type == BMTshort) {
    /* fast bombs */
    newExpl->count     = -curBombTime/4;
    newExpl->anime     = 3*MAX_BOMB_ANIME/4;
    newExpl->nextAnime = 1 + curBombTime/(MAX_BOMB_ANIME - 1) - curBombTime/4;
  } else if(newExpl->isSniping!=0){
    newExpl->count=-GAME_TIME;
  }
    else {
    /* standard bombs */
    newExpl->count = -curBombTime;
    newExpl->anime = 0;
    newExpl->nextAnime = 1 + curBombTime/(MAX_BOMB_ANIME - 1) - curBombTime;
  } 
  /* init blilinking */
  newExpl->blink = BOMB_TIME >> 1;
  /* create bomb sprite */
  if ( (type == BMTblastnow) || 
       (type == BMTfirecracker2) || 
       (type == BMTpyro2) ) {
    /* instant explosion => no sprite */
    newExpl->sprite = NULL;
  } else if ( (range ==1) || 
	      (type == BMTfirecracker) ||
	      (type == BMTfungus) ||
	      (type == BMTpyro) ) {
    /* these bombs are shown as mini bombs */
    newExpl->sprite = CreateBombSprite (BB_MINI, newExpl->x*BLOCK_WIDTH, newExpl->y*BLOCK_HEIGHT,
					newExpl->anime, SPM_MAPPED);
  } else {
    /* just the normalbomb sprite */
    newExpl->sprite = CreateBombSprite (BB_NORMAL, newExpl->x*BLOCK_WIDTH, newExpl->y*BLOCK_HEIGHT,
					newExpl->anime, SPM_MAPPED);
  }
  return XBTrue;
} /* NewExplosion */

/*
 *
 */
XBBool
NewPlayerBomb (BMPlayer *ps, BMBombType type)
{
  return NewExplosion (ps, (ps->x + BLOCK_WIDTH/2)/BLOCK_WIDTH,
		       (ps->y + BLOCK_HEIGHT + BLOCK_HEIGHT/2)/BLOCK_HEIGHT,
		       (ps->illness == IllMini) ? 1 : ps->range,
		       (ps->remote_control > 0),
		       (ps->illness == IllMalfunction),
		       type, 0, GoDefault );
} /* NewPlayerBomb */

/*
 *
 */
XBBool
NewEvilBomb (int x, int y)
{
  return NewExplosion (NULL, x, y, 3, XBFalse, XBFalse, evilBMT, 0, GoDefault);
} /* NewEvilBomb */

/* 
 * check if any moving stun (hit) any player
 */
void 
StunPlayers (BMPlayer *ps, int numPlayer)
{
  Explosion *ptr;
  int player;
  unsigned clickFlags;

  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    /* only if bomb is moving */
    if (ptr->dir != GoStop) {
      clickFlags = 0;
      for (player = 0; player < numPlayer; player ++) {
	/* check if any is vulnerable player is in range */
	if ( (ps[player].invincible == 0) && 
	     (ps[player].morphed == 0) &&
	     -	     (ps[player].daleifing ==0) &&  /* (galatius) Sky */
	     (ABS(ptr->x*BLOCK_WIDTH + ptr->dx - ps[player].x ) < BOMB_STUN_X ) && 
	     (ABS(ptr->y*BLOCK_HEIGHT + ptr->dy - ps[player].y - BLOCK_HEIGHT ) < BOMB_STUN_Y ) ) {
	  /* we need to correct some graphics here */
	  if (ptr->dx == 0) {
	    if (ptr->dy < 0) {
	      MarkMazeTile(ptr->x, ptr->y-1);
	    }
	    if (ptr->dy > 0) {
	      MarkMazeTile(ptr->x, ptr->y+1);
	    }
	  }
	  if (ptr->dy == 0) {
	    if (ptr->dx < 0) {
	      MarkMazeTile(ptr->x-1, ptr->y-1);
	    }
	    if (ptr->dx > 0) {
	      MarkMazeTile(ptr->x+1, ptr->y-1);
	    }
	  }
	  /* mark that player has been hit */
	  clickFlags |= (1<<player);
	}
      }
      /* do player click after all players are checked */
      if (clickFlags) {
	for (player = 0; player < numPlayer; player ++) {
	  /* has player been hit ... */
	  if (clickFlags & (1<<player)) {
	    /* ... stun player */
	    if (0 == ps[player].stunned) {
	      ps[player].stunned = STUN_TIME;
	      SND_Play (SND_STUN, ps[player].x / (PIXW/MAX_SOUND_POSITION) );
	    }
	  }
	}
	/* do bomb-player click */
	(*doPlayerClick)(ptr);
      }
    }
  }
} /* StunPlayers */

/* 
 * check if there is a bomb on a tile
 */
XBBool
CheckBomb (int x, int y)
{
  return (bombMaze[x][y] != NULL) && (bombMaze[x][y]->count < 0);
} /* CheckBomb */

/* 
 * how many bombs and explosions do we have
 */
int 
NumberOfExplosions (void)
{
  return numExpl;
} /* NumberOfExplosions */

/*
 * delete a bomb at a given position
 */
void 
DeleteBombAt (int x, int y)
{
  if (NULL != bombMaze[x][y]) {
    DelExplosion (bombMaze[x][y]);
  }
} /* DeleteBombAt */

/* 
 * move a bomb 
 */
void 
MoveBomb (int x, int y, int dir)
{
  Explosion *ptr;
  
  if(NULL != (ptr = bombMaze[x][y]) ) {
    if (ptr->dir == GoStop) {
      ptr->dir = dir;
      switch(dir) {
      case GoUp:
      case GoDown:
	ptr->dx = 0;
	break;
      case GoLeft:
      case GoRight:
	ptr->dy = 0;
	break;
      }
    }
  }
} /* MoveBomb */

/*
 * check where we can savely distribute extras
 */
int 
CheckDistribExpl (unsigned *distExtra, int freeBlocks) 
{
  Explosion *ptr;
  int x, y, ra;

  /* Go through explosions */
  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->count >=0) {
      ra = MIN(ptr->range, ptr->count + 1);
      /* right */
      for (x = ptr->x; x <= (ptr->x + ra) && x < MAZE_W; x ++) {
	if (distExtra[ptr->y] & (1 << x)) {
	  freeBlocks --;
	}
	distExtra[ptr->y] &= ~ (1 << x);
      }
      /* left */
      for (x = ptr->x; x >= (ptr->x - ra) && x >= 0; x --)	{
	if (distExtra[ptr->y] & (1 << x)) {
	  freeBlocks --;
	}
	distExtra[ptr->y] &= ~ (1 << x);
      }
      /* down */
      for (y = ptr->y; y <= (ptr->y + ra) && y < MAZE_H; y ++) {
	if (distExtra[y] & (1 << ptr->x)) {
	  freeBlocks --;
	}
	distExtra[y] &= ~ (1 << ptr->x);
      }
      for (y = ptr->y; y >= (ptr->y - ra) && y >= 0; y --)	{
	if (distExtra[y] & (1 << ptr->x) ) {
	  freeBlocks --;
	}
	distExtra[y] &= ~ (1 << ptr->x);
      }
    }
  }
  return freeBlocks;
} /* CheckDistribExpl */

/* 
 * public function do_air (Garth Denley)
 * Shoots bombs away if within 2 square radius 
 * Direction based on angle from bomb 
 */
void 
DoAir (BMPlayer *ps)
{
  Explosion *ptr;
  int x, y, ex, ey;

  assert (NULL != ps);
  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if (ptr->dir == GoStop)  {
      x  = (ptr->x)   * BLOCK_WIDTH;
      y  = (ptr->y-1) * BLOCK_HEIGHT;
      ex = x - ps->x;
      ey = y - ps->y;
      if ( ABS (ex) < 2*BLOCK_WIDTH && 
	   ABS (ey) < 2*BLOCK_HEIGHT && 
	   (ex != 0 || ey != 0) ) {
        if (ABS (ex) * BLOCK_HEIGHT >= ABS(ey) * BLOCK_WIDTH)  {
          if (ex < 0) {
	    ptr->dir = GoLeft;
	    if(CheckMaze(ptr->x-1,ptr->y)) {
	      if (ey < 0) {
		ptr->dir = GoUp; 
	      } else if(ey > 0) {
		ptr->dir = GoDown;
	      }
	    }
	  } else { 
	    ptr->dir = GoRight;
	    if(CheckMaze(ptr->x+1,ptr->y)) {
	      if (ey < 0) {
		ptr->dir = GoUp; 
	      } else if(ey > 0) {
		ptr->dir = GoDown;
	      }
	    }
	  }
        } else {
          if (ey < 0) {
	    ptr->dir = GoUp; 
	  } else {
	    ptr->dir = GoDown;
	  }
        }
      }
    }
  }
} /* DoAir */

 /* public function DoSuck (Stephan Natschlaeger)*/
 
 /* Shoots bombs away if within 2 square radius */
 /* Direction based on angle from bomb */
 
void 
DoSuck (BMPlayer *ps)
 {
   Explosion *ptr;
   int x,y,ex,ey;
 
  assert (NULL != ps);
   for (ptr = explList; ptr != NULL; ptr = ptr->next) {
     if ((ptr->dir == GoStop))  {
       x = (ptr->x) * BLOCK_WIDTH;
       y = (ptr->y-1) * BLOCK_HEIGHT;
       ex = x - ps->x;
       ey = y - ps->y;
       if ((ABS(ex)<BLOCK_WIDTH*2)&&
	   (ABS(ey)<BLOCK_HEIGHT*2)
 	  && ( (ex!=0)||(ey!=0) ) ) {
         if (ABS(ex)*BLOCK_HEIGHT >= ABS(ey)*BLOCK_WIDTH)  {
           if (ex<0) {
 	    ptr->dir=GoRight; 
 	  } else { 
 	    ptr->dir=GoLeft;
 	  }
         } else {
           if (ey<0) {
 	    ptr->dir=GoDown; 
 	  } else {
 	    ptr->dir=GoUp;
 	  }
         }
       }
     }
   }
 } /* do suck */


/* Added by Fouf on 09/02/99 22:46:25 */ /* Added by "Belgium Guys" */ 
void
DoJump (BMPlayer *ps)
{
  Explosion *ptr;
  int x,y,ex,ey;
  
  for (ptr = explList; ptr != NULL; ptr = ptr->next) {
    if ((ptr->dir == GoStop))  {
      x = (ptr->x) * BLOCK_WIDTH;
      y = (ptr->y-1) * BLOCK_HEIGHT;
      ex = x - ps->x;
      ey = y - ps->y;
      if ((ABS(ex)<BLOCK_WIDTH*2)&&(ABS(ey)<BLOCK_HEIGHT*2)
	  && ((ex!=0)||(ey!=0))) {
#ifdef SCORE
	ptr->score_flag |= 1<<(ps->id);
#endif
	ptr->jump=(int)(7*ps->jump_button);
	if (ABS(ex)*BLOCK_HEIGHT >= ABS(ey)*BLOCK_WIDTH)  {
	  if (ex<0) {
	    ptr->dir=GoLeft;
           } else {
             ptr->dir=GoRight;
           }
	} else {
	  if (ey<0) {
	    ptr->dir=GoUp;
	  } else {
	    ptr->dir=GoDown;
	  }
	}
      }
    }
  }
}

/*
 *
 */
void
DoNastyWalls (int gameTime)
{
  int x, y;
  int dir = GoStop;

  if (gameTime >= nextNasty) {
    if (gameTime >= nextNasty + NASTY_INC) {
      nextNasty += NASTY_INC*2;
    }
    if (GameRandomNumber (gentleNasty) < gameTime) {
      if(ceilNasty){
	x = GameRandomNumber (MAZE_W-2) + 1;
	y = GameRandomNumber (MAZE_H-2) + 1;
        dir=GoDown;
        y=1;
      }else{
	dir = GoUp + GameRandomNumber (4);
	switch (dir) {
	case GoUp:
	  x   = GameRandomNumber (MAZE_W-2) + 1;
	  y   = MAZE_H-2;
	  break;
	case GoLeft:
	  x   = MAZE_W-2;
	  y   = GameRandomNumber (MAZE_H-2) + 1;
	  break;
	case GoDown:
	  x   = GameRandomNumber (MAZE_W-2) + 1;
	  y   = 1;
	  break;
	case GoRight:
	  x   = 1;
	  y   = GameRandomNumber (MAZE_H-2) + 1;
	  break;
	default:
	  return;
	}
      }
      NewExplosion (NULL, x, y, rangeNasty, XBFalse, XBFalse, BMTclose, 0, dir);
    }
  }
} /* DoNastyWalls */

/* EPFL SHIT */

static XBBool SpecialBombDiagThree (Explosion *ptr)
{
  int i;

  if (ptr->count == -curBombTime) {
    ptr->type = BMTnormal;
    for (i=0; i<=2; i++) {
      SpreadExplosion (ptr->x+i, ptr->y+i, ptr->range, BMTnormal, 0);
    }
  }
  return XBTrue;
}


/* scissor */  /* added by stn */
static XBBool
SpecialBombScissor (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    //fprintf(stderr,"PLACE SCISSOR\n");
    SpreadExplosion(ptr->x+1,ptr->y+1,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+1,ptr->y-1,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+2,ptr->y+2,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+2,ptr->y-2,ptr->range,BMTnormal,0);
  }
  return XBTrue;
}

/* scissor2 */  /* added by stn */
static XBBool
SpecialBombScissor2 (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    // fprintf(stderr,"PLACE SCISSOR 2\n");
    SpreadExplosion(ptr->x-1,ptr->y+1,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-1,ptr->y-1,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-2,ptr->y+2,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-2,ptr->y-2,ptr->range,BMTnormal,0);
    ptr->type = BMTnormal;
  }
  return XBTrue;
}

/* parallel */  /* added by stn */
static XBBool
SpecialBombParallel (Explosion *ptr)
{
  int i;

  if (ptr->count == -curBombTime) {
    for (i=-2; i<=2; i++) {
      SpreadExplosion(ptr->x+2*i,ptr->y-i,ptr->range/(ABS(i)+1),BMTnormal,0);
    }
  }
  ptr->type = BMTnormal;
  return XBTrue;
}

/* distance */  /* added by stn */
static XBBool
SpecialBombDistance (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    int i, j;
    i = 14-ptr->x;
    j = 12-ptr->y;
    SpreadExplosion(i,j,ptr->range,BMTnormal,0);
  }
  ptr->type = BMTnormal;
  return XBTrue;
}

/* lucky */  /* added by stn */
static XBBool
SpecialBombLucky (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    int i, j;

    i = ptr->x + GameRandomNumber(6) + 1;
    j = ptr->y + GameRandomNumber(5) + 1;
    SpreadExplosion(i,j,ptr->range,defaultBMT,0);
  }
  ptr->type = BMTnormal;
  return XBTrue;
}

/* parasol */  /* added by stn */
static XBBool
SpecialBombParasol (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    SpreadExplosion(ptr->x-2,ptr->y-1,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-1,ptr->y-2,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x,ptr->y-3,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+1,ptr->y-2,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+2,ptr->y-1,ptr->range,BMTnormal,0);
  }
  ptr->type = BMTnormal;
  return XBTrue;
}

/* comb */  /* added by stn */
static XBBool
SpecialBombComb (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    SpreadExplosion(ptr->x-2,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-2,ptr->y-2,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-1,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x,ptr->y-2,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+1,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+2,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+2,ptr->y-2,ptr->range,BMTnormal,0);
  }
  ptr->type = BMTnormal;
  return XBTrue;
}

/* farpyro */  /* added by stn */
static XBBool
SpecialBombFarpyro (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    SpreadExplosion(ptr->x-2,ptr->y-2,ptr->range,BMTpyro,0);
    SpreadExplosion(ptr->x+2,ptr->y-2,ptr->range,BMTpyro,0);
  }
  ptr->type = BMTpyro;
  return XBTrue;
}

/* nuclear */  /* added by stn */
static XBBool
SpecialBombNuclear (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    SpreadExplosion (ptr->x+1,ptr->y+1,ptr->range+15,BMTgrenade,0);
    SpreadExplosion (ptr->x-1,ptr->y+1,ptr->range+15,BMTgrenade,0);
    SpreadExplosion (ptr->x+1,ptr->y-1,ptr->range+15,BMTgrenade,0);
    SpreadExplosion (ptr->x-1,ptr->y-1,ptr->range+15,BMTgrenade,0);
  }
  ptr->type = BMTgrenade;
  return XBTrue;
}

/* Added by x-bresse on 06.04.2000 */ /* Ring Of Fire */
/* Added by x-bresse on 06.04.2000 */ static XBBool SpecialBombRingofire (Explosion *ptr)
/* Added by x-bresse on 06.04.2000 */ {
/* Added by x-bresse on 06.04.2000 */     int x,y,c,r;
/* Added by x-bresse on 06.04.2000 */     x = ptr->x;
/* Added by x-bresse on 06.04.2000 */     y = ptr->y;
/* Added by x-bresse on 06.04.2000 */     c = ptr->count;
/* Added by x-bresse on 06.04.2000 */     r = ptr->range;
/* Added by x-bresse on 06.04.2000 */ 
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*24/24 || c == (-curBombTime)*12/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+1, y-2, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*23/24 || c == (-curBombTime)*11/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+0, y-2, 0, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*22/24 || c == (-curBombTime)*10/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x-1, y-2, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*21/24 || c == (-curBombTime)*9/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x-2, y-1, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*20/24 || c == (-curBombTime)*8/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x-2, y+0, 0, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*19/24 || c == (-curBombTime)*7/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x-2, y+1, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*18/24 || c == (-curBombTime)*6/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x-1, y+2, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*17/24 || c == (-curBombTime)*5/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+0, y+2, 0, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*16/24 || c == (-curBombTime)*4/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+1, y+2, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*15/24 || c == (-curBombTime)*3/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+2, y+1, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*14/24 || c == (-curBombTime)*2/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+2, y+0, 0, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*13/24 || c == (-curBombTime)*1/24)
/* Added by x-bresse on 06.04.2000 */         SpreadExplosion (x+2, y-1, r, BMTblastnow,0);
/* Added by x-bresse on 06.04.2000 */     
/* Added by x-bresse on 06.04.2000 */     if (c == (-curBombTime)*1/24) {
/* Added by x-bresse on 06.04.2000 */     /*    SpreadExplosion (x, y, 0, BMTblastnow,0);*/
/* Added by x-bresse on 06.04.2000 */     DelExplosion(ptr);
    return XBFalse;
/* Added by x-bresse on 06.04.2000 */     }
/* Added by x-bresse on 06.04.2000 */     return XBTrue;
/* Added by x-bresse on 06.04.2000 */ }
/*  321          */
/* 4   C   --> x */
/* 5 P B   |     */
/* 6   A  \/ y   */
/*  789          */

/* Added by x-bresse on 10.04.2000 START HERE*/
/* Mine */
/* this bomb only explodes when a player is near  321 */
/*         (5 is also bomb position)              654 */
/*                                                987 */
static XBBool
SpecialBombMine (Explosion *ptr)
{
  int x,y,c,r,player,gridx,gridy,goboom;
  x = ptr->x;
  y = ptr->y;
  c = ptr->count;
  r = ptr->range;
  goboom = 0;

  /* mine is inactive during first 2/3 of normal bomb time */
  if (c >= (-curBombTime)*1/3) {
    /* if a player is near : go BOOM ! */ /* should be < num_player, but I can't get it ?? */
    for (player = 0; player < MAX_PLAYER; player ++) {
      gridx = (player_stat[player].x + (BLOCK_WIDTH>>1))/BLOCK_WIDTH;
      gridy = (player_stat[player].y + (BLOCK_HEIGHT>>1))/BLOCK_HEIGHT +1;

      if ( player_stat[player].lives && (
	  (x+1==gridx && y-1==gridy) || (x==gridx && y-1==gridy) || (x-1==gridx && y-1==gridy) ||
	  (x+1==gridx && y==gridy)   || (x==gridx && y==gridy)   || (x-1==gridx && y==gridy) ||
	  (x+1==gridx && y+1==gridy) || (x==gridx && y+1==gridy) || (x-1==gridx && y+1==gridy)) ) {
	goboom++;
      }
    }
  }
  /* bomb doesn't explode (comme une mouillee en gros) */
  if (c == -2 && goboom <= 0) { ptr->count = -3; }

  if (goboom > 0) {
    DelExplosion(ptr);
    SpreadExplosion (x, y, r, BMTblastnow,0);
    if (r > 2) {
      SpreadExplosion (x-1, y-1, 0, BMTblastnow,0); 
      SpreadExplosion (x-1, y+1, 0, BMTblastnow,0); 
      SpreadExplosion (x+1, y+1, 0, BMTblastnow,0); 
      SpreadExplosion (x+1, y-1, 0, BMTblastnow,0); 
    }
    return XBFalse;
  }

  //ptr->type = BMTnormal;

  return XBTrue;
}

 /* Written by Amilhastre */ /* protectbombs */
 /* Written by Amilhastre */ static XBBool
 /* Written by Amilhastre */ SpecialBombProtectbombs (Explosion *ptr)
 /* Written by Amilhastre */ {
 /* Written by Amilhastre */     int x, y; x = ptr->x; 
 /* Written by Amilhastre */     y = ptr->y; 
 /* Written by Amilhastre */     DelExplosion(ptr); 
 /* Written by Amilhastre */ 
 /* Written by Amilhastre */     if (x > 0 
				     && CheckMazeFree(x-1,y) 
				     // && CheckPlayerNear(x-1,y)==1
				     ) {
   /* Written by Amilhastre */       SetMazeBlock(x-1,y,BTExtra); 
 /* Written by Amilhastre */       SetBlockExtra(x-1,y,BTFree);
 /* Written by Amilhastre */     }
 /* Written by Amilhastre */     if (x < MAZE_W-1 
				     //&& CheckPlayerNear(x+1,y) == 1 
				     && CheckMazeFree(x+1,y)) {
   /* Written by Amilhastre */      SetMazeBlock(x+1,y,BTExtra); 
 /* Written by Amilhastre */       SetBlockExtra(x+1,y,BTFree); 
 /* Written by Amilhastre */     }
 /* Written by Amilhastre */     if (y > 0 
				     //&& CheckPlayerNear(x,y-1) == 1 
				     && CheckMazeFree(x,y-1)) {
   /* Written by Amilhastre */       SetMazeBlock(x,y-1,BTExtra);  
 /* Written by Amilhastre */       SetBlockExtra(x,y-1,BTFree); 
 /* Written by Amilhastre */     }
 /* Written by Amilhastre */     if (y < MAZE_H-1 
				     //&& CheckPlayerNear(x,y+1) == 1 
				     && CheckMazeFree(x,y+1)) {
   /* Written by Amilhastre */     SetMazeBlock(x,y+1,BTExtra); 
 /* Written by Amilhastre */       SetBlockExtra(x,y+1,BTFree); 
 /* Written by Amilhastre */     }
 /* Written by Amilhastre */     
 /* Written by Amilhastre */     ptr->type = BMTnormal;
 /* Written by Amilhastre */   return XBFalse;
 /* Written by Amilhastre */   }

/* bomb_row */
static XBBool SpecialBombRow (Explosion *ptr)
{
  if (ptr->player->d_ist == GoUp || ptr->player->d_ist == GoDown) {
    if (ptr->count == -curBombTime) {
      SpreadExplosion(ptr->x,ptr->y-3,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-5,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-7,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-9,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-11,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-13,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+3,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+5,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+7,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+9,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+11,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+13,ptr->range,BMTnormal,0);
    }
  } 
  if (ptr->player->d_ist == GoLeft || ptr->player->d_ist == GoRight) {
    if (ptr->count == -curBombTime) {
      SpreadExplosion(ptr->x-3,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-5,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-7,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-9,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-11,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-13,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+3,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+5,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+7,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+9,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+11,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+13,ptr->y,ptr->range,BMTnormal,0);
    }
  }
  ptr->type = BMTnormal;

  return XBTrue;
}

/* bomb_column */
static XBBool SpecialBombColumn (Explosion *ptr)
{
  if (ptr->player->d_ist == GoUp || ptr->player->d_ist == GoDown) {
    if (ptr->count == -curBombTime) {
      SpreadExplosion(ptr->x-3,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-5,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-7,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-9,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-11,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x-13,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+3,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+5,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+7,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+9,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+11,ptr->y,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x+13,ptr->y,ptr->range,BMTnormal,0);
        }
  }
  if (ptr->player->d_ist == GoLeft || ptr->player->d_ist == GoRight) {
    if (ptr->count == -curBombTime) {
      SpreadExplosion(ptr->x,ptr->y-3,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-5,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-7,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-9,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-11,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y-13,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+3,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+5,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+7,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+9,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+11,ptr->range,BMTnormal,0);
      SpreadExplosion(ptr->x,ptr->y+13,ptr->range,BMTnormal,0);
    }
  } 
  ptr->type = BMTnormal;

  return XBTrue;
}

/* bomb_psycho */
static XBBool SpecialBombPsycho (Explosion *ptr)
{
  if (ptr->count == -curBombTime) {
    SpreadExplosion(ptr->x-3,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x-5,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+3,ptr->y,ptr->range,BMTnormal,0);
    SpreadExplosion(ptr->x+5,ptr->y,ptr->range,BMTnormal,0);
  }
  ptr->type = BMTnormal;

  return XBTrue;
}
/* Added by x-bresse on 23.05.2000 ENDS HERE */

/* bomb_ChangeDirectionAtHalf */
static XBBool SpecialBombChangeDirectionAtHalf(Explosion *ptr)
{ 
  if ((int)(GAME_TIME)%2==0) {
    if(initialBombDir==GoStop){
      initialBombDir=GoDown;
    }
  
    if(initialBombDir==GoUp) {
      initialBombDir=GoDown;
    } else {
      if(initialBombDir==GoLeft) {
	initialBombDir=GoRight; 
      } else {
	if(initialBombDir==GoDown) {
	  initialBombDir=GoUp;
	} else {
	  initialBombDir=GoLeft;
	}
      }
    }
    /*fprintf(stderr," bombdir %i %i\n",initialBombDir,ptr->dir);*/
  }
  return XBTrue;
}
/* Added by Skywalker ENDS HERE */

/*
 * end of file bomb.c
 */



