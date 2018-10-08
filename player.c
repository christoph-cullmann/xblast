/*
 * file player.c - ingame player mangment 
 *
 * $Id: player.c,v 1.48 2006/06/13 11:11:27 fzago Exp $
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
 * local constants
 */
#define BOMB_STEP 2
#define STEP_HORI BASE_X
#define STEP_VERT BASE_Y

#define MAX_RANGE 10
#define ILLTIME 256
#define ILL_X (4*BASE_X)
#define ILL_Y (5*BASE_Y)

#define JUNKIE_ILL_TIME (ILLTIME)
#define JUNKIE_STUN_TIME 12
#define JUNKIE_TIME_1 360
#define JUNKIE_TIME_2 210
#define JUNKIE_TIME_3 60		/* Speed */

#define NUM_TELE_TRIES 25

/*
 * init flags for extras
 *   LF_ level start flag
 *   RF_ revive flag
 *   IF_ both of the above
 */
#define IF_None     0
/* kick extra */
#define LF_Kick     (1<<29)
#define RF_Kick     (1<<30)
#define IF_Kick     (RF_Kick|LF_Kick)
/* remote control */
#define LF_RC       1
#define RF_RC       2
//#define IF_RC       3
/* teleporter */
#define LF_Teleport 4
#define RF_Teleport 5
//#define IF_Teleport 6
/* airpump */
#define LF_Airpump  7
#define RF_Airpump  8
//#define IF_Airpump  9 
/* cloak extra */
#define LF_Cloak    10
#define RF_Cloak    11
//#define IF_Cloak    12
/* morph extra */
#define LF_Morph    13
#define RF_Morph    14
//#define IF_Morph    15
/** Skywalker **/
#define LF_Snipe    16
#define RF_Snipe    17
//#define IF_Snipe    18
#define LF_Frogger  19
#define RF_Frogger  20
//#define IF_Frogger  21
#define LF_Fart    22
#define RF_Fart    23
//#define IF_Fart    24
#define LF_Bfart   25
#define RF_Bfart   26
//#define IF_Bfart   27
#define LF_Choice  28
#define RF_Choice  29
//#define IF_Choice  30
#define LF_Stop    31
#define RF_Stop    32
//#define IF_Stop    33
#define LF_Phantom 34
#define RF_Phantom 35
//#define IF_Phantom 36   
#define LF_Electrify 37
#define RF_Electrify 38
//#define IF_Electrify 39 

#define LF_Daleif    40
#define RF_Daleif    41
//#define IF_Daleif    42
#define LF_Suck  43
#define RF_Suck  44
//#define IF_Suck  45

/* revive extra */
#define LF_Revive    46
#define RF_Revive   47
//#define IF_Revive    (RF_Revive|LF_Revive)
#define LF_Jump    48
#define RF_Jump   49
//#define IF_Revive    (RF_Revive|LF_Revive)

#define LF_Reverse2    50
#define RF_Reverse2   51
#define LF_Through    52
#define RF_Through   53
/** **/
#define NUM_IF 12

/*
 * global variables
 */
BMPlayer player_stat[2 * MAX_PLAYER];
PlayerStrings p_string[2 * MAX_PLAYER];

/*
 * extern variables
 */
extern void (*special_extra_function) (void);

/*
 * local variables - retrieved from level data
 */
static int minRange;
static int minBombs;
static int recLives;
static int specialBombs;
static int reviveHealth;
static int initHealth;
static unsigned initFlags;
static unsigned revFlags;
static BMPosition pos0[MAX_PLAYER];
static int pos0Shuffle[MAX_PLAYER];
static int pos0Cnt;
static int playersAllowed;

/*
 * local variables - defined by game data
 */
static XBBool randomPlayerPos;
static XBBool ifRecLives;
static int maxLives;
static int numPlayer;

/*
 * local variables - for final setup
 */
static BMPosition pos[MAX_PLAYER];
static unsigned iniplayerflags;
static unsigned revplayerflags;

#if 0
static int game_mode;
#endif

/* conversion tables */
static DBToInt healthTable[] = {
	{"bomb", (int)IllBomb},
	{"empty", (int)IllEmpty},
	{"healthy", (int)Healthy},
	{"invisible", (int)IllInvisible},
	{"malfunction", (int)IllMalfunction},
	{"mini", (int)IllMini},
	{"reverse", (int)IllReverse},
	{"reverse2", (int)IllReverse2},
	{"run", (int)IllRun},
	{"slow", (int)IllSlow},
	{"teleport", (int)IllTeleport},
	{NULL, -1},
};
static DBToInt initFlagsTable[] = {
	{"airpump", LF_Airpump},
	{"bfarter", LF_Bfart},
	{"choicebombtype", LF_Choice},	/* skywalker */
	{"cloak", LF_Cloak},
	{"daleif", LF_Daleif},
	{"electrify", LF_Electrify},
	{"farter", LF_Fart},		/* skywalker */
	{"frogger", LF_Frogger},	/* skywalker */
	{"jump", LF_Jump},			/* skywalker */
	{"kick", LF_Kick},
	{"morph", LF_Morph},
	{"none", 0},
	{"phantom", LF_Phantom},
	{"rc", LF_RC},
	{"revive", LF_Revive},
	{"snipe", LF_Snipe},			/** Skywalker **/
	{"stop", LF_Stop},
	{"sucker", LF_Suck},
	{"teleport", LF_Teleport},
	{"through", LF_Through},
	{NULL, -1},
};
static DBToInt reviveFlagsTable[] = {
	{"airpump", RF_Airpump},
	{"bfarter", RF_Bfart},
	{"choicebombtype", RF_Choice},	/* skywalker */
	{"cloak", RF_Cloak},
	{"daleif", RF_Daleif},
	{"electrify", RF_Electrify},
	{"farter", RF_Fart},		/* skywalker */
	{"frogger", RF_Frogger},	/* skywalker */
	{"jump", RF_Jump},			/* skywalker */
	{"kick", RF_Kick},
	{"morph", RF_Morph},
	{"none", 0},
	{"phantom", RF_Phantom},
	{"rc", RF_RC},
	{"revive", RF_Revive},
	{"snipe", RF_Snipe},		   /** Skywalker **/
	{"stop", RF_Stop},
	{"sucker", RF_Suck},
	{"teleport", RF_Teleport},
	{"through", RF_Through},
	{NULL, -1},
};

/* info text data */
static const char *permHealthInfo[MAX_ILL] = {
	NULL,
	N_("Permanent random bombing"),
	N_("Permanent slowdown"),
	N_("Permanent running"),
	N_("Permanent mini bombs"),
	N_("No bomb while healthy"),
	N_("Permanent invisibility"),
	N_("Permanent malfunctions"),
	N_("Permanent reverse controls"),
	N_("Permanent reverse(2) controls"),
	N_("Permanent random teleporting"),
};
static const char *initHealthInfo[MAX_ILL] = {
	NULL,
	N_("Initial random bombing"),
	N_("Initial slowdown"),
	N_("Initial running"),
	N_("Initial mini bombs"),
	N_("No bomb while healthy"),
	N_("Initial invisibility"),
	N_("Initial malfunctions"),
	N_("Initial reverse controls"),
	N_("Initial reverse(2) controls"),
	N_("Initial random teleporting"),
};
static const char *reviveHealthInfo[MAX_ILL] = {
	NULL,
	N_("Revived with random bombing"),
	N_("Revived with slowdown"),
	N_("Revived with running"),
	N_("Revived with mini bombs"),
	N_("Revived with bombs while healthy"),
	N_("Revived with invisibility"),
	N_("Revived with malfunctions"),
	N_("Revived with reverse controls"),
	N_("Revived with reverse(2) controls"),
	N_("Revived with random teleporting"),
};

/*
 * parse player section in level data
 */
XBBool
ParseLevelPlayers (const DBSection * section, unsigned gameMode, DBSection * warn)
{
	int i, k;
	/* check existence of section */
	if (NULL == section) {
		Dbg_Out ("LEVEL: player section is missing!\n");
		DB_CreateEntryString (warn, atomMissing, "true");
		return XBFalse;
	}
	/* Bombs entry is required */
	if (!DB_GetEntryInt (section, atomBombs, &minBombs)) {
		Dbg_Out ("LEVEL: critical failure, %s\n", DB_SectionEntryString (section, atomBombs));
		DB_CreateEntryString (warn, atomBombs, "missing!");
		return XBFalse;
	}
	/* Range entry is required */
	if (!DB_GetEntryInt (section, atomRange, &minRange)) {
		Dbg_Out ("LEVEL:  critical failure, %s\n", DB_SectionEntryString (section, atomRange));
		DB_CreateEntryString (warn, atomRange, "missing!");
		return XBFalse;
	}
	/* RecLives has default */
	if (!DB_GetEntryInt (section, atomRecLives, &recLives)) {
		Dbg_Level ("default for %s\n", DB_SectionEntryString (section, atomRecLives));
		recLives = 0;
	}
	/* SpecialBombs has default */
	if (!DB_GetEntryInt (section, atomSpecialBombs, &specialBombs)) {
		Dbg_Level ("default for %s\n", DB_SectionEntryString (section, atomSpecialBombs));
		specialBombs = 0;
	}
	/* ReviveVirus has default */
	switch (DB_ConvertEntryInt (section, atomReviveVirus, (int *)&reviveHealth, healthTable)) {
	case DCR_NoSuchEntry:
		Dbg_Level ("default for %s\n", DB_SectionEntryString (section, atomReviveVirus));
		reviveHealth = Healthy;
		break;
	case DCR_Failure:
		Dbg_Out ("LEVEL: warning for %s\n", DB_SectionEntryString (section, atomReviveVirus));
		reviveHealth = Healthy;
		DB_CreateEntryString (warn, atomReviveVirus, DB_IntToString (healthTable, reviveHealth));
		break;
	default:
		break;
	}
	/* InitVirus has default */
	switch (DB_ConvertEntryInt (section, atomInitVirus, (int *)&initHealth, healthTable)) {
	case DCR_NoSuchEntry:
		Dbg_Level ("default for %s\n", DB_SectionEntryString (section, atomInitVirus));
		initHealth = Healthy;
		break;
	case DCR_Failure:
		Dbg_Out ("LEVEL: warning %s\n", DB_SectionEntryString (section, atomInitVirus));
		initHealth = Healthy;
		DB_CreateEntryString (warn, atomInitVirus, DB_IntToString (healthTable, initHealth));
		break;
	default:
		break;
	}
	/* InitExtra has default */
	switch (DB_ConvertEntryFlags (section, atomInitExtra, &initFlags, initFlagsTable)) {
	case DCR_NoSuchEntry:
		Dbg_Level ("default for %s\n", DB_SectionEntryString (section, atomInitExtra));
		initFlags = 0;
		break;
	case DCR_Failure:
		Dbg_Out ("LEVEL: warning for %s\n", DB_SectionEntryString (section, atomInitExtra));
		initFlags = 0;
		DB_CreateEntryString (warn, atomInitExtra, DB_IntToString (initFlagsTable, initFlags));
		break;
	default:
		break;
	}
	/* ReviveExtra has default */
	switch (DB_ConvertEntryFlags (section, atomReviveExtra, &revFlags, reviveFlagsTable)) {
	case DCR_NoSuchEntry:
		Dbg_Level ("default for %s\n", DB_SectionEntryString (section, atomReviveExtra));
		revFlags = 0;
		break;
	case DCR_Failure:
		Dbg_Out ("LEVEL: warning for %s\n", DB_SectionEntryString (section, atomReviveExtra));
		revFlags = 0;
		DB_CreateEntryString (warn, atomReviveExtra, DB_IntToString (reviveFlagsTable, revFlags));
		break;
	default:
		break;
	}
	/* retrieve all player positions, count them */
	k = 0;
	for (i = 0; i < MAX_PLAYER; i++) {
		if (DB_GetEntryPos (section, atomArrayPos0[k + 1], pos0 + k)) {
			if ((pos0[k].x > 0) && (pos0[k].y > 0) &&
				(pos0[k].x <= MAZE_W) && (pos0[k].y <= MAZE_H)) {
				pos0Shuffle[k] = k;
				k++;
			}
		}
	}
	pos0Cnt = k;
	/* check if there are player positions */
	if (pos0Cnt == 0) {
		Dbg_Out ("LEVEL: critical failure, no player positions found!\n");
		DB_CreateEntryString (warn, atomArrayPos0[0], "missing!");
		return XBFalse;
	}
	Dbg_Level ("%u player positions found\n", pos0Cnt);
	/* retrieve number of allowed players from gameMode */
	k = 0;
	for (i = 5; (i > 0) && (k == 0); i--) {
		if ((gameMode & (1 << i)) != 0) {
			k = i + 1;
		}
	}
	playersAllowed = k;
	/* check if players are allowed at all */
	if (playersAllowed == 0) {
		Dbg_Out ("LEVEL: critical failure, no players allowed!\n");
		return XBFalse;
	}
	Dbg_Level ("%u players allowed\n", playersAllowed);
	return XBTrue;
}								/* ParseLevelPlayers */

/*
 * configure players for game
 */
void
ConfigLevelPlayers (const DBSection * section, XBBool allowRandomPos, unsigned gameMode)
{
	BMPlayer *ps;
	int i, j, k, m;
	int pl[MAX_PLAYER];
	int numActive;
	const char *s;
    /* AbsInt start */
    int didchange=0;
    int team_lives[2 * MAX_PLAYER];
    int team_maxlives;
    /* AbsInt end */

	assert (section != NULL);
	revplayerflags = 0;
	iniplayerflags = 0;

	/* min of allowed players and defined positions */
	k = MIN (pos0Cnt, playersAllowed);
	assert (k > 0);
	/* determine number of active players and shuffle list */
	numActive = 0;
	for (i = 0; i < numPlayer; i++) {
		if (!player_stat[i].in_active) {
			pl[numActive] = i;
			numActive++;
		}
	}
	Dbg_Level ("%u of %u players are active, %u defined positions - assigning positions...\n",
			   numActive, numPlayer, pos0Cnt);
	/* shuffle active players */
	for (i = numActive - 1; i > 0; i--) {
		j = GameRandomNumber (i + 1);
		j = (j >= 0) ? j : 0;
		j = (j >= i + 1) ? i : j;
		m = pl[j];
		pl[j] = pl[i];
		pl[i] = m;
	}
	/* shuffle retrieved positions */
	for (i = k - 1; i > 0; i--) {
		j = GameRandomNumber (i + 1);
		j = (j >= 0) ? j : 0;
		j = (j >= i + 1) ? i : j;
		m = pos0Shuffle[j];
		pos0Shuffle[j] = pos0Shuffle[i];
		pos0Shuffle[i] = m;
	}
	/* setup shuffled player positions */
	for (i = 0; i < numActive; i++) {
		if (i < k) {
			pos[pl[i]] = pos0[pos0Shuffle[i]];
			Dbg_Level ("active player %i on defined position %i (%i, %i)\n",
					   pl[i], pos0Shuffle[i], pos[pl[i]].x, pos[pl[i]].y);
		}
		else {
			j = GameRandomNumber (k);
			j = (j >= 0) ? j : 0;
			j = (j >= k) ? k - 1 : j;
			pos[pl[i]] = pos0[j];
			Dbg_Level ("active player %i on random position %i (%i, %i)\n",
					   pl[i], j, pos[pl[i]].x, pos[pl[i]].y);

		}
	}
	/* store positions in player stats */
	j = 0;
	for (i = 0; i < numPlayer; i++) {
		ps = player_stat + i;
		if (ps->in_active) {
			ps->x = 0;
			ps->y = i * BLOCK_HEIGHT;
			SetSpriteMode (ps->sprite, SPM_UNMAPPED);
		}
		else {
			ps->x = pos[i].x * BLOCK_WIDTH;
			ps->y = (pos[i].y - 1) * BLOCK_HEIGHT;

			j++;
		}
	}

    /* AbsInt: count lives per team */
    memset(team_lives, 0, sizeof(team_lives));
    for (i = 0; i < numPlayer; ++i) {
        ps = player_stat+i;
        team_lives[ps->team]+=(ifRecLives && recLives) ? recLives : maxLives;
    }
    team_maxlives=0;
    for (i=0; i<2 * MAX_PLAYER; ++i)
        if (team_lives[i] > team_maxlives)
            team_maxlives = team_lives[i];
    /* AbsInt end */

	/* setup other player attributes */
	for (i = 0; i < numPlayer; i++) {
		ps = player_stat + i;
		/* Added by VVL (Chat) 12/11/99 : Begin */
		ps->chatmode = 0;
		ps->chatstring[0] = '\0';
		ps->chatlen = 0;
		/* Added by VVL (Chat) 12/11/99 : End */
		ps->iniextra_flags = initFlags;
		ps->revextra_flags = revFlags;
		iniplayerflags = (ps->iniextra_flags & ((0xffffff) >> 2));
		revplayerflags = (ps->revextra_flags & ((0xffffff) >> 2));
		ps->kick = (LF_Kick & ps->iniextra_flags) ? XBTrue : XBFalse;
		ps->invincible = NEW_INVINCIBLE;
		ps->illness = initHealth;
		ps->health = initHealth;
        /* AbsInt start */
        ps->ai_revived = 0;
        /* AbsInt end */
		ps->illtime = 0;
		ps->junkie = 0;
		ps->ghost = 0;
		ps->dying = 0;
		ps->stunned = 0;
		if (ifRecLives && recLives) {
			ps->lives = ps->in_active ? 0 : recLives;
		}
		else {
			ps->lives = ps->in_active ? 0 : maxLives;
		}
		ps->range = minRange;
		ps->bombs = minBombs;
		ps->special_bombs = specialBombs;
		ps->jump_button = (LF_Jump == iniplayerflags) ? XBTrue : XBFalse;
		ps->remote_control = (LF_RC == iniplayerflags) ? XBTrue : XBFalse;
		ps->teleport = (LF_Teleport == iniplayerflags) ? XBTrue : XBFalse;
		ps->air_button = (LF_Airpump == iniplayerflags) ? XBTrue : XBFalse;
		ps->cloaking = (LF_Cloak == iniplayerflags) ? -GAME_TIME : 0;
		ps->stop = (LF_Stop == iniplayerflags) ? XBTrue : XBFalse;
		ps->phantom = (LF_Phantom == iniplayerflags) ? GAME_TIME : XBFalse;
		ps->electrify = (LF_Electrify == iniplayerflags) ? EXTRA_ELECTRIFY_COUNT : XBFalse;
		ps->revive = (LF_Revive == iniplayerflags) ? XBTrue : XBFalse;
		ps->suck_button = (LF_Suck == iniplayerflags) ? XBTrue : XBFalse;
		ps->num_extras =
			(LF_Snipe == iniplayerflags) ? 1000 : ((LF_Morph == iniplayerflags) ? 1000 : 0);
		ps->speed = 0;
		ps->daleif = (LF_Daleif == iniplayerflags) ? XBTrue : XBFalse;	/* Daleif illness (galatius) */
		ps->farted = (LF_Fart == iniplayerflags) ? XBTrue : XBFalse;	/* Fart counter (galatius) */
		ps->bfarter = (LF_Bfart == iniplayerflags) ? XBTrue : XBFalse;	/* Fart counter (galatius) */
		ps->num_snipe = (LF_Snipe == iniplayerflags) ? 1000 : 0;
		ps->num_morph = (LF_Morph == iniplayerflags) ? 1000 : 0;
		ps->daleifing = 0;
		ps->abort = ABORT_NONE;
		ps->d_ist = GoStop;
		ps->d_soll = GoStop;
		ps->d_look = GoDown;
		ps->morphed = XBFalse;
		/* Written by VVL */
		ps->through = (LF_Through == iniplayerflags) ? XBTrue : XBFalse;
		ps->throughCount = ps->through ? 255 : 0;
		ps->evilill = 0;
	/** Skywalker **/
		ps->sniping = 0;
		ps->frogger = (LF_Frogger == iniplayerflags) ? XBTrue : XBFalse;
	/** **/
		if (LF_Choice == iniplayerflags) {
			char tutu[40];
			int h;
			for (h = ChoiceDefaultBomb; bomb_name_choice[h] == NULL; h = ((h + 1) % NUM_BMT)) ;

			ps->choice_bomb_type = h;
			if (ps->local) {
				sprintf (tutu, "%s : ", p_string[ps->id].name);
				strcat (tutu, bomb_name_choice[(ps->choice_bomb_type)]);
				SetMessage (tutu, XBTrue);
			}
		}
		else {
			ps->choice_bomb_type = NUM_BMT;
			/* fprintf(stderr," bomb typ1 %i\n", ps->choice_bomb_type); */
		}
	}

    /* AbsInt: distribute bonus lives */
#if 0
    do {
        didchange = 0;
        for (i=0; i<numPlayer; ++i) {
            ps = player_stat+i;
            if (!ps->in_active) {
                int bonus = team_maxlives - team_lives[ps->team];
                if (bonus) {
                    ps->lives++;
                    team_lives[ps->team]++;
                    didchange = 1;
                }
            }
        }
    } while(didchange);
#endif
    /* AbsInt end */

	/* set text for info screen */
	switch (minBombs) {
	case 0:
		AddPlayerInfo (N_("No bomb"));
		break;
	case 1:
		AddPlayerInfo (N_("1 bomb"));
		break;
	default:
                /* TRANSLATORS: %d > 1 (multiple bombs only) */
		AddPlayerInfo (N_("%d bombs"), minBombs);
		break;
	}
	switch (minRange) {
	case 0:
		AddPlayerInfo (N_("No initial range"));
		break;
	case 1:
		AddPlayerInfo (N_("Only mini bombs"));
		break;
	default:
		AddPlayerInfo (N_("Initial range %d"), minRange);
		break;
	}
	if (initHealth == reviveHealth) {
		if (NULL != (s = permHealthInfo[initHealth])) {
			AddPlayerInfo (s);
		}
	}
	else {
		if (NULL != (s = initHealthInfo[initHealth])) {
			AddPlayerInfo (s);
		}
		if (NULL != (s = reviveHealthInfo[reviveHealth])) {
			AddPlayerInfo (s);
		}
	}
	if (iniplayerflags == LF_Daleif) {
		if (revplayerflags == RF_Daleif) {
			AddPlayerInfo (N_("Daleif as default"));
		}
		else {
			AddPlayerInfo (N_("Initial Daleif"));
		}
	}
	else if ((revplayerflags == RF_Daleif)) {
		AddPlayerInfo (N_("Revived with Daleif"));
	}
	if (iniplayerflags == LF_RC) {
		if (revplayerflags == RF_RC) {
			AddPlayerInfo (N_("Remote control as default"));
		}
		else {
			AddPlayerInfo (N_("Initial remote control"));
		}
	}
	else if ((revplayerflags == RF_RC)) {
		AddPlayerInfo (N_("Revived with remote control"));
	}
	if (iniplayerflags == LF_Jump) {
		if (revplayerflags == RF_Jump) {
			AddPlayerInfo (N_("Jump as default"));
		}
		else {
			AddPlayerInfo (N_("Initial Jump"));
		}
	}
	else if ((revplayerflags == RF_Jump)) {
		AddPlayerInfo (N_("Revived with Jump"));
	}
	if (iniplayerflags == LF_Airpump) {
		if (revplayerflags == RF_Airpump) {
			AddPlayerInfo (N_("Airpump as default"));
		}
		else {
			AddPlayerInfo (N_("Initial airpump"));
		}
	}
	else if (revplayerflags == RF_Airpump) {
		AddPlayerInfo (N_("Revived with airpump"));
	}
	if (iniplayerflags == LF_Cloak) {
		if (revplayerflags == RF_Cloak) {
			AddPlayerInfo (N_("Cloak as default"));
		}
		else {
			AddPlayerInfo (N_("Initial cloak"));
		}
	}
	else if (revplayerflags == RF_Cloak) {
		AddPlayerInfo (N_("Revived with cloak"));
	}
	if (iniplayerflags & LF_Kick) {
		if (revplayerflags == RF_Kick) {
			AddPlayerInfo (N_("Initial kick"));
		}
		else {
			AddPlayerInfo (N_("Initial kick"));
		}
	}
	else if (revplayerflags == RF_Kick) {
		AddPlayerInfo (N_("Revived with kick"));
	}
	if (iniplayerflags == LF_Morph) {
		if (revplayerflags == RF_Morph) {
			AddPlayerInfo (N_("Morphing as default"));
		}
		else {
			AddPlayerInfo (N_("Initial morphing"));
		}
	}
	else if (revplayerflags == RF_Morph) {
		AddPlayerInfo (N_("Revived with morphing"));
	}
	if (iniplayerflags == LF_Through) {
		if (revplayerflags == RF_Through) {
			AddPlayerInfo (N_("Throughing as default"));
		}
		else {
			AddPlayerInfo (N_("Initial throughing"));
		}
	}
	else if (revplayerflags == RF_Through) {
		AddPlayerInfo (N_("Revived with throughing"));
	}

	if (iniplayerflags == LF_Suck) {
		if (revplayerflags == RF_Suck) {
			AddPlayerInfo (N_("Sucker as default"));
		}
		else {
			AddPlayerInfo (N_("Initial sucker"));
		}
	}
	else if (revplayerflags == RF_Suck) {
		AddPlayerInfo (N_("Revived with sucker"));
	}
	/* that's all folks */
}								/* ConfigLevelPlayers */

/*
 * Create Welcome messages for players at start of level
 */
void
WelcomePlayers (void)
{
	int i, j, num;
	const char *list[MAX_PLAYER];
	const char *swap;

	/* get messages */
	for (i = 0, num = 0; i < numPlayer; i++) {
		if (NULL != p_string[i].welcome && !player_stat[i].in_active) {
			list[num++] = p_string[i].welcome;
		}
	}
	/* shuffle them */
	for (i = 0; i < num; i++) {
		j = OtherRandomNumber (num);
		swap = list[i];
		list[i] = list[j];
		list[j] = swap;
	}
	/* show them */
	for (i = 0; i < num; i++) {
		SetMessage (list[i], XBFalse);
	}
}								/* WelcomePlayers */

/*
 *
 */
int
NumSpecialBombs (void)
{
	return specialBombs;
}								/* NumSpecialBombs */

/*
 * set player stat to default values
 */
static void
InitPlayerStat (BMPlayer * ps, int player, int ctrl, XBPlayerTeam team, int PID, XBBool local)	// XBCC
{
	/* set default values */
	ps->victories = 0;
	ps->PID = PID;				// XBCC
	ps->id = player;
	ps->disp = local ? SPM_MAPPED : SPM_UNMAPPED;
	ps->local = local;
	ps->sprite = CreatePlayerSprite (ps->id, 0, 0, 0, SPM_UNMAPPED);
	Dbg_Out (" sprite player %i %i", ((ps->sprite)->player).player, ps->id);
	((ps->sprite)->player).player = ps->id;
	Dbg_Out (" new sprite player %i %i\n", ((ps->sprite)->player).player, ps->id);
	ps->in_active = XBFalse;
	/* evaluate team mode */
	if (team == XBPT_None) {
		ps->team = ps->id;
	}
	else {
		ps->team = team - XBPT_None - 1;
	}
}								/* InitPlayerStat */

/*
 * set all messages for given player
 */
static void
InitPlayerMessages (PlayerStrings * str, const CFGPlayer * cfgPlayer)
{
	char tmp[128];

	/* player name */
	str->name = DupString (cfgPlayer->name);
	assert (NULL != str->name);
	str->tag = DupString (cfgPlayer->name);
	assert (NULL != str->tag);
	/* pause string */
	sprintf (tmp, "Game paused by %s", str->name);
	str->pause = DupString (tmp);
	assert (NULL != str->pause);
	/* win a level */
	if (NULL != cfgPlayer->messages.msgWinLevel) {
		str->winlevel = DupString (cfgPlayer->messages.msgWinLevel);
	}
	else {
		sprintf (tmp, _("%s wins"), str->name);
		str->winlevel = DupString (tmp);
	}
	assert (NULL != str->winlevel);
	/* win the game */
	if (NULL != cfgPlayer->messages.msgWinGame) {
		str->wingame = DupString (cfgPlayer->messages.msgWinGame);
	}
	else {
		str->wingame = DupString (N_("CONGRATULATIONS!"));
	}
	assert (NULL != str->wingame);
	/* request abort */
	sprintf (tmp, "Abort requested by %s", str->tag);
	str->abort = DupString (tmp);
	assert (NULL != str->abort);
	/* cancel abort */
	sprintf (tmp, "%s cancels abort", str->tag);
	str->abortcancel = DupString (tmp);
	assert (str->abortcancel != NULL);
	/* loosing a life */
	if (NULL != cfgPlayer->messages.msgLoseLife) {
		str->loselife = DupString (cfgPlayer->messages.msgLoseLife);
	}
	else {
		str->loselife = NULL;
	}
	/* loosing a level */
	if (NULL != cfgPlayer->messages.msgLoseLevel) {
		str->loselevel = DupString (cfgPlayer->messages.msgLoseLevel);
	}
	else {
		str->loselevel = NULL;
	}
	/* gloating */
	if (NULL != cfgPlayer->messages.msgGloat) {
		str->gloat = DupString (cfgPlayer->messages.msgGloat);
	}
	else {
		str->gloat = NULL;
	}
	/* laola */
	if (NULL != cfgPlayer->messages.msgLaola) {
		str->laola = DupString (cfgPlayer->messages.msgLaola);
	}
	else {
		str->laola = NULL;
	}
	/* looser */
	if (NULL != cfgPlayer->messages.msgLoser) {
		str->loser = DupString (cfgPlayer->messages.msgLoser);
	}
	else {
		str->loser = NULL;
	}
	/* welcome to the game */
	if (NULL != cfgPlayer->messages.msgWelcome) {
		str->welcome = DupString (cfgPlayer->messages.msgWelcome);
	}
	else {
		str->welcome = NULL;
	}
}								/* InitPlayerMessages */

/*
 *
 */
void
InitPlayers (XBPlayerHost host, const CFGGame * cfgGame, const CFGPlayer * cfgPlayer)
{
	int i, j, cnt;
	XBBool local;
	BMPlayer *ps;

	assert (NULL != cfgGame);
	assert (NULL != cfgPlayer);

	/* global settings */
	numPlayer = cfgGame->players.num;
	maxLives = cfgGame->setup.numLives;
	ifRecLives = cfgGame->setup.ifRecLives;
	randomPlayerPos = cfgGame->setup.randomPlayers;
	cnt = 0;
	/* player settings */
	for (ps = player_stat, i = 0; i < cfgGame->players.num; ps++, i++) {
		assert (ATOM_INVALID != cfgGame->players.player[i]);
		local = (host == cfgGame->players.host[i]);
		if (local) {
			ps->localDisplay = cnt;
			cnt++;
			ps->bot = cfgGame->setup.bot;
		}
		else {
			ps->localDisplay = -1;
			ps->bot = XBFalse;
			if (XBPT_None != cfgGame->players.team[i]) {
				for (j = 0; j < cfgGame->players.num; j++) {

					if (i != j && host == cfgGame->players.host[j]) {
						local = local || (cfgGame->players.team[i] == cfgGame->players.team[j]);
					}
				}
			}
		}
		ps->away = XBTrue;
		InitPlayerStat (player_stat + i, i, i, cfgGame->players.team[i], (cfgPlayer + i)->id.PID, local);	// XBCC
		InitPlayerMessages (p_string + i, cfgPlayer + i);
	}
}								/* InitPlayers */

/*
 *
 */
void
FinishPlayers (void)
{
	int i;
	PlayerStrings *str;

	for (i = 0; i < numPlayer; i++) {
		str = p_string + i;
		if (NULL != str->name) {
			free (str->name);
		}
		if (NULL != str->tag) {
			free (str->tag);
		}
		if (NULL != str->pause) {
			free (str->pause);
		}
		if (NULL != str->winlevel) {
			free (str->winlevel);
		}
		if (NULL != str->wingame) {
			free (str->wingame);
		}
		if (NULL != str->loselife) {
			free (str->loselife);
		}
		if (NULL != str->loselevel) {
			free (str->loselevel);
		}
		if (NULL != str->gloat) {
			free (str->gloat);
		}
		if (NULL != str->laola) {
			free (str->laola);
		}
		if (NULL != str->loser) {
			free (str->loser);
		}
		if (NULL != str->welcome) {
			free (str->welcome);
		}
		if (NULL != str->abort) {
			free (str->abort);
		}
		if (NULL != str->abortcancel) {
			free (str->abortcancel);
		}
	}
	numPlayer = 0;
}								/* FinishPlayers */

/*
 *
 */
void
DeletePlayerSprites (void)
{
	int player;

	for (player = 0; player < numPlayer; player++) {
		DeleteSprite (player_stat[player].sprite);
	}
}								/* DeletePlayerSprites */

/*
 *
 */
void
DropBomb (BMPlayer * ps, int type)
{
	if ((ps->bombs != 0) && (ps->sniping != 1) &&	/* skywalker / koen */
		(ps->illness != IllEmpty) &&
		(ps->morphed < 2) &&
		(type == BMTdefault || ps->special_bombs > 0 || ps->choice_bomb_type != NUM_BMT)) {
		if (ps->lives > 0) {
			if (ps->choice_bomb_type != NUM_BMT)
				type = ps->choice_bomb_type;
			if (NewPlayerBomb (ps, type)) {
				SND_Play (SND_DROP, ps->x / (PIXW / MAX_SOUND_POSITION));
				ps->bombs--;
				if (ps->morphed) {
					ps->morphed = 2;
					ps->num_morph--;
				}
	/** Skywalker **/
				if (ps->sniping) {
					ps->num_snipe--;
				}
	/** **/
				if (type != BMTdefault && ps->choice_bomb_type == NUM_BMT) {
					ps->special_bombs--;
				}
			}
			else {
				if (ps->morphed) {
					ps->morphed = 0;
				}
			}
		}
	}
}								/* DropBomb */

/*
 *
 */
static void
WalkStop (BMPlayer * ps, int flag, int mazex, int mazey)
{
	if (ps->illness != IllReverse) {
		if (ps->illness == IllReverse2) {
			switch (ps->d_look) {
			case GoDown:
				SetSpriteAnime (ps->sprite, SpriteStopLeft);
				break;
			case GoUp:
				SetSpriteAnime (ps->sprite, SpriteStopRight);
				break;
			case GoLeft:
				SetSpriteAnime (ps->sprite, SpriteStopDown);
				break;
			case GoRight:
				SetSpriteAnime (ps->sprite, SpriteStopUp);
				break;
			default:
				break;
			}
		}
		else {
			switch (ps->d_look) {
			case GoDown:
				SetSpriteAnime (ps->sprite, SpriteStopDown);
				break;
			case GoUp:
				SetSpriteAnime (ps->sprite, SpriteStopUp);
				break;
			case GoLeft:
				SetSpriteAnime (ps->sprite, SpriteStopLeft);
				break;
			case GoRight:
				SetSpriteAnime (ps->sprite, SpriteStopRight);
				break;
			default:
				break;
			}
		}
	}
	else {
		switch (ps->d_look) {
		case GoDown:
			SetSpriteAnime (ps->sprite, SpriteStopUp);
			break;
		case GoUp:
			SetSpriteAnime (ps->sprite, SpriteStopDown);
			break;
		case GoLeft:
			SetSpriteAnime (ps->sprite, SpriteStopRight);
			break;
		case GoRight:
			SetSpriteAnime (ps->sprite, SpriteStopLeft);
			break;
		default:
			break;
		}
	}
}								/* WalkStop */

/* 
 *
 */
static void
WalkUp (BMPlayer * ps, int flag, int mazex, int mazey)
{
	if (!(flag && CheckMazeGhost (ps->ghost, mazex, mazey - 1))
		||
		(!((ps->phantom) ? CheckMazePhantomWall (mazex, mazey - 1) : CheckMaze (mazex, mazey - 1))
		 && (mazey > 1))) {
		ps->y -= STEP_VERT;
		if (ps->y < 0)
			ps->y = PIXH - BLOCK_HEIGHT * 2;	// 02-05-2002
		mazey = ps->y / BLOCK_HEIGHT + 1;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteWalkLeft0 + ((ps->y / STEP_VERT) % 4));
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteWalkUp0 + ((ps->y / STEP_VERT) % 4));
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteWalkDown0 + ((ps->y / STEP_VERT) % 4));
		}
	}
	else {
		ps->d_ist = GoStop;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteStopLeft);
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteStopUp);
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteStopDown);
		}
	}

	/* try a kick */
	if (CheckBomb (mazex, mazey)
		&& ((ps->y % BLOCK_HEIGHT) == (STEP_VERT * BOMB_STEP) && (!ps->through)) && (!ps->through)) {
		if (ps->kick) {
			SND_Play (SND_KICK, ps->x / (PIXW / MAX_SOUND_POSITION));
			/* added by Galatius */
			switch (ps->daleif & GameRandomNumber (2)) {
			case 0:
				MoveBomb (mazex, mazey, GoUp);
				break;
			case 1:
				ps->daleifing = DALEIF_TIME;
				MoveBomb (mazex, mazey, GoDown);
				break;
			}					/* end added by Galatius */
			ps->d_soll = GoStop;
		}
		ps->y += STEP_VERT;
		ps->y = (ps->y + PIXH) % PIXH;
	}
	//   if(oldy!=ps->y)SND_Play (SND_STEP4, ps->x / (PIXW / MAX_SOUND_POSITION));
}								/* WalkUp */

/* 
 * local function walk_left 
 */
static void
WalkLeft (BMPlayer * ps, int flag, int mazex, int mazey)
{
	if (!(flag && CheckMazeGhost (ps->ghost, mazex - 1, mazey)) ||
		(!((ps->phantom) ? CheckMazePhantomWall (mazex - 1, mazey) : CheckMaze (mazex - 1, mazey))
		 && (mazex > 1))) {
		ps->x -= STEP_HORI;
		if (ps->x < 0)
			ps->x = PIXW;		// 02-05-2002
		mazex = ps->x / BLOCK_WIDTH;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteWalkDown0 + ((ps->x / STEP_VERT) % 4));
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteWalkLeft0 + ((ps->x / STEP_HORI) % 4));
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteWalkRight0 + ((ps->x / STEP_HORI) % 4));
		}
	}
	else {
		ps->d_ist = GoStop;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteStopDown);
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteStopLeft);
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteStopRight);
		}
	}

	/* try a kick */
	if (CheckBomb (mazex, mazey)
		&& ((ps->x % BLOCK_WIDTH) == (STEP_HORI * BOMB_STEP)) && (!ps->through)) {
		if (ps->kick) {
			SND_Play (SND_KICK, ps->x / (PIXW / MAX_SOUND_POSITION));
			/* added by Galatius */
			switch (ps->daleif & GameRandomNumber (2)) {
			case 0:

				MoveBomb (mazex, mazey, GoLeft);
				break;
			case 1:
				ps->daleifing = DALEIF_TIME;
				MoveBomb (mazex, mazey, GoRight);

				break;
			default:
				break;

			}
			ps->d_soll = GoStop;
			/* end added by Galatius */
		}
		ps->x += STEP_HORI;
	}
	//  if(oldx!=ps->x)SND_Play (SND_STEP1, ps->x / (PIXW / MAX_SOUND_POSITION));
}								/* WalkLeft */

/* 
 *
 */
static void
WalkDown (BMPlayer * ps, int flag, int mazex, int mazey)
{
	if (!(flag && CheckMazeGhost (ps->ghost, mazex, mazey + 1)) ||
		(!((ps->phantom) ? CheckMazePhantomWall (mazex, mazey + 1) : CheckMaze (mazex, mazey + 1))
		 && (mazey < (MAZE_H - 2)))) {
		ps->y += STEP_VERT;
		if (ps->y >= (PIXH - BLOCK_HEIGHT * 2))
			ps->y = 0;			// 02-05-2002

		mazey = ps->y / BLOCK_HEIGHT + 1;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteWalkRight0 + ((ps->y / STEP_VERT) % 4));
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteWalkDown0 + ((ps->y / STEP_VERT) % 4));
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteWalkUp0 + ((ps->y / STEP_VERT) % 4));
		}
	}
	else {
		ps->d_ist = GoStop;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteStopRight);
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteStopDown);
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteStopUp);
		}
	}

	/* try a kick */
	if (CheckBomb (mazex, mazey + 1)
		&& ((ps->y % BLOCK_HEIGHT) == (BLOCK_HEIGHT - STEP_VERT * BOMB_STEP))
		&& (!ps->through)
		) {
		if (ps->kick) {
			SND_Play (SND_KICK, ps->x / (PIXW / MAX_SOUND_POSITION));
			/* added by Galatius */
			switch (ps->daleif & GameRandomNumber (2)) {
			case 0:
				MoveBomb (mazex, mazey + 1, GoDown);
				break;
			case 1:
				ps->daleifing = DALEIF_TIME;
				MoveBomb (mazex, mazey + 1, GoUp);
				break;
			}
			/* end added by Galatius */
			ps->d_soll = GoStop;
		}
		ps->y -= STEP_VERT;
	}
	//if(oldy!=ps->y) SND_Play (SND_STEP2, ps->x / (PIXW / MAX_SOUND_POSITION));
}								/* WalkDown */

/* 
 *
 */
static void
WalkRight (BMPlayer * ps, int flag, int mazex, int mazey)
{
	if (!(flag && CheckMazeGhost (ps->ghost, mazex + 1, mazey)) ||
		(!((ps->phantom) ? CheckMazePhantomWall (mazex + 1, mazey) : CheckMaze (mazex + 1, mazey))
		 && (mazex < (MAZE_W - 2)))) {
		ps->x += STEP_HORI;
		if (ps->x >= PIXW - BLOCK_WIDTH)
			ps->x = 0;			// 02-05-2002
		mazex = ps->x / BLOCK_WIDTH;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteWalkUp0 + ((ps->x / STEP_VERT) % 4));
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteWalkRight0 + ((ps->x / STEP_HORI) % 4));
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteWalkLeft0 + ((ps->x / STEP_HORI) % 4));
		}
	}
	else {
		ps->d_ist = GoStop;
		if (ps->illness != IllReverse) {
			if (ps->illness == IllReverse2) {
				SetSpriteAnime (ps->sprite, SpriteStopUp);
			}
			else {
				SetSpriteAnime (ps->sprite, SpriteStopRight);
			}
		}
		else {
			SetSpriteAnime (ps->sprite, SpriteStopLeft);
		}
	}

	/* try kick */
	if (CheckBomb (mazex + 1, mazey)
		&& ((ps->x % BLOCK_WIDTH) == (BLOCK_WIDTH - STEP_HORI * BOMB_STEP))
		&& (!ps->through)) {
		if (ps->kick) {
			SND_Play (SND_KICK, ps->x / (PIXW / MAX_SOUND_POSITION));
			/* added by Galatius */
			switch (ps->daleif & GameRandomNumber (2)) {
			case 0:
				MoveBomb (mazex + 1, mazey, GoRight);
				break;
			case 1:
				ps->daleifing = DALEIF_TIME;
				MoveBomb (mazex + 1, mazey, GoLeft);
				break;
			}
			/* end added by Galatius */
			ps->d_soll = GoStop;
		}
		ps->x -= STEP_HORI;
	}
	//  if(oldx!=ps->x)SND_Play (SND_STEP3, ps->x / (PIXW / MAX_SOUND_POSITION));
}								/* WalkRight */

/*
 * try to teleport player
 */
static XBBool
TeleportPlayer (BMPlayer * ps, int mazeX, int mazeY)
{
	int newMazeX, newMazeY;
	int i, j, n;
	int fs[MAZE_W * MAZE_H];

	n = 0;
	for (i = 0; i < MAZE_W; i++) {
		for (j = 0; j < MAZE_H; j++) {
			if (!CheckMaze (i, j)) {
				fs[n] = i + j * MAZE_W;
				n++;
			}
		}
	}

	if (n > 0) {
		i = fs[GameRandomNumber (n)];
		newMazeX = i % MAZE_W;
		newMazeY = i / MAZE_W;
		if (((ps->
			  phantom) ? (!CheckMazePhantomWall (newMazeX, newMazeY)) : (!CheckMaze (newMazeX,
																					 newMazeY)))
			&& ((mazeX != newMazeX) || (mazeY != newMazeY))) {
			SND_Play (SND_TELE1, ps->x / (PIXW / MAX_SOUND_POSITION));
			ps->x = newMazeX * BLOCK_WIDTH;
			ps->y = (newMazeY - 1) * BLOCK_HEIGHT;
			ps->d_soll = GoStop;
			ps->d_look = GoDown;
			SND_Play (SND_TELE2, ps->x / (PIXW / MAX_SOUND_POSITION));
			return XBTrue;
		}
	}

	return XBFalse;
}								/* TeleportPlayer */

/* 
 * local function do_walk 
 */
static void
DoWalk (BMPlayer * ps, int gameTime)
{
	XBBool flag;
	int mazeX, mazeY;
	int i;
	int xalt, yalt;
	int spm_mode;

	xalt = ps->x;
	yalt = ps->y;

	if (ps->illness != IllSlow || 0 == gameTime % 2) {
		for (i = 0; i <= (ps->illness == IllRun) * ((ps->speed == 0) ? 1 : ps->speed); i++) {
			flag = XBFalse;
			mazeX = ps->x / BLOCK_WIDTH;
			mazeY = ps->y / BLOCK_HEIGHT + 1;

			if (0 == (ps->x % BLOCK_WIDTH) && 0 == (ps->y % BLOCK_HEIGHT)) {
				flag = XBTrue;
				/* check if player has deliberately teleported */
				if (ps->teleport == TELEPORT_TIME) {
					if (TeleportPlayer (ps, mazeX, mazeY)) {
						ps->teleport--;
					}
				}
				/* change direction if needed */
				ps->d_ist = ps->d_soll;
				if (ps->d_ist != GoStop) {
					ps->d_look = ps->d_ist;
				}
			}
			/* random teleporting */
			if ((ps->illness == IllTeleport) && (0 == GameRandomNumber (32))) {
				TeleportPlayer (ps, mazeX, mazeY);
				ps->d_ist = GoStop;
				ps->d_soll = GoStop;
			}
			/* let the player walk */
			if (ps->sniping != 1) {	/* skywalker / koen */
				switch (ps->d_ist) {
				case GoStop:
					WalkStop (ps, flag, mazeX, mazeY);
					break;
				case GoLeft:
					WalkLeft (ps, flag, mazeX, mazeY);
					break;
				case GoRight:
					WalkRight (ps, flag, mazeX, mazeY);
					break;
				case GoDown:
					WalkDown (ps, flag, mazeX, mazeY);
					break;
				case GoUp:
					WalkUp (ps, flag, mazeX, mazeY);
					break;
				default:
					break;
				}
			}
			MoveSprite (ps->sprite, ps->x, ps->y);

			/* insert get _extra here */
			if ((ps->x % BLOCK_WIDTH == 0) && (ps->y % BLOCK_HEIGHT == 0)) {
				switch (GetExtra (ps->invincible, ps->x / BLOCK_WIDTH, ps->y / BLOCK_HEIGHT + 1)) {
				case BTBomb:
					SND_Play (SND_NEWBOMB, ps->x / (PIXW / MAX_SOUND_POSITION));
					ps->bombs++;
					break;
				case BTRange:
					SND_Play (SND_MOREFIRE, ps->x / (PIXW / MAX_SOUND_POSITION));
					if (ps->range < MAX_RANGE) {
						ps->range++;
					}
					break;
				case BTSick:
					ps->illtime = ILLTIME;
					ps->illness = GameRandomNumber (MAX_ILL) + 1;
					if (ps->illness == IllInvisible) {
						SND_Play (SND_INVIS, ps->x / (PIXW / MAX_SOUND_POSITION));
					}
					else {
						SND_Play (SND_BAD, ps->x / (PIXW / MAX_SOUND_POSITION));
					}
					if (ps->illness == IllReverse) {
						switch (ps->d_ist) {
						case GoDown:
							ps->d_ist = GoUp;
							break;
						case GoUp:
							ps->d_ist = GoDown;
							break;
						case GoLeft:
							ps->d_ist = GoRight;
							break;
						case GoRight:
							ps->d_ist = GoLeft;
							break;
						default:
							break;
						}
					}
					if (ps->illness == IllReverse2) {
						switch (ps->d_ist) {
						case GoDown:
							ps->d_ist = GoLeft;
							break;
						case GoUp:
							ps->d_ist = GoRight;
							break;
						case GoLeft:
							ps->d_ist = GoUp;
							break;
						case GoRight:
							ps->d_ist = GoDown;
							break;
						default:
							break;
						}
					}
					break;

				case BTSpecial:
					ps->num_extras++;
					(*specialExtraFunc) (ps);
					break;
				}
			}
		}
	}							/*decrement phantom time */
	/* Written by Amilhastre */
	if (ps->phantom > 0) {
		ps->phantom--;
	}
/* Added by "Belgium Guys" */
	if (ps->through) {
		ps->through--;
	}
	if (ps->ghost) {
		ps->ghost--;
/* Written by VVL */
		if ((ps->ghost == 0) && (ps->lives > 0)) {
			if (CheckMaze ((ps->x / BLOCK_WIDTH), (ps->y / BLOCK_HEIGHT) + 1)) {
				ps->lives = 1;
				ps->dying = DEAD_TIME;
			}
		}
	}
	/* Skywalker */
	if (ps->laola) {
		static BMSpriteAnimation laola_animation[6] = {
			SpriteWinner3, SpriteWinner2, SpriteWinner,
			SpriteWinner, SpriteWinner2,
			SpriteWinner3
		};
		SetSpriteAnime (ps->sprite, laola_animation[ps->laola - 1]);
		ps->laola--;
	}
	else {
		if (ps->looser) {
			static BMSpriteAnimation looser_animation[10] = {
				SpriteLooser, SpriteLooser, SpriteLooser1, SpriteLooser1, SpriteLooser,
				SpriteLooser, SpriteLooser, SpriteLooser2, SpriteLooser2, SpriteLooser,
			};
			SetSpriteAnime (ps->sprite, looser_animation[ps->looser - 1]);
			ps->looser--;
		}
	}
	/* */
	// 02-05-2002, reinco BUG fixed
	if (ps->invincible > 0) {
		ps->invincible--;
        /* AbsInt start: */
        if (!ps->invincible)
            ps->ai_revived = 0;
        /* AbsInt end */
	}
	else if (ps->teleport > 1) {
		ps->teleport--;
	}

	/* draw player if not totally invisible or morphed */
	if (ps->in_active) {
		spm_mode = SPM_UNMAPPED;
	}
	else if (ps->illness != IllInvisible) {
		/* set default mode */
		spm_mode = SPM_MAPPED;
		/* first check for cloak */
		if (ps->cloaking < 0) {
			ps->cloaking++;
			if (ps->cloaking & 0x01) {
				spm_mode = ps->disp;
			}
			else {
				spm_mode = SPM_UNMAPPED;
			}
		}
		/* Added by "Belgium Guys" *//* blinking if gost_time < 64 */
		if ((ps->ghost < 64) && (ps->ghost)) {
			if (ps->ghost & 0x01) {
				spm_mode |= SPM_MASKED;
			}
		}

		/* blinking if invincible */
		if (ps->invincible > 0) {
			if (ps->invincible & 0x01) {
				spm_mode |= SPM_MASKED;
			}
			/* or slower blinking if arrived from teleport */
		}
		else if (ps->teleport > 1) {
			if ((ps->teleport >> 1) & 0x01) {
				spm_mode |= SPM_MASKED;
			}
		}
	}
	else {
		spm_mode = SPM_UNMAPPED;
	}
	SetSpriteMode (ps->sprite, spm_mode);

	/* is player still sick? */
	if (ps->illness != ps->health) {
		/* decrement illness timer */
		if ((ps->illtime--) == 0) {
			/* heal if time is over */
			ps->illness = ps->health;
		}
	}

	/* drop random bombs if needed */
	if ((ps->x % BLOCK_WIDTH == 0) && (ps->y % BLOCK_HEIGHT == 0)) {
		if (ps->illness == IllBomb) {
			if (GameRandomNumber (4) != 0) {
				if (ps->choice_bomb_type != NUM_BMT)
					DropBomb (ps, ps->choice_bomb_type);
				else
					DropBomb (ps, BMTdefault);

			}
		}
	}
}								/* DoWalk */

/* 
 * 
 */
static void
DoMorph (BMPlayer * ps)
{
	SetSpriteAnime (ps->sprite, SpriteMorphed);
	SetSpriteMode (ps->sprite, (ps->morphed == 2) ? SPM_MAPPED : SPM_UNMAPPED);
	if (ps->d_soll != GoStop) {
		MoveBomb (ps->x / BLOCK_WIDTH, ps->y / BLOCK_HEIGHT + 1, ps->d_soll);
		ps->d_soll = GoStop;
	}
}								/* DoMorph */

/*
 * 
 */
void
DoJunkie (void)
{
	BMPlayer *ps1;

	/* Junkie countdown */
	for (ps1 = player_stat; ps1 < player_stat + numPlayer; ps1++) {
		if ((ps1->lives) && (ps1->junkie)) {
			/* Junkie sickness */
			switch (--(ps1->junkie)) {
			case JUNKIE_TIME_1:
			case JUNKIE_TIME_2:
				/* Give a random illness */
				ps1->illtime = JUNKIE_ILL_TIME;
				ps1->illness = GameRandomNumber (MAX_ILL) + 1;
				break;

			case JUNKIE_TIME_3:
				/* Stun player and give speed */
				ps1->stunned += JUNKIE_STUN_TIME;
				ps1->illtime = JUNKIE_ILL_TIME;
				ps1->illness = IllRun;
				break;

			case 0:
				/* Too long! Take a hit. */
				ps1->dying = DEAD_TIME;
				ps1->junkie = MAX_JUNKIE_TIME;
				break;
			}
		}
	}
}								/* DoJunkie */

 /**/
/* public function Electrify_other_players */
	 /**/ int
ElectrifyOtherPlayers (int nplayer)
{
	int player;
	int count = 0;

	for (player = 0; player < numPlayer; player++) {
		if (nplayer != player) {
			if (player_stat[player].lives) {
				if ((ABS (player_stat[player].x - player_stat[nplayer].x) < (BLOCK_WIDTH * 3 / 4))
					&& (ABS (player_stat[player].y - player_stat[nplayer].y) <
						(BLOCK_HEIGHT * 3 / 4))) {
					player_stat[player].dying = DEAD_TIME;
					player_stat[player].electrify = 0;
					count++;
				}
			}
		}
	}
	return (count > 0);
}

/*
 *
 */
void
InfectOtherPlayers (int *active_player)
{
	BMPlayer *ps1, *ps2;
	BMPlayer *ptr;
	int i, team_alive, equipe;

	for (ps1 = player_stat; ps1 < player_stat + numPlayer; ps1++) {
		for (ps2 = ps1 + 1; ps2 < player_stat + numPlayer; ps2++) {
			if ((ABS (ps1->x - ps2->x) < ILL_X)
				&& (ABS (ps1->y - ps2->y) < ILL_Y)) {
				if (ps1->lives && ps2->lives) {
					/* infection with "evil grail" virus */

					if (ps1->evilill && (!ps2->invincible)) {
						ps1->illtime = ps1->evilill = 0;
						ps1->illness = ps1->health = IllRun;
						ps1->kick = XBTrue;
						ps1->invincible += BONUSEVIL;
						ps1->phantom += BONUSEVIL;
						if (ps1->cloaking > 0)
							ps1->cloaking = -(ps1->cloaking + BONUSEVIL);
						else
							ps1->cloaking -= BONUSEVIL;
						ps1->revive += 1;
						ps2->evilill = ILLDEATHTIME;

					}
					else if (ps2->evilill && (!ps1->invincible)) {
						ps2->illtime = ps2->evilill = 0;
						ps2->illness = ps2->health = IllRun;
						ps2->kick = XBTrue;
						ps2->invincible += BONUSEVIL;
						ps2->phantom += BONUSEVIL;
						if (ps2->cloaking > 0)
							ps2->cloaking = -(ps2->cloaking + BONUSEVIL);
						else
							ps2->cloaking -= BONUSEVIL;
						ps2->revive += 1;
						ps1->evilill = ILLDEATHTIME;
					}
					/* infection with "normal" viruses */
					if (ps1->illness != ps2->illness) {
						if ((!ps2->invincible) && (ps1->illtime > ps2->illtime)) {
							ps2->illness = ps1->illness;
							ps2->illtime = ILLTIME;
						}
						else if ((!ps1->invincible) && (ps2->illtime > ps1->illtime)) {
							ps1->illness = ps2->illness;
							ps1->illtime = ILLTIME;
						}
					}
					/* infection with junkie virus */
					if (((ps2->junkie) && (!ps1->invincible)) || (ps1->junkie)) {
						ps1->junkie = MAX_JUNKIE_TIME;
					}
					if (((ps1->junkie) && (!ps2->invincible)) || (ps2->junkie)) {
						ps2->junkie = MAX_JUNKIE_TIME;
					}
				}
				else {
					if ((ps2->lives) && (ps1->revive)) {
						equipe = -1;
						team_alive = XBFalse;
						for (i = 0, ptr = player_stat; i < numPlayer; i++, ptr++) {
							if (ptr->team == ps1->team) {
								team_alive |= (ptr->lives != 0);
								equipe++;
							}
						}

						if (!team_alive)
							(*active_player)++;
						ps1->lives = 1;
						ps1->revive--;
					}

					if ((ps1->lives) && (ps2->revive)) {
						equipe = -1;
						team_alive = XBFalse;
						for (i = 0, ptr = player_stat; i < numPlayer; i++, ptr++) {
							if (ptr->team == ps2->team) {
								team_alive |= (ptr->lives != 0);
								equipe++;
							}
						}

						if (!team_alive)
							(*active_player)++;
						ps2->lives = 1;
						ps2->revive--;

					}
				}
			}
		}
	}
}								/* InfectOtherPlayers */

/*
 *
 */
static void
HaveAGloat (int player)
{
	int g, gloatpl, gloatpltt;

	gloatpl = -1;
	for (g = 0; g < 6; g++) {
		gloatpltt = OtherRandomNumber (numPlayer);
		if (gloatpltt != player && player_stat[gloatpltt].lives > 0) {
			gloatpl = gloatpltt;
			break;
		}
	}
	if (gloatpl > -1) {
		SetMessage (p_string[gloatpl].gloat, XBFalse);
	}
}								/* HaveAGloat */

void
SetMsgLaola (int player)
{
	SetMessage (p_string[player].laola, XBFalse);

}

void
SetMsgLoser (int player)
{
	SetMessage (p_string[player].loser, XBFalse);

}

																				 /* Added by Fouf on 09/02/99 22:46:25 *//* Added by "Belgium Guys" */ /**/
	/* local function kill_player_at_ghost */
	 /**/ void
KillPlayerAtGhost (int block, int x, int y)
{
	BMPlayer *ps;
	int player;

	for (player = 0; player < numPlayer; player++) {
		ps = player_stat + player;
		if (!ps->ghost || block == BTVoid) {
			if (ps->lives > 0) {
				if ((ps->x < (x + 1) * BLOCK_WIDTH)
					&& (ps->x > (x - 1) * BLOCK_WIDTH)
					&& (ps->y < (y) * BLOCK_HEIGHT)
					&& (ps->y > (y - 2) * BLOCK_HEIGHT)) {
					ps->lives = 1;
					ps->dying = DEAD_TIME;
				}
			}
		}
	}
}

/*
 * 
 */
void
KillPlayerAt (int x, int y)
{
	BMPlayer *ps;
	int player;

	for (player = 0; player < numPlayer; player++) {
		ps = player_stat + player;
/* Added by "Belgium Guys" */
		if (!ps->ghost) {
			if (ps->lives > 0) {
				if ((ps->x < (x + 1) * BLOCK_WIDTH)
					&& (ps->x > (x - 1) * BLOCK_WIDTH)
					&& (ps->y < (y) * BLOCK_HEIGHT)
					&& (ps->y > (y - 2) * BLOCK_HEIGHT)) {
					ps->lives = 1;
					ps->dying = DEAD_TIME;
				}
			}
		}
	}
}								/* KillPlayerAt */

/*
 *
 */
int
KillOtherPlayers (int team)
{
	int count = 0;
	int player;

	for (player = 0; player < numPlayer; player++) {
		if ((player_stat[player].team != team)
			&& (player_stat[player].lives > 0)) {
			player_stat[player].dying = DEAD_TIME;
			count++;
		}
	}

	return count;
}								/* KillOtherPlayers */

/*
 *
 */
int
StunOtherPlayers (int team, int time)
{
	int count = 0;
	int player;

	for (player = 0; player < numPlayer; player++) {
		if ((player_stat[player].team != team)
			&& (!player_stat[player].invincible > 0)) {
			SND_Play (SND_STUN, player_stat[player].x / (PIXW / MAX_SOUND_POSITION));
			player_stat[player].stunned = time;
			count++;
		}
	}
	return count;
}								/* StunOtherPlayers */

/*Skywalker */

 /**/
/* public function fart_on_other_players (galatius) */
	 /**/ int
FartOnOtherPlayers (BMPlayer * ps)
{
	BMPlayer *ps1;
	int ex, ey;
	int count = 0;

	/*  do_bell(); */
	if (ps->stunned || ps->smelly) {
		return (count > 0);
	}
	ps->smelly = SMELLY_TIME;

	for (ps1 = player_stat; ps1 < player_stat + numPlayer; ps1++) {
		if ((ps1->lives == 0) || (ps1->invincible) ||
			((ps->x == ps1->x) && (ps->y == ps1->y)) ||
			(ABS (ps1->x - ps->x) >= (BLOCK_WIDTH * 2)) ||
			(ABS (ps1->y - ps->y) >= (BLOCK_HEIGHT * 2))) {
			continue;
		}						/* Now ps will fart ps1 */

		ps1->farted = 20;		/* Fart counter */

		ex = (ps1->x - ps->x) * BLOCK_HEIGHT;
		ey = (ps1->y - ps->y) * BLOCK_WIDTH;

		switch (ps1->d_ist) {
		case GoStop:
			if (ABS (ex) >= ABS (ey)) {
				ps1->d_soll = (ex < 0 ? GoLeft : GoRight);
			}
			else {
				ps1->d_soll = (ey < 0 ? GoUp : GoDown);
			}
			break;
		case GoRight:
		case GoLeft:
			ps1->d_soll = (ex < 0 ? GoLeft : GoRight);
			break;
		case GoUp:
		case GoDown:
			ps1->d_soll = (ey < 0 ? GoUp : GoDown);
			break;
		default:
			break;
		}

		ps1->illness = IllRun;
		ps1->illtime = 20;
		ps1->stunned = 20;

		count++;
	}
	return (count > 0);
}

 /**/
/* public function Swap_color_other_players */
	 /**/ void
SwapColorOtherPlayers (int team)
{
	int count = 0;
	int D[MAX_PLAYER];
	int RR;
	int player;

	if (numPlayer == 2) {
		RR = ((player_stat[0].sprite)->player).player;
		((player_stat[0].sprite)->player).player = ((player_stat[1].sprite)->player).player;
		((player_stat[1].sprite)->player).player = RR;
		MarkMazeSprite (player_stat[0].sprite);
		MarkMazeSprite (player_stat[1].sprite);
	}
	else {

		for (player = 0; player < numPlayer; player++) {
			if (((player_stat[player].team) != team) && player_stat[player].lives) {
				D[count] = player;
				count++;
			}
		}

		if (count > 2) {
			RR = ((player_stat[D[0]].sprite)->player).player;
			for (player = 0; player < count - 1; player++) {
				((player_stat[D[player]].sprite)->player).player =
					((player_stat[D[player + 1]].sprite)->player).player;
				MarkMazeSprite (player_stat[D[player]].sprite);
			}
			((player_stat[D[count - 1]].sprite)->player).player = RR;
			MarkMazeSprite (player_stat[D[count - 1]].sprite);
		}
		else {
			if (count == 2) {
				RR = ((player_stat[D[0]].sprite)->player).player;
				((player_stat[D[0]].sprite)->player).player =
					((player_stat[D[1]].sprite)->player).player;
				((player_stat[D[1]].sprite)->player).player = RR;
				MarkMazeSprite ((player_stat[D[0]].sprite));
				MarkMazeSprite ((player_stat[D[1]].sprite));
			}
		}
	}
}

#if unused
 /**/
/* public function Swap_color_other_players2 */
	 /**/ void
SwapColorOtherPlayers2 (int team)
{
	int count = 0;
	int D[MAX_PLAYER];
	int RR;
	int player;
	int swapper = 0;

	fprintf (stderr, "count %i \n", count);
	if (numPlayer == 2) {
		RR = ((player_stat[0].sprite)->player).player;
		((player_stat[0].sprite)->player).player = ((player_stat[1].sprite)->player).player;
		((player_stat[1].sprite)->player).player = RR;
		MarkMazeSprite (player_stat[0].sprite);
		MarkMazeSprite (player_stat[1].sprite);
	}
	else {

		for (player = 0; player < numPlayer; player++) {
			if (player_stat[player].lives) {
				if (((player_stat[player].team) != team)) {
					D[count] = player;
					count++;
				}
				else {
					swapper = player;
				}
			}
		}

		if (count > 2) {
			RR = ((player_stat[D[0]].sprite)->player).player;
			for (player = 0; player < count - 1; player++) {
				((player_stat[D[player]].sprite)->player).player =
					((player_stat[D[player + 1]].sprite)->player).player;
				MarkMazeSprite (player_stat[D[player]].sprite);
			}
			((player_stat[D[count - 1]].sprite)->player).player = RR;
			MarkMazeSprite (player_stat[D[count - 1]].sprite);
		}
		/* 2 or 3 players, count 1 or 2 */
		else {
			if (count > 0) {
				fprintf (stderr, "count %i numPlayer %i\n", count, numPlayer);
				if (count == 1)
					D[1] = swapper;

				RR = ((player_stat[D[0]].sprite)->player).player;
				((player_stat[D[0]].sprite)->player).player =
					((player_stat[D[1]].sprite)->player).player;
				((player_stat[D[1]].sprite)->player).player = RR;
				MarkMazeSprite ((player_stat[D[0]].sprite));
				MarkMazeSprite ((player_stat[D[1]].sprite));
			}

		}
	}
}
#endif

/*Skywalker */

 /**/
/* public function steal_bombs_other_players */
	 /**/ int
StealBombsOtherPlayers (int team)
{
	int count = 0;
	int player;

	for (player = 0; player < numPlayer; player++) {
		if (player_stat[player].team != team) {
			if (player_stat[player].bombs > 0) {
				player_stat[player].bombs--;
				count++;
			}
		}
	}
	return count;
}

 /**/
/* public function steal_range_other_players */
	 /**/ int
StealRangeOtherPlayers (int team)
{
	int count = 0;
	int player;
	for (player = 0; player < numPlayer; player++) {
		if (player_stat[player].team != team) {
			if (player_stat[player].range > 1) {
				player_stat[player].range--;
				count++;
			}
		}
	}
	return count;
}

 /**/
/* public function Swap_position_other_players */
	 /**/ void
SwapPositionOtherPlayers (int team)
{
	int x = 0, y = 0;
	int first = -1;
	int player;
	int count = 0;

	for (player = 0; player < numPlayer; player++) {
		if ((player_stat[player].team) != team) {
			count++;
			if (player_stat[player].lives) {
				if (first > -1) {
					player_stat[first].x = (player_stat[player].x / BLOCK_WIDTH) * BLOCK_WIDTH;
					player_stat[first].y = (player_stat[player].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
					first = player;
				}
				else {
					first = player;
					x = (player_stat[player].x / BLOCK_WIDTH) * BLOCK_WIDTH;;
					y = (player_stat[player].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;;
				}
			}
		}
	}
	if (count > 1) {
		if (first > -1) {
			player_stat[first].x = x;
			player_stat[first].y = y;
		}
	}

	if (numPlayer == 2) {
		x = (player_stat[0].x / BLOCK_WIDTH) * BLOCK_WIDTH;
		y = (player_stat[0].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
		player_stat[0].x = (player_stat[1].x / BLOCK_WIDTH) * BLOCK_WIDTH;
		player_stat[0].y = (player_stat[1].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
		player_stat[1].x = x;
		player_stat[1].y = y;
	}
}

#ifdef unused
 /**/
/* public function Swap_position_other_players2 */
	 /**/ void
SwapPositionOtherPlayers2 (int team)
{
	int x = 0, y = 0;
	int first = -1;
	int player, swapper = 0;
	int count = 0;

	for (player = 0; player < numPlayer; player++) {
		if (player_stat[player].lives) {
			if ((player_stat[player].team) != team) {
				count++;
				if (first > -1) {
					player_stat[first].x = (player_stat[player].x / BLOCK_WIDTH) * BLOCK_WIDTH;
					player_stat[first].y = (player_stat[player].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
					first = player;
				}
				else {
					first = player;
					x = (player_stat[player].x / BLOCK_WIDTH) * BLOCK_WIDTH;;
					y = (player_stat[player].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;;
				}
			}
			else {
				swapper = player;
			}
		}
	}
	if (count > 1) {
		if (first > -1) {
			player_stat[first].x = x;
			player_stat[first].y = y;
		}
	}
	/* count==1 */
	else {
		if (first > -1) {
			player_stat[first].x = (player_stat[swapper].x / BLOCK_WIDTH) * BLOCK_WIDTH;
			player_stat[first].y = (player_stat[swapper].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
			player_stat[swapper].x = x;
			player_stat[swapper].y = y;
		}
	}

	if (numPlayer == 2) {
		x = (player_stat[0].x / BLOCK_WIDTH) * BLOCK_WIDTH;
		y = (player_stat[0].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
		player_stat[0].x = (player_stat[1].x / BLOCK_WIDTH) * BLOCK_WIDTH;
		player_stat[0].y = (player_stat[1].y / BLOCK_HEIGHT) * BLOCK_HEIGHT;
		player_stat[1].x = x;
		player_stat[1].y = y;
	}
}
#endif

/* added by Skywalker */
 /**/
/* public function do_frog (galatius) */
	 /**/ void
DoFrog (BMPlayer * ps)
{
	int ex, ey;
	int frogs = ps->frogger;

	if (!ps->d_ist == GoStop) {
		/* If you are already moving: */
		ex = ps->x % BLOCK_WIDTH;
		ey = ps->y % BLOCK_HEIGHT;

		switch (ps->d_ist) {
		case GoDown:
		case GoUp:
			ps->y -= ey + ((ps->d_ist == GoUp) - 1) * BLOCK_HEIGHT;
			break;
		case GoRight:
		case GoLeft:
			ps->x -= ex + ((ps->d_ist == GoLeft) - 1) * BLOCK_WIDTH;
			break;
		default:
			break;
		}

		frogs--;
	}

	ex = 0;
	ey = 0;
	switch (ps->d_look) {
	case GoDown:
		ey = 1;
		break;
	case GoUp:
		ey = -1;
		break;
	case GoRight:
		ex = 1;
		break;
	case GoLeft:
		ex = -1;
		break;
	default:
		break;
	}

	ps->x += ex * BLOCK_WIDTH * frogs;
	ps->y += ey * BLOCK_HEIGHT * frogs;

	ps->x = MAX (0, MIN (14 * BLOCK_WIDTH, ps->x));
	ps->y = MAX (-BLOCK_HEIGHT, MIN (11 * BLOCK_HEIGHT, ps->y));

	if (CheckMaze (ps->x / BLOCK_WIDTH, ps->y / BLOCK_HEIGHT + 1)) {
		MoveSprite (ps->sprite, ps->x, ps->y);
		ps->lives = 1;
		ps->dying = DEAD_TIME;
	}

	ps->d_soll = GoStop;

}

/* end added by Skywalker */

/*
 * 
 */
static void
RevivePlayer (BMPlayer * ps, int *active_player)
{
	BMPlayer *ptr;
	PlayerStrings *st;
	int i, team_alive;
	int playerflags;

	st = p_string + ps->id;

	ps->lives--;
	/* check if player has lost all lives? */
	if (ps->lives == 0) {
		SetSpriteMode (ps->sprite, SPM_UNMAPPED);
		SND_Play (SND_DEAD, ps->x / (PIXW / MAX_SOUND_POSITION));
		team_alive = XBFalse;
		for (i = 0, ptr = player_stat; i < numPlayer; i++, ptr++) {
			if (ptr->team == ps->team) {
				team_alive |= (ptr->lives != 0);
			}
		}
		if (!team_alive) {
			(*active_player)--;
		}
		DistributeExtras (ps->bombs - minBombs, ps->range - minRange, ps->num_extras,
						  ps->special_bombs);
		SetMessage (st->loselevel, XBFalse);
	}
	else {
		SND_Play (SND_OUCH, ps->x / (PIXW / MAX_SOUND_POSITION));
		DistributeExtras (0, 0, ps->num_extras, ps->special_bombs);
		SetMessage (st->loselife, XBFalse);
	}
	HaveAGloat (ps->id);
	/* reset values */
	playerflags = (ps->revextra_flags & ((0xffffff) >> 2));
	ps->invincible = NEW_INVINCIBLE;
	ps->dying = 0;
	ps->stunned = 0;
	ps->illness = reviveHealth;
	ps->health = reviveHealth;
    /* AbsInt: no bombs when revived */
    ps->ai_revived = 1;
    /* AbsInt end */
	ps->illtime = 0;
	ps->teleport = 0;
	ps->cloaking = XBFalse;
	ps->morphed = XBFalse;
	ps->num_morph = 0;
	ps->evilill = 0;
	ps->phantom = 0;
	ps->through = 0;
	ps->throughCount = 0;
	ps->farted = (RF_Fart == playerflags) ? XBTrue : XBFalse;	/* (galatius) */
	ps->smelly = 0;				/* (galatius) */
	ps->bfarter = (RF_Bfart == playerflags) ? XBTrue : XBFalse;	/* (galatius) */
	if (!ps->revive)
		ps->cloaking = 0;

	/* Note that junkie ISN'T reset (not a bug) */
	/* very important */
	if (ps->remote_control > 0) {
		IgnitePlayersBombs (ps);
	}
	ps->daleifing = 0;			/* (galatius) */
	ps->daleif = (RF_Daleif == playerflags) ? XBTrue : XBFalse;	/* (galatius) */

	ps->remote_control = XBFalse;
	ps->kick = XBFalse;
	ps->air_button = XBFalse;
	ps->frogger = (RF_Frogger == playerflags) ? XBTrue : XBFalse;	/*Skywalker */
	ps->jump_button = (RF_Jump == playerflags) ? XBTrue : XBFalse;	/*Skywalker */
	ps->evilill = 0;
	ps->stop = (RF_Stop == playerflags) ? XBTrue : XBFalse;	/*Skywalker */
	ps->suck_button = (RF_Suck == playerflags) ? XBTrue : XBFalse;	/*Skywalker */
	ps->phantom = (RF_Phantom == playerflags) ? GAME_TIME : XBFalse;	/*Skywalker */
	ps->electrify = (RF_Electrify == playerflags) ? EXTRA_ELECTRIFY_COUNT : XBFalse;	/*Skywalker */
	/* If special bombs are distributed, then zero the count */
	if (DistribSpecial ()) {
		ps->special_bombs = 0;
	}
	/* Reset extra pickup count */
	ps->num_extras = 0;
	/* reset inital extras */
	if (RF_RC == playerflags) {
		ps->remote_control = 1;
	}
	if (RF_Teleport == playerflags) {
		ps->teleport = 1;
	}

	if (RF_Kick & ps->revextra_flags) {
		ps->kick = 1;
	}
	if (RF_Morph == playerflags) {
		ps->num_morph = 1000;
	}
	if (RF_Through == playerflags) {
		ps->through = XBTrue;
		ps->throughCount = 255;
	}
	if (RF_Snipe == playerflags) {
		ps->num_snipe = 1000;
	}
	ps->speed = 0;

	if (RF_Revive == playerflags) {
		ps->revive = 1;
	}
	ps->choice_bomb_type = NUM_BMT;
	if (ps->local)
		ResetMessage ();
	if (RF_Choice == playerflags) {
		int h;
		for (h = ChoiceDefaultBomb; bomb_name_choice[h] == NULL; h = ((h + 1) % NUM_BMT)) ;
		ps->choice_bomb_type = h;
		if (ps->local && ps->lives) {
			char tutu[40];
			sprintf (tutu, "%s : ", p_string[ps->id].name);
			strcat (tutu, bomb_name_choice[(ps->choice_bomb_type)]);
			SetMessage (tutu, XBTrue);
		}
	}
	if (RF_Airpump == playerflags) {
		ps->air_button = 1;
	}
	if (RF_Cloak == playerflags) {
		ps->cloaking = -GAME_TIME;
	}
	/* if revived ignite the bombs! */
	if (ps->sniping == 1) {
		if (IgnitePlayersBombs (ps)) {
			ps->sniping = 1;
			ps->d_soll = GoStop;
		}
	}
}								/* RevivePlayer */

/*
 * 
 */
static void
DoStunned (BMPlayer * ps)
{
	switch ((ps->d_look + ps->stunned - 1) % 4 + GoStop + 1) {
	case GoDown:
		SetSpriteAnime (ps->sprite, SpriteStopDown);
		break;
	case GoUp:
		SetSpriteAnime (ps->sprite, SpriteStopUp);
		break;
	case GoLeft:
		SetSpriteAnime (ps->sprite, SpriteStopLeft);
		break;
	case GoRight:
		SetSpriteAnime (ps->sprite, SpriteStopRight);
		break;
	}

	ps->stunned--;
}								/* DoStunned */

void
DoEvilIll (void)
{
	BMPlayer *ps1;

	/* evil-ill countdown */
	for (ps1 = player_stat; ps1 < player_stat + numPlayer; ps1++) {
		if ((ps1->lives) && (ps1->evilill)) {
			if (ps1->evilill == 1)
				/* Too long! Take a hit. */
				ps1->dying = DEAD_TIME;
			ps1->evilill--;
		}
	}
}

/*
 *
 */
static void
DoDie (BMPlayer * ps)
{
	if (ps->dying == DEAD_TIME) {
		SetSpriteMode (ps->sprite, SPM_MAPPED);
	}
	ps->dying--;

	if (ps->lives > 1) {
		switch (ps->d_look) {
		case GoLeft:
			SetSpriteAnime (ps->sprite, SpriteDamagedLeft);
			break;
		case GoUp:
			SetSpriteAnime (ps->sprite, SpriteDamagedUp);
			break;
		case GoRight:
			SetSpriteAnime (ps->sprite, SpriteDamagedRight);
			break;
		default:
			SetSpriteAnime (ps->sprite, SpriteDamagedDown);
			break;
		}
	}
	else {
		switch (ps->d_look) {
		case GoLeft:
			SetSpriteAnime (ps->sprite, SpriteDeadLeft);
			break;
		case GoUp:
			SetSpriteAnime (ps->sprite, SpriteDeadUp);
			break;
		case GoRight:
			SetSpriteAnime (ps->sprite, SpriteDeadRight);
			break;
		default:
			SetSpriteAnime (ps->sprite, SpriteDeadDown);
			break;
		}
	}

    /* AbsInt start: */
    ps->ai_revived = 1;
    /* AbsInt end */
}								/* DoDie */

/*
 *
 */
XBBool
CheckPlayerNear (int x, int y)
{
	int player;

	for (player = 0; player < numPlayer; player++) {
		if ((ABS (x * BLOCK_WIDTH - player_stat[player].x) < BLOCK_WIDTH) &&
			(ABS (y * BLOCK_HEIGHT - BLOCK_HEIGHT - player_stat[player].y) < BLOCK_HEIGHT)) {
			return XBTrue;
		}
	}
	return XBFalse;
}								/* CheckPlayerNear */

/*
 *
 */
void
DoAllPlayers (int game_time, int *active_player)
{
	int spm_mode;
	int i, p, player;
	int plist[MAX_PLAYER];		// SMPF

	/* if time is over, kill them all */
	if (game_time == (GAME_TIME - DEAD_TIME + 1)) {
		for (player = 0; player < numPlayer; player++) {
			if (player_stat[player].lives > 0) {
				player_stat[player].lives = 1;
				player_stat[player].dying = DEAD_TIME;
			}
		}
	}

	for (i = 0; i < numPlayer; i++) {
		plist[i] = i;
	}
	for (i = numPlayer - 1; i > 0; i--) {
		p = GameRandomNumber (i + 1);
		player = plist[p];
		plist[p] = plist[i];
		plist[i] = player;
	}

	/* check player status */
	for (p = 0; p < numPlayer; p++) {
		/* to permute player when drawing and stunning */
		/* quick and dirty but hopefully it solves some problems */
		player = plist[p];

		if (player_stat[player].lives != 0) {

			switch (player_stat[player].dying) {
			case 0:
				/* player is alive and ... */
				if (player_stat[player].morphed) {
					/* ... or morphed */
					DoMorph (player_stat + player);
				}
				else if (player_stat[player].stunned) {
					if (player_stat[player].farted) {
						/* (galatius) */
						DoWalk (player_stat + player, game_time);
						DoStunned (player_stat + player);
						player_stat[player].farted--;
					}
					else {
						/* ... and stunned */
						DoStunned (player_stat + player);
					}
				}
				else {
					/* ... walks around */
					DoWalk (player_stat + player, game_time);
				}
				/* added by Galatius */
				if (player_stat[player].smelly) {
					player_stat[player].smelly--;
				}
				if (player_stat[player].daleifing) {
					player_stat[player].daleifing--;
				}
				break;

			case 1:
				/* try to revive player */
				RevivePlayer (player_stat + player, active_player);
				break;

			default:
				/* player is dying */
				DoDie (player_stat + player);
				break;
			}
		}
		else {
			if (player_stat[player].revive) {
				if (player_stat[player].cloaking < 0) {
					player_stat[player].cloaking++;
					if (player_stat[player].cloaking & 0x01) {
						spm_mode = player_stat[player].disp;
					}
					else {
						spm_mode = SPM_UNMAPPED;
					}
				}
				else
					spm_mode = SPM_MAPPED;
				SetSpriteMode (player_stat[player].sprite, spm_mode);
				SetSpriteAnime (player_stat[player].sprite, SpriteZombie);

			}
		}
	}
}								/* DoAllPlayers */

/*
 *
 */
void
CheckPlayerHit (void)
{
	int player;
	int gridx, gridy;

	for (player = 0; player < numPlayer; player++) {
		gridx = (player_stat[player].x + (BLOCK_WIDTH >> 1)) / BLOCK_WIDTH;
		gridy = (player_stat[player].y + (BLOCK_HEIGHT >> 1)) / BLOCK_HEIGHT + 1;
		if (0 != player_stat[player].lives &&
			0 == player_stat[player].invincible &&
			0 == player_stat[player].morphed &&
			0 == player_stat[player].dying && CheckExplosion (gridx, gridy)) {
			player_stat[player].dying = DEAD_TIME;
		}
	}
}								/* CheckPlayerHit */

/*
 * check if local players are away, make them bot, reset away flag
 */
void
Player_CheckLocalAway (void)
{
	BMPlayer *ps;
	for (ps = player_stat; ps < player_stat + numPlayer; ps++) {
		if (ps->localDisplay >= 0) {
			if (ps->away) {
				ps->bot = XBTrue;
			}
			else {
				ps->away = XBTrue;
			}
		}
	}
}								/* Player_CheckLocalAway */

/*
 * check if all local players are bots
 */
XBBool
Player_CheckLocalBot (void)
{
	BMPlayer *ps;
	for (ps = player_stat; ps < player_stat + numPlayer; ps++) {
		if (ps->localDisplay >= 0 && !ps->bot) {
			return XBFalse;
		}
	}
	return XBTrue;
}								/* Player_CheckLocalBot */

/*
 * determine action for all (local) bots
 */
void
Player_BotAction (PlayerAction * pa)
{
	BMPlayer *ps;
	for (ps = player_stat; ps < player_stat + numPlayer; ps++) {
		if (ps->bot && ps->localDisplay >= 0) {
			gestionBot (player_stat, pa, ps->id, numPlayer);
		}
	}
}								/* Player_BotAction */

/*
 * de/activate bot
 */
void
Player_ActivateBot (BMPlayer * ps, XBBool activate)
{
	assert (ps != NULL);
	ps->away = activate;
	if (ps->bot == activate) {
		return;
	}
	ps->bot = activate;
	if (activate) {
		SetMessage (N_("Bot Activated"), XBFalse);
	}
	else {
		SetMessage (N_("Bot Deactivated"), XBFalse);
	}
}								/* Player_ActivateBot */

/*
 * return if player is local bot
 */
XBBool
Player_isLocalBot (BMPlayer * ps)
{
	assert (ps != NULL);
	return (ps->bot && ps->localDisplay >= 0);
}								/* Player_isLocalBot */

/*
 * end of file player.c
 */
