/*
 * Programm XBLAST V1.2.14 or higher
 * (C) by Oliver Vogel (e-mail: vogel@ikp.uni-koeln.de)
 * May 9th 1996
 * started August 1993
 *
 * Bot by Didier PLANTET (e-mail: plantet@info.enserb.u-bordeaux.fr)
 * and Grégoire ROBERT (e-mail : robert@info.enserb.u-bordeaux.fr)
 * File: bot.c
 * Bot IA ...
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licences as published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Publis License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xblast.h"

#define _BOT_C
#define BOMB_STEP     2
#define NUM_FUSES 3

#define PROB_POSER       10
#define PROB_SWAP       100

#define BONUS_EXPLOSE    40
#define BONUS_TUE        60
#define BONUS_SIMPLE    100
#define BONUS_EXTRA     200
#define MALUS_MORT    -1000

#define SLOW_SPEED       16
#define NORMAL_SPEED      8
#define FAST_SPEED        4

typedef struct coord
{
	short x, y;
	short temps;
	short malus;
	struct coord *suivant;
} coord;

static int fuse_times[NUM_FUSES] = { SHORT_FUSE, BOMB_TIME, LONG_FUSE };

/* Tableau des bombes */
/*static Explosion *laby_bomb[MAZE_W][MAZE_H];*/
/* Liste des bombes */
static Explosion bombes_tmp[100];
/* Tableau des cases dangereuses : temps avant explosion */
static int laby_perf[MAZE_W][MAZE_H];
/* Nb de points dans chaque direction */
static int vote[NB_DIR];
/* Vaut VRAI si je pose */
static int vote_poser;
/* Vaut VRAI si je swappe */
static int vote_swap;
/* Vaut VRAI si j'effectue une action spéciale (appui sur +) */
static int vote_action_speciale;
/* Tableau des cases visitées (booléen) */
static int laby_visit[MAZE_W][MAZE_H];
/* Tableau des cases disparaissant */
static int laby_shrink[MAZE_W][MAZE_H];

static int perf_max[NB_DIR];
static coord *file;

static int gameTime;

static BMPlayer *statut_joueurs;
/* Mon numéro à moi */
static int num_bot;
static int nb_joueurs;
static int nb_ennemis_vivants;
static int temps_jeu;
/* La duree de deplacement entre 2 cases */
static int duree_deplacement;

/* Score accordé au bonus du niveau */
static int bonus_extra;

int dirige_vers_bonus (int x, int y, int dir);
										   /* Fouf lenteur -- *//* static int trace; */

void
SetBotTime (int game_time)
{
	gameTime = game_time;
}

coord *
creer_coord (int x, int y, int temps, int malus)
{
	coord *c = (coord *) malloc (sizeof (coord));
	c->x = x;
	c->y = y;
	c->temps = temps;
	c->malus = malus;
	c->suivant = NULL;

	return c;
}

void
enfiler (int x, int y, int temps, int malus)
{
	coord *tmpc;
	coord *c = creer_coord (x, y, temps, malus);

	if (file == NULL)
		file = c;
	else {
		tmpc = file;
		while (tmpc->suivant != NULL)
			tmpc = tmpc->suivant;
		tmpc->suivant = c;
	}
}

coord *
defiler (void)
{
	coord *c = file;

	file = file->suivant;

	return c;
}

/* Renvoie le nb de murs m'entourant */
int
test_murs (BMPlayer * ps)
{
	int x = ps->x / BLOCK_WIDTH;
	int y = ps->y / BLOCK_HEIGHT + 1;
	int nb_murs = 0;

	if (CheckMaze (x, y - 1)) {
		vote[UP] = MALUS_MORT;
		nb_murs++;
	}
	if (CheckMaze (x - 1, y)) {
		vote[LEFT] = MALUS_MORT;
		nb_murs++;
	}
	if (CheckMaze (x, y + 1)) {
		vote[DOWN] = MALUS_MORT;
		nb_murs++;
	}
	if (CheckMaze (x + 1, y)) {
		vote[RIGHT] = MALUS_MORT;
		nb_murs++;
	}

	return nb_murs;
}

/*Explosion *
foundBomb (int x, int y)
{
  Explosion *p;

  for (p = expl_list; p != NULL; p = p->next)
    if (p->x == x && p->y == y)
      return p; 
  
  return NULL;
}*/

Explosion *
bombe_trouvee_tmp (int x, int y)
{
	int i;
	for (i = 0; i < 100; i++)
		if (bombes_tmp[i].range != -1 && bombes_tmp[i].x == x && bombes_tmp[i].y == y)
			return &bombes_tmp[i];

	return NULL;
}

/*void
developpe_bombe (int range, int mazex, int mazey, int type)
{
  Explosion *p;
  int i,j;
    
  // right 
  for (i = 1; (i <= range) && CheckMazeOpen(mazex+i-1,mazey) ; i ++) 
    if (mazex+i < MAZE_W)
      if (laby_perf[mazex][mazey] + i < laby_perf[mazex+i][mazey])
	{
	  laby_perf[mazex+i][mazey] = laby_perf[mazex][mazey] + i;
	  p = CheckBomb (mazex+i, mazey);
	  if (p != NULL && p->dir == GoStop)
	    developpe_bombe (p->range, mazex+i, mazey, BMTnormal);
	}
  
	// Left 
  for (i = 1; (i <= range) && CheckMazeOpen(mazex-i+1,mazey) ; i ++) 
    if (mazex-i >= 0)
      if (laby_perf[mazex][mazey] + i < laby_perf[mazex-i][mazey])
	{
	  laby_perf[mazex-i][mazey] = laby_perf[mazex][mazey] + i;
	  p = CheckBomb (mazex-i, mazey);
	  if (p != NULL && p->dir == GoStop)
	    developpe_bombe (p->range, mazex-i, mazey, BMTnormal);
	}

	// Down 
  for (i = 1; (i <= range) && CheckMazeOpen(mazex,mazey+i-1) ; i ++) 
    if (mazey+i < MAZE_H)
      if (laby_perf[mazex][mazey] + i < laby_perf[mazex][mazey+i])
	{
	  laby_perf[mazex][mazey+i] = laby_perf[mazex][mazey] + i;
	  p = CheckBomb (mazex, mazey+i);
	  if (p != NULL && p->dir == GoStop)
	    developpe_bombe (p->range, mazex, mazey+i, BMTnormal);
	}
  
	// Up 
  for (i = 1; (i <= range) && CheckMazeOpen(mazex,mazey-i+1) ; i ++) 
    if (mazey-i >= 0)
      if (laby_perf[mazex][mazey] + i < laby_perf[mazex][mazey-i])
	{
	  laby_perf[mazex][mazey-i] = laby_perf[mazex][mazey] + i;
	  p = CheckBomb (mazex, mazey-i);
	  if (p != NULL && p->dir == GoStop)
	    developpe_bombe (p->range, mazex, mazey-i, BMTnormal);
	}
  
  
  if (type == BMTnapalm)
    for (i= -2; i<=2; i++) {
      if (mazex+i < MAZE_W && mazex+i >= 0)
	developpe_bombe (range/(ABS(i)+1), mazex+i, mazey, BMTnormal);
      if (mazey+i < MAZE_H && mazey+i >= 0)
	developpe_bombe (range/(ABS(i)+1), mazex, mazey+i, BMTnormal);
    }
  
  if (type == BMTgrenade)
    if (range == 1) {
      developpe_bombe(0, mazex-1, mazey-1, BMTblastnow);
      developpe_bombe(0, mazex+1, mazey-1, BMTblastnow);
      developpe_bombe(0, mazex-1, mazey+1, BMTblastnow);
      developpe_bombe(0, mazex+1, mazey+1, BMTblastnow);
    } else {
      for (i = -((range)-1); i<=((range)-1); i++) {
	for (j = -((range)-1); j<=((range)-1); j++) {
	  developpe_bombe (1, mazex+i, mazey+j, BMTblastnow);
	}
      }
    }
}*/

void
developpe_bombe_tmp (int range, int mazex, int mazey, int type, int count)
{
	// Explosion *p;
	int i, j;

	/* right */
	for (i = 1; (i <= range) && CheckMazeOpen (mazex + i - 1, mazey); i++)
		if (mazex + i < MAZE_W)
			if (laby_perf[mazex][mazey] + i < laby_perf[mazex + i][mazey])
				laby_perf[mazex + i][mazey] = laby_perf[mazex][mazey] + i * count;

	/* Left */
	for (i = 1; (i <= range) && CheckMazeOpen (mazex - i + 1, mazey); i++)
		if (mazex - i >= 0)
			if (laby_perf[mazex][mazey] + i < laby_perf[mazex - i][mazey])
				laby_perf[mazex - i][mazey] = laby_perf[mazex][mazey] + i * count;

	/* Down */
	for (i = 1; (i <= range) && CheckMazeOpen (mazex, mazey + i - 1); i++)
		if (mazey + i < MAZE_H)
			if (laby_perf[mazex][mazey] + i < laby_perf[mazex][mazey + i])
				laby_perf[mazex][mazey + i] = laby_perf[mazex][mazey] + i * count;

	/* Up */
	for (i = 1; (i <= range) && CheckMazeOpen (mazex, mazey - i + 1); i++)
		if (mazey - i >= 0)
			if (laby_perf[mazex][mazey] + i < laby_perf[mazex][mazey - i])
				laby_perf[mazex][mazey - i] = laby_perf[mazex][mazey] + i * count;

	if (type == BMTnapalm)
		for (i = -2; i <= 2; i++) {
			if (mazex + i < MAZE_W && mazex + i >= 0)
				developpe_bombe_tmp (range / (ABS (i) + 1), mazex + i, mazey, BMTnormal, count);
			if (mazey + i < MAZE_H && mazey + i >= 0)
				developpe_bombe_tmp (range / (ABS (i) + 1), mazex, mazey + i, BMTnormal, count);
		}

	if (type == BMTgrenade) {
		if (range == 1) {
			developpe_bombe_tmp (0, mazex - 1, mazey - 1, BMTblastnow, count);
			developpe_bombe_tmp (0, mazex + 1, mazey - 1, BMTblastnow, count);
			developpe_bombe_tmp (0, mazex - 1, mazey + 1, BMTblastnow, count);
			developpe_bombe_tmp (0, mazex + 1, mazey + 1, BMTblastnow, count);
		}
		else {
			for (i = -((range) - 1); i <= ((range) - 1); i++) {
				for (j = -((range) - 1); j <= ((range) - 1); j++) {
					developpe_bombe_tmp (1, mazex + i, mazey + j, BMTblastnow, count);
				}
			}
		}
	}
}

/* Traduit une direction en dx et dy */
void
conversion_direction (int dir, int *dx, int *dy)
{
	*dx = 0;
	*dy = 0;
	switch (dir) {
	case UP:
		*dy = -1;
		break;
	case DOWN:
		*dy = 1;
		break;
	case LEFT:
		*dx = -1;
		break;
	case RIGHT:
		*dx = 1;
	}
}

/* Les bonus extras sont-ils positifs ou négatifs ? */
void
def_score_extra (BMPlayer * ps)
{
	bonus_extra = BONUS_EXTRA;
	/* La tortue */
	if (specialExtraFunc == SpecialExtraSlow) {
		if (!ps->iniextra_flags) {
			bonus_extra = -BONUS_SIMPLE / 2;
		}
		else {
			if (!ps->revextra_flags)
				bonus_extra = -BONUS_SIMPLE / 2;
			else
				bonus_extra = 0;	/* Si je suis déjà lent, reprendre une tortue ne change rien */
		}
	}
	/* Le "bonus" invisible qui fait tourner */
	if (specialExtraFunc == SpecialExtraStunOthers) {
		bonus_extra = -2 * BONUS_SIMPLE;
	}
	/* La seringue */
	if (specialExtraFunc == SpecialExtraJunkie && ps->junkie == 0) {
		/* Si j'y ai déjà touché, j'ai intérêt à en reprendre... */
		bonus_extra = -5 * BONUS_SIMPLE;
	}
	/* La mort immédiate */
	if (specialExtraFunc == SpecialExtraPoison) {
		bonus_extra = -10 * BONUS_SIMPLE;
	}

}

/* Un bonus sur la case ? */
int
score_bonus (int x, int y)
{
	int bonus;

	bonus = GetBlockExtra (x, y);

	if (bonus == BTBomb || bonus == BTRange)
		bonus = BONUS_SIMPLE + 200;
	else if (bonus == BTSpecial || bonus == BTExtra) {
		if (specialExtraFunc != SpecialExtraPoison) {
			bonus = bonus_extra;
		}
		else {
			bonus = -BONUS_SIMPLE;
		}

	}
	else						/* if (bonus == BTSwapper)
								   bonus = BONUS_EXTRA;
								   else */ if (bonus == BTEvil)
		bonus = -BONUS_SIMPLE;

	else
		bonus = 0;
	return bonus;
}

/* Est-il intéressant de poser sur cette case ? */
int
score_explose (int x, int y, int portee_bombe)
{
	int explose = 0;
	int dir, dx, dy;
	int i;

	for (dir = 1; dir < NB_DIR; dir++) {
		conversion_direction (dir, &dx, &dy);
		i = 1;
		while (CheckMazeFree (x + i * dx, y + i * dy) && i <= portee_bombe)
			i++;
		/* Y a-t-il un mur à détruire ? Et sinon, un bonus ? */
		if (GetBlockExtra (x + i * dx, y + i * dy) == BTExtra)
			explose += BONUS_EXPLOSE;
		else
			explose -= (score_bonus (x + i * dx, y + i * dy) / 4);
	}
	explose += 75;
	return explose;
}

/* Est-ce que poser ici va menacer un autre joueur ? */
int
score_tue (int x, int y, int portee_bombe)
{
	int tue = 0;
	int xmin = -1, xmax = -1, ymin = -1, ymax = -1;
	int j, jx, jy;
	int points;
	BMPlayer *ps;

	/* Quelles cases puis-je atteindre ? */
	for (j = 1; j <= portee_bombe; j++) {
		if (xmin == -1 && (!CheckMazeFree (x - j, y) || j == portee_bombe))
			xmin = x - j;
		if (xmax == -1 && (!CheckMazeFree (x + j, y) || j == portee_bombe))
			xmax = x + j;
		if (ymin == -1 && (!CheckMazeFree (x, y - j) || j == portee_bombe))
			ymin = y - j;
		if (ymax == -1 && (!CheckMazeFree (x, y + j) || j == portee_bombe))
			ymax = y + j;
	}
	nb_ennemis_vivants = 0;
	for (j = 0; j < nb_joueurs; j++) {
		ps = statut_joueurs + j;

		/* Le joueur n'est pas le bot et vit encore */
		if ((j != num_bot) && (ps->lives != 0)) {
			jx = ps->x / BLOCK_WIDTH;
			jy = ps->y / BLOCK_HEIGHT + 1;

			points = BONUS_TUE;

			/* Si le joueur est mon partenaire.... je vais éviter de le tuer ! */
			if (ps->team == (statut_joueurs + num_bot)->team)
				points = -points;
			else
				nb_ennemis_vivants++;

			/* Je suis sur la même colonne */
			if ((x == jx) && ((ymin <= jy) && (jy <= ymax)))
				tue += points;

			/* Je suis sur la même ligne (et pas sur la même colonne) */
			if ((y == jy) && (x != jx) && ((xmin <= jx) && (jx <= xmax)))
				tue += points;

			/* Si le joueur a une mauvaise mort (poseuse, lenteur, mini, non-poseuse,
			   malfunction, teleport, ou *seringue*), je dois l'éviter */
			if ((x == jx) && (y == jy)) {
				if ((ps->illness == IllBomb) ||
					(ps->illness == IllSlow) ||
					(ps->illness == IllMini) ||
					(ps->illness == IllEmpty) ||
					(ps->illness == IllMalfunction) || (ps->illness == IllTeleport) || (ps->junkie))
					tue -= BONUS_SIMPLE;
			}
			/* sinon, j'essaie toujours de m'en approcher (s'il n'est pas mon partenaire) */
			else
				tue += points;
		}
	}
	return tue;
}

/* Est-ce que je risque de me faire shrinker sur cette case ? */
int
score_shrink (int x, int y, int temps)
{
	int score = 0;

	temps += temps_jeu;
	if (laby_shrink[x][y] - temps <= 16)
		score = MALUS_MORT;
	else if (laby_shrink[x][y] - temps <= 50)
		score = -BONUS_EXTRA;
	else if (laby_shrink[x][y] - temps <= 100)
		score = -BONUS_SIMPLE;

	return score;
}

/* Teste le danger et l'intérêt d'une case... puis poursuit le parcours par niveau */
void
teste_case (coord * c, int dir_init, BMPlayer * ps)
{
	int perf = laby_perf[c->x][c->y] - c->temps;
	int portee_bombe = (ps->illness == IllMini) ? 1 : ps->range;
	int score, malus;
	int invincible = ps->invincible - c->temps;
	int tourne = ps->stunned - c->temps;
	int dir, dx, dy;

	if (invincible > 0)
		perf += invincible;
	if (tourne < 0)
		tourne = 0;

	/* Une bombe explosera sur le passage */
	if (perf <= 4)
		return;

	/* La case disparaîtra */
	if (score_shrink (c->x, c->y, c->temps) == MALUS_MORT)
		return;

	score = score_bonus (c->x, c->y);
	if (score <= -500)
		return;

	/* Mise à jour du malus (score négatif le + faible du chemin) */
	malus = MIN (c->malus, score);

	/* Calcul du score de la case */
	score += score_explose (c->x, c->y, portee_bombe);
	score += score_tue (c->x, c->y, portee_bombe);
	score += score_shrink (c->x, c->y, c->temps);

	/* Mise à jour de la performance maximale de la direction dir_init */
	perf += score;
	if (perf > 10 * LONG_FUSE)
		perf += malus;
	perf_max[dir_init] = MAX (perf_max[dir_init], perf);

	for (dir = 1; dir < NB_DIR; dir++) {
		conversion_direction (dir, &dx, &dy);
		/* Si je ne suis pas déjà passé par là, s'il n'y a ni mur, ni bombe, je place la case dans la file */
		if (!laby_visit[c->x + dx][c->y + dy] && !CheckMaze (c->x + dx, c->y + dy)
			) {
			if (!ps->kick) {
				if (!CheckBomb (c->x + dx, c->y + dy)) {
					enfiler (c->x + dx, c->y + dy, c->temps + duree_deplacement + tourne, malus);
					laby_visit[c->x + dx][c->y + dy] = 1;
				}

			}
			else {
				enfiler (c->x + dx, c->y + dy, c->temps + duree_deplacement + tourne, malus);
				laby_visit[c->x + dx][c->y + dy] = 1;
			}
		}
	}
}

/* Remplissage du tableau laby_shrink (en début de niveau) */
void
init_laby_shrink ()
{
	ShrinkGeneric *shrink_ptr = GetShrinkPtr ();
	XBScrambleData *scramble = GetScrDraw ();
	int temps;
	int x, y;
	unsigned mask;

	for (x = 0; x < MAZE_W; x++)
		for (y = 0; y < MAZE_H; y++)
			laby_shrink[x][y] = 2 * GAME_TIME;

	/* La liste pointée par shrink_ptr se termine par une cellule (2 * GAME_TIME, 0, 0) */
	while (shrink_ptr->time < 2 * GAME_TIME) {
		temps = shrink_ptr->time;
		x = shrink_ptr->x;
		y = shrink_ptr->y;
		if (laby_shrink[x][y] > temps)
			laby_shrink[x][y] = temps;
		shrink_ptr++;
	}

	/* Blocs surgissant sur le plateau */
	temps = scramble->time - 4;
	for (y = 0; y < MAZE_H; y++) {
		if (0 != scramble->row[y]) {
			for (x = 0, mask = 1; x < MAZE_W; x++, mask <<= 1) {
				if (mask & scramble->row[y]) {
					if (laby_shrink[x][y] > temps)
						laby_shrink[x][y] = temps;
				}
			}
		}
	}

/*  for (y = 0; y < MAZE_H; y++) {
      for (x = 0; x < MAZE_W; x++)
	fprintf (stderr, "%d ", laby_shrink[x][y]);
      fprintf (stderr, "\n");
    }*/
}

/* Aucune case n'a été visitée (début d'un parcours par niveau) */
void
init_laby_visit (int x, int y)
{
	int mazex, mazey;

	for (mazex = 0; mazex < MAZE_W; mazex++)
		for (mazey = 0; mazey < MAZE_H; mazey++)
			laby_visit[mazex][mazey] = 0;

/*  memset (&laby_visit, 0, MAZE_W * MAZE_H * sizeof (int));*/

	laby_visit[x][y] = 1;
}

static BMDirection turn_clockwise[MAX_DIR] = {
	GoStop, GoRight, GoUp, GoLeft, GoDown, GoDefault
};

static BMDirection turn_anticlockwise[MAX_DIR] = {
	GoStop, GoLeft, GoDown, GoRight, GoUp, GoDefault
};

static BMDirection turn_opposite[MAX_DIR] = {
	GoStop, GoDown, GoRight, GoUp, GoLeft, GoDefault
};

void
calc_laby_perf ()
{
	int player;
	int i, j, k;
	int dx, dy;
	int temps;
	Explosion *p;
	Explosion *p2;

	/* On traite d'abord les bombes en train d'exploser (count > 0) */
	for (i = 0; i < 100; i++) {
		p = &bombes_tmp[i];

		if (p->range == -1)
			continue;

		if (p->count < 0)
			continue;

		if (laby_perf[p->x][p->y] > -p->count) {
			laby_perf[p->x][p->y] = -p->count;
			developpe_bombe_tmp (p->range, p->x, p->y, p->type, -p->count);
		}
		else
			developpe_bombe_tmp (p->range, p->x, p->y, BMTnormal, -p->count);

		p->range = -1;
	}

	for (temps = 0; temps < LONG_FUSE; temps++) {	/* On deplace case par case */
		for (i = 0; i < 100; i++) {	/* Pour chaque bombe */
			p = &bombes_tmp[i];

			if (p->range == -1)
				continue;

			if (temps == -p->count || laby_perf[p->x][p->y] <= temps) {	/* On explose la */
				if (laby_perf[p->x][p->y] > -p->count) {
					laby_perf[p->x][p->y] = -p->count;
					developpe_bombe_tmp (p->range, p->x, p->y, p->type, -p->count);
				}
				else
					developpe_bombe_tmp (p->range, p->x, p->y, BMTnormal, -p->count);

				p->range = -1;
				continue;
			}

			if (p->dir == GoStop) {
				continue;
			}
			conversion_direction (p->dir, &dx, &dy);
			/* Si on cogne une bombe */
			if (p->dx * dx >= 0 && p->dy * dy >= 0 && bombe_trouvee_tmp (p->x + dx, p->y + dy)) {
				if (doBombClick == BombClickNone) {
					p->dir = GoStop;
					p->dx = 0;
					p->dy = 0;
				}
				else if (doBombClick == BombClickInitial) {
					p->dir = initialBombDir;
					conversion_direction (p->dir, (int *)&p->dx, (int *)&p->dy);
				}
				else if (doBombClick == BombClickThru) {
				}
				else if (doBombClick == BombClickSnooker) {
					p2 = bombe_trouvee_tmp (p->x + dx, p->y + dy);
					p2->dir = p->dir;
					/* On met a 0 l'offset de l'autre direction */
					p2->dx = p2->dx * ABS (dx);
					p2->dy = p2->dy * ABS (dy);
					/* La bombe qui cogne arrete de bouger */
					p->dir = GoStop;
					p->dx = 0;
					p->dy = 0;
				}
				else if (doBombClick == BombClickContact) {
					p->dir = GoStop;
					p->dx = 0;
					p->dy = 0;
					if (laby_perf[p->x][p->y] > -p->count)
						laby_perf[p->x][p->y] = -p->count;
					developpe_bombe_tmp (p->range, p->x, p->y, p->type, -p->count);
					bombes_tmp[i].range = -1;
				}
				else if (doBombClick == BombClickContact) {
					p->dir = turn_clockwise[p->dir];
					p->dx = 0;
					p->dy = 0;
				}
				else if (doBombClick == BombClickAnticlockwise) {
					p->dir = turn_anticlockwise[p->dir];
					p->dx = 0;
					p->dy = 0;
				}
				else if (doBombClick == BombClickRandomdir) {
					p->dir = (int)(((float)rand () / (RAND_MAX + 1.0)) * (4)) + 1;
					p->dx = 0;
					p->dy = 0;
				}
				else if (doBombClick == BombClickRebound) {
					p->dir = turn_opposite[p->dir];
				}
			}
			/* Ou un mur */
			else if (p->dx == 0 && p->dy == 0 && !CheckMazeFree (p->x + dx, p->y + dy)) {
				if (doWallClick == BombClickNone) {
					p->dir = GoStop;
					p->dx = 0;
					p->dy = 0;
				}
				else if (doWallClick == BombClickInitial) {
					p->dir = initialBombDir;
					conversion_direction (p->dir, (int *)&p->dx, (int *)&p->dy);
				}
				else if (doWallClick == BombClickThru) {
				}
				else if (doWallClick == BombClickContact) {
					p->dir = GoStop;
					p->dx = 0;
					p->dy = 0;
					if (laby_perf[p->x][p->y] > -p->count)
						laby_perf[p->x][p->y] = -p->count;
					developpe_bombe_tmp (p->range, p->x, p->y, p->type, -p->count);
					bombes_tmp[i].range = -1;
				}
				else if (doWallClick == BombClickContact) {
					p->dir = turn_clockwise[p->dir];
					p->dx = 0;
					p->dy = 0;
				}
				else if (doWallClick == BombClickAnticlockwise) {
					p->dir = turn_anticlockwise[p->dir];
					p->dx = 0;
					p->dy = 0;
				}
				else if (doWallClick == BombClickRandomdir) {
					p->dir = (BMDirection) (int)((float)rand () / (RAND_MAX + 1.0)) * MAX_DIR;
					p->dx = 0;
					p->dy = 0;
				}
				else if (doWallClick == BombClickRebound) {
					p->dir = turn_opposite[p->dir];
				}
			}
			/* Un joueur */
			else {

				for (player = 0; player < nb_joueurs; player++) {
					if ((statut_joueurs[player].invincible == 0)
						&& (ABS (p->x * BLOCK_WIDTH + p->dx - statut_joueurs[player].x)
							< BOMB_STUN_X)
						&& (ABS (p->y * BLOCK_HEIGHT + p->dy
								 - statut_joueurs[player].y - BLOCK_HEIGHT) < BOMB_STUN_Y))
						break;
				}

				if (player < nb_joueurs) {
					if (doPlayerClick == BombClickNone) {
						p->dir = GoStop;
						p->dx = 0;
						p->dy = 0;
					}
					else if (doPlayerClick == BombClickInitial) {
						p->dir = initialBombDir;
						conversion_direction (p->dir, (int *)&p->dx, (int *)&p->dy);
					}
					else if (doPlayerClick == BombClickThru) {
					}
					else if (doPlayerClick == BombClickContact) {
						p->dir = GoStop;
						p->dx = 0;
						p->dy = 0;
						if (laby_perf[p->x][p->y] > -p->count)
							laby_perf[p->x][p->y] = -p->count;
						developpe_bombe_tmp (p->range, p->x, p->y, p->type, -p->count);
						bombes_tmp[i].range = -1;
					}
					else if (doPlayerClick == BombClickContact) {
						p->dir = turn_clockwise[p->dir];
						p->dx = 0;
						p->dy = 0;
					}
					else if (doPlayerClick == BombClickAnticlockwise) {
						p->dir = turn_anticlockwise[p->dir];
						p->dx = 0;
						p->dy = 0;
					}
					else if (doPlayerClick == BombClickRandomdir) {
						p->dir = (int)(((float)rand () / (RAND_MAX + 1.0)) * (4)) + 1;
						p->dx = 0;
						p->dy = 0;
					}
					else if (doPlayerClick == BombClickRebound) {
						p->dir = turn_opposite[p->dir];
					}

				}
			}

			conversion_direction (p->dir, &dx, &dy);

			p->dy += dy * BOMB_VY;
			p->dx += dx * BOMB_VX;

			if (p->dx * dx >= BLOCK_WIDTH / 2) {
				p->dx -= dx * BLOCK_WIDTH;
				p->x += dx;
			}

			if (p->dy * dy >= BLOCK_HEIGHT / 2) {
				p->dy -= dy * BLOCK_HEIGHT;
				p->y += dy;
			}
			if (p->x < 0)
				p->x = 0;
			if (p->y < 0)
				p->y = 0;

			/* On aime pas tres bien etre sur la trajectoire d'une bombe */
			laby_perf[p->x][p->y] = MIN (laby_perf[p->x][p->y], 50 * LONG_FUSE - 500 + temps * 4);

		}

		for (i = 0; i < 100; i++) {	/* Pour chaque bombe */
			p = &bombes_tmp[i];

			if (p->range == -1)
				continue;

			/* Les bombes en train d'exploser ne se repoduisent pas ... */
			if (p->count <= 0)
				continue;

			if (p->type == BMTfungus) {	/* On developpe les bombes fungus ... */
				if (p->count == (fuse_times[curBombTime] * 3 / 5)) {

					for (j = -1; j <= 1; j++) {
						if ((p->x + j < MAZE_W) && (p->x + j > -1)
							&& !CheckMazeSolid (p->x + j, p->y)) {
							/* On trouve un emplacement libre ... */
							for (k = 0; k < 100; k++)
								if (bombes_tmp[k].range == -1)
									break;
							bombes_tmp[k].x = p->x + j;
							bombes_tmp[k].y = p->y;
							bombes_tmp[k].range = p->range;
							bombes_tmp[k].type = BMTfungus;
							bombes_tmp[k].dir = GoStop;
							bombes_tmp[k].count = fuse_times[curBombTime];
						}

						if ((p->y + j < MAZE_H) && (p->y + j > -1)
							&& !CheckMazeSolid (p->x, p->y + j)) {
							/* On trouve un emplacement libre ... */
							for (k = 0; k < 100; k++)
								if (bombes_tmp[k].range == -1)
									break;
							bombes_tmp[k].x = p->x;
							bombes_tmp[k].y = p->y + j;
							bombes_tmp[k].range = p->range;
							bombes_tmp[k].type = BMTfungus;
							bombes_tmp[k].dir = GoStop;
							bombes_tmp[k].count = fuse_times[curBombTime];
						}
					}
				}
			}
		}
	}
}

void
init_laby_perf ()
{
	int i;
	int mazex, mazey;
	Explosion *p;

	for (mazex = 0; mazex < MAZE_W; mazex++)
		for (mazey = 0; mazey < MAZE_H; mazey++)
			laby_perf[mazex][mazey] = 50 * LONG_FUSE;

	for (i = 0; i < 100; i++)
		bombes_tmp[i].range = -1;

	for (p = exploList, i = 1; p != NULL; p = p->next, i++)
		bombes_tmp[i] = *p;

	calc_laby_perf ();
}

void
init_laby_perf_pose (int x, int y, int range, int count, int type)
{
	int i;
	int mazex, mazey;
	Explosion *p;

	for (mazex = 0; mazex < MAZE_W; mazex++)
		for (mazey = 0; mazey < MAZE_H; mazey++)
			laby_perf[mazex][mazey] = 50 * LONG_FUSE;

	for (i = 0; i < 100; i++)
		bombes_tmp[i].range = -1;

	for (p = exploList, i = 1; p != NULL; p = p->next, i++)
		bombes_tmp[i] = *p;

	bombes_tmp[i].x = x;
	bombes_tmp[i].y = y;
	bombes_tmp[i].range = range;
	bombes_tmp[i].count = -count;
	bombes_tmp[i].type = type;

	calc_laby_perf ();

}

void
init_laby_perf_biscotte ()
{
	int i;
	int mazex, mazey;
	Explosion *p;

	for (mazex = 0; mazex < MAZE_W; mazex++)
		for (mazey = 0; mazey < MAZE_H; mazey++)
			laby_perf[mazex][mazey] = 50 * LONG_FUSE;

	for (i = 0; i < 100; i++)
		bombes_tmp[i].range = -1;

	for (p = exploList, i = 1; p != NULL; p = p->next, i++) {
		bombes_tmp[i] = *p;
		if (p->player)
			if (p->player->id == num_bot)
				bombes_tmp[i].count = 0;
	}

	calc_laby_perf ();
}

void
parcours_par_niveau (BMPlayer * ps)
{
	coord *c;
	int dir;
	int x = ps->x / BLOCK_WIDTH;
	int y = ps->y / BLOCK_HEIGHT + 1;
	int dx, dy;
	int malus;
	int portee_bombe = (ps->illness == IllMini) ? 1 : ps->range;

	/* for (dir = 0; dir < NB_DIR; dir++)
	   perf_max[dir] = 0;
	 */
	perf_max[STOP] = score_shrink (x, y, ps->stunned);
	/* Ajout de la sécurité et de l'invincibilité du joueur si la case ne se fait pas shrinker immédiatement */
	if (perf_max[STOP] > MALUS_MORT)
		perf_max[STOP] += laby_perf[x][y] + ps->invincible;

	/* Ajout des bonus explose et tue si la case est sûre */
	if (perf_max[STOP] == 50 * LONG_FUSE) {
		perf_max[STOP] += score_explose (x, y, portee_bombe);
		perf_max[STOP] += score_tue (x, y, portee_bombe);
	}
	perf_max[STOP] += 60;

	for (dir = 1; dir < NB_DIR; dir++) {
		conversion_direction (dir, &dx, &dy);
		init_laby_visit (x, y);
		/* Eviter mur et shrink immédiat */
		if (!CheckMaze (x + dx, y + dy) && score_shrink (x + dx, y + dy, ps->stunned) > MALUS_MORT
			&& (dirige_vers_bonus (x, y, dir) != -1))
			if (!CheckBomb (x + dx, y + dy)) {
				malus = MIN (0, score_bonus (x + dx, y + dy));
				enfiler (x + dx, y + dy, duree_deplacement + ps->stunned, malus);
				while (file) {
					c = defiler ();
					teste_case (c, dir, ps);
					free (c);
				}
			}
	}
}

/* Renvoie VRAI s'il est possible et intéressant de poser une bombe */
int
souhaite_poser_bombe (BMPlayer * ps)
{
	int score;
	int mazex, mazey;
	int portee_bombe = (ps->illness == IllMini) ? 1 : ps->range;
	// int j;

	mazex = ps->x / BLOCK_WIDTH;
	mazey = ps->y / BLOCK_HEIGHT + 1;

	/* Est-il possible de poser ? */

	/* Pas de bombe sur la case */
	if (CheckBomb (mazex, mazey))
		return XBFalse;

	/* Je n'ai ni la non poseuse, ni la téléporte, ni la mal-function */
	if (ps->illness == IllEmpty || ps->illness == IllTeleport || ps->illness == IllMalfunction)
		return XBFalse;

	/* Il me reste des bombes... */
	if (ps->bombs == 0)
		return XBFalse;

	/* Est-il intéressant de poser ? */
	score = score_explose (mazex, mazey, portee_bombe) + score_tue (mazex, mazey, portee_bombe);

	//  fprintf(stderr," socre expl %i \n",score);
	if (score < 0)
		return XBFalse;

	/* Aucune raison particulière de poser... au hasard près */
	if (score == 0 && (int)(((float)rand () / (RAND_MAX + 1.0)) * (PROB_POSER)) != 0)
		return XBFalse;

	/* S'il reste au moins un autre joueur vivant, j'essaie de poser */
	return (nb_ennemis_vivants > 0);
}

/* Renvoie VRAI si je me dirige vers un bonus */
int
dirige_vers_bonus (int x, int y, int dir)
{
	int dx, dy;

	conversion_direction (dir, &dx, &dy);
	if ((CheckBonuses2 (x + dx, y + dy)) && (specialExtraFunc == SpecialExtraPoison)) {	//fprintf(stderr," field  poisoned\n");
		return -1;				// POISON!!! 
	}
	else {

		return CheckBonuses (x + dx, y + dy);
	}

}

/* Choix de la meilleure direction */
static void
choix_direction (int *choix_dir, int *max_perf)
{
	int dir = 0, dir1 = 0;
	static int old_dir = 0;
	*choix_dir = STOP;
	*max_perf = 0;
	for (dir = 0; dir < NB_DIR; dir++) {
		/* Ajout d'un petit bonus aléatoire si la case est sûre */
		if (perf_max[dir] > 10 * LONG_FUSE)
			perf_max[dir] += (int)(((float)rand () / (RAND_MAX + 1.0)) * (10));	//
		//  GameRandomNumber1 (10);

		/* Je choisis la destination ayant la meilleure performance */
		// if(dir==old_dir && dir!=0) perf_max[dir]+=50;
		if (dir == old_dir && dir == 0)
			perf_max[dir] -= 100;
		if (perf_max[dir] > *max_perf) {
			*max_perf = perf_max[dir];
			*choix_dir = dir;
		}
		else if ((perf_max[dir] == *max_perf) &&
				 ((int)((float)rand () / (RAND_MAX + 1.0)) * (5) == dir))
			*choix_dir = dir;
	}
	for (dir = 0; dir < NB_DIR; dir++) {

		for (dir1 = 0; dir1 < NB_DIR; dir1++) {
			if (perf_max[dir] == perf_max[dir1] && dir != dir1) {
				if (dir < dir1) {
					if (dir == old_dir)
						perf_max[dir] += 75;
					else
						perf_max[dir]++;

				}
			}
		}
	}
	for (dir = 0; dir < NB_DIR; dir++) {
		//    fprintf(stderr,"dir %i prfmax %i olddir %i\n",dir, perf_max[dir],old_dir);
		if (perf_max[dir] > *max_perf) {
			*max_perf = perf_max[dir];
			*choix_dir = dir;
		}

	}
	for (dir = 0; dir < NB_DIR; dir++) {
		perf_max[dir] = 0;
	}
	old_dir = *choix_dir;
}

/* Renvoie VRAI si je décide d'utiliser la télécommande MAINTENANT */
int
test_biscotte (BMPlayer * ps)
{
	int declenche = XBFalse;
	int mes_bombes = 0;
	int securite_init;
	Explosion *bombe;
	int x, y;

	x = ps->x / BLOCK_WIDTH;
	y = ps->y / BLOCK_HEIGHT + 1;
	securite_init = laby_perf[x][y];

	/* Parcours de toutes les bombes du plateau */
	for (bombe = exploList; bombe != NULL; bombe = bombe->next)
		/* Si cette bombe m'appartient... */
		if (bombe->player == ps) {
			mes_bombes++;
			/* ... je la fais exploser ! */
			/*      laby_perf[bombe->x][bombe->y] = 0;
			   developpe_bombe (bombe->range, bombe->x, bombe->y, bombe->type); */
			init_laby_perf_biscotte ();
		}
	if ((mes_bombes > 0) && ((laby_perf[x][y] == securite_init) || (ps->invincible > 5)))
		declenche = XBTrue;

	return declenche;
}

/* Renvoie VRAI si j'ai envie de swapper MAINTENANT.*/
int
test_swap (BMPlayer * ps, int perf)
{
	int vote_swap = XBFalse;
	int partenaire;
	int equipe_complete = XBFalse;
	int joueur;

	/* Swap aléatoire si j'ai au moins 2 swaps */
	/*  if (ps->swapposition > 1)
	   vote_swap = (GameRandomNumber1 (PROB_SWAP) == 0); */

	/* Si nous jouons par équipe */
	if (statut_joueurs->team == (statut_joueurs + 1)->team) {
		partenaire = ((num_bot) / 2) * 2 + 1 - (num_bot % 2);
		/* Si mon partenaire est mort */
		if ((statut_joueurs + partenaire)->lives == 0) {
			for (joueur = 0; joueur < nb_joueurs; joueur += 2)
				/* Si les deux joueurs d'une autre équipe sont encore vivants. */
				if (((statut_joueurs + joueur)->team != ps->team)
					&& ((statut_joueurs + joueur)->lives > 0)
					&& ((statut_joueurs + joueur + 1)->lives > 0))
					equipe_complete = XBTrue;
			/* S'il n'y a pas d'équipe complète... je swappe ! */
			if (!equipe_complete)
				vote_swap = XBTrue;
		}
	}
	/* Je swappe si je suis coincé entre 4 murs (et que l'on ne joue pas par équipe) */
	else if (ps->teleport == 0 && test_murs (ps) == 4)
		vote_swap = XBTrue;

	/* Je swappe si je vais mourir */
	if (ps->dying || perf <= duree_deplacement)
		vote_swap = XBTrue;

	return vote_swap;
}

void
actions (BMPlayer * ps)
{
	int i;
	int dx = 0, dy = 0;
	static int olddx = 0, olddy = 0;
	int dir;
	int max_perf;
	int choix_dir, prec_choix_dir;
	int x, y;
	int duree_meche, portee_bombe, type_bombe;

	x = ps->x / BLOCK_WIDTH;
	y = ps->y / BLOCK_HEIGHT + 1;
	portee_bombe = (ps->illness == IllMini) ? 1 : ps->range;
	type_bombe = BMTdefault;
	def_score_extra (ps);

	/* Remplissage du tableau my_maze */
	init_laby_perf ();

	/* Je cherche les cases les plus sûres, en partant dans toutes les directions */
	parcours_par_niveau (ps);

	/* Choix de la meilleure direction */
	choix_direction (&choix_dir, &max_perf);
	prec_choix_dir = choix_dir;

	/* Et si j'essayais de poser une tit' bombe ? */
	vote_poser = XBFalse;
	if (souhaite_poser_bombe (ps)) {
		/* Calcul de la durée de mèche */
		duree_meche = fuse_times[curBombTime];
		/* Calcul des effets de l'explosion de cette bombe */
		/*    if (laby_perf[x][y] > duree_meche)
		   laby_perf[x][y] = duree_meche;
		   developpe_bombe (portee_bombe, x, y, type_bombe); */
		//  init_laby_perf_pose (x, y, portee_bombe, duree_meche, type_bombe);

		/* Je cherche les cases les plus sûres, en partant dans toutes les directions */
		parcours_par_niveau (ps);
		/* Est-ce que poser me laisse la vie sauve ? */
		for (dir = 0; dir < NB_DIR; dir++) {
			if (perf_max[dir] > 10 * LONG_FUSE || duree_meche < ps->invincible) {
				vote_poser = XBTrue;
				break;
			}
			//      fprintf(stderr," perf_max %i ",perf_max[dir]);

		}
		//    fprintf(stderr," dest %i \n",10 * LONG_FUSE);
		/* Si oui, je pose... et choisis la direction de départ */
		if (vote_poser) {
			choix_direction (&choix_dir, &max_perf);
			/* Je ne pose pas si j'ai décidé d'aller vers un bonus (risque de mourir) */
			/* Remarque : méthode provisoire à améliorer... */
			if (dirige_vers_bonus (x, y, choix_dir)) {
				//  fprintf(stderr, " go for bonus!\n");

				vote_poser = XBFalse;
				//  choix_dir = prec_choix_dir;
			}
			/*je ne pose pas si il ny a place pour ca (Skywalker) */
			if (!(ps->remote_control > 0)) {
				i = 1;
				dx = 1;
				conversion_direction (choix_dir, &dx, &dy);
				if (!CheckBomb (x - olddx, y - olddy))
					i = 0;
				/* pas verifier si ma position est libre */

				if (dx - 1 != 0 && dy != 0)
					if (		//CheckBomb (x-olddx, y-olddy)&&
						   CheckMazeFree2 (x + dx - 1, y + dy) && !CheckBomb (x + dx - 1, y + dy)) {
						i = 0;
					}

				if (dx != 0 && dy + 1 != 0)
					if (		//CheckBomb (x-olddx, y-olddy)&&
						   CheckMazeFree2 (x + dx, y + dy + 1) && !CheckBomb (x + dx, y + dy + 1)) {
						i = 0;
					}
				if (dx != 0 && dy - 1 != 0)
					if (		//CheckBomb (x-olddx, y-olddy)&&
						   CheckMazeFree2 (x + dx, y + dy - 1) && !CheckBomb (x + dx, y + dy - 1)) {
						i = 0;
					}
				if (dx + 1 != 0 && dy != 0)
					/* verifier si il y a place pour escape de la bomb posee */
					if (		//CheckBomb (x-olddx, y-olddy)&&
						   CheckMazeFree2 (x + dx + 1, y + dy) && !CheckBomb (x + dx + 1, y + dy)) {
						i = 0;
					}

				if (i) {
					vote_poser = XBFalse;
				}
				if (olddx == -dx && olddy == -dy) {
					vote_poser = XBFalse;
				}
				if (ps->invincible > 5 && ps->remote_control > 0) {
					vote_poser = XBTrue;
				}
			}
			//  if(
		}
	}

	/* Si je vais mourir, j'essaie de toute façon de poser et de changer de case. */
	if (max_perf <= 30) {
		vote[STOP] -= 300;
		vote_poser = XBTrue;
	}

	/* Est-ce que j'effectue une action spéciale ? */
	vote_action_speciale = XBFalse;

	/* Si j'ai la télécommande (biscotte pour les intimes) */
	if (ps->remote_control > 0 && (max_perf > 10 * LONG_FUSE || ps->invincible > 5))
		if (test_biscotte (ps)) {
			vote_action_speciale = XBTrue;
			choix_dir = STOP;
			vote_poser = XBFalse;
		}
	/* Si je peux morpher, je morphe */
	if (ps->num_morph > 0) {
		vote_action_speciale = XBTrue;
	}
	/* Si je peux me téléporter et que je risque de mourir */
	if (ps->teleport > 0 && max_perf < 50) {
		vote_action_speciale = XBTrue;
		choix_dir = STOP;
		vote_poser = XBFalse;
	}

	/* Si j'ai des bombes spéciales */
	if (ps->special_bombs > 0) {
		/* Si ce sont des bombes explosant immédiatement 
		   if (get_current_level ()->bomb.buttonBMT == BMTblastnow)
		   algo simpliste à modifier 
		   if (vote_poser && ps->invincible > 5)
		   {
		   vote_action_speciale = XBTrue;
		   vote_poser = XBFalse;
		   } */
	}
	/* Ai-je envie de swapper ? */
	vote_swap = XBFalse;
	/* if (ps->swapper > 0 && test_swap (ps, max_perf))
	   vote_swap = XBTrue; */

	vote[choix_dir] += 200;
	olddy = dy;
	olddx = dx;
}

/* Inversion des directions */
int
inversion (int dir)
{
	switch (dir) {
	case UP:
		return DOWN;
	case DOWN:
		return UP;
	case LEFT:
		return RIGHT;
	case RIGHT:
		return LEFT;
	default:
		return dir;
	}
}

/* Mouvement à faire si je suis coincé entre deux cases */
int
decoince (int dir)
{
	static int compteur = 0;
	int nouv_dir = GoDefault;

	/* Pour éviter de gigoter entre 2 bombes */
	compteur++;
	if (compteur > 4) {
		compteur = 0;
		nouv_dir = dir;
	}

	return nouv_dir;
}

/* Retourne VRAI si je peux tuer des ennemis */
int
repose_en_mourant (BMPlayer * ps)
{
	int mazex, mazey;
	int portee_bombe;

	mazex = (int)(ps->x + 0.5 * BLOCK_WIDTH) / BLOCK_WIDTH;
	mazey = (int)(ps->y + 0.5 * BLOCK_HEIGHT) / BLOCK_HEIGHT + 1;
	portee_bombe = (ps->illness == IllMini) ? 1 : ps->range;

	if (score_tue (mazex, mazey, portee_bombe) >= 0 && nb_ennemis_vivants > 0)
		return XBTrue;
	else
		return XBFalse;
}

void
gestionBot (BMPlayer * player_stat, PlayerAction * player_action, int numero_bot, int num_player)
{
	int i, j;
	int max = MALUS_MORT;
	int choice = -1;
	int mazex, mazey;
	int inverse;
	BMPlayer *ps;
	PlayerAction *pa;
	/* Fouf lenteur -- *//*  trace = 0; */

	/* Fouf lenteur ++ *//* fprintf(stderr, "gestionBot\n"); */
	/* Fouf lenteur ++ *//* fflush(stderr); */

	ps = player_stat + numero_bot;
	pa = player_action + numero_bot;
	statut_joueurs = player_stat;
	num_bot = numero_bot;
	nb_joueurs = num_player;

	mazex = ps->x / BLOCK_WIDTH;
	mazey = ps->y / BLOCK_HEIGHT + 1;

	/* Suis-je rapide ? */
	if (ps->illness == IllRun)
		duree_deplacement = FAST_SPEED;
	else /* Suis-je Lent ? */ if (ps->illness == IllSlow)
		duree_deplacement = SLOW_SPEED;
	else
		duree_deplacement = NORMAL_SPEED;

	temps_jeu = gameTime;
	/* Début d'un niveau */
	if (temps_jeu == 1)
		init_laby_shrink ();

	/* Suis-je inversé ? */
	if (ps->illness == IllReverse)
		inverse = XBTrue;
	else
		inverse = XBFalse;

	/* Si je me trouve entre deux cases */
	/* Fouf lenteur -- *//*  if ((ps->x % BLOCK_WIDTH) != 0 || (ps->y % BLOCK_HEIGHT) != 0) */
/* Fouf lenteur ++ */ if (((ps->x % BLOCK_WIDTH) != 0 || (ps->y % BLOCK_HEIGHT) != 0)
						  || ((ps->d_ist == GoStop) && (temps_jeu % 3))) {
		switch (ps->d_ist) {
		case GoUp:
			/* kick si est possible */
			if (CheckBomb (mazex, mazey) && !(ps->kick && CheckMazeFree2 (mazex, mazey - 1)))
				pa->dir = decoince (GoDown);
			break;
		case GoDown:
			if (CheckBomb (mazex, mazey + 1) && !(ps->kick && CheckMazeFree2 (mazex, mazey + 2)))
				pa->dir = decoince (GoUp);
			break;
		case GoLeft:
			if (CheckBomb (mazex, mazey) && !(ps->kick && CheckMazeFree2 (mazex - 1, mazey)))
				pa->dir = decoince (GoRight);
			break;
		case GoRight:
			if (CheckBomb (mazex + 1, mazey) && !(ps->kick && CheckMazeFree2 (mazex + 2, mazey)))
				pa->dir = decoince (GoLeft);
			break;
		default:
			break;
		}
		/* Si je meurs, je pose */
		if (ps->dying)
			pa->bomb = repose_en_mourant (ps);
		if (inverse)
			pa->dir = inversion (pa->dir);
		return;
	}

	if (pa->dir == GoStop && !(temps_jeu & 1))
		return;

	/* Initialisation */
	for (i = 0; i < NB_DIR; i++)
		vote[i] = 0;

	test_murs (ps);				/* Don't walk into walls ... */

	/* Choix de la bonne direction et des actions à effectuer */
	actions (ps);

	/*  getBonuses (ps);
	   putBombs (ps);  break the walls ... */
	/*  randChoice (); */

	for (j = 0; j < NB_DIR; j++)
		if (vote[j] > max) {
			/* Fouf lenteur -- *//*    if (trace) printf ("Ok for %d %d\n", j, vote[j]); */
			max = vote[j];
			choice = j;
		}
	/* Fouf lenteur -- *//*   else */
	/* Fouf lenteur -- *//*      if (trace) printf ("Not ok for %d %d\n", j, vote[j]); */

	switch (choice) {
	case STOP:
		/* Fouf lenteur -- *//* pa->dir = GoStop; */
/* Fouf lenteur ++ */ if (ps->d_ist == GoStop) {
/* Fouf lenteur ++ */ pa->dir = GoDefault;
			/* Fouf lenteur ++ *//* fprintf(stderr, "GoDefault (already stop)\n"); */
/* Fouf lenteur ++ */ }
		else {
/* Fouf lenteur ++ */ pa->dir = GoStop;
			/* Fouf lenteur ++ *//* fprintf(stderr, "GoStop (stop NOW *********)\n"); */
/* Fouf lenteur ++ */ }
		break;
	case UP:
		pa->dir = GoUp;
		/* Fouf lenteur ++ *//* fprintf(stderr, "GoUp\n"); */
		break;
	case LEFT:
		pa->dir = GoLeft;
		/* Fouf lenteur ++ *//* fprintf(stderr, "GoLeft\n"); */
		break;
	case DOWN:
		pa->dir = GoDown;
		/* Fouf lenteur ++ *//* fprintf(stderr, "GoDown\n"); */
		break;
	case RIGHT:
		pa->dir = GoRight;
		/* Fouf lenteur ++ *//* fprintf(stderr, "GoRight\n"); */
		break;
	}
	/* Fouf lenteur ++ *//* fflush(stderr); */

	/* Ai-je décidé de poser ? */
	if (vote_poser)
		pa->bomb = XBTrue;

	/* Ai-je décidé de faire une action spéciale ? */
	if (vote_action_speciale)
		pa->special = XBTrue;

	/* Ai-je envie de swapper ? 
	   if (vote_swap)
	   pa->swap = XBTrue; */

	if (inverse)
		pa->dir = inversion (pa->dir);
}

/* BMFuncData :
special_init : bonus EXTRA au départ
special_game : jeu spécial (bombes hantées, lancées par les murs, ...)
special_extra : bonus EXTRA à trouver
special_key : action du +
*/
