/*
 * file cfg_xblast.h - general xblast configurations
 *
 * $Id: cfg_xblast.h,v 1.6 2006/02/09 21:21:23 fzago Exp $
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
#ifndef _CFG_XBLAST_H
#define _CFG_XBLAST_H

/*
 * modes of sound
 */
typedef enum
{
	XBSM_None,
	XBSM_Beep,
	XBSM_Waveout,
	/* no new elements after this line */
	NUM_XBSM
} XBSoundMode;

/*
 * Video modes
 */
typedef enum
{
	XBVM_Windowed,
	XBVM_Full,
	NUM_VBSM
} XBVideodMode;

/*
 * default central XBCC
 */
typedef struct
{
	const char *name;
	int port;
} CFGCentralSetup;

/*
 * mode definitions
 */
typedef struct
{
	XBSoundMode mode;
	XBBool stereo;
    /* AbsInt begin */
    XBBool beep;
    /* AbsInt end */
} CFGSoundSetup;

/*
 * mode definitions
 */
typedef struct
{
	XBVideodMode mode;
	XBBool full;
} CFGVideoSetup;

/*
 * global prototypes
 */
extern void LoadXBlastConfig (void);
extern void SaveXBlastConfig (void);
extern void FinishXBlastConfig (void);

extern void StoreSoundSetup (const CFGSoundSetup *);
extern XBBool RetrieveSoundSetup (CFGSoundSetup *);

extern void StoreVideoSetup (const CFGVideoSetup * video);
extern XBBool RetrieveVideoSetup (CFGVideoSetup * video);
extern void SetupVideo (CFGVideoSetup * video);

extern void StoreCentralSetup (const CFGCentralSetup *);	// XBCC
extern XBBool RetrieveCentralSetup (CFGCentralSetup *);
#endif
/*
 * end of file cfg_xblast.h
 */
