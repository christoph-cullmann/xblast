/*
 * file w32_msgbox.c - message boxes 
 *
 * $Id: w32_msgbox.c,v 1.2 2004/05/14 10:00:36 alfie Exp $
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
#include "gui.h"

#include "w32_common.h"

static char msgText[1024];

/*
 *
 */
void
GUI_ErrorMessage (const char *fmt, ...)
{
  va_list argList;

  va_start (argList, fmt);
  vsprintf (msgText, fmt, argList);
  va_end (argList);
  MessageBox (window, msgText, "XBlast Error", MB_ICONSTOP | MB_OK);
} /* GUI_ErrorMessage */

/*
 * end of file w32_msgbox.c
 */
