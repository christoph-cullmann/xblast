/*
 * file com_central.c - server answers to broadcasts by clients
 *
 * $Id: com_central.c,v 1.8 2005/01/23 14:19:59 lodott Exp $
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
#include "com_central.h"

#include "com_browse.h"
#include "version.h"
#include "str_util.h"
#ifdef WMS
#include "timeval.h"
#endif
#include "ini_file.h"

/*
 * local types
 */

#define MAX_GAMES 256
#define DEAD_GAME_TIME 30

typedef struct {
  XBCommBrowse    browse;
  unsigned short port;
  unsigned char  serial;
  char          *addrBroadcast;
  struct timeval tvSend;
} XBCommCentral;

typedef struct {
  unsigned short  port;
  char            game[48];
  char            host[32];
  unsigned char   version[3]; /* version numbers */
  int             gameID;
  int             numLives;
  int             numWins;
  int             frameRate;
  time_t          time;
} XBCentralGames;

XBCentralGames centralGames[MAX_GAMES];
XBBool isFree[MAX_GAMES];
XBBool isMess[MAX_GAMES];
int nextFreeSlot, usedSlots;

/*
 * return number of open games
 */
int
C2B_GetOpenGames()
{
  return usedSlots;
} /* C2B_GetOpenGames */

/*
 * create a html file with current game entries
 */
static void
C2B_MakeGameTextFile()
{
  FILE *f;
  int i;
  f = fopen("games.html","w");
  fprintf(f, "<html><body><table border=1>");
  for (i=0; i<MAX_GAMES; i++) {
    if (!isFree[i]) {
      if (isMess[i]) {
	fprintf(f,"<tr><td colspan=6>%s</td></tr>",centralGames[i].game);
      } else {
	fprintf(f,"<tr><td>%s</td><td>%s:%d</td><td>%d</td><td>%d</td><td>%d</td><td>V%u.%u.%u</td></tr>",
		centralGames[i].game,centralGames[i].host,centralGames[i].port,
		centralGames[i].numLives,centralGames[i].numWins,centralGames[i].frameRate,
		centralGames[i].version[0],centralGames[i].version[1],centralGames[i].version[2]);
      }
    }
  }
  fprintf(f,"</table></body></html>");
  fclose(f);
} /* C2B_MakeGameTextFile */

/*
 * check all game entries for last update and delete dead games
 */
void
C2B_ClearOldGames()
{
  int n = 0, i;
  time_t now;
  now = time(NULL);
  for (i=0; i<MAX_GAMES; i++) {
    if (!isFree[i] & !isMess[i]) {
      if ( (now - centralGames[i].time) > DEAD_GAME_TIME) {
	usedSlots--;
	isFree[i] = XBTrue;
	n++;
      }
    }
  }
  if (n>0) {
    Dbg_C2B("%i dead game(s) removed\n",n);
  }
  C2B_MakeGameTextFile();
} /* C2B_ClearOldGames */

/*
 * Handle incoming request for games
 */
static void
HandleQuery (XBCommCentral *rComm, const XBBrowseTeleQuery *query, const char *host, unsigned short port)
{
  XBBrowseTeleReply tele;
  int i, n=0;
  Dbg_C2B("receiving query from %s:%u\n", host, port);
  memset (&tele, 0, sizeof (tele));
  /* build central */
  tele.any.type   = XBBT_Reply;
  tele.any.serial = query->any.serial;
  for (i=0; i<MAX_GAMES; i++) {
    if (!isFree[i]) {
      tele.port       = centralGames[i].port;
      tele.version[0] = centralGames[i].version[0];
      tele.version[1] = centralGames[i].version[1];
      tele.version[2] = centralGames[i].version[2];
      tele.numLives   = centralGames[i].numLives;
      tele.numWins    = centralGames[i].numWins;
      tele.frameRate  = centralGames[i].frameRate;
      strncpy (tele.game, centralGames[i].game, sizeof (tele.game));
      strncpy (tele.host, centralGames[i].host, sizeof (tele.host));
      Dbg_C2B("queueing game data: %s %s:%u\n", centralGames[i].game, centralGames[i].host, centralGames[i].port);
      /* send it */
      Browse_Send (&rComm->browse, host, port, XBFalse, &tele.any);
      n++;
    }
  }
  Dbg_C2B ("%i games queued\n", n);
} /* HandleQuery */

/*
 * find a game entry matching host:port
 */
static int
FindHostPortEntry(const char *host, unsigned short port) {
  int i;
  for (i=0; i<MAX_GAMES; i++) {
    if (!isFree[i] & !isMess[i]) {
      if ( (centralGames[i].port == port) && (0 == strcmp(centralGames[i].host,host)) ){
	return(i);
      }
    }
  }
  return(-1);
} /* FindHostPortEntry */

/*
 * validate a game ID
 */
static int
ValidateGameID(int i) {
  int j;
  if ( (i>MAX_GAMES) || (i<0) || isFree[i] ) {
    /* id either invalid or not occupied - choose next free central id */
    for (i=nextFreeSlot, j=0; j<=MAX_GAMES; i=(i+1)%MAX_GAMES, j++) {
      if (isFree[i] && !isMess[i]) {
	usedSlots++;
	nextFreeSlot = i+1;
	break;
      }
    }
    if (j==MAX_GAMES) { i=-1; }
  }
  return i;
} /* ValidateGameID */

/*
 * Handle incoming game configs
 */
static void
HandleNewGame (XBCommCentral *rComm, const XBBrowseTeleNewGame *query, const char *host, unsigned short port)
{
  XBBrowseTeleNewGameOK tele;
  int i;
  Dbg_C2B("receiving new game from %s:%u\n", host, port);
  tele.any.type   = XBBT_NewGameOK;
  tele.any.serial = query->any.serial;
  /* find entry with received host:port */
  i = FindHostPortEntry(host,query->port);
  /* on failure, validate the received ID */
  if (i<0) {
    i = ValidateGameID(query->gameID);
    Dbg_C2B("assigning game ID = %i\n", i);
  } else {
    Dbg_C2B("found existing game ID = %i for that address\n", i);
  }
  /* id is now valid (init/update entry) or <0 (no free slots) */
  if (i>=0) {
    isFree[i] = XBFalse;
    centralGames[i].port = query->port;
    centralGames[i].version[0] = query->version[0];
    centralGames[i].version[1] = query->version[1];
    centralGames[i].version[2] = query->version[2];
    centralGames[i].numLives = query->numLives;
    centralGames[i].numWins = query->numWins;
    centralGames[i].frameRate = query->frameRate;
    centralGames[i].time = time(NULL);
    sprintf(centralGames[i].host,"%s",host);
    sprintf(centralGames[i].game,"%s",query->game);
    Dbg_C2B("New game %s at %s:%i\n", centralGames[i].game, centralGames[i].host, centralGames[i].port);
  } else {
    Dbg_C2B("failed to register, all slots occupied\n");
  }
  /* queue game id back to game server */
  tele.gameID = i;
  Browse_Send (&rComm->browse, host, port, XBFalse, &tele.any);
  Dbg_C2B ("queued game ID = %i\n", i);
  /* update local html file with game entries */
  C2B_MakeGameTextFile();
} /* HandleNewGame */

/*
 * Handle incoming close requests
 */
static void
HandleCloseGame (XBCommCentral *rComm, const XBBrowseTeleNewGameOK *query, const char *host, unsigned short port)
{
  int i = query->gameID;
  Dbg_C2B("received game close for id = %i by server %s:%u\n", i, host, port);
  if (!isFree[i]) {
    Dbg_C2B("removing from game list\n");
    usedSlots--;
    if (usedSlots<0) {
      usedSlots = 0;
    }
    isFree[i] = XBTrue;
  } else {
    Dbg_C2B("failed to remove, already free\n");
  }
} /* HandleCloseGame */

/*
 * receive central data
 */
static void
ReceiveCentral (XBCommBrowse *bComm, const XBBrowseTele *tele, const char *host, unsigned short port)
{
  assert (NULL != bComm);
  assert (NULL != tele);
  assert (NULL != host);
  switch (tele->type) {
  case XBBT_Query  : HandleQuery ((XBCommCentral *) bComm, &tele->query, host, port); break;
  case XBBT_NewGame: HandleNewGame ((XBCommCentral *) bComm, &tele->newGame, host, port); break;
  case XBBT_NewGameOK: HandleCloseGame ((XBCommCentral *) bComm, &tele->newGameOK, host, port); break;
  default:         break;
  }
} /* ReceiveCentral */

/*
 * handle browse event
 */
static XBBool
EventCentral (XBCommBrowse *bComm, XBBrowseEvent ev)
{
  switch (ev) {
  case XBBE_Wait:  Dbg_C2B("all datagrams sent\n"); break;
  case XBBE_Dgram: Dbg_C2B("received invalid datagram, ignoring\n"); break;
  case XBBE_Browse: Dbg_C2B("received invalid browse, ignoring\n"); break;
  case XBBE_Write: Dbg_C2B("datagrams wait to be sent\n"); break;
  case XBBE_Close: Dbg_C2B("browse shutdown complete\n"); break;
  default: Dbg_C2B("unknown browse event, ignoring!\n");
  }
  return XBFalse;
} /* EventCentral */

/*
 * XBComm delete handler for i/o errors
 */
static XBCommResult
DeleteCentral (XBComm *comm)
{
  XBCommCentral *rComm = (XBCommCentral *) comm;
  assert (NULL != rComm);
  Browse_Finish (&rComm->browse);
  free (rComm);
  /* inform central, in case this is due to an error ! */
  return XCR_OK;
} /* DeleteCentral */

/*
 * create udp socket waiting for client queries
 */
XBComm *
C2B_CreateComm (unsigned short port)
{
  XBSocket    *pSocket;
  XBCommCentral *rComm;
  int i;
  char s[1024],q[256];
  int l[16],n;

  /* create socket */
  pSocket = Net_BindUdp (NULL, port);
  if (NULL == pSocket) {
    Dbg_C2B("failed to create udp socket on port %u\n", port);
    return NULL;
  }
  Dbg_C2B("listening on udp port %u\n", port);
  /* create communication data structure */
  rComm = calloc (1, sizeof (*rComm));
  assert (NULL != rComm);
  for(i=0; i<MAX_GAMES; i++) {
    isFree[i] =XBTrue;
    isMess[i] =XBFalse;
  }
  nextFreeSlot=0;
  /* set handlers */
  Browse_CommInit (&rComm->browse, COMM_NewGameOK, pSocket, ReceiveCentral, EventCentral, DeleteCentral);
  /* init messages */
  for (i=0; i<MAX_GAMES; i++) {
    isMess[i] = XBFalse;
  }
  /* read messages */
  n = ReadMessageOfTheDay(1024,s,l);
  Dbg_C2B("-- MESSAGE OF THE DAY (%d)--\n",n);
  /* enter messages */
  for (i=0; i<n; i++) {
    isMess[i] = XBTrue;
    isFree[i] = XBFalse;
    centralGames[i].port = 0;
    centralGames[i].version[0] = VERSION_MAJOR;
    centralGames[i].version[1] = VERSION_MINOR;
    centralGames[i].version[2] = VERSION_PATCH;
    centralGames[i].numLives = 0;
    centralGames[i].numWins = 0;
    centralGames[i].frameRate = 0;
    centralGames[i].time = time(NULL);

    strncpy(q, (s+l[i]), (l[i+1]-l[i]));
    q[l[i+1]-l[i]] = 0;
    Dbg_C2B("-- %s --\n",q);
    sprintf(centralGames[i].game,"%s",q);
  }
  return &rComm->browse.comm;
} /* C2B_CreateComm */

/*
 * end of file com_central.c
 */
