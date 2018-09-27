/*
 * file debug.h - memory debugging
 *
 * $Id: debug.h,v 1.24 2005/01/23 14:18:41 lodott Exp $
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
#ifndef XBLAST_DEBUG_H
#define XBLAST_DEBUG_H

#include "common.h"

/*
 * global macros
 */
#ifndef DEBUG
#define DEBUG
#endif

#if defined(__GNUC__) && !defined(DEBUG)
#define Dbg_Out(fmt, ...) (void)0
#else
extern void   Dbg_StartClock (void);
extern time_t Dbg_FinishClock (void);
extern void   Dbg_Out (const char *fmt, ...);
#endif

#ifdef DEBUG_ALLOC
#define malloc(a)   Dbg_Malloc (__FILE__,__LINE__,a)
#define calloc(a,b) Dbg_Calloc (__FILE__,__LINE__,a,b)
#define free(a)     Dbg_Free   (__FILE__,__LINE__,a)
#endif

/*
 * global prototypes
 */

#if defined(__GNUC__) && !defined(DEBUG_LEVEL)
#define Dbg_Level(fmt,...)  (void)0
#else
extern void   Dbg_Level (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_GAME)
#define Dbg_Game(fmt,...)  (void)0
#else
extern void   Dbg_Game (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_NETWORK)
#define Dbg_Network(fmt,...)  (void)0
#else
extern void   Dbg_Network (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_SERVER)
#define Dbg_Server(fmt,...)  (void)0
#else
extern void   Dbg_Server (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_CLIENT)
#define Dbg_Client(fmt,...)  (void)0
#else
extern void   Dbg_Client (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_USER)
#define Dbg_User(fmt,...)  (void)0
#else
extern void   Dbg_User (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_LISTEN)
#define Dbg_Listen(fmt,...)  (void)0
#else
extern void   Dbg_Listen (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_STREAM)
#define Dbg_Stream(fmt,...)  (void)0
#else
extern void   Dbg_Stream (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_S2C)
#define Dbg_S2C(fmt,...)  (void)0
#else
extern void   Dbg_S2C (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_C2S)
#define Dbg_C2S(fmt,...)  (void)0
#else
extern void   Dbg_C2S (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_X2C)
#define Dbg_X2C(fmt,...)  (void)0
#else
extern void   Dbg_X2C (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_C2X)
#define Dbg_C2X(fmt,...)  (void)0
#else
extern void   Dbg_C2X (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_C2B)
#define Dbg_C2B(fmt,...)  (void)0
#else
extern void   Dbg_C2B (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_DGRAM)
#define Dbg_Dgram(fmt,...)  (void)0
#else
extern void   Dbg_Dgram (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_D2C)
#define Dbg_D2C(fmt,...)  (void)0
#else
extern void   Dbg_D2C (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_D2S)
#define Dbg_D2S(fmt,...)  (void)0
#else
extern void   Dbg_D2S (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_BROWSE)
#define Dbg_Browse(fmt,...)  (void)0
#else
extern void   Dbg_Browse (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_NEWGAME)
#define Dbg_Newgame(fmt,...)  (void)0
#else
extern void   Dbg_Newgame (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_REPLY)
#define Dbg_Reply(fmt,...)  (void)0
#else
extern void   Dbg_Reply (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_QUERY)
#define Dbg_Query(fmt,...)  (void)0
#else
extern void   Dbg_Query (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_CENTRAL)
#define Dbg_Central(fmt,...)  (void)0
#else
extern void   Dbg_Central (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_SOCKET)
#define Dbg_Socket(fmt,...)  (void)0
#else
extern void   Dbg_Socket (const char *fmt, ...);
#endif

#if defined(__GNUC__) && !defined(DEBUG_VERSION)
#define Dbg_Version(fmt,...)  (void)0
#else
extern void   Dbg_Version (const char *fmt, ...);
#endif

#ifdef DEBUG_ALLOC
extern void *Dbg_Malloc (const char *file, int line, size_t nBytes);
extern void *Dbg_Calloc (const char *file, int line, size_t nElem, size_t sElem);
extern void  Dbg_Free (const char *file, int line, void *ptr);
extern void  Dbg_Vfree (const char *file, int line, void *ptr);
extern void  Dbg_FinishAlloc (void);
#endif

#endif
/*
 * end of file alloc.h
 */
