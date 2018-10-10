/*
 * file user.c - communication interface for users
 *
 * $Id: user.c,v 1.6 2005/01/23 16:12:49 lodott Exp $
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
#include "user.h"
#include "atom.h"
#include "cfg_player.h"
#include "util.h"
#include "str_util.h"
/*
 * local macros
 */
#define TIME_POLL_QUERY 5

#define PID_NONE -1001
#define PID_INVALID -3

/*
 * local variables
 */
static XBComm  *comm  = NULL;
static int PID = PID_NONE;
static int received = 0;
static XBBool complete = XBFalse;

/**********************
 * connect/disconnect *
 **********************/

/*
 * try to connect to server
 */
XBBool
User_Connect (CFGCentralSetup *cfg)
{
  int  j;
  char tmp[16];
  /* create communication */
  assert (comm == NULL);
  comm = X2C_CreateComm (cfg);
  if (NULL == comm) {
    Dbg_User("failed to establish tcp connection to central\n");
    return XBFalse;
  }
  Dbg_User("successfully established tcp to central\n");
  /* init data */
  PID = PID_NONE;
  received = 0;
  complete = XBFalse;
  /* is that needed ? */
  j = sprintf(tmp,"tmpPlayer");
  tmp[j+1] = 0;
  Network_SetPlayer (0, 0, GUI_StringToAtom (tmp)) ;
  return XBTrue;
} /* User_Connect */

/*
 * disconnect from server by shutting down
 */
void
User_Disconnect ()
{
  if (comm != NULL) {
    Dbg_User("disconnecting from central\n");
    CommDelete (comm);
  } else {
    Dbg_User("already disconnected from central\n");
  }
} /* User_Disconnect */

/*
 * handle stream events
 */
XBBool
User_EventToCentral (const XBEventToCentral ev)
{
  switch (ev) {
  case XBE2C_IORead:
    Dbg_User("read error to central, shutdown!\n");
    return XBTrue;
  case XBE2C_IOWrite:
    Dbg_User("write error to central, shutdown!\n");
    return XBTrue;
  case XBE2C_InvalidCot:
    Dbg_User("invalid telegram CoT from central, shutdown!\n");
    return XBTrue;
  case XBE2C_InvalidID:
    Dbg_User("invalid telegram id central, ignoring!\n");
    return XBTrue;
  case XBE2C_UnexpectedEOF:
    Dbg_User("unexpected eof to central, shutdown!\n");
    return XBTrue;
  case XBE2C_StreamWaiting:
    Dbg_User("all queued data sent to central\n");
    return XBFalse;
  case XBE2C_StreamBusy:
    /* Dbg_User("data waits to be sent to central\n"); */
    return XBFalse;
  case XBE2C_StreamClosed:
    Dbg_User("connection to central has been removed\n");
    comm = NULL;
    return XBFalse;
  default:
    Dbg_User("unknown event on stream, ignoring\n");
    return XBFalse;
  }
} /* User_EventToCentral */

/****************
 * receive data *
 ****************/

/*
 * receive player config from server
 */
void
User_ReceivePlayerConfig (const char *data)
{
  XBAtom atom,atomID;
  CFGPlayerEx tmpPlayer;
  int         i;

  atom=Network_ReceivePlayerConfig (CT_Central, 0,0, data);
  /* if atom is valid, data is complete */
  if (ATOM_INVALID != atom) {
    Dbg_User("Got player from central\n");
    RetrievePlayerEx(CT_Central, atom, &tmpPlayer);
    i=tmpPlayer.id.PID;
    if (i>=0) {
      /* store player under valid pid */
      received++;
      atomID=GUI_IntToAtom(i);
      StorePlayerEx(CT_Central, atomID, &tmpPlayer);
    }
    /* remove the received database */
    DeletePlayerConfig(CT_Central, atom);
  }
} /* User_ReceivePlayerConfig */

/*
 * received last player
 */
void
User_NoMorePlayers() {
  Dbg_User("received %u players\n", received);
  complete = XBTrue;
  User_SendDisconnect();
} /* User_NoMorePlayers */

/*
 * receive player pid from server
 */
void
User_ReceivePlayerPID (const char *data)
{
  if ( !sscanf(data, "%i", &PID) ) {
    PID = PID_INVALID;
  }
} /* User_ReceivePlayerPID */

/*
 * central has disconnected
 */
void
User_ReceiveDisconnect (unsigned id)
{
  Network_QueueEvent (XBNW_Disconnected, id);
} /* User_ReceiveDisconnect */

/******************
 * get local data *
 ******************/

/*
 * return if connection is up
 */
XBBool
User_Connected()
{
  return ( comm != NULL );
} /* User_Connected */

/*
 * return PID
 */
int
User_GetPID()
{
  return PID;
} /* User_GetPID */

/*
 * return number of players received
 */
int
User_Received() {
  return received;
} /* User_Received */

/*
 * return if players completely received
 */
XBBool
User_Complete()
{
  return complete;
} /* User_Complete */

/**************
 * queue data *
 **************/

/*
 * send disconnect sequence, will shutdown after send
 */
void
User_SendDisconnect() {
  Dbg_User("queueing disconnect sequence to central\n");
  X2C_SendDisconnect(comm);
} /* User_SendDisconnect */

/*
 * queue registration data
 */
void
User_SendRegisterPlayer (XBAtom atom)
{
  PID = PID_NONE;
  X2C_SendPlayerConfig(comm, atom);
} /* Use_SendRegisterPlayer */

/*
 * unregister player
 */
void
User_SendUnregisterPlayer (XBAtom atom)
{
} /* User_SendUnregisterPlayer */

/*
 * queue request for scores
 */
void
User_RequestUpdate()
{
  complete = XBFalse;
  received = 0;
  RemoveAllPlayers(CT_Central);
  X2C_QueryPlayerConfig(comm);
  Dbg_User("queueing update request, old rankings deleted\n");
} /* User_RequestUpdate */

/*
 * queue current score
 */
void
User_SendGameStat (int numPlayers, BMPlayer *playerStat, int *pa)
{
  int PID[MAX_PLAYER];
  int Score[MAX_PLAYER];
  BMPlayer *ps,*ps2;
  int i,j,t=0,k;

  if(numPlayers>0) {
    for (i=0,j=0, ps = playerStat; i < numPlayers; ps ++, i++) {
      if(pa[i]) {
	PID[j]=ps->PID;
	Score[j]=0;
	for (k=0, ps2 = playerStat; k < numPlayers; ps2 ++, k++) {
	  if((ps->team==ps2->team) && (ps2->lives>0)) Score[j]=1;
	}
	t+=Score[j];
	j++;
      }
    }
    if(t==0) { // draw
      for(i=0;i<j;i++) {
	Score[i]=1;
      }
    }
  } else { // game stat
    j=-numPlayers;
    for (i=0, ps = playerStat; i < j; ps ++, i++) {
      PID[i]=ps->PID;
      Score[i]=ps->lives;
      Dbg_Out("send %d %d\n",PID[i],Score[i]);
    }
  }

  X2C_SendGameStat(comm, j, PID, Score);
} /* User_SendGameStat */

/*
 * end of file user.c
 */
