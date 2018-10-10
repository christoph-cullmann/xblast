/*
 * file mi_string.h -
 *
 * $Id: mi_string.h,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#ifndef _MI_STRING_H
#define _MI_STRING_H

#include "mi_base.h"

extern XBMenuItem *MenuCreateString (int x, int y, int w_text, const char *text, int w, char *buffer, size_t len);
extern void MenuDeleteString (XBMenuItem *item);

#endif
/*
 * end of file mi_string.h
 */
