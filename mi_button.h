/*
 * file mi_button.h -
 *
 * $Id: mi_button.h,v 1.6 2006/02/10 15:07:42 fzago Exp $
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
#ifndef _MI_BUTTON_H
#define _MI_BUTTON_H

extern XBMenuItem *MenuCreateHButton (int x, int y, int w, const char *text, MIC_button func,
									  void *funcData);
extern XBMenuItem *MenuCreateVButton (int x, int y, int h, const char *text, MIC_button func,
									  void *funcData);
extern void MenuSetButtonIcon (XBMenuItem * item, IconSpriteAnimation anime);
extern void MenuDeleteButton (XBMenuItem * item);
extern void MenuActivateButton (XBMenuItem * ptr, XBBool flag);
extern XBBool MenuExecButton (void);
extern void MenuButtonSetNextExec (MIC_button func, void *data);
#endif
/*
 * end of file mi_button.h
 */
