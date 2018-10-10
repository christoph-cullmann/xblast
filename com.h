/*
 * file com.h - toplevel functions for all communications
 *
 * $Id: com.h,v 1.3 2004/05/14 10:00:33 alfie Exp $
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
#ifndef XBLAST_COM_H
#define XBLAST_COM_H

/*
 * type declarations
 */
typedef struct _xb_comm XBComm;

/* types of communication */
typedef enum {
  XCR_OK = 0,
  XCR_Error,
  XCR_Finished,
  XCR_NoComm
} XBCommResult;

/*
 * global prototypes
 */
extern void CommWriteable (int fd);
extern void CommReadable (int fd);
extern void CommDelete (XBComm *comm);

#endif
/*
 * end of file com.h
 */
