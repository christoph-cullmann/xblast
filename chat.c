/*
 * file chat.c - manage chat data for both client and server
 *
 * $Id: chat.c,v 1.4 2004/08/05 19:51:42 iskywalker Exp $
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

#include "chat.h"
#include "bad_words.h"

static XBChat *listFirst = NULL;
static XBChat *listLast = NULL;

/*
 * clear all stored lines
 */
void
Chat_Clear() {
  XBChat *next;
  Dbg_Out("+++CHAT+++ Clearing");
  while (listFirst != NULL) {
    Dbg_Out("*");
    next = listFirst->next;
    free(listFirst);
    listFirst = next;
  }
  Dbg_Out("\n");
  listLast = NULL;
} /* Chat_Clear*/

/*
 * flush out all messages with status 0
 */
void
Chat_Flush () {
  XBChat *next;
  while (listFirst != NULL) {
    if (listFirst->status != 0) {
      return;
    }
    next = listFirst->next;
    free(listFirst);
    listFirst = next;
  }
  if (listFirst == NULL) {
    listLast = NULL;
  }
} /* Chat_Flush */

/*
 * remove and return first line
 */
XBChat *
Chat_Pop() {
  XBChat *ret = NULL;
  Chat_Flush();
  if (listFirst != NULL) {
    ret = listFirst;
    listFirst = listFirst->next;
    if (listFirst == NULL) {
      listLast = NULL;
    }
  }
  return(ret);
} /* Chat_Pop */

/*
 * create a chat structure
 */
XBChat *
Chat_Create(unsigned fh, unsigned fp, unsigned th, unsigned tp, unsigned how, const char *chat)
{
  XBChat *dat;
  Dbg_Out("+++CHAT+++ creating a chat\n");
  dat = calloc(1,sizeof(XBChat));
  assert (dat != NULL);
  dat->fh = fh;
  dat->fp = fp;
  dat->th = th;
  dat->tp = tp;
  dat->how = how;
  dat->status = 0;
  dat->len = remove_bad_words((char *)chat);
  dat->next = NULL;
  if (dat->len > CHAT_LINE_SIZE-1) {
    dat->len = CHAT_LINE_SIZE-1;
  }
  memcpy(dat->txt, chat, dat->len);
  dat->txt[dat->len]=(char)'\0';// memset(dat->txt + dat->len-1, 0, 1);
  if (listLast == NULL) {
    listFirst = dat;
  } else {
    assert (listLast->next == NULL);
    listLast->next = dat;
  }
  listLast = dat;
  return dat;
} /* Chat_Create */

/*
 * make a chat line visible
 */
void
Chat_Receive(XBChat *chat)
{
  assert(NULL != chat);
  Dbg_Out("+++CHAT+++ received a chat from %i:%i to %i:%i, mode %i: %s (len=%i)\n",
	  chat->fh,chat->fp,chat->th,chat->tp,chat->how,chat->txt,chat->len);
  chat->status = 1;
} /* Chat_Receive */

/*
 * end of file com.c
 */
