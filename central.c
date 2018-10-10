/*
 * file central.c - communication interface for the server
 *
 * $Id: central.c,v 1.7 2005/01/23 16:55:24 lodott Exp $
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
#include "central.h"

#include "cfg_player.h"
#include "com_listen.h"
#include "com_from_central.h"
#include "random.h"

#include <math.h>
#include <limits.h>

#define RATING_WEIGTH 1.0
#define RATING_STEP 0.1

#define GJFAC 0.005
#define GJFAC2 1.005

#define SIZE 32

/*
 * local variables
 */
static XBComm *listenComm   = NULL;
static XBComm *centralComm  = NULL;

static int cPlayers;
static int cGames;
static int cGamesPlayed;
static int cTotalGames;
static int cLogins;
static int connected;
static XBAtom adminAtom;
CFGPlayerRating adminRating;

/*
 * Start to listen for clients
 */
XBBool
Central_StartListen (CFGGameHost *cfg)
{
  int               i,j;
  char              tmp[256];
  /* prepare database */
  LoadPlayerCentral(XBTrue);
  /* store current time */
  adminAtom = GUI_IntToAtom(0);
  RetrievePlayerRating (CT_Central, adminAtom, &adminRating);
  adminRating.timeRegister = time (NULL);
  StorePlayerRating (CT_Central, adminAtom, &adminRating);
  SavePlayerCentral();
  /* init stats */
  cPlayers = GetNumPlayerConfigs (CT_Central)-1; /* subtract administrator */
  cGames=0;
  cGamesPlayed=0;
  cTotalGames=adminRating.gamesPlayed;
  cLogins=0;
  connected=0;
  /* init connection slots */
  Network_Clear();
  for (i=0; i < NUM_LOCAL_PLAYER; i ++) {
    Network_SetPlayer (0, i, ATOM_INVALID);
  }
  for (i=0;i < MAX_PLAYER; i++) {
    j=sprintf(tmp,"tmpPlayer%i",i);
    tmp[j+1]=0;
    Network_SetPlayer (i, 0, GUI_StringToAtom (tmp)) ;
  };
  /* listen on tcp-port for clients */
  assert (listenComm == NULL);
  listenComm = CommCreateListen (cfg, XBTrue); /* central */
  if (NULL == listenComm) {
    Dbg_Central("failed to open tcp listen socket on port %u\n", cfg->port);
    return XBFalse;
  }
  Dbg_Central("listening on tcp socket on port %u\n", cfg->port);
  /* owse for games */
  assert (NULL == centralComm);
  centralComm = C2B_CreateComm (cfg->port);
  if (NULL == centralComm) {
    Dbg_Central ("failed to open udp socket on port %u\n", cfg->port);
    return XBFalse;
  }
  Dbg_Central("listening on udp socket on port %u\n", cfg->port);
  /* that's all */
  return XBTrue;
} /* Central_StartListen */

/*
 * delete port for listening
 */
void
Central_StopListen (void)
{
  /* delete listen port */
  if (NULL != listenComm) {
    CommDelete (listenComm);
    listenComm = NULL;
    Dbg_Central("stopped listening on tcp\n");
  }
  /* delete reply socket */
  if (NULL != centralComm) {
    CommDelete (centralComm);
    centralComm = NULL;
    Dbg_Central("stopped listening on udp\n");
  }
  /* Update administrator */
  adminRating.gamesPlayed=cTotalGames;
  adminRating.timeUpdate=time (NULL);
  StorePlayerRating (CT_Central, adminAtom, &adminRating);
  /* save and close rating DB */
  SavePlayerCentral();
  FinishPlayerCentral();
  Dbg_Central("finished");
} /* Central_StopListen */

/*
 * queue a disconnect to client
 */
void
Central_SendDisconnect (unsigned clientID)
{
  assert (clientID > 0);
  assert (clientID < MAX_HOSTS);
  /* disconnect from client */
  if (C2X_Connected (clientID) ) {
    C2X_Disconnect (clientID);
    Dbg_Central("queueing disconnect to client %u\n", clientID);
  }
} /* Central_SendDisconnect */

/*
 * disconnect from clients
 */
void
Central_SendDisconnectAll (void)
{
  unsigned clientID;

  /* disconnect from client */
  for (clientID = 1 ; clientID < MAX_HOSTS; clientID ++) {
    Central_SendDisconnect (clientID);
  }
} /* Central_SendDisconnectAll */

/*
 * central get statistics
 */
void
Central_GetStatistics(int *nPlayers, int *nGames, int *nGamesPlayed, int *nTotalGames, int *nLogins)
{
  *nPlayers = cPlayers;
  *nGames = C2B_GetOpenGames();
  *nGamesPlayed = cGamesPlayed;
  *nTotalGames = cTotalGames;
  *nLogins = cLogins;
} /* Central_GetStatistics */

/*
 * a client has connected
 */
void
Central_Accept (unsigned id, const char *hostName, unsigned port)
{
  CFGGameHost cfg;
  assert (hostName != NULL);
  assert (id < MAX_HOSTS);
  Dbg_Central ("client at adr=%s:%u connected, assigned id=%u\n", hostName, port, id);
  /* clear host entry in database */
  DeleteGameConfig (CT_Remote, atomArrayHost0[id]);
  /* store in database */
  memset (&cfg, 0, sizeof (cfg));
  cfg.name = hostName;
  cfg.port = port;
  StoreGameHost (CT_Remote, atomArrayHost0[id], &cfg);
  /* create message */
  Network_QueueEvent (XBNW_Accepted, id);
  cLogins++;
} /* Central_Accept */

/*
 * reverse byte order in an int
 */
static int
endian(int x) {
  char *i,*o;
  int  y;
  i = (char *)&x;
  o = (char *)&y;
  o[0] = i[3];
  o[1] = i[2];
  o[2] = i[1];
  o[3] = i[0];
  return y;
} /* endian */

/*
 * game config retrieved from client
 */
void
Central_ReceiveGameStat (const char *line)
{
  /*  line = numplayers, pid1, score1, pid2, score2, ... */
  int numPlayers,i,j,k,m,endia;
  int PID[MAX_PLAYER];
  int Score[MAX_PLAYER];
  int regPl[MAX_PLAYER];
  float q,plus[MAX_PLAYER],i1,j1;
  XBAtom pAtom[MAX_PLAYER];
  CFGPlayerRating rating[MAX_PLAYER];
  XBBool gamestat;

  char buffer[SIZE];
  time_t curtime;
  struct tm *loctime;

  Dbg_Central("receiving game stat\n");
  /* extract number of players from received data, check both byte orders */
  memcpy(&numPlayers,line,4);
  endia = 0;
  if ( (numPlayers < 0) || (numPlayers > MAX_PLAYER) ) {
    i = endian(numPlayers);
    if ((i >= 0) && (i <= MAX_PLAYER)) {
      Dbg_Central("reversing byte order for game stats!\n");
      numPlayers = i;
      endia = 1;
    } else {
      Dbg_Central("illegal game stat, ignoring\n");
      return;
    }
  }
  /* extract PIDs and scores */
  for (i=0; i<numPlayers; i++) {
    memcpy(PID+i,line+4+i*8,4);
    memcpy(Score+i,line+8+i*8,4);
    if (endia) {
      PID[i] = endian(PID[i]);
      Score[i] = endian(Score[i]);
    }
  }
  Dbg_Central("number scores = %u\n", numPlayers);
  /* determine rated players (rating>0) */
  for (i=0, k=0; i<numPlayers; i++) {
    if (PID[i] > 0) {
      pAtom[i] = GUI_IntToAtom(PID[i]);
      if (RetrievePlayerRating (CT_Central, pAtom[i], rating+i)) {
	if ((rating+i)->rating > 0) {
	  regPl[k] = i;
	  k++;
	}
      } else {
	Dbg_Central("player pid %u has no rating\n");
      }
    } else {
      Dbg_Central("invalid player pid for score %u\n", i);
    }
  }
  Dbg_Central("rated players = %u\n", k);
  /* time stamp all rated players, count number m of level winners */
  m=0;
  gamestat = XBFalse;
  for (i=0; i<k; i++) {
    j = regPl[i];
    Dbg_Central("Player %i, PID = %i, score = %i\n", j, PID[j], Score[j]);
    (rating+j)->timeUpdate = time(NULL);
    if (Score[j]<0) {
      gamestat = XBTrue;
    }
    m += Score[j];
    plus[j] = 0.0;
  }
  /* at least one player has negative score: match winners, summary update */
  if (gamestat) {
    Dbg_Central("received stat is game result\n");
    m = 0; /* empty pool */
    cTotalGames++;
    cGamesPlayed++;
    sprintf(buffer,"game_%i", cTotalGames);
    AppendGameResult (CT_Central, atomResults, GUI_StringToAtom(buffer), k, regPl, PID, Score); /*  XBST */
    for (i=0; i<k; i++) {
      j = regPl[i];
      (rating+j)->gamesPlayed++;
      if (Score[j]<0) {
	(rating+j)->realWins++;
	Score[j] = 1;
      } else {
	Score[j] = 0;
      }
      m += Score[j];
    }
    Dbg_Central("number of winners = %u\n",m);
    adminRating.gamesPlayed=cTotalGames;
    adminRating.timeUpdate=time (NULL);
    StorePlayerRating (CT_Central, adminAtom, &adminRating);
  } else {
    Dbg_Central("received stat is level result, number of winners = %u\n", m);
  }

#if 0
  /* old calculation method */
  for (i=0; i<k; i++) {
    i0 = regPl[i];
    i1 = (rating+i0)->rating;
    for (j = i+1; j<k; j++) {
      j0 = regPl[j];
      j1 = (rating+j0)->rating;
      if (Score[i0] == Score[j0]) {
	q = 0.5;
      } else if (Score[i0] > Score[j0]) {
	q = 1.0;
	(rating+i0)->relativeWins++;
      } else {
	q = 0.0;
	(rating+j0)->relativeWins++;
      }
      b = 1/(1+exp(RATING_WEIGTH*(j1-i1)));
      plus[i0] += RATING_STEP*(q-b);
      plus[j0] -= RATING_STEP*(q-b);
    }
  }
#endif

  /* update rating if a winner present and at least two rated players */
  if ((m>0) && (k>1)) {
    q = 0;
    j1 = 0;
    /* each player first spends a fixed fraction of his rating to a pool */
    for (i=0; i<k; i++) {
      j = regPl[i];
      i1 = (rating+j)->rating*GJFAC;
      plus[j] = -i1;
      j1 += i1;
    }
    /* the pool is blown up with another fixed factor */
    j1 *= GJFAC2;
    Dbg_Central("total value to distribute = %f points\n", j1);
    /* pool is evenly distributed among the winners */
    for (i=0; i<k; i++) {
      j = regPl[i];
      plus[j] += (j1*Score[j])/m;
    }
    /* apply the rating changes */
    for (i=0; i<k; i++) {
      j = regPl[i];
      plus[j] += (rating+j)->rating;
      Dbg_Central("rating change player %i = %f  ->  %f\n", PID[j], (rating+j)->rating, plus[j]);
      (rating+j)->rating = plus[j];
      (rating+j)->relativeWins += Score[j];
      /* store changes to database if registered */
      if (PID[j]>0) {
	StorePlayerRating (CT_Central, pAtom[j], rating+j);
      }
    }
    curtime = time (NULL);
    loctime = localtime (&curtime);
    strftime (buffer, SIZE, "%H_%d_%m_%Y", loctime);
    /* store time of the change now */
    AppendTimePlayerRating (CT_Central, atomTimeRatings, GUI_StringToAtom(buffer), k, regPl, PID, plus); /*  XBST     */
  } else {
    Dbg_Central("no points to distribute\n");
  }
  /* update administrator and save */
  SavePlayerCentral();
} /* Central_ReceiveGameConfig */

/*
 * check passwords, for update request
 */
XBBool
PasswordCheck(const char *a, const char *b)
{
  if ((NULL == a) && (NULL == b)) {
    return XBTrue;
  }
  if ((NULL != a) || (NULL == b)) {
    return XBFalse;
  }
  return (0 == strcmp(a,b)) ? XBTrue : XBFalse;
} /* PasswordCheck */

/*
 * player config received from client
 */
void
Central_ReceivePlayerConfig (unsigned id, const char *line)
{
  XBAtom      atom,atomID;
  CFGPlayerEx tmpPlayer, idPlayer;
  int         i;
  /* store player for config */
  atom = Network_ReceivePlayerConfig (CT_Central, id, 0, line);
  /* if atom is valid, data is complete */
  if (ATOM_INVALID != atom) {
    RetrievePlayerEx(CT_Central, atom, &tmpPlayer);
    i=tmpPlayer.id.PID;
    /* register or update */
    if (i>0) {
      Dbg_Central("receive update request for PID = %i\n",i);
      atomID = GUI_IntToAtom(i);
      if (RetrievePlayerEx(CT_Central, atomID, &idPlayer)) {
	Dbg_Central("PID is registered\n");
	if (PasswordCheck(tmpPlayer.id.pass, idPlayer.id.pass)) {
	  Dbg_Central("Password verified\n");
	  /* copy rating stuff, do not delete, big mistake */
	  tmpPlayer.rating.rating = idPlayer.rating.rating;
	  tmpPlayer.rating.gamesPlayed = idPlayer.rating.gamesPlayed;
	  tmpPlayer.rating.realWins = idPlayer.rating.realWins;
	  tmpPlayer.rating.relativeWins = idPlayer.rating.relativeWins;
	  tmpPlayer.rating.timeUpdate = idPlayer.rating.timeUpdate;
	  tmpPlayer.rating.timeRegister = idPlayer.rating.timeRegister;
	  StorePlayerEx(CT_Central, atomID, &tmpPlayer);
	  i = tmpPlayer.id.PID;
	  Dbg_Central("User updated\n");
	} else {
	  Dbg_Central("Bad password, no update\n");
	  i = -1;
	}
      } else {
	Dbg_Out("PID not registered, no update\n");
	i = -2;
      }
    } else {
      Dbg_Central("received registration request\n");
      if (-1 == FindDoubleName(CT_Central,atom)) {
	i = GameRandomNumber(INT_MAX);
	atomID = GUI_IntToAtom(i);
	while (NULL != GetPlayerName(CT_Central, atomID)) {
	  i = GameRandomNumber(INT_MAX);
	  atomID = GUI_IntToAtom(i);
	}
	tmpPlayer.rating.rating = 1000;
	tmpPlayer.rating.timeUpdate = 0;
	tmpPlayer.rating.timeRegister = time(NULL);
	tmpPlayer.id.PID = i;
	StorePlayerEx(CT_Central, atomID, &tmpPlayer);
	cPlayers++;
	Dbg_Central("registered player +%s+ with PID %i\n", GetPlayerName(CT_Central,atom), i);
      } else {
	Dbg_Central("name +%s+ already in use!\n", GetPlayerName(CT_Central,atom));
	i = -3;
      }
    }
    /* Delete temp */
    DeletePlayerConfig(CT_Central, atom);
    C2X_SendUserPID(id, i);
    Dbg_Central("queued PID = %i",i);
    SavePlayerCentral();
  }
} /* Central_ClientPlayerConfig */

/*
 * a client has connected
 */
void
Central_ReceiveDisconnect (unsigned id)
{
  /* create message */
  Network_QueueEvent (XBNW_Disconnected, id);
  Dbg_Central ("User id=%u disconnected\n", id);
} /* Central_ClientAccepted */

/*
 * end of file central.c
 */
