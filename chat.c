/*
 * file chat.c - manage chat data for both client and server
 *
 * $Id: chat.c,v 1.20 2006/03/28 11:41:19 fzago Exp $
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
/* AbsInt: bad word handling */
#include "bad_words.h"
/* AbsInt end */

/* max size for names */
#define MAX_CHAT_NAME_DISP 4
/* chat description size, at least 5 */
#define CHAT_DESCR_SIZE 5

#if CHAT_DESCR_SIZE<=MAX_CHAT_NAME_SIZE
#error "CHAT_DESCR_SIZE must be at least as large as MAX_CHAT_NAME_SIZE!"
#endif

#if CHAT_DESCR_SIZE<5
#error "CHAT_DESCR_SIZE must be at least 5!"
#endif

/* chat structure */
struct _xb_chat
{
	XBChat *next;
	unsigned char fh;			/* sending host */
	unsigned char fp;			/* sending local player */
	unsigned char th;			/* receiving host */
	unsigned char tp;			/* receiving local player */
	XBChatMode how;				/* 0..numofplayers-1 (private), public, team */
	char txt[CHAT_LINE_SIZE];
	size_t len;
	XBChatStatus status;
};

/* input lines for local players */
static XBChat *input[NUM_LOCAL_PLAYER + 1];
static XBEventCode codes[NUM_LOCAL_PLAYER + 1];
static XBBool initialized = XBFalse;
static XBBool listening = XBFalse;
static int active = -1;
/* AbsInt: check whether chat messages are displayed */
static int display_active = 1;
/* AbsInt end */
/* list of all chats */
static XBChat *listFirst = NULL;
static XBChat *listLast = NULL;

/****************************
 * initialization, creation *
 ****************************/

/*
 * clear all chat data
 */
void
Chat_Clear (void)
{
	XBChat *next;
	memset (input, 0, sizeof (input));
	memset (codes, 0, sizeof (codes));
	Dbg_Chat ("clearing\n");
	while (listFirst != NULL) {
		next = listFirst->next;
		free (listFirst);
		listFirst = next;
	}
	listLast = NULL;
	initialized = XBTrue;
	listening = XBFalse;
	active = -1;
}								/* Chat_Clear */

/*
 * listen to chat events
 */
void
Chat_Listen (XBBool flag)
{
	if (!initialized) {
		Chat_Clear ();
	}
	listening = flag;
	Dbg_Chat ("%s\n", listening ? "active" : "inactive");
}								/* Chat_Listen */

/*
 * return if listening
 */
XBBool
Chat_isListening (void)
{
	if (!initialized) {
		return XBFalse;
	}
	return listening;
}								/* Chat_isListening */

/*
 * create a chat structure
 */
XBChat *
Chat_Create (void)
{
	XBChat *dat;
	Dbg_Chat ("creating\n");
	/* get memory */
	dat = calloc (1, sizeof (XBChat));
	assert (dat != NULL);
	/* append to list */
	if (listLast == NULL) {
		listFirst = dat;
	}
	else {
		assert (listLast->next == NULL);
		listLast->next = dat;
	}
	listLast = dat;
	/* set successor */
	dat->next = NULL;
	/* mark as created */
	dat->status = XBCS_Created;
	return dat;
}								/* Chat_Create */

/*
 * create a chat structure
 */
XBChat *
Chat_CreateSys (void)
{
	XBChat *chat = Chat_Create ();
	Chat_Set (chat, Network_LocalHostId (), NUM_LOCAL_PLAYER, 0x00, 0x00, XBCM_System, "");
	return chat;
}								/* Chat_CreateSys */

/*
 * set chat parameters
 */
void
Chat_Set (XBChat * chat, unsigned char fh, unsigned char fp, unsigned char th, unsigned char tp,
		  unsigned char how, const char *txt)
{
	assert (chat != NULL);
	/* store sender */
	chat->fh = fh;
	chat->fp = fp;
	/* store target */
	chat->th = th;
	chat->tp = tp;
	/* store mode */
	chat->how = how;
	/* mark as set */
	chat->status = XBCS_Inactive;
	/* set text */
	Chat_SetText (chat, txt);
}								/* Chat_Set */

/*
 * set chat text
 */
void
Chat_SetText (XBChat * chat, const char *txt)
{
	assert (chat != NULL);
	/* store length of message, truncate if necessary */
	chat->len = strlen (txt);
	if (chat->len > CHAT_LINE_SIZE - 1) {
		chat->len = CHAT_LINE_SIZE - 1;
	}
	/* copy message */
	memcpy (chat->txt, txt, chat->len);
	chat->txt[chat->len] = (char)'\0';
}								/* Chat_SetText */

/* AbsInt: start */
static char const *get_time(void)
{
	time_t now;
	struct tm *tm_s;
	static char time_string[20];

	/* Determine the current time */
	time(&now);
	tm_s = localtime(&now);

	/* Build the timestamp string */
	sprintf(time_string, "%02d:%02d:%02d", tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);

	return time_string;
}
/* AbsInt: end */

/*
 * make a chat line visible
 */
void
Chat_Receive (XBChat * chat)
{
	static char buf[CHAT_LINE_SIZE + 2 * CHAT_DESCR_SIZE + 6];
	static char snd[20];
	static char snd0[CHAT_DESCR_SIZE + 1];
	static char sep[3];
	static char trg[20];
	XBBool own;
	XBBool any;
	XBAtom from;
	XBAtom to;
	assert (NULL != chat);
	/* clear strings */
	memset (snd, 0, sizeof (snd));
	memset (snd0, 0, sizeof (snd0));
	memset (trg, 0, sizeof (trg));
	/* chat originated on local machine? */
	own = (Network_LocalHostId () == chat->fh);
	/* chat didn't come from a specific player? */
	any = (chat->fp == NUM_LOCAL_PLAYER);
	/* get name atom from sending player */
	from = any ? ATOM_INVALID : Network_GetPlayer2 (chat->fh, chat->fp);
	/* determine sender string */
	if (from == ATOM_INVALID) {
		/* sender is host */
		sprintf (snd, "#%u", chat->fh);	/* length<=5 */
	}
	else {
		/* sender is player, truncate full name if necessary */
		strncpy (snd, GUI_AtomToString (from), sizeof (snd) - 1);
	}
	/* truncate sender for display */
	strncpy (snd0, snd, MAX_CHAT_NAME_DISP);
	/* set separator */
	sprintf (sep, "->");
	/* now check mode for target string */
	switch (chat->how) {
	case XBCM_Public:
		sprintf (trg, "%s", "all");	/* length<=5 */
		break;
	case XBCM_Team:
		sprintf (trg, "%s", "team");	/* length<=5 */
		break;
	case XBCM_Private:
		to = Network_GetPlayer2 (chat->th, chat->tp);
		if (to == ATOM_INVALID) {
			/* target is host */
			sprintf (trg, "#%u", chat->th);	/* length<=5 */
		}
		else {
			/* target is player */
			strncpy (trg, GUI_AtomToString (to), sizeof (trg) - 1);
		}
		break;
	case XBCM_System:
		sprintf (snd, "SYS");
		sprintf (snd0, "SYS");
		sprintf (sep, "-");
		sprintf (trg, "#%u", chat->fh);
		break;
	}
	/* full print to stdout */
    /* AbsInt: start */
    fprintf (stdout, "CHAT [%s]: %s%s%s: %s\n", get_time(), snd, sep, trg, chat->txt);
    /* AbsInt: end */
	/* truncated display in GUI */
	if (chat->how != XBCM_System) {
		sprintf (buf, "%s:%s", snd0, chat->txt);
		SetChat (buf, XBTrue);
	}
	/* mark as received */
	chat->status = XBCS_Received;
}								/* Chat_Receive */

/*
 * flush out all messages with status 0
 */
static void
Chat_Flush (void)
{
	XBChat *next;
	while (listFirst != NULL) {
		if (listFirst->status != 0) {
			return;
		}
		next = listFirst->next;
		free (listFirst);
		listFirst = next;
	}
	if (listFirst == NULL) {
		listLast = NULL;
	}
}								/* Chat_Flush */

/*
 * remove and return first line
 */
XBChat *
Chat_Pop (void)
{
	XBChat *ret = NULL;
	Chat_Flush ();
	if (listFirst != NULL) {
		ret = listFirst;
		listFirst = listFirst->next;
		if (listFirst == NULL) {
			listLast = NULL;
		}
	}
	return (ret);
}								/* Chat_Pop */

/*********************
 * packing/unpacking *
 *********************/

/*
 * pack chat data for transmission
 */
size_t
Chat_PackData (XBChat * chat, char **data, unsigned *iob)
{
	static char buf[CHAT_LINE_SIZE + 2];
	unsigned char from;
	assert (chat != NULL);
	Dbg_Chat ("packing (%u,%u)->(%u,%u)-%u-%s(%lu)\n", chat->fh, chat->fp, chat->th, chat->tp,
			  chat->how, chat->txt, (unsigned long)chat->len);
	/* redefine local sender */
	from = (chat->fp == NUM_LOCAL_PLAYER) ? 0xFF : chat->fp;
	/* build buffer */
	buf[0] = 0xFF & ((chat->fh << 4) + (from & 0x0F));
	buf[1] = 0xFF & ((chat->th << 4) + (chat->tp & 0x0F));
	memcpy (buf + 2, chat->txt, chat->len);
	buf[chat->len + 2] = (char)'\0';
	/* return data */
	*data = buf;
	*iob = chat->how & 0xFF;
	return (chat->len + 3);
}								/* Chat_PackData */

/*
 * unpack chat data
 */
XBChat *
Chat_UnpackData (const char *data, size_t len, unsigned iob)
{
	XBChat *chat = NULL;
	unsigned char from;
	if (len > 2) {
		chat = Chat_Create ();
		from = data[0] & 0x0F;
		if (from == 0x0F) {
			from = NUM_LOCAL_PLAYER;
		}
		Chat_Set (chat, data[0] >> 4, from, data[1] >> 4, data[1] & 0x0F, iob, data + 2);
		Dbg_Chat ("unpacking (%u,%u)->(%u,%u)-%u-%s(%lu)\n", chat->fh, chat->fp, chat->th, chat->tp,
				  chat->how, chat->txt, (unsigned long)chat->len);
	}
	return chat;
}								/* Chat_UnpackData */

/*********
 * input *
 *********/

/*
 * deactivate all input
 */
static void
Chat_Deactivate (void)
{
	int p;
	assert (initialized);
	/* deactivating all input */
	for (p = 0; p <= NUM_LOCAL_PLAYER; p++) {
		if (input[p] != NULL) {
			input[p]->status = XBCS_Inactive;
		}
	}
	Dbg_Chat ("deactivating all current input\n");
	active = -1;
}								/* Chat_Deactivate */

/*
 * activate chat input for a player
 */
static void
Chat_ActivateInput (unsigned int local)
{
	assert (local >= 0);
	assert (local <= NUM_LOCAL_PLAYER);
	assert (initialized);
	assert (input[local] != NULL);
	Chat_Deactivate ();
	/* activate */
	input[local]->status = XBCS_Input;
	active = local;
}								/* Chat_Activate */

/*
 * start chat input for a local player
 */
static void
Chat_StartInput (unsigned int local)
{
	unsigned char id = Network_LocalHostId ();
	if (id >= MAX_HOSTS) {
		Dbg_Chat ("cannot start input, no host id\n");
		return;
	}
	assert (local <= NUM_LOCAL_PLAYER);
	assert (initialized);
	/* activating input for local */
	if (input[local] == NULL) {
		input[local] = Chat_Create ();
		Chat_Set (input[local], id, local, 0, 0, XBCM_Public, "");
		Dbg_Chat ("initializing chat input for player %u\n", local);
	}
	SetGet ("Start Chatting", XBTrue);
	Chat_ActivateInput (local);
}								/* Chat_StartInput */

/*
 * send input for a local player
 */
static void
Chat_SendInput (unsigned int local)
{
	assert (local <= NUM_LOCAL_PLAYER);
	assert (initialized);
	if (input[local] == NULL) {
		Dbg_Chat ("no chat input for player %u, cannot send\n", local);
		return;
	}
	switch (input[local]->status) {
	case XBCS_Inactive:
		Dbg_Chat ("activating chat input for player %u before send\n", local);
		SetGet (input[local]->txt, XBTrue);
		Chat_ActivateInput (local);
		break;
	case XBCS_Input:
		Dbg_Chat ("sending chat input #%u\n", local);
        /* AbsInt: Check for chat (de)actication control message */
        if (!strncmp(DEACTIVATE_DISPLAY, input[local]->txt, CHAT_LINE_SIZE) ) {
            /* Deactivate chat display */
            display_active = 0;
        } else if (!display_active && !strncmp(ACTIVATE_DISPLAY,input[local]->txt, CHAT_LINE_SIZE) ) {
            /* Activate chat display */
            display_active = 1;
        }
        /* AbsInt end */
        /* AbsInt: Remove all words/snippets that are declared bad */
#if defined(XBLAST_CHAT_FILTER)
        size_t text_size = remove_bad_words(input[local]->txt);
        input[local]->len = text_size;
#endif
        /* AbsInt end */
		switch (Network_GetType ()) {
		case XBNT_Server:
			Server_ReceiveChat (input[local]);
			break;
		case XBNT_Client:
			Client_SendChat (input[local]);
			break;
		default:
			Dbg_Chat ("failed to send, invalid net type\n");
			return;
		}
		input[local]->status = XBCS_Sent;
		SetGet (NULL, XBTrue);
		input[local] = NULL;
		active = -1;
		break;
	default:
		Dbg_Chat ("failed to send chat input #%u, invalid status %u\n", local,
				  input[local]->status);
		break;
	}
}								/* Chat_SendInput */

/*
 * cancel input for local player
 */
static void
Chat_CancelInput (unsigned int local)
{
	assert (local <= NUM_LOCAL_PLAYER);
	assert (initialized);
	if (input[local] == NULL) {
		Dbg_Chat ("no chat input for player %u, cannot cancel\n", local);
		return;
	}
	switch (input[local]->status) {
	case XBCS_Inactive:
		Dbg_Chat ("activating chat input for player %u before cancel\n", local);
		SetGet (input[local]->txt, XBTrue);
		Chat_ActivateInput (local);
		break;
	case XBCS_Input:
		Dbg_Chat ("canceling chat input #%u\n", local);
		memset (input[local]->txt, 0, sizeof (input[local]->txt));
		input[local]->len = 0;
		SetGet ("Chat canceled", XBTrue);
		input[local]->status = XBCS_Inactive;
		active = -1;
		break;
	default:
		Dbg_Chat ("failed to send chat input #%u, invalid status %u\n", local,
				  input[local]->status);
		break;
	}
}								/* Chat_CancelInput */

/*
 * choose next target
 */
static void
Chat_NextTarget (unsigned int local)
{
	XBAtom atom;
	XBChat *chat;
	assert (local <= NUM_LOCAL_PLAYER);
	assert (initialized);
	if (input[local] == NULL) {
		Chat_StartInput (local);
	}
	chat = input[local];
	switch (chat->status) {
	case XBCS_Inactive:
		Dbg_Chat ("activating chat input #%u\n", local);
		SetGet (input[local]->txt, XBTrue);
		Chat_ActivateInput (local);
	case XBCS_Input:
		switch (chat->how) {
		case XBCM_Public:
			chat->how = XBCM_Team;
			SetGet ("Team message", XBTrue);
			return;
		case XBCM_Team:
			chat->how = XBCM_Private;
			if (Network_GetFirstOtherPlayer (chat->fh, local, &chat->th, &chat->tp)) {
				atom = Network_GetPlayer2 (chat->th, chat->tp);
				SetGet (GUI_AtomToString (atom), XBTrue);
				Dbg_Chat ("first other player = %s (%u,%u)\n", GUI_AtomToString (atom), chat->th,
						  chat->tp);
			}
			else {
				Dbg_Chat ("no other players found, skipping private target\n");
				Chat_NextTarget (local);
			}
			break;
		case XBCM_Private:
			if (Network_GetNextOtherPlayer (chat->fh, local, &chat->th, &chat->tp)) {
				atom = Network_GetPlayer2 (chat->th, chat->tp);
				SetGet (GUI_AtomToString (atom), XBTrue);
				Dbg_Chat ("next other player = %s (%u,%u)\n", GUI_AtomToString (atom), chat->th,
						  chat->tp);
			}
			else {
				chat->how = XBCM_Public;
				SetGet ("Public message", XBTrue);
			}
			break;
		default:
			break;
		}
		Dbg_Chat ("how = %u\n", chat->how);
		break;
	default:
		Dbg_Chat ("failed to change target for input #%u, invalid status %u\n", local,
				  input[local]->status);
		break;
	}
}								/* Chat_NextTarget */

/*
 * handle backspace for active input
 */
static void
Chat_Backspace (void)
{
	assert (initialized);
	if (active < 0) {
		Dbg_Chat ("no active input, backspace failed\n");
		return;
	}
	assert (active <= NUM_LOCAL_PLAYER);
	if (input[active] != NULL && input[active]->status == XBCS_Input) {
		size_t len = input[active]->len;
		if (len > 0) {
			input[active]->txt[--len] = (char)0;
			input[active]->len = len;
			Dbg_Chat ("backspacing chat input #%u\n", active);
			SetGet (input[active]->txt, XBTrue);
		}
		else {
			Dbg_Chat ("chat input #%u empty, backspace failed\n", active);
		}
		return;
	}
}								/* Chat_Backspace */

/*
 * add a character for active input, return if overflow
 */
static XBBool
Chat_AddAscii (char ascii)
{
	size_t len;
	assert (initialized);
	if (active < 0) {
		Dbg_Chat ("no active input, failed to add char\n");
		return XBFalse;
	}
	assert (active <= NUM_LOCAL_PLAYER);
	/* find active input */
	len = input[active]->len;
	if (len < CHAT_LINE_SIZE - 1) {
		input[active]->txt[len++] = ascii;
		input[active]->txt[len] = (char)0;
		input[active]->len = len;
		SetGet (input[active]->txt, XBTrue);
		Dbg_Chat ("adding character to chat input #%u, length %lu = %s\n", active,
				  (unsigned long)strlen (input[active]->txt), input[active]->txt);
		return XBTrue;
	}
	else {
		Dbg_Chat ("ignoring character, line for display #%u too long\n", active);
		return XBFalse;
	}
}								/* Chat_AddAscii */

/****************************
 * event checking for menus *
 ****************************/

/*
 * add event code
 */
void
Chat_AddEventCode (unsigned local, XBEventCode ev)
{
	assert (initialized);
	assert (local <= NUM_LOCAL_PLAYER);
	codes[local] = ev;
	Dbg_Chat ("assigning event type %u to local player %u\n", ev, local);
}								/* Chat_AddEventCode */

/*
 * find display to which event code corresponds to
 */
unsigned char
Chat_FindCode (XBEventCode ev)
{
	unsigned id;
	if (!initialized || !listening) {
		return 0xFF;
	}
	for (id = 0; id <= NUM_LOCAL_PLAYER; id++) {
		if (codes[id] == ev) {
			return id;
		}
	}
	return 0xFF;
}								/* Chat_FindCode */

/*
 * get current event code of active chatter
 */
XBEventCode
Chat_GetCurrentCode (void)
{
	if (!initialized || !listening || active < 0) {
		return XBE_NONE;
	}
	return codes[active];
}								/* Chat_GetCurrentCode */

/*
 * chat keys
 */
XBBool
Chat_Event (XBEventCode event, XBEventData data)
{
	unsigned int local;
	/* check if chat mode active */
	if (!listening) {
		return XBFalse;
	}
	/* if so, grab the chat events */
	switch (event) {
		/* a fixed chat key has been entered */
	case XBE_CHAT:
		/* redefine event to currently active */
		event = Chat_GetCurrentCode ();
		/* restart event routine */
		assert (event != XBE_CHAT);
		return Chat_Event (event, data);
		/* ascii character for current input, if any */
	case XBE_ASCII:
		Chat_AddAscii ((char)data.value);
		break;
		/* control characters */
	case XBE_CTRL:
		return XBFalse;
		/* chat keys */
	default:
		/* TODO: find better solution than +-1000 */
		if (data.value < 1000) {
			return XBFalse;
		}
		local = Chat_FindCode (event);
		if (local > NUM_LOCAL_PLAYER) {
			return XBFalse;
		}
		Dbg_Chat ("chat event %u for display %u\n", data.value, local);
		switch (data.value - 1000) {
		case XBCE_START:
			Chat_StartInput (local);
			break;
		case XBCE_ESCAPE:
		case XBCE_CANCEL:
			Chat_CancelInput (local);
			break;
		case XBCE_CHANGE:
			Chat_NextTarget (local);
			break;
		case XBCE_SEND:
		case XBCE_ENTER:
			Chat_SendInput (local);
			break;
		case XBCE_BACK:
			Chat_Backspace ();
			break;
		default:
			return XBFalse;
		}
		break;
	}
	return XBTrue;
}

/*
 * end of file chat.c
 */
