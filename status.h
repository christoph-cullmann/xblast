/*
 * file status.h
 *
 * $Id: status.h,v 1.7 2006/02/10 15:07:35 fzago Exp $
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
#ifndef XBLAST_STATUS_H
#define XBLAST_STATUS_H

extern void ClearStatusBar (int tile1, int tile2);
extern void InitStatusBar (int numPlayer);
extern void ResetStatusBar (const BMPlayer * ps, const char *msg, XBBool flag);
extern void UpdateStatusBar (const BMPlayer * ps, int g_time);
extern void SetMessage (const char *msg, XBBool perm);
extern void ResetMessage (void);
extern void SetGet (const char *msg, int perm);
extern void SetChat (char *msg, int perm);

#endif
/*
 * end of file status.c
 */
