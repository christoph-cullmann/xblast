/*
 * file com_central.c - server answers to broadcasts by clients
 *
 * $Id: com_central.c,v 1.17 2006/03/28 11:41:19 fzago Exp $
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
 * local types
 */

#define MAX_GAMES 256
#define DEAD_GAME_TIME 30

/* local XBComm data */
typedef struct
{
	XBCommBrowse browse;
	unsigned short port;
	unsigned char serial;
	char *addrBroadcast;
	struct timeval tvSend;
} XBCommCentral;

/* entry types for game table */
typedef enum
{
	EntryFree,
	EntryMess,
	EntryGame
} XBEntryType;

/* an entry */
typedef struct
{
	XBEntryType type;
	unsigned short port;
	char game[48];
	char host[32];
	unsigned char version[3];	/* version numbers */
	int gameID;
	int numLives;
	int numWins;
	int frameRate;
	time_t time;
} XBEntry;

/* game table and entry count */
XBEntry entries[MAX_GAMES];
unsigned int used = 0;
/* state of games.html */
XBBool html = XBFalse;

/********************
 * manage game data *
 ********************/

/*
 * clear game table
 */
static void
ClearGameTable (void)
{
	unsigned int i;
	for (i = 0; i < MAX_GAMES; i++) {
		entries[i].type = EntryFree;
	}
	Dbg_C2B ("game table cleared\n");
	used = 0;
}								/* ClearGameTable */

/*
 * find free slot
 */
static XBBool
FindFreeEntry (unsigned int *i)
{
	assert (NULL != i);
	/* check entry count first */
	if (used == MAX_GAMES) {
		return XBFalse;
	}
	/* loop through table */
	for (*i = 0; *i < MAX_GAMES; *i += 1) {
		if (entries[*i].type == EntryFree) {
			return XBTrue;
		}
	}
	/* should never be reached */
	return XBFalse;
}								/* FindFreeEntry */

/*
 * add a message
 */
static XBBool
AddMessage (char *txt, unsigned int len)
{
	unsigned int i;
	/* find free entry */
	if (!FindFreeEntry (&i)) {
		return XBFalse;
	}
	/* message */
	entries[i].type = EntryMess;
	entries[i].port = 0;
	entries[i].version[0] = VERSION_MAJOR;
	entries[i].version[1] = VERSION_MINOR;
	entries[i].version[2] = VERSION_PATCH;
	entries[i].numLives = 0;
	entries[i].numWins = 0;
	entries[i].frameRate = 0;
	entries[i].time = time (NULL);
	memset (entries[i].game, 0, sizeof (entries[i].game));
	if (len > sizeof (entries[i].game) - 1) {
		len = sizeof (entries[i].game) - 1;
	}
	memcpy (entries[i].game, txt, len);
	memset (entries[i].host, 0, sizeof (entries[i].host));
	used++;
	Dbg_C2B ("adding message at #%u =%s=\n", i, entries[i].game);
	return XBTrue;
}								/* AddMessage */

/*
 * add a game entry, return index to fill in details
 */
static XBBool
AddGame (unsigned int *i)
{
	assert (NULL != i);
	/* find free entry */
	if (!FindFreeEntry (i)) {
		return XBFalse;
	}
	/* game */
	entries[*i].type = EntryGame;
	used++;
	return XBTrue;
}								/* AddGame */

/*
 * clear a single slot
 */
static void
ClearEntry (unsigned int i)
{
	assert (i < MAX_GAMES);
	Dbg_C2B ("clearing entry #%u, was type %u\n", i, entries[i].type);
	entries[i].type = EntryFree;
	used--;
}								/* ClearEntry */

/*
 * create a html file with current game entries
 */
static void
MakeGameHtml (void)
{
	FILE *f;
	time_t ltime;
	unsigned int i;
	unsigned int cnt = 0;
	html = XBFalse;
	f = fopen ("games.html", "w");
	if (NULL == f) {
		Dbg_C2B ("failed to write games.html, trying later\n");
		return;
	}
	fprintf (f, "<html><body><table border=1>");
	for (i = 0; i < MAX_GAMES; i++) {
		switch (entries[i].type) {
		case EntryMess:
			fprintf (f, "<tr><td colspan=7>%s</td></tr>", entries[i].game);
			break;
		case EntryGame:
			fprintf (f, "<tr><td>%s</td>", entries[i].game);
			fprintf (f, "<td>%s:%d</td>", entries[i].host, entries[i].port);
			fprintf (f, "<td>%d</td>", entries[i].numLives);
			fprintf (f, "<td>%d</td>", entries[i].numWins);
			fprintf (f, "<td>%d</td>", entries[i].frameRate);
			fprintf (f, "<td>V%u.%u.%u</td>", entries[i].version[0], entries[i].version[1],
					 entries[i].version[2]);
			fprintf (f, "<td>%s</td></tr>", ctime (&entries[i].time));
			cnt++;
			break;
		default:
			break;
		}
	}
	ltime = time (NULL);
	fprintf (f, "<tr><td colspan=7>%u registered games at %s</td></tr>", cnt, ctime (&ltime));
	fclose (f);
	html = XBTrue;
}								/* MakeGameTextFile */

/*
 * create html file with shutdown message
 */
static void
MakeShutdownHtml (void)
{
	FILE *f;
	time_t ltime;
	f = fopen ("games.html", "w");
	ltime = time (NULL);
	fprintf (f, "<html><body>Game Central was shutdown at %s</body></html>", ctime (&ltime));
	fclose (f);
}								/* MakeShutDownHtml */

/*
 * init game table
 */
static void
InitGameTable (void)
{
	char s[1024];
	int l[16];
	unsigned int n;
	unsigned int i;
	/* clear it first */
	ClearGameTable ();
	/* read messages */
	n = ReadMessageOfTheDay (1024, s, l);
	fprintf (stderr, "--CENTRAL MESSAGE OF THE DAY (%d)--\n", n);
	/* enter messages */
	for (i = 0; i < n; i++) {
		assert (l[i + 1] >= l[i]);
		AddMessage (s + l[i], l[i + 1] - l[i]);
	}
	MakeGameHtml ();
}								/* InitGameTable */

/****************
 * get/set data *
 ****************/

/*
 * return number of open games
 */
int
C2B_GetOpenGames (void)
{
	return used;
}								/* C2B_GetOpenGames */

/*
 * check all game entries for last update and delete dead games
 */
void
C2B_ClearOldGames (void)
{
	int n = 0, i;
	time_t now;
	now = time (NULL);
	for (i = 0; i < MAX_GAMES; i++) {
		switch (entries[i].type) {
		case EntryGame:
			if ((now - entries[i].time) > DEAD_GAME_TIME) {
				ClearEntry (i);
				n++;
			}
			break;
		default:
			break;
		}
	}
	if (n > 0 || !html) {
		Dbg_C2B ("%i dead game(s) removed, updating html\n", n);
		MakeGameHtml ();
	}
}								/* C2B_ClearOldGames */

/************************
 * XBCommBrowse handler *
 ************************/

/*
 * Handle incoming request for games
 */
static void
HandleQuery (XBCommCentral * rComm, const XBBrowseTeleQuery * query, const char *host,
			 unsigned short port)
{
	XBBrowseTeleReply tele;
	int i, n = 0;
	Dbg_C2B ("receiving query from %s:%u\n", host, port);
	memset (&tele, 0, sizeof (XBBrowseTeleReply));
	/* build central */
	tele.any.type = XBBT_Reply;
	tele.any.serial = query->any.serial;
	for (i = 0; i < MAX_GAMES; i++) {
		switch (entries[i].type) {
		case EntryMess:
		case EntryGame:
			tele.port = entries[i].port;
			tele.version[0] = entries[i].version[0];
			tele.version[1] = entries[i].version[1];
			tele.version[2] = entries[i].version[2];
			tele.numLives = entries[i].numLives;
			tele.numWins = entries[i].numWins;
			tele.frameRate = entries[i].frameRate;
			strncpy (tele.game, entries[i].game, sizeof (tele.game));
			strncpy (tele.host, entries[i].host, sizeof (tele.host));
			Dbg_C2B ("queueing game data: %s %s:%u\n", entries[i].game, entries[i].host,
					 entries[i].port);
			/* send it */
			Browse_Send (&rComm->browse, host, port, XBFalse, &tele.any);
			n++;
			break;
		default:
			break;
		}
	}
	Dbg_C2B ("%i games queued\n", n);
}								/* HandleQuery */

/*
 * find a game entry matching host:port
 */
static XBBool
FindHostEntry (const char *host, unsigned int *i)
{
	assert (NULL != i);
	for (*i = 0; *i < MAX_GAMES; *i += 1) {
		switch (entries[*i].type) {
		case EntryGame:
			if (0 == strcmp (entries[*i].host, host)) {
				return XBTrue;
			}
			break;
		default:
			break;
		}
	}
	return XBFalse;
}								/* FindHostEntry */

/*
 * validate a received game ID
 */
static XBBool
ValidateGameID (const char *host, unsigned int i)
{
	/* is it valid game id? */
	if (i >= MAX_GAMES || entries[i].type != EntryGame) {
		return XBFalse;
	}
	/* does host name match? */
	if (0 != strcmp (entries[i].host, host)) {
		return XBFalse;
	}
	return XBTrue;
}								/* ValidateGameID */

/*
 * update game entry
 */
static void
UpdateGameEntry (unsigned int i, const XBBrowseTeleNewGame * query, const char *host)
{
	assert (i < MAX_GAMES);
	entries[i].port = query->port;
	entries[i].version[0] = query->version[0];
	entries[i].version[1] = query->version[1];
	entries[i].version[2] = query->version[2];
	entries[i].numLives = query->numLives;
	entries[i].numWins = query->numWins;
	entries[i].frameRate = query->frameRate;
	entries[i].time = time (NULL);
	sprintf (entries[i].host, "%s", host);
	sprintf (entries[i].game, "%s", query->game);
	Dbg_C2B ("New game %s at %s:%i\n", entries[i].game, entries[i].host, entries[i].port);
	MakeGameHtml ();
}								/* UpdateGameEntry */

/*
 * Handle incoming game configs
 */
static void
HandleNewGame (XBCommCentral * rComm, const XBBrowseTeleNewGame * query, const char *host,
			   unsigned short port)
{
	XBBrowseTeleNewGameOK tele;
	unsigned int i = query->gameID;;
	Dbg_C2B ("receiving new game from %s:%u\n", host, port);
	tele.any.type = XBBT_NewGameOK;
	tele.any.serial = query->any.serial;
	/* determine entry to write */
	if (ValidateGameID (host, i)) {
		Dbg_C2B ("received valid game id = %u\n", i);
		UpdateGameEntry (i, query, host);
	}
	else if (FindHostEntry (host, &i)) {
		Dbg_C2B ("found other game from same host at id = %u, overwriting\n", i);
		UpdateGameEntry (i, query, host);
	}
	else if (AddGame (&i)) {
		Dbg_C2B ("adding a new game entry at %u\n", i);
		UpdateGameEntry (i, query, host);
	}
	else {
		Dbg_C2B ("failed to add, game table is full!\n");
	}
	/* queue game id back to game server */
	tele.gameID = i;
	Browse_Send (&rComm->browse, host, port, XBFalse, &tele.any);
	Dbg_C2B ("queued game ID = %i\n", i);
	/* update local html file with game entries */
	MakeGameHtml ();
}								/* HandleNewGame */

/*
 * Handle incoming close requests
 */
static void
HandleCloseGame (XBCommCentral * rComm, const XBBrowseTeleNewGameOK * query, const char *host,
				 unsigned short port)
{
	int i = query->gameID;
	Dbg_C2B ("receiving game close from %s:%u\n", host, port);
	if (ValidateGameID (host, i)) {
		Dbg_C2B ("received valid game id = %u, removing\n", i);
		ClearEntry (i);
		MakeGameHtml ();
	}
	else {
		Dbg_C2B ("failed to close game id = %u, invalid\n", i);
	}
}								/* HandleCloseGame */

/*
 * receive central data
 */
static void
ReceiveCentral (XBCommBrowse * bComm, const XBBrowseTele * tele, const char *host,
				unsigned short port)
{
	assert (NULL != bComm);
	assert (NULL != tele);
	assert (NULL != host);
	switch (tele->type) {
	case XBBT_Query:
		HandleQuery ((XBCommCentral *) bComm, &tele->query, host, port);
		break;
	case XBBT_NewGame:
		HandleNewGame ((XBCommCentral *) bComm, &tele->newGame, host, port);
		break;
	case XBBT_NewGameOK:
		HandleCloseGame ((XBCommCentral *) bComm, &tele->newGameOK, host, port);
		break;
	default:
		break;
	}
}								/* ReceiveCentral */

/*
 * handle browse event
 */
static XBBool
EventCentral (XBCommBrowse * bComm, XBBrowseEvent ev)
{
	switch (ev) {
	case XBBE_Wait:
		Dbg_C2B ("all datagrams sent\n");
		break;
	case XBBE_Dgram:
		Dbg_C2B ("received invalid datagram, ignoring\n");
		break;
	case XBBE_Browse:
		Dbg_C2B ("received invalid browse, ignoring\n");
		break;
	case XBBE_Write:
		/* Dbg_C2B("datagrams wait to be sent\n"); */
		break;
	case XBBE_Close:
		Dbg_C2B ("browse shutdown complete\n");
		/* safe to free the XBComm structure now */
		free (&bComm->comm);
		break;
	default:
		Dbg_C2B ("unknown browse event, ignoring!\n");
		break;
	}
	return XBFalse;
}								/* EventCentral */

/*
 * XBComm delete handler for i/o errors
 */
static XBCommResult
DeleteCentral (XBComm * comm)
{
	XBCommCentral *rComm = (XBCommCentral *) comm;
	assert (NULL != rComm);
	/* remove the XBCommBrowse, trigger XBBE_Close */
	Browse_Finish (&rComm->browse);
	/* clear */
	ClearGameTable ();
	/* shutdown notice in games.html */
	MakeShutdownHtml ();
	return XCR_OK;
}								/* DeleteCentral */

/*
 * create udp socket waiting for client queries
 */
XBComm *
C2B_CreateComm (unsigned short port)
{
	XBSocket *pSocket;
	XBCommCentral *rComm;
	/* create socket */
	pSocket = Net_BindUdp (NULL, port);
	if (NULL == pSocket) {
		Dbg_C2B ("failed to create udp socket on port %u\n", port);
		return NULL;
	}
	Dbg_C2B ("listening on udp port %u\n", port);
	/* init game data */
	InitGameTable ();
	/* create communication data structure */
	rComm = calloc (1, sizeof (XBCommCentral));
	assert (NULL != rComm);
	/* set handlers */
	Browse_CommInit (&rComm->browse, COMM_NewGameOK, pSocket, ReceiveCentral, EventCentral,
					 DeleteCentral);
	return &rComm->browse.comm;
}								/* C2B_CreateComm */

/*
 * end of file com_central.c
 */
