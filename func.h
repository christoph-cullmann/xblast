/*
 * func.h - special ingame function (pointers)
 *
 * $Id: func.h,v 1.7 2004/11/29 14:44:49 lodott Exp $
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
#ifndef XBLAST_FUNC_H
#define XBLAST_FUNC_H

#include "player.h"

/*
 * global variables
 */
extern void (*specialExtraFunc) (BMPlayer *);
extern void (*specialKeyFunc) (BMPlayer *);

/*
 * prototypes
 */
extern  void SpecialExtraJunkie (BMPlayer *ps);
extern  void SpecialExtraSlow (BMPlayer *ps);
extern  void SpecialExtraStunOthers (BMPlayer *ps);
extern  void SpecialExtraPoison (BMPlayer *ps);
extern  void SpecialExtraGhost (BMPlayer *ps);
extern XBBool ParseLevelFunc (const DBSection *section, DBSection *);
extern void ConfigLevelFunc (const DBSection *section);
extern XBBool HasSpecialBombs (void);
extern const char *GetKeyNameFunc(void *type);
extern const char *GetExtraNameFunc(void *type);
extern const char *GetKeyNameInt(int type);
extern const char *GetExtraNameInt(int type);
extern int GetNumberOfKeys( void  );
extern int GetNumberOfExtras( void  );
extern void SpecialExtraVoid (BMPlayer *ps);
extern void SpecialKeyVoid (BMPlayer *ps);
#endif
/*
 * end of file func.h
 */
