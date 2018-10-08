/*
 * file menu_layout.h - macros for menu layouts
 *
 * $Id: menu_layout.h,v 1.4 2006/02/09 18:31:45 fzago Exp $
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
#ifndef XBLAST_MENU_LAYOUT_H
#define XBLAST_MENU_LAYOUT_H

/*
 * global macros
 */
#define TITLE_WIDTH    	 (5*CELL_W)
#define TITLE_LEFT     	 (5*CELL_W)
#define TITLE_TOP      	 (2*CELL_H)

#define MENU_WIDTH     	 (6*CELL_W)
#define MENU_LEFT      	 (9*CELL_W/2)
#define MENU_TOP       	 (3*CELL_H)
#define MENU_ROW(y)    	 (MENU_TOP+(y)*CELL_H)
#define MENU_BOTTOM    	 (11*CELL_H)
#define MENU_LEVEL_TOP 	 (0*CELL_H)

#define DLG_TOP        	 (3*CELL_H)
#define DLG_ROW(y)     	 (DLG_TOP+(y)*CELL_H)
#define DLG_WIDTH      	 (7*CELL_W)
#define DLG_LEFT       	 (4*CELL_W)

#define PLAYER_TOP     	 (0*CELL_W)
#define PLAYER_LEFT(i,n) ((2*(i) - (n) + 16)*CELL_W/2)
#define PLAYER_WIDTH     (2*CELL_W)

#define GSTAT_TOP         ( 3*CELL_H)
#define GSTAT_ROW(i)      (GSTAT_TOP+((i)+1)*CELL_H/2)
#define GSTAT_WIDTH       (10*CELL_W)
#define GSTAT_LEFT        ( 5*CELL_W/2)
#define GSTAT_B_WIDTH     ( 4*CELL_W)
#define GSTAT_BACK_LEFT   ( 3*CELL_W/2)
#define GSTAT_ESCAPE_LEFT (11*CELL_W/2)
#define GSTAT_FORW_LEFT   (19*CELL_W/2)
#define MAX_GSTAT_ROWS     15

#define SEARCH_LEFT     ( 3*CELL_W/2)
#define SEARCH_WIDTH    (24*CELL_W/2)
#define SEARCH_TOP      ( 3*CELL_H)
#define SEARCH_ROW(i)   (SEARCH_TOP+((i)+1)*CELL_H/2)
#define NUM_SEARCH_ROWS  15

#define MSG_LEFT         (3*CELL_W)
#define MSG_WIDTH        (9*CELL_W)
#define MSG_EDIT         (6*CELL_W)
#define MSG_TOP          (TITLE_TOP+CELL_H)

#define COLOR_LEFT       ( 3*CELL_W)
#define COLOR_RIGHT      ( 8*CELL_W)
#define COLOR_WIDTH      ( 4*CELL_W)

#define LEVEL_COLS 4
#define LEVEL_ROWS 16

#endif
/*
 * end of file menu_layout.h
 */
