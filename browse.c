/*
 * file browse.c - datatype used in game browsing
 *
 * $Id: browse.c,v 1.8 2006/02/24 21:29:16 fzago Exp $
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
 * local constants
 */
const unsigned char magicWord[6] = { 'X', 'B', 'L', 'A', 'S', 'T' };
const unsigned char protocolVersion = 1u;

/*
 * check for query format
 */
static XBBrowseTeleType
ParseQuery (XBBrowseTeleQuery * tele, const unsigned char *buf, size_t len)
{
	return (0 == len) ? XBBT_Query : XBBT_None;
}								/* ParseQuery */

/*
 * check for reply format
 */
static XBBrowseTeleType
ParseReply (XBBrowseTeleReply * tele, const unsigned char *buf, size_t len)
{
	size_t i = 0;
	/* port number */
	if (i + 1 >= len) {
		return XBBT_None;
	}
	tele->port = (unsigned short)buf[i] + ((unsigned short)buf[i + 1] << 8);
	i += 2;
	/* versions */
	if (i + 2 >= len) {
		return XBBT_None;
	}
	tele->version[0] = buf[i++];
	tele->version[1] = buf[i++];
	tele->version[2] = buf[i++];
	/* game */
	if (i + sizeof (tele->game) - 1 >= len) {
		return XBBT_None;
	}
	memcpy (tele->game, buf + i, sizeof (tele->game));
	i += sizeof (tele->game);
	memcpy (tele->host, buf + i, sizeof (tele->host));
	i += sizeof (tele->host);
	/* num lives */
	if (i >= len) {
		return XBBT_None;
	}
	tele->numLives = buf[i++];
	/* num wins */
	if (i >= len) {
		return XBBT_None;
	}
	tele->numWins = buf[i++];
	/* frame rate */
	if (i >= len) {
		return XBBT_None;
	}
	tele->frameRate = buf[i++];
	return (i == len) ? XBBT_Reply : XBBT_None;
}								/* ParseReply */

/*
 * check for newgame format
 */
static XBBrowseTeleType
ParseNewGame (XBBrowseTeleNewGame * tele, const unsigned char *buf, size_t len)
{
	size_t i = 0;
	/* port number */
	if (i + 1 >= len) {
		return XBBT_None;
	}
	tele->port = (unsigned short)buf[i] + ((unsigned short)buf[i + 1] << 8);
	i += 2;
	/* versions */
	if (i + 2 >= len) {
		return XBBT_None;
	}
	tele->version[0] = buf[i++];
	tele->version[1] = buf[i++];
	tele->version[2] = buf[i++];
	/* game */
	if (i + sizeof (tele->game) - 1 >= len) {
		return XBBT_None;
	}
	memcpy (tele->game, buf + i, sizeof (tele->game));
	i += sizeof (tele->game);
	/* num lives */
	if (i + 4 >= len) {
		return XBBT_None;
	}
	tele->gameID =
		(int)buf[i] + ((int)buf[i + 1] << 8) + ((int)buf[i + 2] << 16) + ((int)buf[i + 3] << 24);
	i += 4;
	/* num wins */
	if (i >= len) {
		return XBBT_None;
	}
	tele->numLives = buf[i++];
	/* num wins */
	if (i >= len) {
		return XBBT_None;
	}
	tele->numWins = buf[i++];
	/* frame rate */
	if (i >= len) {
		return XBBT_None;
	}
	tele->frameRate = buf[i++];
	return (i == len) ? XBBT_NewGame : XBBT_None;
}								/* ParseNewGame */

/*
 * check for newgameok = close format
 */
static XBBrowseTeleType
ParseNewGameOK (XBBrowseTeleNewGameOK * tele, const unsigned char *buf, size_t len)
{
	size_t i = 0;
	/* port number */
	if (i + 4 > len) {
		return XBBT_None;
	}
	tele->gameID =
		(int)buf[i] + ((int)buf[i + 1] << 8) + ((int)buf[i + 2] << 16) + ((int)buf[i + 3] << 24);
	i += 4;
	return (i == len) ? XBBT_NewGameOK : XBBT_None;
}								/* ParseNewGameOk */

/*
 * check browse layer and parse contained data
 */
XBBrowseTeleType
BrowseTele_Parse (XBBrowseTele * tele, const unsigned char *buf, size_t len)
{
	size_t i;

	assert (NULL != tele);
	assert (NULL != buf);

	/* parse magic word */
	if (len < sizeof (magicWord)) {
		return XBBT_None;
	}
	if (0 != memcmp (buf, magicWord, sizeof (magicWord))) {
		return XBBT_None;
	}
	i = sizeof (magicWord);
	/* check protocol version */
	if (i >= len) {
		return XBBT_None;
	}
	if (buf[i] != protocolVersion) {
		return XBBT_None;
	}
	i++;
	/* get telegram type */
	if (i >= len) {
		return XBBT_None;
	}
	tele->type = buf[i++];
	/* get serial */
	if (i >= len) {
		return XBBT_None;
	}
	tele->any.serial = buf[i++];
	/* now type specific data */
	switch (tele->type) {
	case XBBT_Query:
		return ParseQuery (&tele->query, buf + i, len - i);
	case XBBT_Reply:
		return ParseReply (&tele->reply, buf + i, len - i);
	case XBBT_NewGame:
		return ParseNewGame (&tele->newGame, buf + i, len - i);
		break;
	case XBBT_NewGameOK:
		return ParseNewGameOK (&tele->newGameOK, buf + i, len - i);
		break;
	default:
		return XBBT_None;
	}
}								/* BrowseTele_Parse */

/*
 * write the browse header to buffer
 */
static size_t
WriteAny (const XBBrowseTeleAny * tele, unsigned char *buf)
{
	size_t i;

	/* magic word */
	memcpy (buf, magicWord, sizeof (magicWord));
	i = sizeof (magicWord);
	/* protocol version */
	buf[i++] = protocolVersion;
	/* tele type */
	buf[i++] = tele->type;
	/* serial */
	buf[i++] = tele->serial;
	return i;
}								/* WriteAny */

/*
 * write query data to buffer
 */
static size_t
WriteQuery (const XBBrowseTeleQuery * tele, unsigned char *buf)
{
	return 0;
}								/* WriteQuery */

/*
 * write reply data to buffer
 */
static size_t
WriteReply (const XBBrowseTeleReply * tele, unsigned char *buf)
{
	size_t i = 0;

	buf[i++] = tele->port & 0xFF;
	buf[i++] = (tele->port >> 8) & 0xFF;
	buf[i++] = tele->version[0];
	buf[i++] = tele->version[1];
	buf[i++] = tele->version[2];
	memcpy (buf + i, tele->game, sizeof (tele->game));
	i += sizeof (tele->game);
	memcpy (buf + i, tele->host, sizeof (tele->host));
	i += sizeof (tele->host);
	buf[i++] = tele->numLives;
	buf[i++] = tele->numWins;
	buf[i++] = tele->frameRate;
	return i;
}								/* WriteReply */

/*
 * write NewGame data to buffer
 */
static size_t
WriteNewGame (const XBBrowseTeleNewGame * tele, unsigned char *buf)
{
	size_t i = 0;

	buf[i++] = tele->port & 0xFF;
	buf[i++] = (tele->port >> 8) & 0xFF;
	buf[i++] = tele->version[0];
	buf[i++] = tele->version[1];
	buf[i++] = tele->version[2];
	memcpy (buf + i, tele->game, sizeof (tele->game));
	i += sizeof (tele->game);
	buf[i++] = tele->gameID & 0xFF;
	buf[i++] = (tele->gameID >> 8) & 0xFF;
	buf[i++] = (tele->gameID >> 16) & 0xFF;
	buf[i++] = (tele->gameID >> 24) & 0xFF;

	buf[i++] = tele->numLives;
	buf[i++] = tele->numWins;
	buf[i++] = tele->frameRate;
	return i;
}								/* WriteNewGame */

/*
 * write NewGameOk data to buffer
 */
static size_t
WriteNewGameOK (const XBBrowseTeleNewGameOK * tele, unsigned char *buf)
{
	size_t i = 0;

	buf[i++] = tele->gameID & 0xFF;
	buf[i++] = (tele->gameID >> 8) & 0xFF;
	buf[i++] = (tele->gameID >> 16) & 0xFF;
	buf[i++] = (tele->gameID >> 24) & 0xFF;
	return i;
}								/* WriteNewGameOk */

/*
 * write complete browse layer to buffer
 */
size_t
BrowseTele_Write (const XBBrowseTeleAny * tele, unsigned char *buf)
{
	size_t nBytes;
	assert (NULL != tele);
	assert (NULL != buf);
	/* write header */
	nBytes = WriteAny (tele, buf);
	switch (tele->type) {
	case XBBT_Query:
		nBytes += WriteQuery ((const XBBrowseTeleQuery *) tele, buf + nBytes);
		break;
	case XBBT_Reply:
		nBytes += WriteReply ((const XBBrowseTeleReply *) tele, buf + nBytes);
		break;
	case XBBT_NewGame:
		nBytes += WriteNewGame ((const XBBrowseTeleNewGame *) tele, buf + nBytes);
		break;
	case XBBT_NewGameOK:
		nBytes += WriteNewGameOK ((const XBBrowseTeleNewGameOK *) tele, buf + nBytes);
		break;
	default:
		break;
	}
	/* that's all */
	return nBytes;
}								/* BrowseTele_Write */

/*
 * end of file browse.c
 */
