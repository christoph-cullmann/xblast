/*
 * file com.c - toplevel functions for all communications
 *
 * $Id: com.c,v 1.6 2006/02/09 21:21:23 fzago Exp $
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
 * file descriptor becomes writeable
 */
void
CommWriteable (int fd)
{
	/* find associated XBComm */
	XBComm *comm = CommFind (fd);
	if (NULL != comm) {
		/* call write handler */
		assert (NULL != comm->writeFunc);
		switch ((*comm->writeFunc) (comm)) {
		case XCR_Finished:
		case XCR_Error:
			/* remove on error or finish */
			assert (NULL != comm->deleteFunc);
			(void)(*comm->deleteFunc) (comm);
			break;
		default:
			break;
		}
	}
}								/* CommWriteable */

/*
 * file descriptor becomes readable
 */
void
CommReadable (int fd)
{
	/* find associated XBComm */
	XBComm *comm = CommFind (fd);
	if (NULL != comm) {
		/* call read handler */
		assert (NULL != comm->readFunc);
		switch ((*comm->readFunc) (comm)) {
		case XCR_Finished:
		case XCR_Error:
			/* remove on error or finish */
			assert (NULL != comm->deleteFunc);
			(void)(*comm->deleteFunc) (comm);
			break;
		default:
			break;
		}
	}
}								/* CommReadable */

/*
 * delete given communication
 */
void
CommDelete (XBComm * comm)
{
	assert (NULL != comm);
	assert (NULL != comm->deleteFunc);
	(void)(*comm->deleteFunc) (comm);
}								/* CommDelete */

/*
 * end of file com.c
 */
