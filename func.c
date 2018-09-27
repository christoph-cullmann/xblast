/*
 * func.c - function pointers for special extras
 *
 * $Id: func.c,v 1.13 2005/01/11 22:44:41 lodott Exp $
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
#include "func.h"

#include "atom.h"
#include "bomb.h"
#include "geom.h"
#include "info.h"
#include "snd.h"
#include "random.h"

#include "status.h"


/*
 * global variables
 */
void (*specialExtraFunc) (BMPlayer *);
void (*specialKeyFunc) (BMPlayer *);

/*
 * local types
 */
typedef void (*SpecialExtraFunc) (BMPlayer *);
typedef void (*SpecialKeyFunc) (BMPlayer *);
static char* seVoidName = "No Special Extra";
static char* skVoidName = "No Special Key";

/* 
 * void functions 
 */
void 
SpecialExtraVoid (BMPlayer *ps)
{
}

void 
SpecialKeyVoid (BMPlayer *ps)
{
}

/* 
 * Invincible 
 */
static void 
SpecialExtraInvincible (BMPlayer *ps)
{
  SND_Play (SND_INVINC, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->illtime = 0;
  ps->illness = ps->health;
  ps->invincible += EXTRA_INVINCIBLE;
} /* SpecialExtraInvincible */

/* 
 * Kicking 
 */
static void 
SpecialExtraKick (BMPlayer *ps)
{
  SND_Play (SND_NEWKICK, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->kick = XBTrue;
} /* SpecialExtraKick */

/* 
 * Remote Control 
 */
static void 
SpecialExtraRC (BMPlayer *ps)
{
  SND_Play (SND_NEWRC, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->remote_control = XBTrue;
} /* SpecialExtraRC */

static void 
SpecialKeyRC (BMPlayer *ps)
{
  if (ps->remote_control > 0) {
    if (IgnitePlayersBombs (ps)) {
      SND_Play (SND_SHOOT, ps->x / (PIXW / MAX_SOUND_POSITION));
    }
  }
} /* SpecialKeyRC */


/*
 * bombs choice extra 
 */
static  void
SpecialExtraChoice (BMPlayer *ps)
{  
  char tutu[40];
  if (ps->choice_bomb_type == NUM_BMT){   
    int i;
    for(i = ChoiceDefaultBomb;bomb_name_choice[i]==NULL;i=((++i) % NUM_BMT)) ;
    ps->choice_bomb_type = i;
    if(ps->local){
      sprintf(tutu,"%s : ", p_string[ps->id].name );
      strcat(tutu,bomb_name_choice[(ps->choice_bomb_type)]); 
      SetMessage(tutu,XBTrue);
    }
  }
}


  
static void
SpecialKeyChoice (BMPlayer *ps)
{

  if(ps->choice_bomb_type != NUM_BMT)
    { 
      char tutu[40]; int i;
      i = ps->choice_bomb_type;
      for(i=((++i) % NUM_BMT);bomb_name_choice[i]==NULL;i=((++i) % NUM_BMT)) ;
      ps->choice_bomb_type = i;
      if(ps->local){
	sprintf(tutu,"%s : ", p_string[ps->id].name );
	strcat(tutu,bomb_name_choice[(ps->choice_bomb_type)]);
	SetMessage(tutu,XBTrue);
  	
      }
    }
}
/* Written by VVL */ 
static void
SpecialKeyThrough (BMPlayer *ps)
{
  if ((ps->throughCount > 0) && (ps->lives > 0)) {
    if( ps->through){
      ps->through=0;
    }
    else{
    ps->through=EXTRA_THROUGH_TIME;
    ps->throughCount--;
    }
  }
}
/**/
/* Walk through bomb Mode */
/**/
static void
SpecialExtraThrough (BMPlayer *ps)
{
  ps->throughCount+=1;
}
  

/* 
 * Teleport 
 */
static void 
SpecialExtraTeleport (BMPlayer *ps)
{
  SND_Play (SND_NEWTELE, ps->x / (PIXW / MAX_SOUND_POSITION));
  if (ps->teleport == 0) {
    ps->teleport = 1;
  }
} /* SpecialExtraTeleport */

static void 
SpecialKeyTeleport (BMPlayer *ps)
{
  if (ps->teleport == 1) {
    ps->teleport = TELEPORT_TIME;
  }
} /* SpecialKeyTeleport */

/* 
 * Frogger  extra (galatius(skywalker))
 */
static void 
SpecialExtraFrogger (BMPlayer *ps)
{
  // SND_Play (SND_NEWTELE, ps->x / (PIXW / MAX_SOUND_POSITION));

  ps->frogger ++;
} /* SpecialExtraFrogger */

static void 
SpecialKeyFrogger (BMPlayer *ps)
{
  if ((ps->lives > 0) && (ps->frogger) && (!ps->dying)) 
    {
      /* do_bell(); */
      DoFrog(ps);
    }
} /* SpecialKeyFrogger */

/* 
 * Daleif Illness  (galatius(skywalker))
 * Note that picking up this extra gets you *rid* of the daleif illness *
 */
static void 
SpecialExtraDaleif (BMPlayer *ps)
{
  ps->daleif=0;
} /* SpecialExtraDalif */

/* 
 * Extra Ignite All 
 */
static void 
SpecialExtraIgniteAll (BMPlayer *ps)
{
  SND_Play (SND_BUTT, ps->x / (PIXW / MAX_SOUND_POSITION));
  IgniteAllBombs (ps);
} /* SpecialExtraIgniteAll */

static void 
SpecialKeySpecialBomb (BMPlayer *ps)
{
  DropBomb (ps, BMTspecial);
} /* SpecialKeySpecialBomb */

static void 
SpecialExtraSpecialBomb (BMPlayer *ps)
{
  SND_Play (SND_SPBOMB, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->special_bombs += 3;
} /* SpecialextraSpecialBomb */

/*
 * Junkie (Garth again)
 */
void 
SpecialExtraJunkie (BMPlayer *ps)
{
  SND_Play (SND_INJ, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->junkie = MAX_JUNKIE_TIME;
} /* SpecialExtraJunkie */

/* 
 * Air Pump (Garth Denley) 
 */
static void 
SpecialExtraAir (BMPlayer *ps)
{
  SND_Play (SND_NEWPUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->air_button = XBTrue;
} /* SpecialExtraAir */

static void 
SpecialKeyAir (BMPlayer *ps)
{
  if ((ps->air_button > 0) && (ps->lives > 0)) {
    SND_Play (SND_PUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
    DoAir (ps);
  }
} /* SpecialKeyAir */

/* 
 * Sucker (Stephan Natschlaeger)
 */
static void 
SpecialExtraSuck (BMPlayer *ps)
{
  SND_Play (SND_NEWPUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->suck_button = XBTrue;
} /* SpecialExtraSuck */

/* 
 * Ghost (Belgium Guys)
 */
void 
SpecialExtraGhost (BMPlayer *ps)
{
  SND_Play (SND_NEWPUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->ghost=EXTRA_GHOST_TIME;
  
} /* SpecialExtraGhost */

static void 
SpecialKeySuck (BMPlayer *ps)
{
  if ((ps->suck_button > 0) && (ps->lives > 0)) {
    SND_Play (SND_PUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
    DoSuck (ps);
  }
} /* SpecialKeySuck */

/*
 * poison extra
 */
void
SpecialExtraPoison (BMPlayer *ps)
{
  if (! ps->invincible) {
    ps->dying = DEAD_TIME;
  }
} /* SpecialExtraPoison */

/*
 * spinner extra (long stunned )
 */
static void 
SpecialExtraLongStunned (BMPlayer *ps)
{
  SND_Play (SND_STUN, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->stunned = STUN_TIME*4;
} /* SpecialExtraLongStunned */

/*
 * speed extra
 */
static void
SpecialExtraSpeed (BMPlayer *ps)
{
  SND_Play (SND_FAST, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->illness = IllRun;
  ps->health  = IllRun;
  ps->illtime = 0;
} /* SpecialExtraSpeed */
/*
 * speed extra
 */
static void
SpecialExtraSpeed2 (BMPlayer *ps)
{
  SND_Play (SND_FAST, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->illness = IllRun;
  ps->health  = IllRun;
  ps->illtime = 0;
  ps->speed++;
} /* SpecialExtraSpeed */

/*
 * slow extra
 */
void
SpecialExtraSlow (BMPlayer *ps)
{
  SND_Play (SND_SLOW, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->illness = IllSlow;
  ps->health  = IllSlow;
  ps->illtime = 0;
} /* SpecialExtraSlow */

/*
 * mayhem extra
 */
static void
SpecialExtraMayhem (BMPlayer *ps)
{
  SND_Play (SND_SLAY, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->kick    = XBTrue;
  ps->illness = IllRun;
  ps->health  = IllRun;
  ps->illtime = 0;
} /* SpecialExtraMayhem */

/*
 * holy grail extra
 */
static void
SpecialExtraHolyGrail (BMPlayer *ps)
{
  SND_Play (SND_HOLY, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->lives += KillOtherPlayers (ps->team);
  if (ps->lives > 9) {
    ps->lives = 9;
  }
  ps->invincible = NEW_INVINCIBLE;
  ps->dying      = DEAD_TIME;
} /* SpecialExtraHolyGrail */


/* Added by "Belgium Guys" */ 
/* steal extra */
/**/
static void
SpecialExtraSteal (BMPlayer *ps)
{
  
  ps->bombs += StealBombsOtherPlayers(ps->team);
  ps->range += StealRangeOtherPlayers(ps->team);
}
/*
 * multiple extra 
 */
static void
SpecialExtraLife (BMPlayer *ps)
{
  SND_Play (SND_LIFE, ps->x / (PIXW / MAX_SOUND_POSITION));
  if (ps->lives < 9) {
    ps->lives ++;
  }
} /* SpecialExtraLife */


/*
 * multiple extra 
 */
static void
SpecialExtraMultiple (BMPlayer *ps)
{
  switch (GameRandomNumber (11)) {
  case 0:
  case 1:
    SpecialExtraSpeed (ps);
    break;
  case 2:
  case 3:
    SpecialExtraPoison (ps);
    break;
  case 4:
  case 5:
    SpecialExtraInvincible (ps);
    break;
  case 6:
  case 7:
    SpecialExtraLongStunned (ps);
    break;
  case 8:
  case 9:
    SpecialExtraAir (ps);
    break;
  case 10:
    SpecialExtraLife (ps);
    break;
  }
} /* SpecialExtraMultiple */


/*Skywalker epfl */

/* Written by VVL */
/* Electrify */
void
SpecialExtraElectrify (BMPlayer *ps)
{
  ps->electrify=EXTRA_ELECTRIFY_COUNT;
}
void
SpecialKeyElectrify (BMPlayer *ps)
{
  if ((ps->lives > 0) && (ps->electrify)) {
    if (ElectrifyOtherPlayers(ps->id)) {
      /* Success ... another player is hit! :-) */
      ps->electrify=0;
    } else {
      /* Failure ... just try again! :-( */
      ps->electrify--;
    }
  }
}

static void
SpecialExtraFarter (BMPlayer *ps)
{
  ps->farter=1;
}

static void
SpecialKeyFarter (BMPlayer *ps)
{
  if(ps!=NULL){
    if ((ps->lives > 0) && (ps->farter)) 
      {
	/*     do_bell();*/
	FartOnOtherPlayers(ps);
      }
  }
} 

/* Fart on players and bombs (galatius) */

static void
SpecialExtraBfarter (BMPlayer *ps)
{
  if(ps!=NULL){
    ps->bfarter=1;
  }
}


/* Fart on players and bombs (galatius) */
static void
SpecialKeyBfarter (BMPlayer *ps)
{
  if ((ps->lives > 0) && (ps->bfarter)&& ps!=NULL) 
    {
      /*     do_bell();*/
      FartOnOtherPlayers(ps);
      DoAir(ps);     
    }
} 

static void
SpecialExtraSwapColor (BMPlayer *ps)
{
  SwapColorOtherPlayers(ps->team);
}

static void
SpecialExtraSwapPosition (BMPlayer *ps)

{
  SwapPositionOtherPlayers(ps->team);
}


/* */

/*
 * cloaking device
 */
static void
SpecialExtraCloak (BMPlayer *ps)
{
  int w=1;
  if (ps->cloaking < 0) {
    ps->cloaking *= -1;
    w=-1;
  }
  ps->cloaking += 2*EXTRA_INVINCIBLE;
  ps->cloaking *=w;
  SND_Play (SND_NEWCLOAK, ps->x / (PIXW / MAX_SOUND_POSITION));
} /* SpecialExtraCloak */

static void
SpecialKeyCloak (BMPlayer *ps)
{
  ps->cloaking *= -1;
  if (ps->cloaking > 0) {
    SND_Play (SND_DECLOAK, ps->x / (PIXW / MAX_SOUND_POSITION));
  } else if (ps->cloaking < 0) {
    SND_Play (SND_ENCLOAK, ps->x / (PIXW / MAX_SOUND_POSITION));
  }
} /* SpecialKeyCloak */

/*
 * pow extra (stun others)
 */
void 
SpecialExtraStunOthers (BMPlayer *ps)
{
  StunOtherPlayers (ps->team, STUN_TIME*4);
}

/*
 * morphing extra (player becomes a bomb)
 */
static void
SpecialExtraMorph (BMPlayer *ps)
{
  ps->num_morph ++;
}

static void
SpecialKeyMorph (BMPlayer *ps)
{
  if ( (0 < ps->num_morph) &&
       (0 < ps->bombs) &&
       (! ps->morphed) ) {
#ifdef DEBUG
    fprintf (stderr, "Player %d morphs", ps->id);
#endif
    ps->morphed = 1;
    DropBomb (ps, BMTdefault);
  }
} /* SpecialKeyMorph */

void 
SpecialExtraStop (BMPlayer *ps)
{

  ps->stop = XBTrue;
}

void 
SpecialKeyStop (BMPlayer *ps)
{
  if (ps->stop > 0) {

    StopPlayersBombs(ps);
  }
}

/* Written by Amilhastre */
static   void
SpecialExtraEvilGrail (BMPlayer *ps)
{
  ps->evilill = ILLDEATHTIME;
}

/* phantom extra */
/* Written by Amilhastre */
static void
SpecialExtraPhantom (BMPlayer *ps)
{
  if (!ps->phantom)
         ps->phantom = 3*EXTRA_INVISIBLE;
  else
    ps->phantom += EXTRA_INVISIBLE;
}
 
/* revive extra */
/* Written by Amilhastre */
static void
SpecialExtraRevive (BMPlayer *ps)
{
  ps->revive++;
}

/* ** Skywalker **
 * sniping extra (player moves the bomb)
 */
static void
SpecialExtraSnipe (BMPlayer *ps)
{
  ps->num_snipe ++;
}

static void
SpecialKeySnipe (BMPlayer *ps)
{
  if ( (0 < ps->num_snipe) &&
       (0 < ps->bombs) &&
       ( ps->sniping==0) ) {

    ps->sniping = 2;
    DropBomb(ps, BMTdefault);
    ps->sniping = 1;
  } else {
    if(ps->sniping==1){
      if (IgnitePlayersBombs(ps)) {
	ps->sniping=1;
	ps->d_soll = GoStop;
      }
    }
  }
  
} /* SpecialKeySnipe */

/* EPFL */


void SpecialExtraJump (BMPlayer *ps) {
  SND_Play(SND_NEWPUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
  ps->jump_button += 1;
}

void SpecialKeyJump (BMPlayer *ps) {
  if ((ps->jump_button > 0) && (ps->lives > 0)) {
    SND_Play(SND_PUMP, ps->x / (PIXW / MAX_SOUND_POSITION));
    DoJump(ps);
  }
}

/* END EPFL */

/*
 * conversion tables
 */
/* extra pickup functions */
static DBToData extraTable[] = {
  { "air",                (void *) SpecialExtraAir },         
  { "bfarter",            (void *) SpecialExtraBfarter},        /* skywalker */  
  { "choicebombtype",     (void *) SpecialExtraChoice },        /* skywalker */  
  { "cloak",              (void *) SpecialExtraCloak }, 
  { "daleif",             (void *) SpecialExtraDaleif }, 
  { "electrify",          (void *) SpecialExtraElectrify },
  { "evilgrail",          (void *) SpecialExtraEvilGrail},
  { "farter",             (void *) SpecialExtraFarter},        /* skywalker */     
  { "frogger",            (void *) SpecialExtraFrogger },      /* skywalker */     
  { "ghost",              (void *) SpecialExtraGhost },      /* skywalker */     
  { "holyGrail",          (void *) SpecialExtraHolyGrail },   
  { "igniteAll",          (void *) SpecialExtraIgniteAll },   
  { "invincible",         (void *) SpecialExtraInvincible },
  { "jump",               (void *) SpecialExtraJump },          /* skywalker */ 
  { "junkie",             (void *) SpecialExtraJunkie },      
  { "kick",               (void *) SpecialExtraKick },        
  { "life",               (void *) SpecialExtraLife },        
  { "longStunned",        (void *) SpecialExtraLongStunned }, 
  { "mayhem",             (void *) SpecialExtraMayhem },      
  { "morph",              (void *) SpecialExtraMorph },   
  { "multiple",           (void *) SpecialExtraMultiple },     
  { "phantom",            (void *) SpecialExtraPhantom },     
  { "poison",             (void *) SpecialExtraPoison },      
  { "rc",                 (void *) SpecialExtraRC },          
  { "revive",             (void *) SpecialExtraRevive },          
  { "slow",               (void *) SpecialExtraSlow },        
  { "snipe",              (void *) SpecialExtraSnipe },          /* skywalker */
  { "specialBomb",        (void *) SpecialExtraSpecialBomb }, 
  { "speed",              (void *) SpecialExtraSpeed },     
  { "speed2",             (void *) SpecialExtraSpeed2 },    
  { "steal",              (void *) SpecialExtraSteal },      
  { "stop",               (void *) SpecialExtraStop}, 
  { "stunOthers",         (void *) SpecialExtraStunOthers },  
  { "sucker",             (void *) SpecialExtraSuck }, 
  { "swapcolor",          (void *) SpecialExtraSwapColor },    /* skywalker */   
  { "swapposition",       (void *) SpecialExtraSwapPosition },      /* skywalker */
  { "teleport",           (void *) SpecialExtraTeleport }, 
  { "through",            (void *) SpecialExtraThrough },    
  { "void",	          (void *) SpecialExtraVoid },    
  { NULL, NULL }
};
/* extra key functions */
static DBToData keyTable[] = {
  { "air",	          (void *) SpecialKeyAir },       
  { "bfarter",            (void *) SpecialKeyBfarter},
  { "choicebombtype",     (void *) SpecialKeyChoice },        /* skywalker */   
  { "cloak",	          (void *) SpecialKeyCloak },  
  { "electrify",          (void *) SpecialKeyElectrify },
  { "farter",             (void *) SpecialKeyFarter},        /* skywalker */   
  { "frogger",            (void *) SpecialKeyFrogger },          /* skywalker */
  { "jump",               (void *) SpecialKeyJump },          /* skywalker */
  { "morph",	          (void *) SpecialKeyMorph },       
  { "rc",                 (void *) SpecialKeyRC },          
  { "snipe",	          (void *) SpecialKeySnipe },         /* skywalker */
  { "specialBomb",        (void *) SpecialKeySpecialBomb }, 
  { "stop",               (void *) SpecialKeyStop},
  { "sucker",             (void *) SpecialKeySuck }, 
  { "teleport",	          (void *) SpecialKeyTeleport },
  { "through",	          (void *) SpecialKeyThrough }, 
  { "void",	          (void *) SpecialKeyVoid },    
  { NULL, NULL }
};

/*
 * parse func section of level data
 */
XBBool
ParseLevelFunc (const DBSection *section, DBSection *warn)
{
  void *ptr;
  /* check if section exists */
  if (NULL == section) {
    Dbg_Out("LEVEL: func section is missing!\n");
    DB_CreateEntryString(warn,atomMissing,"true");
    return XBFalse;
  }
  /* Extra has default */
  switch (DB_ConvertEntryData (section, atomExtra, &ptr, extraTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomExtra));
    specialExtraFunc = SpecialExtraVoid;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomExtra));
    specialExtraFunc = SpecialExtraVoid;
    DB_CreateEntryString(warn,atomExtra, DB_DataToString(extraTable, specialExtraFunc));
    break;
  default:
    assert (ptr != NULL);
    specialExtraFunc = (SpecialExtraFunc) ptr;
    break;
  }
  /* Key has default */
  switch (DB_ConvertEntryData (section, atomKey, &ptr, keyTable) ) {
  case DCR_NoSuchEntry:
    Dbg_Level("default for %s\n",  DB_SectionEntryString(section,atomKey));
    specialKeyFunc = SpecialKeyVoid;
    break;
  case DCR_Failure:
    Dbg_Out("LEVEL: warning for %s\n", DB_SectionEntryString(section, atomKey));
    specialKeyFunc = SpecialKeyVoid;
    DB_CreateEntryString(warn,atomExtra, DB_DataToString(keyTable, specialKeyFunc));
    break;
  default:
    assert (ptr != NULL);
    specialKeyFunc = (SpecialKeyFunc) ptr;
    break;
  }
  return XBTrue;
}
/*
 * config section [FUNC]
 */
void
ConfigLevelFunc (const DBSection *section)
{
  /* set extra info */
  if (specialExtraFunc == SpecialExtraAir) {
    AddExtraInfo ("Airpump as an extra");
  } else if (specialExtraFunc == SpecialExtraBfarter) { /* skywalker */
    AddExtraInfo ("Bfarter as an extra");
  } else if (specialExtraFunc == SpecialExtraChoice) { /* skywalker */
    AddExtraInfo ("Choice as an extra");
  } else if (specialExtraFunc == SpecialExtraCloak) {
    AddExtraInfo ("Cloak as an extra");
  } else if (specialExtraFunc == SpecialExtraDaleif) {
    AddExtraInfo ("Daleif as an extra");
  } else if (specialExtraFunc == SpecialExtraElectrify) {
    AddExtraInfo ("Electrify as an extra");
  } else if (specialExtraFunc == SpecialExtraFarter) { /* skywalker */
    AddExtraInfo ("Farter as an extra");
  } else if (specialExtraFunc == SpecialExtraFrogger) { /* skywalker */
    AddExtraInfo ("Frogger as an extra");
  } else if (specialExtraFunc == SpecialExtraGhost) { /* skywalker */
    AddExtraInfo ("Ghost as an extra");
  } else if (specialExtraFunc == SpecialExtraHolyGrail) {
    AddExtraInfo ("The Holy Grail as an extra");
  } else if (specialExtraFunc == SpecialExtraIgniteAll) {
    AddExtraInfo ("Button as an extra");
  } else if (specialExtraFunc == SpecialExtraInvincible) {
    AddExtraInfo ("Invincibility as an extra");
  } else if (specialExtraFunc == SpecialExtraJump) {
    AddExtraInfo ("Jump as an extra");
  } else if (specialExtraFunc == SpecialExtraJunkie) {
    AddExtraInfo ("Junkie virus as an extra");
  } else if (specialExtraFunc == SpecialExtraKick) {
    AddExtraInfo ("Kick as an extra");
  } else if (specialExtraFunc == SpecialExtraLife) {
    AddExtraInfo ("Free life as an extra");
  } else if (specialExtraFunc == SpecialExtraLongStunned) {
    AddExtraInfo ("Spinner as an extra");
  } else if (specialExtraFunc == SpecialExtraMayhem) {
    AddExtraInfo ("Mayhem /Kick & Run) as an extra");
  } else if (specialExtraFunc == SpecialExtraMorph) {
    AddExtraInfo ("Morphing as an extra");
  } else if (specialExtraFunc == SpecialExtraMultiple) {
    AddExtraInfo ("Random special extra");
  } else if (specialExtraFunc == SpecialExtraPhantom) {
    AddExtraInfo ("Phantom as an extra");
  } else if (specialExtraFunc == SpecialExtraPoison) {
    AddExtraInfo ("Poison as an extra");
  } else if (specialExtraFunc == SpecialExtraRC) {
    AddExtraInfo ("Remote control as an extra");
  } else if (specialExtraFunc == SpecialExtraRevive) {
    AddExtraInfo ("Revive control as an extra");
  } else if (specialExtraFunc == SpecialExtraSnipe) { /* skywalker */
    AddExtraInfo ("Snipe as an extra");
  } else if (specialExtraFunc == SpecialExtraStop) { /* skywalker */
    AddExtraInfo ("Stop as an extra");
  } else if (specialExtraFunc == SpecialExtraSuck) { /* skywalker */
    AddExtraInfo ("Sucker as an extra");
  } else if (specialExtraFunc == SpecialExtraSwapColor) { /* skywalker */
    AddExtraInfo ("Swapcolor as an extra");
  } else if (specialExtraFunc == SpecialExtraSwapPosition) { /* skywalker */
    AddExtraInfo ("Swapposition as an extra");
  } else if (specialExtraFunc == SpecialExtraSlow) {
    AddExtraInfo ("Slowdown as an extra");
  } else if (specialExtraFunc == SpecialExtraSpeed) {
    AddExtraInfo ("Speed as an extra");
  } else if (specialExtraFunc == SpecialExtraSpeed2) {
    AddExtraInfo ("SpeedII as an extra");
  } else if (specialExtraFunc == SpecialExtraStop) {
    AddExtraInfo ("Stop as an extra");
  } else if (specialExtraFunc == SpecialExtraStunOthers) {
    AddExtraInfo ("Stunner as an extra");
  } else if (specialExtraFunc == SpecialExtraTeleport) {
    AddExtraInfo ("Teleporter as an extra");
  } else if (specialExtraFunc == SpecialExtraThrough) {
    AddExtraInfo ("Through as an extra");
  }
} /* ConfigLevelFunc */

/*
 *
 */
XBBool
HasSpecialBombs (void)
{
  return (specialExtraFunc == SpecialExtraSpecialBomb);
} /* HasSpecialBombs */
/*
 * return the name of the extra type
 */
const char*
GetExtraNameFunc(void *type) {
  int i;
  if (type ==SpecialExtraVoid )
    return seVoidName;
  for (i = 0;extraTable[i].value!=NULL ; ++i)
    if (extraTable[i].value == type)
      return extraTable[i].key;
  return seVoidName;
}
const char*
GetExtraNameInt(int type) {
  int i;
  if (type ==0 )
    return seVoidName;
  for (i = 0;extraTable[i].value!=NULL ; ++i)
    if (i == type)
      return extraTable[i].key;
  return seVoidName;
}
/*
 * return the name of the key type
 */
const char*
GetKeyNameFunc(void *type) {
  int i;
  if (type ==SpecialKeyVoid )
    return skVoidName;
  for (i = 0;keyTable[i].value!=NULL ; ++i)
    if (keyTable[i].value == type)
      return keyTable[i].key;
  return skVoidName;
}
const char*
GetKeyNameInt(int type) {
  int i;
  if (type ==0 )
    return skVoidName;
  for (i = 0;keyTable[i].value!=NULL ; ++i)
    if (i == type)
      return keyTable[i].key;
  return skVoidName;
}

/*
 * return the number of the key type
 */
int
GetNumberOfKeys( void  ) {
  int i;
  for (i = 0;keyTable[i].value!=NULL ; ++i);
  return i;
}

/*
 * return the number of the extra type
 */
int
GetNumberOfExtras( void ) {
  int i;
  for (i = 0;extraTable[i].value!=NULL ; ++i);
  return i;
}

/*
 * end of file func.c
 */

