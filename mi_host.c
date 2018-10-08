/*
 * file mi_host.c - Menu item for host selection in networked games
 *
 * $Id: mi_host.c,v 1.25 2006/02/09 21:21:24 fzago Exp $
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
 * local macors
 */
#define FF_HOST_NAME_FOCUS     (FF_Small | FF_Black | FF_Center | FF_Outlined)
#define FF_HOST_NAME_NO_FOCUS  (FF_Small | FF_White | FF_Center)
#define FF_HOST_STATE_FOCUS    (FF_Large | FF_Black | FF_Center | FF_Outlined)
#define FF_HOST_STATE_NO_FOCUS (FF_Large | FF_White | FF_Center)

/*
 * local types
 */
typedef struct
{
	XBMenuItem item;			/* generic item data */
	/* host name */
	const char **pText;			/* link to current host name string */
	const char *cText;			/* displayed host name string */
	Sprite *tSprite;			/* host name text sprite */
	/* host state */
	XBHostState *pState;		/* link to current host state */
	XBHostState cState;			/* displayed host state */
	Sprite *sSprite;			/* host state text sprite */
	/* host ping */
	const int *pPing;			/* link to current ping time */
	int cPing;					/* currently displayed ping value */
	char tPing[16];				/* currently displayer ping string */
	Sprite *pSprite;			/* host ping text sprite */

	/* host state requests */
	unsigned id;				/* host id */
	XBHSFocusFunc focusFunc;	/* handler for state focus changes */
	XBHSChangeFunc chgFunc;		/* handler for local state changes */
	XBHSUpdateFunc upFunc;		/* handler for ping/state updates */
	/* locally requested state */
	XBHostState lState;			/* current local requested state */
	Sprite *lSprite;			/* local request text sprite */
	/* requests known to server */
	XBHostState cStateReq[MAX_HOSTS];	/* displayed externak requests */
	char tRequests[MAX_HOSTS + 1];	/* displayed requests string */
	Sprite *rSprite;			/* requests text sprite */
} XBMenuHostItem;

typedef struct
{
	XBMenuItem item;			/* generic item data */
	/* team state */
	XBTeamState *pTeam;			/* link to current team state */
	XBTeamState cTeam;			/* displayed team state */
	Sprite *sSprite;			/* team state text sprite */
	/* team state requests */
	unsigned id;				/* host id */
	unsigned player;			/* player id */
	XBTSFocusFunc focusFunc;	/* handler for team focus changes */
	XBTSChangeFunc chgFunc;		/* handler for local team changes */
	XBTSUpdateFunc upFunc;		/* handler for team updates */
	/* locally requested team */
	XBTeamState lTeam;			/* currently requested team */
	Sprite *lSprite;			/* state request text sprite */
	/* requests known to server */
	XBTeamState cTeamReq[MAX_HOSTS];	/* display team requests */
	char tRequests[MAX_HOSTS + 1];	/* displayed requests string */
	Sprite *rSprite;			/* requests text sprite */
} XBMenuTeamItem;

/*
 * local variables
 */
static XBHostState serverState = XBHS_Server;
static const char *stateText[NUM_XBHS] = {
	"?",						/* XBHS_None */
	"...",						/* XBHS_Wait */
	"In",						/* XBHS_In */
	"Out",						/* XBHS_Out */
	"Srv",						/* XBHS_Server */
	"Go!",						/* XBHS_Ready */
};

static const char *teamText[NUM_XBTS] = {
	"",							/* XBTS_Invalid */
	"None",						/* XBTS_None */
	"Red",						/* XBTS_Red */
	"Green",					/* XBTS_Green */
	"Blue",						/* XBTS_Blue */
	"Out",						/* XBTS_Out */
};

/*******************
 * shared routines *
 *******************/

/*
 * shared host focus handler
 */
static void
MenuHostFocusShared (XBMenuItem * ptr, XBBool flag)
{
	XBMenuHostItem *host = (XBMenuHostItem *) ptr;

	assert (NULL != host);
	assert (NULL != host->tSprite);
	assert (NULL != host->sSprite);
	SetSpriteAnime (host->tSprite, flag ? FF_HOST_NAME_FOCUS : FF_HOST_NAME_NO_FOCUS);
	SetSpriteAnime (host->sSprite, flag ? FF_HOST_STATE_FOCUS : FF_HOST_STATE_NO_FOCUS);
	if (NULL != host->pSprite) {
		SetSpriteAnime (host->pSprite, flag ? FF_HOST_NAME_FOCUS : FF_HOST_NAME_NO_FOCUS);
	}
}								/* MenuHostFocusShared */

/*
 * polling a host item
 */
static void
MenuHostPollShared (XBMenuItem * ptr)
{
	XBMenuHostItem *host = (XBMenuHostItem *) ptr;
	assert (NULL != host);
	assert (NULL != host->tSprite);
	assert (NULL != host->sSprite);
	/* check state change, old style: link to array in menu_network.c */
	if (*host->pState != host->cState) {
		host->cState = *host->pState;
		assert (host->cState < NUM_XBHS);
		SetSpriteText (host->sSprite, stateText[host->cState]);
	}
	/* check name change */
	if (*host->pText != host->cText) {
		host->cText = *host->pText;
		SetSpriteText (host->tSprite, host->cText);
	}
	/* update ping, old style: link to array in menu_network.c */
	if (NULL != host->pPing && *host->pPing != host->cPing) {
		host->cPing = *host->pPing;
		if (host->cPing < 0) {
			host->tPing[0] = 0;
		}
		else {
			sprintf (host->tPing, "%u ms", host->cPing);
		}
		SetSpriteText (host->pSprite, host->tPing);
	}
}								/* MenuSharedPoll */

/*
 * shared team focus handler
 */
static void
MenuTeamFocusShared (XBMenuItem * ptr, XBBool flag)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;

	assert (NULL != team);
	assert (NULL != team->sSprite);
	SetSpriteAnime (team->sSprite, flag ? FF_HOST_NAME_FOCUS : FF_HOST_NAME_NO_FOCUS);

}								/* MenuTeamFocusShared */

/*
 * polling a team item
 */
static void
MenuTeamPollShared (XBMenuItem * ptr)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;

	assert (NULL != team);
	/* state */
	if (*team->pTeam != team->cTeam) {
		team->cTeam = *team->pTeam;
		assert (team->cTeam < NUM_XBTS);
		SetSpriteText (team->sSprite, teamText[team->cTeam]);
	}
}								/* MenuTeamPollShared */

/*********************
 * generic host item *
 *********************/

/*
 * host item receives focus
 */
static void
MenuHostFocus (XBMenuItem * ptr, XBBool flag)
{
	XBMenuHostItem *host = (XBMenuHostItem *) ptr;
	assert (NULL != host->focusFunc);
	(*host->focusFunc) (host->id);
	MenuHostFocusShared (ptr, flag);
}								/* MenuHostFocus */

/*
 * host item selected
 */
static void
MenuHostSelect (XBMenuItem * ptr)
{
	XBMenuHostItem *host = (XBMenuHostItem *) ptr;
	assert (NULL != host);
	assert (NULL != host->chgFunc);
	assert (host->id < MAX_HOSTS);
	if ((*host->chgFunc) (host->id, &host->lState)) {
		/* update the local request sprite */
		if (host->lState == host->cState || host->lState == XBHS_None) {
			SetSpriteText (host->lSprite, "");
		}
		else {
			SetSpriteText (host->lSprite, stateText[host->lState]);
		}
	}
}								/* MenuHostSelect */

/*
 * host item mouse event
 */
static void
MenuHostMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (code == XBE_MOUSE_1) {
		MenuHostSelect (ptr);
	}
}								/* MenuHostMouse */

/*
 * host item polling
 */
static void
MenuHostPoll (XBMenuItem * ptr)
{
	XBMenuHostItem *host = (XBMenuHostItem *) ptr;
#ifdef REQUESTS
	unsigned i;
#endif
	assert (NULL != host);
	assert (NULL != host->tSprite);
	assert (NULL != host->sSprite);
	if ((*host->upFunc) (host->id, &host->cState, &host->cStateReq[0], &host->cPing)) {
		assert (host->cState < NUM_XBHS);
		/* create requests and ping */
		if (host->cState == XBHS_None) {
			host->tRequests[0] = (char)0;
			host->tPing[0] = (char)0;
		}
		else {
#ifdef REQUESTS
			/* update requests text */
			for (i = 0; i < MAX_HOSTS; i++) {
				host->tRequests[i] = *stateText[host->cStateReq[i]];
			}
			host->tRequests[MAX_HOSTS] = (char)0;
#endif
			/* update ping text */
			if (host->cPing < 0) {
				host->tPing[0] = (char)0;	/* ping undefined, display nothing */
			}
			else {
				sprintf (host->tPing, "%u ms", host->cPing);
			}
		}
		/* display requests, ping, current */
#ifdef REQUESTS
		SetSpriteText (host->rSprite, host->tRequests);
#endif
		SetSpriteText (host->sSprite, stateText[host->cState]);
		SetSpriteText (host->pSprite, host->tPing);
	}
	/* clear local request sprite if it coincides with current state or not set */
	if (host->lState == XBHS_None || host->lState == host->cState) {
		SetSpriteText (host->lSprite, "");
	}
	/* check name change, direct link */
	if (*(host->pText + host->id) != host->cText) {
		host->cText = *(host->pText + host->id);
		SetSpriteText (host->tSprite, host->cText);
	}
}								/* MenuHostPoll */

/*
 * create generic host button
 */
XBMenuItem *
MenuCreateHost (int x, int y, int w, unsigned client, const char **pText, XBHSFocusFunc focusFunc,
				XBHSChangeFunc chgFunc, XBHSUpdateFunc upFunc)
{
	/* create item */
	XBMenuHostItem *host = calloc (1, sizeof (*host));
	assert (host != NULL);
	MenuSetItem (&host->item, MIT_Host, x, y, w, CELL_H, MenuHostFocus, MenuHostSelect,
				 MenuHostMouse, MenuHostPoll);
	/* remember host id */
	host->id = client;
	/* set handlers */
	host->focusFunc = focusFunc;
	host->chgFunc = chgFunc;
	host->upFunc = upFunc;
	/* set initial state info */
	host->cState = XBHS_None;
	host->sSprite =
		CreateTextSprite (stateText[host->cState], (x + 1) * BASE_X, (y + 1) * BASE_Y,
						  (w / 4) * BASE_X, CELL_H / 2 * BASE_Y, FF_HOST_NAME_NO_FOCUS, SPM_MAPPED);
	/* set initial local request info */
	host->lState = XBHS_None;
	host->lSprite =
		CreateTextSprite ("", (x + w / 5) * BASE_X, (y + 1) * BASE_Y, (w / 4) * BASE_X,
						  CELL_H / 2 * BASE_Y, FF_HOST_NAME_NO_FOCUS, SPM_MAPPED);
	/* set initial requests info */
	memset (host->cStateReq, 0, sizeof (host->cStateReq));
	host->rSprite =
		CreateTextSprite (host->tRequests, (x + 1) * BASE_X, (y + CELL_H / 2) * BASE_Y,
						  (w / 2) * BASE_X, (CELL_H / 2 - 1) * BASE_Y, FF_HOST_NAME_NO_FOCUS,
						  SPM_MAPPED);
	/* set initial ping info */
	host->cPing = -1;
	host->tPing[0] = (char)0;
	host->pSprite =
		CreateTextSprite (host->tPing, (x + w / 2) * BASE_X, (y + CELL_H / 2) * BASE_Y,
						  (w / 2 - 1) * BASE_X, (CELL_H / 2 - 1) * BASE_Y, FF_HOST_NAME_NO_FOCUS,
						  SPM_MAPPED);
	/* link host name */
	assert (pText != NULL);
	host->pText = pText;
	host->cText = *(pText + host->id);
	host->tSprite =
		CreateTextSprite (host->cText, (x + 2 * w / 5) * BASE_X, (y + 1) * BASE_Y,
						  (3 * w / 5 - 1) * BASE_X, CELL_H / 2 * BASE_Y, FF_HOST_NAME_NO_FOCUS,
						  SPM_MAPPED);
	/* graphics */
	MenuAddLargeFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	/* poll once */
	MenuHostPoll (&host->item);
	return &host->item;
}								/* MenuCreateServer */

/********************
 * server host item *
 ********************/

/*
 * host item has focus
 */
static void
MenuServerFocus (XBMenuItem * ptr, XBBool flag)
{
	MenuHostFocusShared (ptr, flag);
}								/* MenuServerFocus */

/*
 * host item selected
 */
static void
MenuServerSelect (XBMenuItem * ptr)
{
}								/* MenuServerSelect */

/*
 * host item mouse event
 */
static void
MenuServerMouse (XBMenuItem * ptr, XBEventCode code)
{
}								/* MenuServerMouse */

/*
 * polling
 */
static void
MenuServerPoll (XBMenuItem * ptr)
{
	MenuHostPollShared (ptr);
}								/* MenuServerPoll */

/*
 * create server button
 */
XBMenuItem *
MenuCreateServer (int x, int y, int w, const char **pText)
{
	/* create item */
	XBMenuHostItem *host = calloc (1, sizeof (*host));
	assert (host != NULL);
	MenuSetItem (&host->item, MIT_Host, x, y, w, CELL_H, MenuServerFocus, MenuServerSelect,
				 MenuServerMouse, MenuServerPoll);
	/* set server specific data */
	assert (pText != NULL);
	host->pText = pText;
	host->cText = *pText;
	host->pState = &serverState;
	host->cState = XBHS_None;
	/* sprite with host name */
	host->tSprite =
		CreateTextSprite (*pText, (x + 11) * BASE_X, y * BASE_Y, (w - 12) * BASE_X, CELL_H * BASE_Y,
						  FF_HOST_NAME_NO_FOCUS, SPM_MAPPED);
	/* sprite with host state */
	host->sSprite =
		CreateTextSprite (stateText[0], (x + 1) * BASE_X, y * BASE_Y, 9 * BASE_X, CELL_H * BASE_Y,
						  FF_HOST_STATE_NO_FOCUS, SPM_MAPPED);
	/* sprite with state request */
	host->lSprite = NULL;
	/* sprite with requests */
	host->rSprite = NULL;
	/* graphics */
	MenuAddLargeFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	return &host->item;
}								/* MenuCreateServer */

/********************
 * client host item *
 ********************/

/*
 * host item has focus
 */
static void
MenuClientFocus (XBMenuItem * ptr, XBBool flag)
{
	MenuHostFocusShared (ptr, flag);
}								/* MenuClientFocus */

/*
 * host item selected
 */
static void
MenuClientSelect (XBMenuItem * ptr)
{
	XBMenuHostItem *host = (XBMenuHostItem *) ptr;

	assert (NULL != host);
	assert (NULL != host->pState);
	switch (*host->pState) {
	case XBHS_In:
		*host->pState = XBHS_Out;
		break;
	case XBHS_Out:
		*host->pState = XBHS_In;
		break;
	default:
		break;
	}
}								/* MenuClientSelect */

/*
 * host item mouse event
 */
static void
MenuClientMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (code == XBE_MOUSE_1) {
		MenuClientSelect (ptr);
	}
}								/* MenuClientMouse */

/*
 * polling
 */
static void
MenuClientPoll (XBMenuItem * ptr)
{
	MenuHostPollShared (ptr);
}								/* MenuClientPoll */

/*
 * create server button
 */
XBMenuItem *
MenuCreateClient (int x, int y, int w, const char **pText, XBHostState * pState, const int *pPing)
{
	/* create item */
	XBMenuHostItem *host = calloc (1, sizeof (*host));
	assert (host != NULL);
	MenuSetItem (&host->item, MIT_Host, x, y, w, CELL_H, MenuClientFocus, MenuClientSelect,
				 MenuClientMouse, MenuClientPoll);
	/* set server specific data */
	assert (pText != NULL);
	assert (pState != NULL);
	host->pText = pText;
	host->cText = *pText;
	host->pState = pState;
	host->cState = *pState;
	host->pPing = pPing;
	host->cPing = -1;
	host->rSprite = NULL;
	/* sprite with host name */
	host->tSprite =
		CreateTextSprite (*pText, (x + 11) * BASE_X, (y + 1) * BASE_Y, (w - 12) * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_NO_FOCUS, SPM_MAPPED);
	/* sprite with ping to host */
	host->pSprite =
		CreateTextSprite (host->tPing, (x + 11) * BASE_X, (y + CELL_H / 2) * BASE_Y,
						  (w - 12) * BASE_X, (CELL_H / 2 - 1) * BASE_Y, FF_HOST_NAME_NO_FOCUS,
						  SPM_MAPPED);
	/* sprite with host state */
	host->sSprite =
		CreateTextSprite (stateText[*pState], (x + 1) * BASE_X, y * BASE_Y, 9 * BASE_X,
						  CELL_H * BASE_Y, FF_HOST_STATE_NO_FOCUS, SPM_MAPPED);
	/* sprite with state request */
	host->lSprite = NULL;
	/* sprite with requests */
	host->rSprite = NULL;
	/* graphics */
	MenuAddLargeFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	return &host->item;
}								/* MenuCreateServer */

/******************
 * peer host item *
 ******************/

/*
 * host item has focus
 */
static void
MenuPeerFocus (XBMenuItem * ptr, XBBool flag)
{
	MenuHostFocusShared (ptr, flag);
}								/* MenuPeerFocus */

/*
 * host item is selected
 */
static void
MenuPeerSelect (XBMenuItem * ptr)
{
}								/* MenuPeerSelect */

/*
 * host item under mouse
 */
static void
MenuPeerMouse (XBMenuItem * ptr, XBEventCode code)
{
}								/* MenuPeerMouse */

/*
 * polling
 */
static void
MenuPeerPoll (XBMenuItem * ptr)
{
	MenuHostPollShared (ptr);
}								/* MenuPeerPoll */

/*
 * create peer item
 */
XBMenuItem *
MenuCreatePeer (int x, int y, int w, const char **pText, XBHostState * pState, const int *pPing)
{
	/* create item */
	XBMenuHostItem *host = calloc (1, sizeof (*host));
	assert (host != NULL);
	MenuSetItem (&host->item, MIT_Host, x, y, w, CELL_H, MenuPeerFocus, MenuPeerSelect,
				 MenuPeerMouse, MenuPeerPoll);
	/* set server specific data */
	assert (pText != NULL);
	assert (pState != NULL);
	host->pText = pText;
	host->cText = *pText;
	host->pState = pState;
	host->cState = *pState;
	host->pPing = pPing;
	host->cPing = -1;
	host->rSprite = NULL;
	/* sprite with host name */
	host->tSprite =
		CreateTextSprite (*pText, (x + 11) * BASE_X, (y + 1) * BASE_Y, (w - 12) * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_NO_FOCUS, SPM_MAPPED);
	/* sprite with ping to host */
	host->pSprite =
		CreateTextSprite (host->tPing, (x + 11) * BASE_X, (y + CELL_H / 2) * BASE_Y,
						  (w - 12) * BASE_X, (CELL_H / 2 - 1) * BASE_Y, FF_HOST_NAME_NO_FOCUS,
						  SPM_MAPPED);
	/* sprite with host state */
	host->sSprite =
		CreateTextSprite (stateText[*pState], (x + 1) * BASE_X, y * BASE_Y, 9 * BASE_X,
						  CELL_H * BASE_Y, FF_HOST_STATE_NO_FOCUS, SPM_MAPPED);
	/* sprite with state request */
	host->lSprite = NULL;
	/* sprite with requests */
	host->rSprite = NULL;
	/* graphics */
	MenuAddLargeFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	return &host->item;
}								/* MenuCreateServer */

/*********************
 * generic team item *
 *********************/

/*
 * team receives focus
 */
static void
MenuTeamFocus (XBMenuItem * ptr, XBBool flag)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;
	assert (NULL != team->focusFunc);
	(*team->focusFunc) (team->id, team->player);
	MenuTeamFocusShared (ptr, flag);
}								/* MenuTeamFocus */

/*
 * team item is selected
 */
static void
MenuTeamSelect (XBMenuItem * ptr)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;

	assert (NULL != team);
	/* first local request, init to current, if necessary */
	if (team->lTeam == XBTS_Invalid) {
		team->lTeam = team->cTeam;
	}
	/* get next reasonable local request */
	if ((*team->chgFunc) (team->id, team->player, &team->lTeam)) {
		assert (team->lTeam < NUM_XBTS);
		/* no display if equal to current or invalid */
#ifdef REQUESTS
		if (team->cTeam == team->lTeam || team->lTeam == XBTS_Invalid) {
			SetSpriteText (team->lSprite, "");
		}
		else {
			SetSpriteText (team->lSprite, teamText[team->lTeam]);
		}
#endif
	}
}								/* MenuTeamSelect */

/*
 * team item mouse event
 */
static void
MenuTeamMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (code == XBE_MOUSE_1) {
		MenuTeamSelect (ptr);
	}
}								/* MenuTeamMouse */

/*
 * polling a team item
 */
static void
MenuTeamPoll (XBMenuItem * ptr)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;
#ifdef REQUESTS
	unsigned i;
#endif
	assert (NULL != team);
	assert (NULL != team->upFunc);
	/* update team data */
	if ((*team->upFunc) (team->id, team->player, &team->cTeam, team->cTeamReq)) {
		assert (team->cTeam < NUM_XBTS);
		/* reset local request if state invalid */
		if (team->cTeam == XBTS_Invalid) {
			team->lTeam = XBTS_Invalid;
		}
		/* update state */
		SetSpriteText (team->sSprite, teamText[team->cTeam]);
		/* check if local request needs to be cleared */
		if (team->cTeam == team->lTeam) {
			SetSpriteText (team->lSprite, "");
		}
		/* update requests */
#ifdef REQUESTS
		for (i = 0; i < MAX_HOSTS; i++) {
			team->tRequests[i] = *teamText[team->cTeamReq[i]];
		}
		team->tRequests[MAX_HOSTS] = (char)0;
		SetSpriteText (team->rSprite, team->tRequests);
#endif
	}
}								/* MenuTeamPoll */

/*
 * create team item
 */
XBMenuItem *
MenuCreateTeam (int x, int y, int w, unsigned id, unsigned player, XBTSFocusFunc focusFunc,
				XBTSChangeFunc chgFunc, XBTSUpdateFunc upFunc)
{
	/* create item */
	XBMenuTeamItem *team = calloc (1, sizeof (*team));
	assert (team != NULL);
	MenuSetItem (&team->item, MIT_Team, x, y, w, CELL_H, MenuTeamFocus, MenuTeamSelect,
				 MenuTeamMouse, MenuTeamPoll);
	/* store id, player */
	team->id = id;
	team->player = player;
	/* set handler */
	team->focusFunc = focusFunc;
	team->chgFunc = chgFunc;
	team->upFunc = upFunc;
	/* team state */
	team->cTeam = XBTS_Invalid;
	team->sSprite =
		CreateTextSprite (teamText[team->cTeam], (x + 1) * BASE_X, y * BASE_Y, w / 4 * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_FOCUS, SPM_MAPPED);
	/* team state request */
	team->lTeam = XBTS_Invalid;
	team->lSprite =
		CreateTextSprite (teamText[team->lTeam], (x + w / 4) * BASE_X, y * BASE_Y, w / 4 * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_FOCUS, SPM_MAPPED);
	/* team requests */
	memset (team->cTeamReq, 0, sizeof (team->cTeamReq));
	team->tRequests[0] = (char)0;
	team->rSprite =
		CreateTextSprite (team->tRequests, (x + w / 2) * BASE_X, y * BASE_Y, (w / 2 - 1) * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_FOCUS, SPM_MAPPED);
	/* graphics */
	MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	/* poll once */
	MenuTeamPoll (&team->item);
	return &team->item;
}								/* MenuCreateTeam */

/************************
 * team item for a peer *
 ************************/

/*
 * team receives focus
 */
static void
MenuPeerTeamFocus (XBMenuItem * ptr, XBBool flag)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;

	assert (NULL != team);
	assert (NULL != team->sSprite);
	SetSpriteAnime (team->sSprite, flag ? FF_HOST_NAME_FOCUS : FF_HOST_NAME_NO_FOCUS);

}								/* MenuPeerTeamFocus */

/*
 * peer team item selcted
 */
static void
MenuPeerTeamSelect (XBMenuItem * ptr)
{
}								/* MenuPeerTeamSelect */

/*
 * peer team item mouse event
 */
static void
MenuPeerTeamMouse (XBMenuItem * ptr, XBEventCode code)
{
}								/* MenuPeerTeamMouse */

/*
 * polling a team item
 */
static void
MenuPeerTeamPoll (XBMenuItem * ptr)
{
	MenuTeamPollShared (ptr);
}								/* MenuPeerTeamPoll */

/*
 * create team item for a peer
 */
XBMenuItem *
MenuCreatePeerTeam (int x, int y, int w, XBTeamState * pTeam)
{
	/* create item */
	XBMenuTeamItem *team = calloc (1, sizeof (*team));
	assert (team != NULL);
	MenuSetItem (&team->item, MIT_Team, x, y, w, CELL_H, MenuPeerTeamFocus, MenuPeerTeamSelect,
				 MenuPeerTeamMouse, MenuPeerTeamPoll);
	/* set server specific data */
	assert (pTeam != NULL);
	team->pTeam = pTeam;
	team->cTeam = *pTeam;
	/* sprite with team state */
	team->sSprite =
		CreateTextSprite (teamText[*pTeam], (x + 1) * BASE_X, y * BASE_Y, (w - 2) * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_FOCUS, SPM_MAPPED);
	/* sprite with team requests */
	team->lSprite = NULL;
	/* sprite with team requests */
	team->rSprite = NULL;
	/* graphics */
	MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	return &team->item;
}								/* MenuCreateTeam */

/************************
 * team item for server *
 ************************/

/*
 * team receives focus
 */
static void
MenuServerTeamFocus (XBMenuItem * ptr, XBBool flag)
{
	MenuTeamFocusShared (ptr, flag);

}								/* MenuServerTeamFocus */

/*
 * team item is selected
 */
static void
MenuServerTeamSelect (XBMenuItem * ptr)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) ptr;

	assert (NULL != team);
	assert (NULL != team->pTeam);
	if (*team->pTeam < NUM_XBTS - 1) {
		*team->pTeam = *team->pTeam + 1;
	}
	else {
		*team->pTeam = 2;
	}
}								/* MenuTeamSelect */

/*
 * team item mouse event
 */
static void
MenuServerTeamMouse (XBMenuItem * ptr, XBEventCode code)
{
	if (code == XBE_MOUSE_1) {
		MenuServerTeamSelect (ptr);
	}
}								/* MenuTeamMouse */

/*
 * polling a team item
 */
static void
MenuServerTeamPoll (XBMenuItem * ptr)
{
	MenuTeamPollShared (ptr);
}								/* MenuServerTeamPoll */

/*
 * create team item for server
 */
XBMenuItem *
MenuCreateServerTeam (int x, int y, int w, XBTeamState * pTeam)
{
	/* create item */
	XBMenuTeamItem *team = calloc (1, sizeof (*team));
	assert (team != NULL);
	MenuSetItem (&team->item, MIT_Team, x, y, w, CELL_H, MenuServerTeamFocus, MenuServerTeamSelect,
				 MenuServerTeamMouse, MenuServerTeamPoll);
	/* set server specific data */
	assert (pTeam != NULL);
	team->pTeam = pTeam;
	team->cTeam = *pTeam;
	/* sprite with team state */
	team->sSprite =
		CreateTextSprite (teamText[*pTeam], (x + 1) * BASE_X, y * BASE_Y, (w - 2) * BASE_X,
						  (CELL_H / 2) * BASE_Y, FF_HOST_NAME_FOCUS, SPM_MAPPED);
	/* sprite with current team request */
	team->lSprite = NULL;
	/* sprite with team requests */
	team->rSprite = NULL;
	/* graphics */
	MenuAddSmallFrame (x / CELL_W, (x + w - 1) / CELL_W, y / CELL_H);
	return &team->item;
}								/* MenuCreateTeam */

/***************
 * destructors *
 ***************/

/*
 * delete a host item
 */
void
MenuDeleteHost (XBMenuItem * item)
{
	XBMenuHostItem *host = (XBMenuHostItem *) item;

	assert (NULL != host);
	assert (NULL != host->tSprite);
	assert (NULL != host->sSprite);
	DeleteSprite (host->tSprite);
	DeleteSprite (host->sSprite);
	if (NULL != host->pSprite) {
		DeleteSprite (host->pSprite);
	}
	if (NULL != host->rSprite) {
		DeleteSprite (host->rSprite);
	}
	if (NULL != host->lSprite) {
		DeleteSprite (host->lSprite);
	}
}								/* MenuDeleteHost */

/*
 * delete a team item
 */
void
MenuDeleteTeam (XBMenuItem * item)
{
	XBMenuTeamItem *team = (XBMenuTeamItem *) item;

	assert (NULL != team);
	assert (NULL != team->sSprite);
	DeleteSprite (team->sSprite);
	if (NULL != team->rSprite) {
		DeleteSprite (team->rSprite);
	}
	if (NULL != team->lSprite) {
		DeleteSprite (team->lSprite);
	}
}								/* MenuDeleteTeam */

/*
 * end of file mi_host.c
 */
