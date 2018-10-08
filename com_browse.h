/*
 * file com_browse.h - base communication to browse for network games
 *
 * $Id: com_browse.h,v 1.6 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_BROWSE_H
#define XBLAST_COM_BROWSE_H

/*
 * type definitions
 */
typedef struct _xb_comm_browse XBCommBrowse;
typedef struct _xb_browse_data XBBrowseData;

typedef enum
{
	XBBE_Wait,
	XBBE_Write,
	XBBE_Dgram,
	XBBE_Browse,
	XBBE_Close,
} XBBrowseEvent;

/* receive handler */
typedef void (*BrowseReceiveFunc) (XBCommBrowse *, const XBBrowseTele * tele, const char *,
								   unsigned short);
/* activity handler */
typedef XBBool (*BrowseEventFunc) (XBCommBrowse *, XBBrowseEvent);

struct _xb_comm_browse
{
	XBComm comm;
	XBBrowseData *sndFirst;
	XBBrowseData *sndLast;
	BrowseReceiveFunc receiveFunc;
	BrowseEventFunc eventFunc;
};

/*
 * global prototypes
 */
extern XBComm *Browse_CommInit (XBCommBrowse *, XBCommType, XBSocket *, BrowseReceiveFunc,
								BrowseEventFunc, XBCommFunc);
extern void Browse_Send (XBCommBrowse * bComm, const char *hostname, unsigned port,
						 XBBool broadcast, const XBBrowseTeleAny * tele);
extern void Browse_Finish (XBCommBrowse *);

#endif
/*
 * end of file com_browse.h
 */
