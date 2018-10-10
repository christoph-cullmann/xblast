/*
 * net_tele.c - telegrams for cleint/server communication
 *
 * $Id: net_tele.c,v 1.7 2004/11/08 20:43:35 lodott Exp $
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
#include "net_tele.h"

/*
 * local macros
 */
#define MAX_HEADER_SIZE 5
#define MAX_DATA_SIZE   255
#define MAX_TAIL_SIZE   1
#define MAX_TOTAL_SIZE (MAX_HEADER_SIZE + MAX_DATA_SIZE + MAX_TAIL_SIZE)

#define START_BYTE      0x68
#define STOP_BYTE       0x16

/*
 * local types
 */
typedef int (*WriteFunc) (const XBSocket *, const void *, size_t);
typedef int (*ReadFunc)  (const XBSocket *, void *, size_t);

/* details the parse state of a telegram */
typedef enum {
  PS_Start,
  PS_Len,
  PS_COT,
  PS_ID,
  PS_IOB,
  PS_Data,
  PS_Stop
} ParseState;

/* extracted telegram information */
struct _xb_telegram {
  XBTeleCOT       cot;
  XBTeleID        id;
  unsigned char   iob;
  void           *data;
  size_t          len;
  XBTelegram     *next;
};

/* telegram queue */
typedef struct _xb_any_queue {
  XBTelegram *first;
  XBTelegram *last;
} XBAnyQueue;

/* for sending */
struct _xb_snd_queue {
  XBAnyQueue  any;
  size_t      wIndex;                   /* next byte to write */
  size_t      wLen;                     /* bytes to write */
  char        wBuf[MAX_TOTAL_SIZE];     /* data to write */
};

/* for receiving */
struct _xb_rcv_queue {
  XBAnyQueue  any;
  size_t      rIndex;                   /* next byte to read */
  size_t      rLen;                     /* length to be read */
  XBBool      rFlag;                    /* flag to read more data */
  ParseState  rState;                   /* current state of reading */
  size_t      dCount;                   /* length of read data block */
  char        rBuf[MAX_TOTAL_SIZE];     /* buffer read from socket */
  char        tBuf[MAX_TOTAL_SIZE];     /* extracted telegram */
};

/*------------------------------------------------------------------------*
 *
 * Debugoutput
 *
 *------------------------------------------------------------------------*/

#ifdef DEBUG_TELE

/*
 * translate COT codes
 */
const char *
StringCOT (XBTeleCOT cot)
{
  switch (cot) {
  case XBT_COT_Activate:         return "act";
  case XBT_COT_Spontaneous:      return "spont";
  case XBT_COT_SendData:         return "snd_dat";
  case XBT_COT_RequestData:      return "req_dat";
  case XBT_COT_DataNotAvailable: return "dat_not";
  case XBT_COT_DataAvailable:    return "dat_ava";
  default:                       return "???";
  }
} /* StringCOT */

/*
 * translate ID codes
 */
const char *
StringID (XBTeleID id)
{
  switch (id) {
  case XBT_ID_GameConfig:        return "gam_cfg";
  case XBT_ID_PlayerConfig:      return "plr_cfg";
  case XBT_ID_RequestDisconnect: return "req_dis";
  case XBT_ID_HostDisconnected:  return "hst_dis";
  case XBT_ID_StartGame:         return "sta_gam";
  case XBT_ID_RandomSeed:        return "rnd_sed";
  case XBT_ID_LevelConfig:       return "lvl_cfg";
  case XBT_ID_DgramPort:         return "dgm_prt";
  case XBT_ID_Sync:              return "sync";
  case XBT_ID_HostIsIn:          return "hst_in";
  case XBT_ID_HostIsOut:         return "hst_out";
  case XBT_ID_TeamChange:        return "tm_cng";
  case XBT_ID_GameStat:          return "gm_stat";
  case XBT_ID_PID:,              return "pid";
  case XBT_ID_WinnerTeam:,       return "win_tm";
  case XBT_ID_Async:             return "async";
  case XBT_ID_Chat:              return "chat";
  case XBT_ID_HostChange:        return "hst_chg";
  case XBT_ID_HostChangeReq:     return "hst_req";
  case XBT_ID_TeamChangeReq:     return "tm_req";
  default:                       return "???";
  }
} /* StringID */

/*
 * output telegram
 */
static void
DebugTelegram (FILE *fout, const XBTelegram *tele)
{
  fprintf (fout, "%s %s %03u (%03u Bytes)",
	   StringID (tele->id), StringCOT (tele->cot), (unsigned) tele->iob, (unsigned) tele->len);
  if (tele->len > 0) {
    size_t i;
    const char *s = tele->data;
    fputs (" \"", fout);
    for (i = 0; i < tele->len; i ++) {
      if (isprint (s[i])) {
	fputc (s[i], fout);
      } else {
	fprintf (fout, "\\%03o", (unsigned) s[i]);
      }
    }
    fputc ('\"', fout);
  }
} /* DebugTelegram */

#endif

/*------------------------------------------------------------------------*
 *
 * XBTelegram
 *
 *------------------------------------------------------------------------*/

/*
 * create a new telegram from given data
 */
XBTelegram *
Net_CreateTelegram (XBTeleCOT cot, XBTeleID id, XBTeleIOB iob, const void *buf, size_t len)
{
  XBTelegram *tele;
  assert (len < MAX_DATA_SIZE);
  tele = calloc (1, sizeof (XBTelegram));
  assert (tele != NULL);
  tele->cot = cot;
  tele->id  = id;
  tele->iob = iob;
  /* copy buffer if needed */
  if (buf != NULL && len != 0) {
    tele->len  = len;
    tele->data = malloc (len);
    memcpy (tele->data, buf, len);
  }
  return tele;
} /* Net_CreateTelegram */

/*
 * delete a telegram
 */
void
Net_DeleteTelegram (XBTelegram *tele)
{
  assert (tele != NULL);
  if (NULL != tele->data) {
    free (tele->data);
  }
  free (tele);
} /* Net_DeleteTelegram */

/*
 * write telegram to a given buffer, return size
 */
static size_t
WriteTelegram (const XBTelegram *tele, char *buf)
{
  char *ptr = buf;
  /* write header */
  *ptr ++ = START_BYTE;
  *ptr ++ = tele->len;
  *ptr ++ = (char) tele->cot;
  *ptr ++ = (char) tele->id;
  *ptr ++ = (char) tele->iob;
  /* write data */
  assert (tele->len < MAX_DATA_SIZE);
  if (tele->len > 0) {
    assert (tele->data != NULL);
    memcpy (ptr, tele->data, tele->len);
    ptr += tele->len;
  }
  /* write tail */
  *ptr ++ = STOP_BYTE;
  /* return len */
  return ptr - buf;
} /* WriteTelegram */

/*------------------------------------------------------------------------*
 *
 * XBAnyQueue
 *
 *------------------------------------------------------------------------*/

/*
 * delete a queue completely
 */
static void
DeleteAnyQueue (XBAnyQueue *list)
{
  XBTelegram *teleNext;
  for ( ; list->first != NULL; list->first = teleNext) {
    teleNext = list->first->next;
    Net_DeleteTelegram (list->first);
  }
} /* DeleteAnyQueue */

/*
 * add telegram to to a queue
 */
static void
AddTelegram (XBAnyQueue *list, XBTelegram *tele)
{
  assert (list != NULL);
  assert (tele != NULL);
  if (NULL == list->last) {
    list->first      = tele;
  } else {
    list->last->next = tele;
  }
  list->last         = tele;
#ifdef DEBUG_TELE
  fputs ("add tele:", stderr);
  DebugTelegram (stderr, tele);
  fputc ('\n', stderr);
#endif
} /* AddTelegram */

/*
 * get first telegram from queue
 */
static XBTelegram *
GetTelegram (XBAnyQueue *list)
{
  XBTelegram *tele;
  assert (list != NULL);
  /* first element from list */
  tele = list->first;
  /* update list if needed */
  if (list->first != NULL) {
    list->first = list->first->next;
  }
  if (list->first == NULL) {
    list->last = NULL;
  }
  return tele;
} /* GetTelegram */

/*------------------------------------------------------------------------*
 *
 * XBSndQueue
 *
 *------------------------------------------------------------------------*/

/*
 * create a snd queue
 */
XBSndQueue *
Net_CreateSndQueue (XBBool server)
{
  XBSndQueue *list = calloc (1, sizeof (*list));
  assert (list != NULL);
  return list;
} /* Net_CreateSndQueue */

/*
 * delete a sndqueue
 */
void
Net_DeleteSndQueue (XBSndQueue *list)
{
  DeleteAnyQueue (&list->any);
  free (list);
} /* Net_DeleteSndQueue */

/*
 * write a first telegram fro queue to socket
 */
XBTeleResult
Net_Send (XBSndQueue *list, const XBSocket *pSocket)
{
  int         result;
  XBTelegram *tele;
  assert (list    != NULL);
  /* check if new telegram must be written to buffer */
  if (0    == list->wLen &&
      NULL != (tele = GetTelegram (&list->any) ) ) {
#ifdef DEBUG_TELE
    Dbg_Out ("wrt tele %d (%03u bytes)\n", Socket_Fd (pSocket), tele->len);
#endif
    list->wLen   = WriteTelegram (tele, list->wBuf);
    list->wIndex = 0;
    Net_DeleteTelegram (tele);
  }
  /* check if any unwritten bytes are left in the buffer */
  if (list->wIndex < list->wLen) {
    result = Socket_Send (pSocket, list->wBuf + list->wIndex, list->wLen - list->wIndex);
    switch (result) {
    case XB_SOCKET_ERROR:
      Dbg_Out ("ERROR while writing\n");
      return XBT_R_IOError;
    case XB_SOCKET_END_OF_FILE:
      Dbg_Out ("END_OF_FILE while writing\n");
      return XBT_R_IOError;
    case XB_SOCKET_WOULD_BLOCK:
      return XBT_R_Continue;
    default:
      break;
    }
    list->wIndex += result;
    /* mark as complete send */
    if (list->wIndex == list->wLen) {
      list->wLen = list->wIndex = 0;
    }
  }
  /* check if we need to call write again */
  if (list->wLen > 0 || list->any.first != NULL) {
    return XBT_R_Continue;
  } else {
    return XBT_R_Complete;
  }
} /* Net_Send */

/*
 * add a telegram to the send queue
 */
void
Net_SendTelegram (XBSndQueue *list, XBTelegram *tele)
{
#ifdef DEBUG_TELE
  Dbg_Out (" > snd ");
#endif
  AddTelegram (&list->any, tele);
} /* Net_SendTelegram */

/*------------------------------------------------------------------------*
 *
 * XBRcvQueue
 *
 *------------------------------------------------------------------------*/

/*
 * create rcv queue
 */
XBRcvQueue *
Net_CreateRcvQueue (XBBool server)
{
  XBRcvQueue *list = calloc (1, sizeof (*list));
  assert (list != NULL);
  list->rState   = PS_Start;
  list->rFlag    = XBTrue;
  return list;
} /* Net_CreateRcvQueue */

/*
 * delete rcv queue
 */
void
Net_DeleteRcvQueue (XBRcvQueue *list)
{
  DeleteAnyQueue (&list->any);
  free (list);
} /* Net_DeleteRcvQueue */

/*
 * fill read buffer from socket
 */
static XBTeleResult
ReadBuffer (XBRcvQueue *list, const XBSocket *pSocket)
{
#ifdef W32
  long result;
#else
  ssize_t result;
#endif
  result = Socket_Receive (pSocket, list->rBuf, MAX_TOTAL_SIZE);
  switch (result) {
  case XB_SOCKET_ERROR:
    return XBT_R_IOError;
  case XB_SOCKET_END_OF_FILE:
    return XBT_R_EndOfFile;
  case XB_SOCKET_WOULD_BLOCK:
    result = 0;
    break;
  }
  list->rLen   = result;
  list->rIndex = 0;
  return XBT_R_Continue;
} /* ReadBuffer */

/*
 * fetch byte from internal read buffer
 */
static XBBool
GetByte (XBRcvQueue *list, size_t tIndex)
{
  if (list->rIndex < list->rLen) {
    list->tBuf[tIndex] = list->rBuf[list->rIndex ++];
    return XBTrue;
  } else {
    return XBFalse;
  }
} /* GetByte */

/*
 * read telegram data from socket
 */
XBTeleResult
Net_Receive (XBRcvQueue *list, const XBSocket *pSocket)
{
  size_t i, len;
  XBTelegram *tele;
  assert (list != NULL);
  /* check if data has to be read to complete */
  if (list->rFlag) {
    XBTeleResult result;
    if (XBT_R_Continue != (result = ReadBuffer (list, pSocket) ) ) {
      return result;
    }
    list->rFlag = XBFalse;
  }
  /* parse read buffer until empty */
  while (1) {
    switch (list->rState) {
    case PS_Start:
      do {
	/* try to get start byte */
	if (! GetByte (list, 0)) {
	  list->rFlag = XBTrue;
	  return XBT_R_Continue;
	}
#ifdef DEBUG_TELE
	fputc ('{', stderr);
#endif
      } while (list->tBuf[0] != START_BYTE);
      list->rState = PS_Len;
      break;
    case PS_Len:
      /* try to get len byte */
      if (! GetByte (list, 1)) {
	list->rFlag = XBTrue;
	return XBT_R_Continue;
      }
#ifdef DEBUG_TELE
      fputc ('L', stderr);
#endif
      list->rState = PS_COT;
      break;
    case PS_COT:
      /* try to rea COT */
      if (! GetByte (list, 2)) {
	list->rFlag = XBTrue;
	return XBT_R_Continue;
      }
#ifdef DEBUG_TELE
      fputc ('C', stderr);
#endif
      list->rState = PS_ID;
      break;
    case PS_ID:
      /* try to read id byte */
      if (! GetByte (list, 3)) {
	list->rFlag = XBTrue;
	return XBT_R_Continue;
      }
#ifdef DEBUG_TELE
      fputc ('I', stderr);
#endif
      list->rState = PS_IOB;
      break;
    case PS_IOB:
      /* try to get IOB byte */
      if (! GetByte (list, 4) ) {
	list->rFlag = XBTrue;
	return XBT_R_Continue;
      }
#ifdef DEBUG_TELE
      fputc ('i', stderr);
#endif
      list->rState = PS_Data;
      list->dCount = 0;
      break;
    case PS_Data:
      /* try to read data byte by byte */
      len = list->tBuf[1];
      for (i = list->dCount; i < len; i ++) {
	if (! GetByte (list, MAX_HEADER_SIZE + i)) {
	  list->rFlag  = XBTrue;
	  list->dCount = i;
	  return XBT_R_Continue;
	}
#ifdef DEBUG_TELE
	fputc ('.', stderr);
#endif
      }
#ifdef DEBUG_TELE
      fputc ('D', stderr);
#endif
      list->rState = PS_Stop;
      break;
    case PS_Stop:
      /* try to read stop byte */
      len = list->tBuf[1];
      if (! GetByte (list, MAX_HEADER_SIZE + len)) {
	list->rFlag = XBTrue;
	return XBT_R_Continue;
      }
      /* check stop byte */
      if (STOP_BYTE != list->tBuf[MAX_HEADER_SIZE + len]) {
	list->rState = PS_Start;
	return XBT_R_TeleError;
      }
#ifdef DEBUG_TELE
      Dbg_Out ("}\n");
#endif
      /* telegram is now complete */
      tele = Net_CreateTelegram (list->tBuf[2], list->tBuf[3], list->tBuf[4], list->tBuf + MAX_HEADER_SIZE, len);
      assert (NULL != tele);
#ifdef DEBUG_TELE
      Dbg_Out (" < rcv ");
#endif
      AddTelegram (&list->any, tele);
      /* start from the beginning */
      list->rState = PS_Start;
    }
  }
  return XBT_R_TeleError;
} /* Net_ReadTelegram */

/*
 * get first telegram in receive queue
 */
XBTelegram *
Net_ReceiveTelegram (XBRcvQueue *list)
{
  return GetTelegram (&list->any);
} /* Net_ReceiveTelegram */

/*
 * get cause of telegram transmission
 */
XBTeleCOT
Net_TeleCOT  (const XBTelegram *tele)
{
  assert (tele != NULL);
  return tele->cot;
} /* Net_TeleCOT */

/*
 * get id of telegram
 */
XBTeleID
Net_TeleID   (const XBTelegram *tele)
{
  assert (tele != NULL);
  return tele->id;
} /* Net_TeleID */

/*
 * get iob of telegram
 */
XBTeleIOB
Net_TeleIOB (const XBTelegram *tele)
{
  assert (tele != NULL);
  return tele->iob;
} /* Net_TeleIOB */

/*
 * get telegram data
 */
const void *
Net_TeleData (const XBTelegram *tele, size_t *len)
{
  assert (tele != NULL);
  assert (len  != NULL);
  *len = tele->len;
  return tele->data;
} /* Net_TeleData */

/*
 * end of file net_tele.c
 */
