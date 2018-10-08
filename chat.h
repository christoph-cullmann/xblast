/*
 * file chat.h - managing chat data for both client and server
 *
 * $Id: chat.h,v 1.11 2006/02/24 21:29:16 fzago Exp $
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
#ifndef XBLAST_CHAT_H
#define XBLAST_CHAT_H

#define CHAT_LINE_SIZE 50

/* AbsInt: control chat messages for display activity */
#define DEACTIVATE_DISPLAY "<--off"
#define ACTIVATE_DISPLAY "<--on"
/* AbsInt end */

/*
 * type declarations
 */
typedef struct _xb_chat XBChat;

typedef enum
{
	XBCM_Public,
	XBCM_Team,
	XBCM_Private,
	XBCM_System,
} XBChatMode;

typedef enum
{
	XBCS_Created,
	XBCS_Input,
	XBCS_Inactive,
	XBCS_Sent,
	XBCS_Received,
} XBChatStatus;

/*
 * global prototypes
 */

/* init */
extern void Chat_Clear (void);

/* start/stop chat handling */
extern void Chat_Listen (XBBool);
extern XBBool Chat_isListening (void);

/* create */
extern XBChat *Chat_Create (void);
extern XBChat *Chat_CreateSys (void);

/* modify */
extern void Chat_Set (XBChat * chat, unsigned char fh, unsigned char fp, unsigned char th,
					  unsigned char tp, unsigned char how, const char *txt);
extern void Chat_SetText (XBChat * chat, const char *txt);

/* get */
extern void Chat_Receive (XBChat *);
extern XBChat *Chat_Pop (void);

/* packing/unpacking */
extern size_t Chat_PackData (XBChat * chat, char **data, unsigned *iob);
extern XBChat *Chat_UnpackData (const char *data, size_t len, unsigned iob);

/* chat events */
extern void Chat_AddEventCode (unsigned local, XBEventCode ev);
extern XBEventCode Chat_GetCurrentCode (void);
extern unsigned char Chat_FindCode (XBEventCode);
extern XBBool Chat_Event (XBEventCode, XBEventData);

#endif
/*
 * end of file chat.h
 */
