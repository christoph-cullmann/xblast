/*
 * file mi_cyclic.c - menu item which periodically calls a given function
 * 
 * $Id: mi_cyclic.c,v 1.5 2006/02/09 21:21:24 fzago Exp $
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

#include "xblast.h"

/*
 * local types
 */
typedef struct
{
	XBMenuItem item;
	MIC_cyclic func;
	void *par;
} XBMenuCyclicItem;

/*
 * cyclic function
 */
static void
MenuPollCyclic (XBMenuItem * item)
{
	XBMenuCyclicItem *cyclic = (XBMenuCyclicItem *) item;

	assert (cyclic != NULL);
	(*cyclic->func) (cyclic->par);
}								/* MenuCyclicCyclic */

/*
 * create new item
 */
XBMenuItem *
MenuCreateCyclic (MIC_cyclic func, void *par)
{
	XBMenuCyclicItem *cyclic;

	assert (func != NULL);
	/* create item */
	cyclic = calloc (1, sizeof (*cyclic));
	assert (NULL != cyclic);
	MenuSetItem (&cyclic->item, MIT_Cyclic, 0, 0, 0, 0, NULL, NULL, NULL, MenuPollCyclic);
	/* set item specific values */
	cyclic->func = func;
	cyclic->par = par;
	/* that's all */
	return &cyclic->item;
}								/* MenuCreateCyclic */

/*
 * delete item
 */
void
MenuDeleteCyclic (XBMenuItem * item)
{
}								/* MenuDeleteCyclic */

/*
 * end of file mi_cyclic.c
 */
