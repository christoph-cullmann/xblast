/*
 * file com_query.h - client queryes for local network game
 *
 * $Id: com_query.h,v 1.9 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_COM_QUERY_H
#define XBLAST_COM_QUERY_H

/*
 * global prototypes
 */
extern XBComm *Query_CreateComm (unsigned id, const char *local, const char *remote,
								 unsigned short port, XBBool broadcast);
extern void Query_Send (XBComm * qComm, const struct timeval *tv);
extern void Query_NewGame (XBComm * qComm, const struct timeval *tv);
extern XBBool Query_isDeleted (XBComm * qComm);

#endif
/*
 * end of file com_query.h
 */
