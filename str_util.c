/*
 * file str_util.c - some own string routines
 *
 * $Id: str_util.c,v 1.8 2006/02/09 21:21:25 fzago Exp $
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

/* temporary string buffer */
static char *tmpstr = NULL;
static unsigned int tmpsize = 0;

/*
 * duplicate string (some systems do not have strdup)
 */
char *
DupString (const char *ptr)
{
	char *result;
	size_t length;

	assert (ptr != NULL);
	/* alloc data */
	length = strlen (ptr);
	result = malloc (length + 1);
	assert (result != NULL);
	/* copy data */
	memcpy (result, ptr, length + 1);
	/* --- */
	return result;
}								/* DupString */

/*
 * duplicate string (some systems do not have strdup)
 */
char *
DupStringNum (const char *ptr, size_t length)
{
	char *result;

	assert (ptr != NULL);
	/* alloc data */
	result = malloc (length + 1);
	assert (result != NULL);
	/* copy data */
	memcpy (result, ptr, length);
	result[length] = 0;
	/* --- */
	return result;
}								/* DupString */

/*
 * public function split_string
 */
char **
SplitString (const char *string, int *largc)
{
	void *ptr;
	char *buf;
	char **argv;
	int i, length, size, argc;
	XBBool space;

	/* get number of words and total character count */
	space = XBTrue;
	*largc = 0;
	size = 0;
	length = strlen (string);
	for (i = 0; i < length; i++) {
		if (isspace ((unsigned char)string[i])) {
			space = XBTrue;
		}
		else {
			if (space) {
				(*largc)++;
				space = XBFalse;
			}
			size++;
		}
	}
	/* alloc array */
	ptr = malloc (((*largc) + 1) * sizeof (char *) + (size + (*largc)) * sizeof (char));
	assert (ptr != NULL);
	/* now store strings */
	buf = (char *)((char **)ptr + ((*largc) + 1));
	argv = (char **)ptr;
	argc = 0;
	space = XBTrue;
	for (i = 0; i < length; i++) {
		if (isspace ((unsigned char)string[i])) {
			if (*largc <= argc)
				break;
			if (!space) {
				space = XBTrue;
				*buf = '\0';
				buf++;
			}
		}
		else {
			if (space) {
				argv[argc] = buf;
				argc++;
				space = XBFalse;
			}
			*buf = string[i];
			buf++;
		}
	}
	*buf = '\0';
	argv[argc] = NULL;

	return argv;
}								/* SplitString */

/*
 * create a date string for XBlast
 */
const char *
DateString (time_t t)
{
	static char tmp[128];

	strftime (tmp, sizeof (tmp), "%b %d %Y  %H:%M", localtime (&t));
	return tmp;
}								/* DateString */

/*
 * create temporary string
 */
const char *
TempString (const char *fmt, ...)
{
	int n;
	va_list argList;
	while (1) {
		va_start (argList, fmt);
		n = vsnprintf (tmpstr, tmpsize, fmt, argList);
		va_end (argList);
		if (n >= 0 && n <= tmpsize) {
			return tmpstr;
		}
		if (n >= 0) {
			tmpsize = n + 1;
		}
		else if (tmpsize == 0) {
			tmpsize = 100;
		}
		else {
			tmpsize *= 2;
		}
		if (tmpstr != NULL) {
			free (tmpstr);
		}
		tmpstr = malloc (tmpsize);
		assert (NULL != tmpstr);
	}
}								/* TempString */

/*
 * end of file str_util.c
 */
