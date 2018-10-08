/*
 * cfg_main.h - managing configuration files
 *
 * $Id: cfg_main.h,v 1.6 2006/02/09 21:21:23 fzago Exp $
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
#ifndef XBLAST_CFG_MAIN_H
#define XBLAST_CFG_MAIN_H

/*
 * global types
 */
typedef enum
{
	CT_Local = 0,
	CT_Remote,
	CT_Demo,
	CT_Central,
	NUM_CT
} CFGType;

/*
 * global prototypes
 */
extern void InitConfig (void);
extern void SaveConfig (void);
extern XBBool CheckConfig (void);
extern void FinishConfig (void);

#endif
/*
 * end of file cfg_main.h
 */
