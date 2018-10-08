/*
 * file debug.c - debugging routines
 *
 * $Id: debug.c,v 1.37 2006/02/18 21:40:02 fzago Exp $
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

#ifdef DEBUG_ALLOC
#undef malloc
#undef calloc
#undef free
#endif

/*
 * local macros
 */
/* size of alloc tracking table */
#define TABLE_SIZE 20000

/*
 * local types
 */
/* data structure for tracking allocs */
typedef struct
{
	const void *ptr;
	size_t bytes;
	const char *file;
	int line;
} AllocData;

/*
 * local variables
 */
#ifdef DEBUG
/* start value for timer */
static struct timeval timeStart;
#endif

#ifdef DEBUG_ALLOC
#define USETABLE
#undef USETABLE
static int count = 0;
static FILE *fout = NULL;
static const char *outFile = NULL;
#ifdef USETABLE
static AllocData *table = NULL;
#endif
static size_t currentAlloc = 0;
static size_t maxAlloc = 0;
#endif

#if defined(DEBUG) || !defined(__GNUC__)
/*
 * global function: Dbg_StartClock
 * description:     start millisec for profiling
 */
void
Dbg_StartClock (void)
{
#ifdef DEBUG
	gettimeofday (&timeStart, NULL);
#endif
}								/* Dbg_StartClock */

/*
 * global function: Dbg_FinishClock
 * description:     stop millisec for profiling
 */
time_t
Dbg_FinishClock (void)
{
#ifdef DEBUG
	struct timeval timeEnd;
	gettimeofday (&timeEnd, NULL);
	return (timeEnd.tv_sec - timeStart.tv_sec) * 1000 + (timeEnd.tv_usec -
														 timeStart.tv_usec) / 1000;
#endif
}								/* Dbg_FinishClock */

/*
 * global function: Dbg_Out
 * description:     formatted debug output to stderr
 * parameters:      fmt - format string as in printf
 *                  ... - variable args as in printf
 */
void
Dbg_Out (const char *fmt, ...)
{
#ifdef DEBUG
	va_list argList;
#ifdef W32
	static int init = 1;
	static FILE *fp;
	fp = stderr;
	if (init) {
		fp = fopen ("error.log", "w+");
		init = 0;
	}
#endif
	va_start (argList, fmt);
#ifdef W32
	vfprintf (fp, fmt, argList);
#else
	vfprintf (stderr, fmt, argList);
#endif
	va_end (argList);
#endif
}								/* Dbg_Out */
#endif

/*
 * debug outputs for certain sections
 * for gcc: use of variadic macro to define away, use -std=c99 with -pedantic
 * for other: use empty routines which should be optimized away
 */

#if defined(DEBUG_CONFIG) || !defined(__GNUC__)
void
Dbg_Config (const char *fmt, ...)
{
#ifdef DEBUG_CONFIG
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "CONFIG: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_FILE) || !defined(__GNUC__)
void
Dbg_File (const char *fmt, ...)
{
#ifdef DEBUG_FILE
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "FILE: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_LEVEL) || !defined(__GNUC__)
void
Dbg_Level (const char *fmt, ...)
{
#ifdef DEBUG_LEVEL
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "LEVEL: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_GAME) || !defined(__GNUC__)
void
Dbg_Game (const char *fmt, ...)
{
#ifdef DEBUG_GAME
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "GAME: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_ACTION) || !defined(__GNUC__)
void
Dbg_Action (const char *fmt, ...)
{
#ifdef DEBUG_ACTION
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "ACTION: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_NETWORK) || !defined(__GNUC__)
void
Dbg_Network (const char *fmt, ...)
{
#ifdef DEBUG_NETWORK
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "NETWORK: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_SERVER) || !defined(__GNUC__)
void
Dbg_Server (const char *fmt, ...)
{
#ifdef DEBUG_SERVER
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "SERVER: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_CLIENT) || !defined(__GNUC__)
void
Dbg_Client (const char *fmt, ...)
{
#ifdef DEBUG_CLIENT
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "CLIENT: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_USER) || !defined(__GNUC__)
void
Dbg_User (const char *fmt, ...)
{
#ifdef DEBUG_USER
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "USER: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_LISTEN) || !defined(__GNUC__)
void
Dbg_Listen (const char *fmt, ...)
{
#ifdef DEBUG_LISTEN
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "LISTEN: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_STREAM) || !defined(__GNUC__)
void
Dbg_Stream (const char *fmt, ...)
{
#ifdef DEBUG_STREAM
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "STREAM: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_S2C) || !defined(__GNUC__)
void
Dbg_S2C (const char *fmt, ...)
{
#ifdef DEBUG_S2C
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "S2C: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_C2S) || !defined(__GNUC__)
void
Dbg_C2S (const char *fmt, ...)
{
#ifdef DEBUG_C2S
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "C2S: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_X2C) || !defined(__GNUC__)
void
Dbg_X2C (const char *fmt, ...)
{
#ifdef DEBUG_X2C
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "X2C: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_C2X) || !defined(__GNUC__)
void
Dbg_C2X (const char *fmt, ...)
{
#ifdef DEBUG_C2X
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "C2X: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_C2B) || !defined(__GNUC__)
void
Dbg_C2B (const char *fmt, ...)
{
#ifdef DEBUG_C2B
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "C2B: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_DGRAM) || !defined(__GNUC__)
void
Dbg_Dgram (const char *fmt, ...)
{
#ifdef DEBUG_DGRAM
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "DGRAM: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_D2C) || !defined(__GNUC__)
void
Dbg_D2C (const char *fmt, ...)
{
#ifdef DEBUG_D2C
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "D2C: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_D2S) || !defined(__GNUC__)
void
Dbg_D2S (const char *fmt, ...)
{
#ifdef DEBUG_D2S
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "D2S: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_BROWSE) || !defined(__GNUC__)
void
Dbg_Browse (const char *fmt, ...)
{
#ifdef DEBUG_BROWSE
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "BROWSE: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_NEWGAME) || !defined(__GNUC__)
void
Dbg_Newgame (const char *fmt, ...)
{
#ifdef DEBUG_NEWGAME
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "NEWGAME: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_REPLY) || !defined(__GNUC__)
void
Dbg_Reply (const char *fmt, ...)
{
#ifdef DEBUG_REPLY
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "REPLY: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_QUERY) || !defined(__GNUC__)
void
Dbg_Query (const char *fmt, ...)
{
#ifdef DEBUG_QUERY
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "QUERY: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_CENTRAL) || !defined(__GNUC__)
void
Dbg_Central (const char *fmt, ...)
{
#ifdef DEBUG_CENTRAL
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "CENTRAL: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_COMM) || !defined(__GNUC__)
void
Dbg_Comm (const char *fmt, ...)
{
#ifdef DEBUG_COMM
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "COMM: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_SOCKET) || !defined(__GNUC__)
void
Dbg_Socket (const char *fmt, ...)
{
#ifdef DEBUG_SOCKET
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "SOCKET: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_VERSION) || !defined(__GNUC__)
void
Dbg_Version (const char *fmt, ...)
{
#ifdef DEBUG_VERSION
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "VERSION: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#if defined(DEBUG_CHAT) || !defined(__GNUC__)
void
Dbg_Chat (const char *fmt, ...)
{
#ifdef DEBUG_CHAT
	va_list argList;
	va_start (argList, fmt);
	fprintf (stderr, "CHAT: ");
	vfprintf (stderr, fmt, argList);
	va_end (argList);
#endif
}
#endif

#ifdef DEBUG_ALLOC
/*
 *
 */
void
Dbg_FinishAlloc (void)
{
#ifdef USETABLE
	int i, j;
	size_t sumBytes = 0;
	size_t sumAlloc = 0;
	size_t sumTotal = 0;
	char *ptr;

	for (i = 0; i < count; i++) {
		if (table[i].ptr != NULL) {
			fprintf (fout, "[%d] unfreed memory at 0x%08lX (%s:%d)", i, (unsigned long)table[i].ptr,
					 table[i].file, table[i].line);
			sumAlloc++;
			sumBytes += table[i].bytes;
			/* output */
			fputc ('\"', fout);
			for (ptr = table[i].ptr, j = 0; j < table[i].bytes && j < 16; j++, ptr++) {
				if (isprint (*ptr)) {
					fputc (*ptr, fout);
				}
				else {
					fprintf (fout, "\\%03o", (unsigned)*ptr);
				}
			}
			fputs ("\"\n", fout);
		}
		sumTotal += table[i].bytes;
	}
	fprintf (stderr, "%u/%u unfree memory segments with total %u/%u/%u bytes\n", sumAlloc, count,
			 sumBytes, maxAlloc, sumTotal);
#else
	fprintf (stderr, "count=%u current=%lu bytes max=%lu bytes\n", count, (unsigned long)currentAlloc, (unsigned long)maxAlloc);
#endif
}								/* Finish */

/*
 * replacement for malloc
 */
void *
Dbg_Malloc (const char *file, int line, size_t nBytes)
{
	void *result = malloc (nBytes);
#ifdef USETABLE
	assert (count < TABLE_SIZE);
	if (NULL == table) {
		table = calloc (TABLE_SIZE, sizeof (*table));
		assert (NULL != table);
	}
	table[count].ptr = result;
	table[count].bytes = nBytes;
	table[count].file = file;
	table[count].line = line;
#endif
	/* statistics */
	currentAlloc += nBytes;
	if (currentAlloc > maxAlloc) {
		maxAlloc = currentAlloc;
	}
	/* log it */
	if (NULL == fout) {
		fout = (outFile == NULL) ? stderr : fopen (outFile, "w");
	}
	fprintf (fout, "[%d] 0x%08lX = malloc (%lu); %s:%d\n", count, (unsigned long)result,(unsigned long)nBytes,
			 file, line);
	count++;
	return result;
}								/* Malloc */

/*
 * replacement for calloc
 */
void *
Dbg_Calloc (const char *file, int line, size_t nElem, size_t sElem)
{
	void *result = calloc (nElem, sElem);
#ifdef USETABLE
	assert (count < TABLE_SIZE);
	if (NULL == table) {
		table = calloc (TABLE_SIZE, sizeof (*table));
		assert (NULL != table);
	}
	table[count].ptr = result;
	table[count].bytes = nElem * sElem;
	table[count].file = file;
	table[count].line = line;
#endif
	/* statistics */
	currentAlloc += nElem * sElem;
	if (currentAlloc > maxAlloc) {
		maxAlloc = currentAlloc;
	}
	/* log it */
	if (NULL == fout) {
		fout = (outFile == NULL) ? stderr : fopen (outFile, "w");
	}
	/* hook in clean up function */
	fprintf (fout, "[%d] 0x%08lX = calloc (%lu,%lu); %s:%d\n", count, (unsigned long)result, 
			 (unsigned long)nElem, (unsigned long)sElem, file, line);
	count++;
	return result;
}								/* Calloc */

/*
 * replacement for free
 */
void
Dbg_Free (const char *file, int line, void *ptr)
{
	int index = 0;
#ifdef USETABLE
	for (index = 0; index < count; index++) {
		if (table[index].ptr == ptr) {
			break;
		}
	}
	table[index].ptr = NULL;
	/* statistics */
	currentAlloc -= table[index].bytes;
	/* log it */
	if (index == count) {
		fprintf (stderr, "[?] free (0x%08lX); %s:%d\n", (unsigned long)ptr, file, line);
		return;
	}
#endif
	if (NULL == fout) {
		fout = (outFile == NULL) ? stderr : fopen (outFile, "w");
	}
	fprintf (fout, "[%d] free (0x%08lX); %s:%d\n", index, (unsigned long)ptr, file, line);
	free (ptr);
}								/* Free */

/*
 * replacement for free
 */
void
Dbg_Vfree (const char *file, int line, void *ptr)
{
	int index = 0;
#ifdef USETABLE
	for (index = 0; index < count; index++) {
		if (table[index].ptr == ptr) {
			break;
		}
	}
	table[index].ptr = NULL;
	/* statistics */
	currentAlloc -= table[index].bytes;
	/* log it */
	if (index == count) {
		fprintf (stderr, "[?] unlock (0x%08lX); %s:%d\n", (unsigned long)ptr, file, line);
		return;
	}
#endif
	if (NULL == fout) {
		fout = (outFile == NULL) ? stderr : fopen (outFile, "w");
	}
	fprintf (fout, "[%d] unlock (0x%08lX); %s:%d\n", index, (unsigned long)ptr, file, line);
}								/* Free */
#endif

/*
 * end of file alloc.
 */
