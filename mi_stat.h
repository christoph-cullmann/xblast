/*
 * file mi_stat.h - menu item for displaying game table
 *
 * $Id: mi_stat.h,v 1.3 2004/05/14 10:00:35 alfie Exp $
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
#ifndef XBLAST_MI_STAT_H
#define XBLAST_MI_STAT_H

#include "mi_base.h"
#include "cfg_stat.h"
#include "dat_rating.h" // XBCC
#include "cfg_demo.h"

extern XBMenuItem *MenuCreateStatEntry (int x, int y, int w, const XBStatData *stat, MIC_button func, void *funcPar);
extern XBMenuItem *MenuCreateStatHeader (int x, int y, int w, const char *title);
extern XBMenuItem *MenuCreateDemoEntry (int x, int y, int w, const CFGDemoEntry *demo, MIC_button func, void *funcPar);
extern XBMenuItem *MenuCreateDemoHeader (int x, int y, int w);
extern XBMenuItem *MenuCreateGameEntry (int x, int y, int w, const XBNetworkGame **, MIC_button func);
extern XBMenuItem *MenuCreateGameHeader (int x, int y, int w);
extern XBMenuItem *MenuCreateCentralEntry (int x, int y, int w, const XBCentralData *data, MIC_button func, void *funcData); // XBCC
extern XBMenuItem *MenuCreateCentralHeader (int x, int y, int w, const char *title); // XBCC
extern XBMenuItem *MenuCreateInfoEntry (int x, int y, int w, const XBCentralInfo *data, MIC_button func, void *funcData); // XBCC
extern XBMenuItem *MenuCreateInfoHeader (int x, int y, int w, const char *title); // XBCC

extern void MenuDeleteTable (XBMenuItem *item);
extern void MenuActivateTable (XBMenuItem *ptr, XBBool flag);

#endif
/*
 * end of file mi_stat.h
 */
