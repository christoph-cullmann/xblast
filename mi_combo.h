/*
 * file mi_combo.h -
 *
 * $Id: mi_combo.h,v 1.5 2006/02/09 21:21:24 fzago Exp $
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
#ifndef _MI_COMBO_H
#define _MI_COMBO_H

extern XBMenuItem *MenuCreateCombo (int x, int y, int w_text, const char *text, int w,
									int *value, void **data, XBAtom * atom,
									XBComboEntryList * table);
extern void MenuDeleteCombo (XBMenuItem * item);

#endif
/*
 * end of file mi_combo.h
 */
