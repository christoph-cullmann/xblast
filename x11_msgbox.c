/*
 * file x11_msgbox.c - 
 *
 * $Id: x11_msgbox.c,v 1.5 2006/02/09 21:21:25 fzago Exp $
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
 *
 */

#include "xblast.h"
#include "x11_common.h"

/*
 * not yet implemented
 */
void
GUI_ErrorMessage (const char *fmt, ...)
{
	va_list argList;

	GUI_Bell ();
	fputs ("*** ", stderr);
	va_start (argList, fmt);
	vfprintf (stderr, fmt, argList);
	va_end (argList);
	fputs (" ***\n", stderr);
}								/* GUI_ErrorMessage */

/*
 * end of file x11_msgbox.c
 */
