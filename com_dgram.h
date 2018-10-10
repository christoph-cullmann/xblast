/*
 * file com_dgram.h - base struct und functions for datagram connections
 *
 * $Id: com_dgram.h,v 1.7 2004/11/06 19:52:36 lodott Exp $
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
 */
#ifndef XBLAST_COM_DGRAM_H
#define XBLAST_COM_DGRAM_H

#include "com_base.h"
#include "action.h"
#include "net_dgram.h"

#ifdef WMS
#include "timeval.h"
#endif

/*
 * macros
 */
#define MAX_DGRAM_BUFFER   32
#define LINK_LOST          10L
#define NUM_PLAYER_ACTION (GAME_TIME+2)

/*
 * type definitions
 */
typedef struct _xb_comm_dgram  XBCommDgram;
typedef enum {
  XBDI_LOSS,        /* dataloss occurred */
  XBDI_CONSUCC,     /* connection success */
  XBDI_CONFAIL,     /* connection failure */
  XBDI_PARSED,      /* data is completely parsed */
  XBDI_FINISH,      /* FINISH received */
  XBDI_WRITEERR,    /* write error */
  XBDI_IGNORE,      /* data for a game time has been ignored */
  XBDI_CLOSE,       /* structure removed */
} XBDgramInfo;

typedef void (*DgramPingFunc)   (XBCommDgram *, unsigned, unsigned short);
typedef void (*DgramFinishFunc) (XBCommDgram *);
typedef void (*DgramActionFunc) (XBCommDgram *, int, const PlayerAction *);
typedef XBBool (*DgramInfoFunc) (XBCommDgram *, XBDgramInfo);

/*
 * local types
 */
typedef struct packed {
  size_t        numBytes;
  unsigned char mask;
  unsigned char action[MAX_PLAYER];
  unsigned char pad;
} PackedPlayerAction;

struct _xb_comm_dgram {
  XBComm             comm;
  unsigned           port;                      /* target port for connect */
  const char *       host;                      /* expected host name for unconnected */
  XBBool             connected;                 /* connect called or not */
  /* extra data for frames */
  size_t             rcvfirst;                  /* first game time for last rcv */
  size_t             rcvnext;                   /* next game time for last rcv */
  size_t             buffirst;                  /* first game time in buffer */
  size_t             bufnext;                   /* next game time to add in buffer */
  size_t             sndfirst;                  /* first game time for last send */
  size_t             sndnext;                   /* next game time for last send */
  size_t             ignore;                    /* ignored gametime, for XBDI_IGNORE */
  size_t             queue;                     /* first  game time for next queuing */
  size_t             expect;                    /* expected game time for next receive */
  /* datagram to send on next writeability */
  XBDatagram        *snd;                       /* queued datagram for next send */
  /* ping data */
  struct timeval     lastSnd;                   /* time of last send */
  struct timeval     lastRcv;                   /* time of last receive */
  /* handlers */
  DgramPingFunc      pingFunc;                  /* ping handling */
  DgramActionFunc    actionFunc;                /* action handling */
  DgramInfoFunc      infoFunc;                  /* info handling */
  /* received player actions per game time */
  PackedPlayerAction ppa[NUM_PLAYER_ACTION];
};

/*
 * global prototypes
 */
extern XBComm *Dgram_CommInit (XBCommDgram *, XBCommType, XBSocket *, DgramPingFunc, DgramInfoFunc, DgramActionFunc);
extern unsigned short Dgram_Port (const XBCommDgram *);
extern void Dgram_Reset (XBCommDgram *);
extern void Dgram_SendPing (XBCommDgram *);
extern void Dgram_SendPingData (XBCommDgram *, const int pingData[]);
extern void Dgram_SendPlayerAction (XBCommDgram *, int, const PlayerAction *);
extern void Dgram_SendFinish (XBCommDgram *, int gameTime);
extern void Dgram_SendChat(XBCommDgram *, unsigned, unsigned, const char *);
extern XBBool Dgram_Flush (XBCommDgram *dgram);

#endif
/*
 * end of file com_dgram.h
 */
