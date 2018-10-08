/* bomb.h - bombs and explosions
 *
 * $Id: bomb.h,v 1.10 2006/02/09 21:21:22 fzago Exp $
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

#ifndef XBLAST_BOMB_H
#define XBLAST_BOMB_H

/*
 * typedef definitions
 */

/* fuse times */
#define BOMB_TIME   64
#define SHORT_FUSE  32
#define LONG_FUSE  128

#define BOMB_VX        (2*BASE_X)
#define BOMB_VY        (2*BASE_Y)

#define BOMB_STUN_X    (4*BASE_X)
#define BOMB_STUN_Y    (4*BASE_Y)

/* bomb sprites */
typedef enum
{
	BB_NORMAL = 0,
	BB_MINI
} BMBombSprite;
/* bomb types */
typedef enum
{
	BMTevil = -3,				/* hidden bomb */
	BMTspecial,					/* bomb dropped with special key */
	BMTdefault,					/* bomb dropped with bomb key */
	BMTnormal,					/* standard bomb */
	BMTnapalm,					/* napalm bomb, area effect */
	BMTblastnow,				/* instant explosion */
	BMTclose,					/* bombs used for "nasty walls" */
	BMTfirecracker,				/* firecracker */
	BMTfirecracker2,			/* bombs created by firecracker explosion */
	BMTconstruction,			/* create a blastable block on explosion */
	BMTthreebombs,				/* three bombs in a row */
	BMTgrenade,					/* grenade, another area effect */
	BMTtrianglebombs,			/* three bombs forming a triangle */
	BMTdestruction,				/* this bombs destroys walls */
	BMTfungus,					/* this bomb mulitplies */
	BMTrenovation,				/* this bomb move blocks */
	BMTpyro,					/* pyro bomb */
	BMTpyro2,					/* bombs created by pyro explosion */
	BMTrandom,					/* random bomb dtermined after drop */
	BMTshort,					/* bomb with very short fuse */
	BMTdiagthreebombs,			/* added by stn */
	BMTscissor,					/* added by stn */
	BMTscissor2,				/* added by stn */
	BMTparallel,				/* added by stn */
	BMTdistance,				/* added by stn */
	BMTlucky,					/* added by stn */
	BMTparasol,					/* added by stn */
	BMTcomb,					/* added by stn */
	BMTfarpyro,					/* added by stn */
	BMTnuclear,					/* added by stn */
	BMTprotectbombs,
	BMTringofire,
	BMTmine,
	BMTrow,
	BMTcolumn,
	BMTpsycho,
	BMTsearch,
	BMTchangedirectionathalf,	/* Skywalker */
	BMTsnipe,					/* Skywalker */
	NUM_BMT
} BMBombType;
extern char *bomb_name_choice[];
extern int ChoiceDefaultBomb;
/* burn out flags */
typedef enum
{
	BO_RIGHT = (1 << 0),
	BO_LEFT = (1 << 1),
	BO_UP = (1 << 2),
	BO_DOWN = (1 << 3),
	BO_TOTAL = (1 << 4) - 1
} BMBurnOut;

/* bomb and explosion */
typedef struct explosion
{
	BMPlayer *player;			/* from which player */
	int range;					/* blast range */
	int x, y;					/* grid position */
	int dx, dy;					/* small deviations from x and y (in pixel) */
	BMDirection dir;			/* Direction */
	XBBool malfunction;			/* flag for malfunction illness */
	int count;					/* time counter */
	int countslower;
	int countslower2;
	int blink;					/* when is bomb due to blick */
	BMBombType type;			/* what type of bomb do we have */
	int typeExtr;				/* extra information for type */
	BMBurnOut burnOut;			/* has bomb already burned out */
	Sprite *sprite;				/* sprite showing bomb */
	int anime;					/* current animation phase */
	int nextAnime;				/* at which count is next animation due */
	XBBool isMorphed;			/* player has morphed into this bomb */
  /** Skywalker **/
	int isSniping;
	int stop;
	/* Added by Fouf on 09/02/99 22:46:25 *//* Added by "Belgium Guys" */
	int jump;
  /** **/
	struct explosion *next;		/* pointer to next bomb in list */
} Explosion;
/*
 * prototypes
 */

/* pointer to function when bomb hits player, wall or other bomb */
typedef void (*XBBombClickFunc) (Explosion *);

extern XBBool ParseLevelBombs (const DBSection * section, DBSection * warn);
extern void ConfigLevelBombs (const DBSection * section);
extern void DeleteAllExplosions (void);
//extern void DoBombs (void);  old
extern void DoBombs (BMPlayer * ps, int numPlayer);
																				 /* Added by Fouf on 09/02/99 22:46:25 *//* Added by "Belgium Guys" */
extern void DoJump (BMPlayer * ps);
extern int IgnitePlayersBombs (BMPlayer * ps);
extern int IgniteAllBombs (BMPlayer * ps);
extern void IgniteBombs (void);
extern void DoExplosions (void);
extern XBBool NewPlayerBomb (BMPlayer * ps, BMBombType type);
extern XBBool NewEvilBomb (int x, int y);
extern void StunPlayers (BMPlayer * ps, int numPlayer);
extern XBBool CheckBomb (int x, int y);
extern int NumberOfExplosions (void);
extern void DeleteBombAt (int x, int y);
extern void MoveBomb (int x, int y, int dir);
extern int CheckDistribExpl (unsigned *distExtra, int freeBlocks);
extern void HauntKick (int prob);
extern void DoAir (BMPlayer * ps);
extern void DoFrog (BMPlayer * ps);
extern void DoSuck (BMPlayer * ps);

extern void DoNastyWalls (int gameTime);
extern XBBombClickFunc doWallClick;
extern XBBombClickFunc doBombClick;
extern XBBombClickFunc doPlayerClick;
extern void BombClickNone (Explosion * bomb);
extern int initialBombDir;
extern void BombClickInitial (Explosion * bomb);
extern void BombClickThru (Explosion * bomb);
extern void BombClickSnooker (Explosion * bomb);
extern void BombClickContact (Explosion * bomb);
extern void BombClickClockwise (Explosion * bomb);
extern void BombClickAnticlockwise (Explosion * bomb);
extern void BombClickRandomdir (Explosion * bomb);
extern void BombClickRebound (Explosion * bomb);
extern const char *GetBombName (BMBombType type);
extern int StopPlayersBombs (BMPlayer * ps);
extern void SetSlowMotionBurst (int flame);

extern Explosion *exploList;
extern int curBombTime;
extern int defaultBMT;
#endif
/*
 * end of file bomb.h
 */
