/*
 * file menu_network.h - user interface for setting up networks games
 *
 * $Id: menu_network.h,v 1.4 2004/06/04 11:24:19 iskywalker Exp $
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
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef XBLAST_MENU_NETWORK_H
#define XBLAST_MENU_NETWORK_H

#include "xblast.h"

/*
 * global prototypes
 */
extern XBBool CreateServerMenu    (void *par);
extern XBBool CreateCentralMenu   (void *par); // XBCC
extern XBBool CreateClientMenu    (void *par);
extern XBBool CreateSearchLanMenu (void *par);
extern XBBool CreateSearchCentralMenu   (void *par); // XBCC
extern void setAutoCentral2(XBBool set);

#endif
/*
 * end of file menu_network.h
 */
