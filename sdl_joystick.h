/*
 * file sdl_joystick.h - joystick support for linux
 *
 * $Id: sdl_joystick.h 149513 2010-10-19 15:16:52Z swegener $
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
#ifndef XBLAST_SDL_JOYSTICK_H
#define XBLAST_SDL_JOYSTICK_H



/*
 * global macros
 */
#define NUM_JOYSTICKS 6

/*
 * global prototypes
 */
extern XBBool InitJoystick (void);
extern void FinishJoystick (void);

extern void HandleMenuJoystick (SDL_Event * event);
extern void HandleXBlastJoystick (SDL_Event * event);

#endif
/*
 * end of file sdl_joystick.h
 */
