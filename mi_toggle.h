/*
 * file mi_toggle.h -
 *
 * $Id: mi_toggle.h,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#ifndef _MI_TOGGLE_H
#define _MI_TOGGLE_H

#include "mi_base.h"

extern XBMenuItem *MenuCreateToggle (int x, int y, int w, const char *text, XBBool *pState);
extern void MenuDeleteToggle (XBMenuItem *item);

#endif
/*
 * end of file mi_toggle.h
 */
