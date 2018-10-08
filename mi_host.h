/*
 * file mi_host.h - Menu item for host selection in networked games
 *
 * $Id: mi_host.h,v 1.9 2006/02/09 21:21:24 fzago Exp $
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
#ifndef XBLAST_MI_HOST_H
#define XBLAST_MI_HOST_H

/*
 * global prototypes
 */
extern XBMenuItem *MenuCreateHost (int x, int y, int w, unsigned client, const char **pText,
								   XBHSFocusFunc focusFunc, XBHSChangeFunc chgFunc,
								   XBHSUpdateFunc upFunc);
extern XBMenuItem *MenuCreateServer (int x, int y, int w, const char **pText);
extern XBMenuItem *MenuCreateClient (int x, int y, int w, const char **pText, XBHostState * pState,
									 const int *pPing);
extern XBMenuItem *MenuCreatePeer (int x, int y, int w, const char **pText, XBHostState * pState,
								   const int *pPing);

extern XBMenuItem *MenuCreateTeam (int x, int y, int w, unsigned id, unsigned player,
								   XBTSFocusFunc focusFunc, XBTSChangeFunc chgFunc,
								   XBTSUpdateFunc upFunc);
extern XBMenuItem *MenuCreateServerTeam (int x, int y, int w, XBTeamState * pTeam);
extern XBMenuItem *MenuCreatePeerTeam (int x, int y, int w, XBTeamState * pTeam);

extern void MenuDeleteHost (XBMenuItem * item);
extern void MenuDeleteTeam (XBMenuItem * item);

#endif
/*
 * end of file mi_host.h
 */
