/*
 * file w32_socket.h - true bsd sockets for xblast
 *
 * $Id: w32_socket.h,v 1.2 2004/05/14 10:00:36 alfie Exp $
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
#ifndef XBLAST_W32_SOCKET_H
#define XBLAST_W32_SOCKET_H

#include "xblast.h"

/*
 * global prototypes
 */
extern void HandleSelect (UINT wParam, LONG lPAram);

#endif
/*
 * end of file w32_socket.h
 */
