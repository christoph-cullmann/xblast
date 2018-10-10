/*
 * file gui.h - interface for system dependent graphics engine
 *
 * $Id: gui.h,v 1.5 2004/10/19 17:59:17 iskywalker Exp $
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
#ifndef XBLAST_GUI_H
#define XBLAST_GUI_H

#include "sprite.h"
#include "cfg_player.h"

/*
 * constants
 */

/* font flags */
#define FF_Large       0x00
#define FF_Medium      0x01
#define FF_Small       0x02
#define FF_Black       0x00
#define FF_White       0x04
#define FF_Boxed       0x08
#define FF_Transparent 0x10
#define FF_Outlined    0x20
#define FF_Center      0x00
#define FF_Left        0x40
#define FF_Right       0x80
#define FF_Vertical    0x100
#define FF_Cursor      0x200
/* font flag mask */
#define FM_Size        0x03
#define FM_Color       0x04
#define FM_Boxed       0x08
#define FM_Transparent 0x10
#define FM_Outlined    0x20
#define FM_Align       0xC0

/*
 * types
 */
typedef enum {
  KB_NONE,   /* no keyboard events are generated */
  KB_MENU,   /* uses XBlast menu keymapping */
  KB_XBLAST, /* uses XBlast game keymapping */
  KB_KEYSYM, /* return string with keysymbol name (for text editor) */
  KB_ASCII   /* return ascii-value or control key (for keyboard config ) */
} XBKeyboardMode;

typedef enum {
  XBFM_IN,        /* blend in current pixmap */
  XBFM_BLACK_OUT, /* fade out current image to black */
  XBFM_WHITE_OUT  /* fade out current image to white */
} XBFadeMode;

typedef void (*XBPollFunction) (const struct timeval *);

typedef void (*XBQuitFunction) (void);

/*
 * prototypes
 */
/* init */
extern XBBool GUI_Init (int argc, char *argv[]);
extern void GUI_Finish (void);
extern void GUI_OnQuit (XBQuitFunction quitFunc);
/* event */
extern void GUI_UpdateKeyTables (void);
extern void GUI_SetTimer (long msec, XBBool periodic);
extern void GUI_SetKeyboardMode (XBKeyboardMode _mode);
extern void GUI_SetMouseMode (XBBool _enable);
extern XBEventCode GUI_WaitEvent (XBEventData *data);
extern XBEventCode GUI_PeekEvent (XBEventData *data);
extern void GUI_SendEventValue (XBEventCode code, int value);
extern void GUI_SendEventPointer (XBEventCode code, void *pointer);
extern void GUI_Sync (void);
extern void GUI_Bell (void);
extern int GUI_NumJoysticks (void);
extern void GUI_AddPollFunction (XBPollFunction func);
extern void GUI_SubtractPollFunction (XBPollFunction func);
/* pixmap */
extern void GUI_AddMazeRectangle (int x, int y);
extern void GUI_AddStatRectangle (int x, int y);
extern void GUI_AddChatRectangle (int x, int y);
extern void GUI_AddTilesRectangle (int x, int y);
extern void GUI_FlushPixmap (XBBool flag);
extern void GUI_FlushScoreBoard (void);
extern void GUI_ClearPixmap (void);
extern void GUI_InitFade (XBFadeMode mode, int maxLines);
extern XBBool GUI_DoFade (void);
/* sprite */
extern XBBool GUI_LoadPlayerSprite (int player, int anime, const CFGPlayerGraphics *config);
extern void GUI_LoadIconSprite (int index, XBColor color);
extern void GUI_DrawExplosionSprite (int x, int y, int block);
extern void GUI_DrawPlayerSprite (const Sprite *spl);
extern void GUI_DrawBombSprite (const Sprite *spl);
extern void GUI_DrawIconSprite (const Sprite *spl);
extern void GUI_DrawTextSprite (const Sprite *spl);
/* text */
extern void GUI_DrawSimpleTextbox (const char *text, unsigned flags, const BMRectangle *rect);
extern void GUI_DrawTextbox (const char *text, unsigned flags, const BMRectangle *rect);
extern void GUI_DrawPolygon (int x, int y, int w, int h, int lw, const BMPoint *points, 
			     int npoints, XBBool black_white);
/* tile */
extern void GUI_DrawBlock (int x, int y, int block);
extern void GUI_DrawExplosion (int x, int y, int block);
extern void GUI_FlushBlocks (void);
extern void GUI_InitExplosionBlocks (void);
extern void GUI_LoadBlockRgb (int id, const char *name);
extern void GUI_LoadBlockCch (int id, const char *name, XBColor fg, XBColor bg, XBColor add);
extern void GUI_FreeBlock (int block);
extern void GUI_FreeExplosionBlocks (void);
extern void GUI_LoadPlayerScoreTiles (int player, const CFGPlayerGraphics *config);
//#ifdef SMPF
extern void GUI_DrawScoreBlock (int x, int y, int block);
/*#else
extern void GUI_DrawScoreBlock (int x, int block);
#endif*/
extern void GUI_DrawTimeLed (int x, int block);
/* atoms */
extern XBBool GUI_InitAtoms (void);
extern XBAtom GUI_StringToAtom (const char *);
extern XBAtom GUI_FormatToAtom (const char *, ...);
extern XBAtom GUI_IntToAtom (int);
extern const char *GUI_AtomToString (XBAtom);
extern int GUI_AtomToInt (XBAtom);
/* config */
extern void GUI_LoadConfig (void);
extern void GUI_SaveConfig (void);
extern void GUI_FinishConfig (void);
/* message boxes */
extern void GUI_ErrorMessage (const char *fmt, ...);
/* images */
extern XBColor GUI_ParseColor (const char *name);

#endif
/*
 * end of file gui.h
 */
