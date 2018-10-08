/*
 * file central.c - communication interface for the server
 *
 * $Id: central.c,v 1.16 2006/03/28 11:41:19 fzago Exp $
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

#include <limits.h>
#include "xblast.h"

#define RATING_WEIGTH 1.0
#define RATING_STEP 0.1

#define GJFAC 0.005
#define GJFAC2 1.005

#define SIZE 32

/*
 * local variables
 */
static XBComm *listenComm = NULL;
static XBComm *centralComm = NULL;

/* central statistics */
static int cPlayers;
static int cGames;
static int cGamesPlayed;
static int cTotalGames;
static int cLogins;
static int connected;
static XBAtom adminAtom;
CFGPlayerRating adminRating;

/*
 * start to listen for clients
 */
XBBool
Central_StartListen (CFGGameHost * cfg)
{
	int i, j;
	char tmp[256];
	/* prepare database */
	Dbg_Central ("Preparing central\n");
	LoadPlayerCentral (XBTrue);
	/* prepare remote game database */
	LoadGameConfig ();
	/* count players in database, minus admin */
	cPlayers = GetNumPlayerConfigs (CT_Central) - 1;
	Dbg_Central ("number of registered players : %d\n", cPlayers);
	/* init admin data and store back */
	adminAtom = GUI_IntToAtom (0);
	RetrievePlayerRating (CT_Central, adminAtom, &adminRating);
	adminRating.timeRegister = time (NULL);
	StorePlayerRating (CT_Central, adminAtom, &adminRating);
	SavePlayerCentral ();
	/* init stats for current session */
	cGames = 0;
	cGamesPlayed = 0;
	cTotalGames = adminRating.gamesPlayed;
	cLogins = 0;
	connected = 0;
	/* setup player atoms for users */
	for (i = 0; i < NUM_LOCAL_PLAYER; i++) {
		Network_SetPlayer (0, i, ATOM_INVALID);
	}
	for (i = 1; i < MAX_PLAYER; i++) {
		j = sprintf (tmp, "tmpPlayer%i", i);
		tmp[j + 1] = 0;
		Network_SetPlayer (i, 0, GUI_StringToAtom (tmp));
	};
	/* listen on tcp-port for clients */
	assert (listenComm == NULL);
	listenComm = CommCreateListen (cfg, XBTrue);	/*  central */
	if (NULL == listenComm) {
		Dbg_Central ("failed to listen on tcp port %u\n", cfg->port);
		return XBFalse;
	}
	/* allow client to browse for games */
	assert (NULL == centralComm);
	centralComm = C2B_CreateComm (cfg->port);
	if (NULL == centralComm) {
		Dbg_Central ("failed to listen on udp port %u\n", cfg->port);
		return XBFalse;
	}
	/* that's all */
	printf ("Listening on port %u (tcp, udp) now...\n", cfg->port);
	return XBTrue;
}								/* Central_StartListen */

/*
 * delete port for listening
 */
void
Central_StopListen (void)
{
	Dbg_Central ("Closing down central...\n");
	/* delete listen port */
	assert (NULL != listenComm);
	CommDelete (listenComm);
	listenComm = NULL;
	/* dlete reply socket */
	if (NULL != centralComm) {
		CommDelete (centralComm);
		centralComm = NULL;
	}
	/* Update administrator with sesssion stats */
	adminRating.gamesPlayed = cTotalGames;
	adminRating.timeUpdate = time (NULL);
	StorePlayerRating (CT_Central, adminAtom, &adminRating);
	/* save and close rating DB */
	SavePlayerCentral ();
	FinishPlayerCentral ();
	/* clean up remote games */
	FinishGameConfig ();
}								/* Central_StopListen */

/*
 * get data
 */

/*
 * central get statistics
 */
void
Central_GetStatistics (int *nPlayers, int *nGames, int *nGamesPlayed, int *nTotalGames,
					   int *nLogins)
{
	*nPlayers = cPlayers;
	*nGames = C2B_GetOpenGames ();
	*nGamesPlayed = cGamesPlayed;
	*nTotalGames = cTotalGames;
	*nLogins = cLogins;
}								/* Central_GetStatistics */

/*
 * return currently connected user
 */
int
Central_Connected (void)
{
	return connected;
}								/* Central_Connected */

/*
 * a user has connected
 */
void
Central_Accept (unsigned id, unsigned cnt, const char *hostName, unsigned port)
{
	CFGGameHost cfg;
	assert (hostName != NULL);
	assert (id > 0);
	assert (id < MAX_HOSTS);
	Dbg_Central ("client adr=%s:%u connected\n", hostName, port);
	/* clear host entry in database */
	DeleteGameConfig (CT_Remote, atomArrayHost0[id]);
	/* store in database */
	memset (&cfg, 0, sizeof (CFGGameHost));
	cfg.name = hostName;
	cfg.port = port;
	StoreGameHost (CT_Remote, atomArrayHost0[id], &cfg);
	/* create message */
	Network_QueueEvent (XBNW_Accepted, id);
	cLogins++;
	connected++;
	Dbg_Central ("User id=%u count=%u added, open connections %u\n", id, cnt, connected);
}								/* Central_ClientAccepted */

/*
 * receive data
 */

/*
 * reverse byte order of an int
 */
static int
endian (int x)
{
	char *i, *o;
	int y;
	i = (char *)&x;
	o = (char *)&y;
	o[0] = i[3];
	o[1] = i[2];
	o[2] = i[1];
	o[3] = i[0];
	return y;
}								/* endian */

/*
 * game results retrieved from client
 */
XBGameStatType
Central_ReceiveGameStat (const char *line)
{
	/*  line = numplayers, pid1, score1, pid2, score2, ... */
	int numPlayers, i, j, k, m, endia;
	int PID[MAX_PLAYER];
	int Score[MAX_PLAYER];
	int regPl[MAX_PLAYER];
	float q, plus[MAX_PLAYER], i1, j1;
	XBAtom pAtom[MAX_PLAYER];
	CFGPlayerRating rating[MAX_PLAYER];
	XBBool gamestat;
	char buffer[SIZE];
	time_t curtime;
	struct tm *loctime;
	XBGameStatType ret = XBGS_Error;
	Dbg_Central ("receiving game stat\n");
	/* flag for byte order */
	endia = 0;
	/* extract number of players from received data, check both byte orders */
	memcpy (&numPlayers, line, 4);
	if ((numPlayers < 0) || (numPlayers > MAX_PLAYER)) {
		i = endian (numPlayers);
		if ((i >= 0) && (i <= MAX_PLAYER)) {
			Dbg_Central ("entering endian mode\n");
			numPlayers = i;
			endia = 1;
		}
		else {
			Dbg_Central ("illegal player count, ignoring result !\n");
			return ret;
		}
	}
	/* extract PIDs and scores */
	for (i = 0; i < numPlayers; i++) {
		memcpy (PID + i, line + 4 + i * 8, 4);
		memcpy (Score + i, line + 8 + i * 8, 4);
		if (endia) {
			PID[i] = endian (PID[i]);
			Score[i] = endian (Score[i]);
		}
	}
	Dbg_Central ("number of received scores = %hu\n", numPlayers);
	/* determine rated players (rating>0) */
	for (i = 0, k = 0; i < numPlayers; i++) {
		if (PID[i] > 0) {
			pAtom[i] = GUI_IntToAtom (PID[i]);
			if (RetrievePlayerRating (CT_Central, pAtom[i], rating + i)) {
				if ((rating + i)->rating > 0) {
					regPl[k] = i;
					k++;
				}
			}
			else {
				Dbg_Central ("player pid %hu unrated (rating = %f)\n", PID[i], (rating + i)->rating);
			}
		}
		else {
			Dbg_Central ("invalid player pid for score %hu\n", i);
		}
	}
	Dbg_Central ("rated players found = %hu\n", k);
	/* time stamp all rated players, count number m of level winners */
	m = 0;
	gamestat = XBFalse;
	for (i = 0; i < k; i++) {
		j = regPl[i];
		Dbg_Central ("Player %i, PID = %i, score = %i\n", j, PID[j], Score[j]);
		(rating + j)->timeUpdate = time (NULL);
		if (Score[j] < 0) {
			gamestat = XBTrue;
		}
		m += Score[j];
		plus[j] = 0.0;
	}
	/* at least one player has negative score: match winners, summary update */
	if (gamestat) {
		Dbg_Central ("received Game Result\n");
		ret = XBGS_Complete;
		m = 0;					/* empty pool */
		cTotalGames++;
		cGamesPlayed++;
		sprintf (buffer, "game_%i", cTotalGames);
		AppendGameResult (CT_Central, atomResults, GUI_StringToAtom (buffer), k, regPl, PID, Score);
		for (i = 0; i < k; i++) {
			j = regPl[i];
			(rating + j)->gamesPlayed++;
			if (Score[j] < 0) {
				(rating + j)->realWins++;
				Score[j] = 1;
			}
			else {
				Score[j] = 0;
			}
			m += Score[j];
		}
		Dbg_Central ("number of winners %hu\n", m);
		adminRating.gamesPlayed = cTotalGames;
		adminRating.timeUpdate = time (NULL);
		StorePlayerRating (CT_Central, adminAtom, &adminRating);
	}
	else {
		ret = XBGS_Level;
		Dbg_Central ("received Level Result\n+ number of winners %hu\n", m);
	}

	/* old scheme
	   for(i=0;i<k;i++) {
	   i0=regPl[i];
	   i1=(rating+i0)->rating;
	   for(j=i+1;j<k;j++) {
	   j0=regPl[j];
	   j1=(rating+j0)->rating;
	   if(Score[i0]== Score[j0]) {
	   q=0.5;
	   } else if(Score[i0]>Score[j0]) {
	   q=1.0;
	   (rating+i0)->relativeWins++;
	   } else {
	   q=0.0;
	   (rating+j0)->relativeWins++;
	   }
	   b=1/(1+exp(RATING_WEIGTH*(j1-i1)));
	   plus[i0]+=RATING_STEP*(q-b);
	   plus[j0]-=RATING_STEP*(q-b);
	   }
	   } */

	/* update rating if a winner present and at least two rated players */
	if ((m > 0) && (k > 1)) {
		q = 0;
		j1 = 0;
		/* each player first spends a fixed fraction of his rating to a pool */
		for (i = 0; i < k; i++) {
			j = regPl[i];
			i1 = (rating + j)->rating * GJFAC;
			plus[j] = -i1;
			j1 += i1;
		}
		/* the pool is blown up with another fixed factor */
		j1 *= GJFAC2;
		Dbg_Central ("total value to distribute = %f points\n", j1);
		/* pool is evenly distributed among the winners */
		for (i = 0; i < k; i++) {
			j = regPl[i];
			plus[j] += (j1 * Score[j]) / m;
		}
		/* apply the rating changes */
		for (i = 0; i < k; i++) {
			j = regPl[i];
			plus[j] += (rating + j)->rating;
			Dbg_Central ("Rating change for player %i = %f  ->  %f\n", PID[j], (rating + j)->rating,
						 plus[j]);
			(rating + j)->rating = plus[j];
			(rating + j)->relativeWins += Score[j];
			/* store changes to database if registered */
			if (PID[j] > 0) {
				StorePlayerRating (CT_Central, pAtom[j], rating + j);
			}
		}
		curtime = time (NULL);
		loctime = localtime (&curtime);
		strftime (buffer, SIZE, "%H_%d_%m_%Y", loctime);
		/* store time of the change now */
		AppendTimePlayerRating (CT_Central, atomTimeRatings, GUI_StringToAtom (buffer), k, regPl,
								PID, plus);
	}
	else {
		Dbg_Central ("nothing to distribute\n");
	}
	Dbg_Central ("game stat applied\n");
	/* update administrator and save */
	SavePlayerCentral ();
	return ret;
}								/* Central_ReceiveGameStat */

/*
 * check passwords, for update request
 */
static XBBool
PasswordCheck (const char *a, const char *b)
{
	if ((NULL == a) && (NULL == b)) {
		return XBTrue;
	}
	if ((NULL != a) || (NULL == b)) {
		return XBFalse;
	}
	return (0 == strcmp (a, b)) ? XBTrue : XBFalse;
}								/* PasswordCheck */

/*
 * player config received from client -> registration
 */
int
Central_ReceivePlayerConfig (unsigned id, const char *line)
{
	XBAtom atom, atomID;
	CFGPlayerEx tmpPlayer, idPlayer;
	int pid;
	/* store player for config */
	atom = Network_ReceivePlayerConfig (CT_Central, id, 0, line);
	if (atom == ATOM_INVALID) {
		return XBPV_Incomplete;
	}
	Dbg_Central ("receiving player data\n");
	RetrievePlayerEx (CT_Central, atom, &tmpPlayer);
	pid = tmpPlayer.id.PID;
	/* register or update */
	if (pid > 0) {
		Dbg_Central ("update request for PID = %i\n", pid);
		atomID = GUI_IntToAtom (pid);
		if (RetrievePlayerEx (CT_Central, atomID, &idPlayer)) {
			Dbg_Central ("PID is registered\n");
			if (PasswordCheck (tmpPlayer.id.pass, idPlayer.id.pass)) {
				Dbg_Central ("Password verified\n");
				/* copy rating stuff, do not delete, big mistake */
				tmpPlayer.rating.rating = idPlayer.rating.rating;
				tmpPlayer.rating.gamesPlayed = idPlayer.rating.gamesPlayed;
				tmpPlayer.rating.realWins = idPlayer.rating.realWins;
				tmpPlayer.rating.relativeWins = idPlayer.rating.relativeWins;
				tmpPlayer.rating.timeUpdate = idPlayer.rating.timeUpdate;
				tmpPlayer.rating.timeRegister = idPlayer.rating.timeRegister;
				StorePlayerEx (CT_Central, atomID, &tmpPlayer);
				pid = tmpPlayer.id.PID;
				Dbg_Central ("User updated\n");
			}
			else {
				Dbg_Central ("Bad password for update\n");
				pid = XBPV_Password;
			}
		}
		else {
			Dbg_Central ("PID not registered, no update!\n");
			pid = XBPV_Unknown;
		}
	}
	else {
		Dbg_Central ("registration request\n");
		if (-1 == FindDoubleName (CT_Central, atom)) {
			Dbg_Central ("name %s is unique\n", GetPlayerName (CT_Central, atom));
			pid = GameRandomNumber (INT_MAX);
			atomID = GUI_IntToAtom (pid);
			while (NULL != GetPlayerName (CT_Central, atomID)) {
				pid = GameRandomNumber (INT_MAX);
				atomID = GUI_IntToAtom (pid);
			}
			tmpPlayer.rating.rating = 1000;
			tmpPlayer.rating.timeUpdate = 0;
			tmpPlayer.rating.timeRegister = time (NULL);
			tmpPlayer.id.PID = (int)pid;
			StorePlayerEx (CT_Central, atomID, &tmpPlayer);
			cPlayers++;
			Dbg_Central ("new player added pid = %i (total = %i)\n", pid, cPlayers);
		}
		else {
			Dbg_Central ("name %s already in use!\n", GetPlayerName (CT_Central, atom));
			pid = XBPV_Double;
		}
	}
	/* delete temp data */
	DeletePlayerConfig (CT_Central, atom);
	SavePlayerCentral ();
	Dbg_Central ("player data handled\n");
	return pid;
}								/* Central_ReceivePlayerConfig */

/*
 * a user has started disconnect announcement
 */
void
Central_ReceiveDisconnect (unsigned id)
{
	Dbg_Central ("User id=%u disconnected\n", id);
}								/* Central_ReceiveDisconnect */

/*
 * a connection has been completely removed
 */
void
Central_ConnectionClosed (unsigned id, unsigned cnt)
{
	connected--;
	Dbg_Central ("User id=%u cnt=%u removed, remaining connections %u\n", id, cnt, connected);
	assert (connected >= 0);
}								/* Central_ConnectionClosed */

/*
 * queue disconnect
 */

/*
 * disconnect from clients
 */
void
Central_QueueDisconnect (unsigned clientID)
{
	assert (clientID > 0);
	assert (clientID < MAX_HOSTS);
	/* disconnect from client */
	if (C2X_Connected (clientID)) {
		Dbg_Central ("announcing disconnect to user %u\n", clientID);
		C2X_QueueDisconnect (clientID);
	}
}								/* Central_QueueDisconnect */

/*
 * disconnect from clients
 */
void
Central_QueueDisconnectAll (void)
{
	unsigned clientID;

	/* disconnect from client */
	for (clientID = 1; clientID < MAX_HOSTS; clientID++) {
		Central_QueueDisconnect (clientID);
	}
}								/* Central_QueueDisconnectAll */

/*
 * end of file central.c
 */
