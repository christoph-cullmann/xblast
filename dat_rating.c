/*
 * file dat_rating.c - level and game statistics
 *
 * $Id: dat_rating.c,v 1.5 2006/02/09 21:21:23 fzago Exp $
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 * Added by Koen De Raedt for central support
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
 * local variables
 */

/*
 * compare stat-data by percent
 */
static int
CompareCentralDataByScore (const void *a, const void *b)
{
	const XBCentralData *pA = a;
	const XBCentralData *pB = b;

	assert (pA != NULL);
	assert (pB != NULL);
	if (pA->score == pB->score) {
		return pB->numTotal - pA->numTotal;
	}
	else if (pA->score > pB->score) {
		return -1;
	}
	else {
		return +1;
	}
}								/* CompareStatDataByScore */

XBCentralData *
CreateCentralStat (size_t * num)
{
	XBAtom atom;
	CFGPlayerEx player;
	XBCentralData *table;
	size_t i, j;

	assert (NULL != num);

	/* alloc data for table */
	*num = GetNumPlayerConfigs (CT_Central);
	table = calloc (*num, sizeof (XBCentralData));
	assert (NULL != table);
	/* copy data */
	for (i = 0, j = 0; i < *num; i++) {
		atom = GetPlayerAtom (CT_Central, i);
		if (RetrievePlayerEx (CT_Central, atom, &player)) {
			(table + j)->atom = atom;
			(table + j)->name = player.name;
			(table + j)->score = player.rating.rating;
			(table + j)->numTotal = player.rating.gamesPlayed;
			(table + j)->numWon = player.rating.realWins;
			if (player.rating.gamesPlayed > 0) {
				(table + j)->percent = player.rating.realWins;
				(table + j)->percent /= player.rating.gamesPlayed;
				(table + j)->percent *= 100.0;
			}
			else {
				(table + j)->percent = 0;
			}
			j++;
		}
	}
	*num = j;
	if (*num == 0) {
		free (table);
		return NULL;
	}
	/* sort by score */
	qsort (table, *num, sizeof (XBCentralData), CompareCentralDataByScore);
	for (i = 0; i < *num; i++) {
		(table + i)->rank = i + 1;
	}
	/* that's all */
	return table;
}

XBCentralInfo *
CreateCentralInfo (XBAtom atom, XBCentralData data)	// TODO pass *centralData (for rating, rank, ..)
{
	XBCentralInfo *table;
	CFGPlayerEx player;
	char d[MAX_MENU_INFO][256];
	int i;

	table = calloc (MAX_MENU_INFO, sizeof (XBCentralInfo));
	for (i = 0; i < MAX_MENU_INFO; i++) {
		(table + i)->name = "";
		(table + i)->value = "";
	}
	if (RetrievePlayerEx (CT_Central, atom, &player)) {
		(table + 0)->name = "Name";
		(table + 0)->value = player.name;
		sprintf (d[0], "%d", data.rank);
		(table + 1)->name = "Rank";
		(table + 1)->value = d[0];
		sprintf (d[1], "%0.1f", data.score);
		(table + 2)->name = "Rating";
		(table + 2)->value = d[1];

		(table + 3)->name = "Winnings";
		if (data.numTotal > 0) {
			sprintf (d[2], "%d won of %d (%0.1f%%)", data.numWon, data.numTotal, data.percent);
			(table + 3)->value = d[2];
		}
		else {
			(table + 3)->value = "No games played yet";
		}

		sprintf (d[3], "%d", player.rating.relativeWins);
		(table + 4)->name = "Total number of levels won";
		(table + 4)->value = d[3];

		(table + 6)->name = "Registration date";
		sprintf (d[4], "%s", DateString (player.rating.timeRegister));
		(table + 6)->value = d[4];
		(table + 7)->name = "Last game played";
		if (player.rating.timeUpdate > 0) {
			sprintf (d[5], "%s", DateString (player.rating.timeUpdate));
			(table + 7)->value = d[5];
		}
		else {
			(table + 7)->value = "No games played yet";
		}

		(table + 9)->name = "Win game message";
		(table + 9)->value = player.messages.msgWinGame;
		(table + 10)->name = "Win level message";
		(table + 10)->value = player.messages.msgWinLevel;
		(table + 11)->name = "Lose level message";
		(table + 11)->value = player.messages.msgLoseLevel;
		(table + 12)->name = "Lose life message";
		(table + 12)->value = player.messages.msgLoseLife;
		(table + 13)->name = "Welcome message";
		(table + 13)->value = player.messages.msgWelcome;
		(table + 14)->name = "Gloat";
		(table + 14)->value = player.messages.msgGloat;
		for (i = 9; i < 15; i++) {
			if ((table + i)->value == NULL) {
				(table + i)->value = "[None]";
			}
		}
	}
	return table;
}
