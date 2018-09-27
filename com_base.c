/*
 * file com_base.c - functions needed for all communications 
 *
 * $Id: com_base.c,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
#include "com_base.h"

/*
 * local variables
 */
XBComm *list = NULL;

/*
 * initialize communication
 */
void
CommInit (XBComm *comm, XBCommType type, XBSocket *socket, XBCommFunc readFunc, XBCommFunc writeFunc, 
	  XBCommFunc deleteFunc)
{
  /* sanity checks */
  assert (NULL != comm);
  assert (NULL != socket);
  /* add to list */
  comm->next = list;
  list       = comm;
  comm->prev = NULL;
  if (NULL != comm->next) {
    comm->next->prev = comm;
  }
  /* set values */
  comm->type       = type;
  comm->socket     = socket;
  comm->readFunc   = readFunc;
  comm->writeFunc  = writeFunc;
  comm->deleteFunc = deleteFunc;
  /* register socket for reading */
  Socket_RegisterRead (comm->socket);
} /* CommInit */

/*
 *
 */
void
CommFinish (XBComm *comm)
{
  assert (comm != NULL);
  /* unregister socket for reading */
  Socket_UnregisterRead  (comm->socket);
  Socket_UnregisterWrite (comm->socket);
  /* delete from list */
  if (comm->next != NULL) {
    comm->next->prev = comm->prev;
  }
  if (comm->prev != NULL) {
    comm->prev->next = comm->next;
  }
  if (comm == list) {
    list = list->next;
  }
  /* close socket */
  Net_Close (comm->socket);
} /* CommFinish */

/*
 * get socket descriptor for communication
 */ 
XBSocket * 
CommSocket (XBComm *comm)
{
  assert (NULL != comm);
  return comm->socket;
} /* CommSocket */

/*
 *
 */
XBComm *
CommFind (int fd)
{
  XBComm *comm;

  for (comm = list; comm != NULL; comm = comm->next) {
    if (fd == Socket_Fd (comm->socket)) {
      return comm;
    }
  }
  return NULL;
} /* CommFind */

/*
 * end of file com_base.c
 */
