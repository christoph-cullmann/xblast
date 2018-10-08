/*
 * file menu_edit.h - user interface for editing levels
 *
 * Program XBLAST
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * This file (C) Lars Luthman <larsl@users.sourceforge.net>
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
#ifndef MENU_EDIT_H
#define MENU_EDIT_H

#define STRING_LENGTH 200

extern XBBool CreateEditMenu (void *par);
extern void SetEditMapBlock (int x, int y);
extern XBBool SetToBlockFree (void);
extern XBBool SetOldBlock (void);
  // class variables

#endif
