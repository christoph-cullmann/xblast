/*
 * file version.c - tools for version data
 *
 * $Id: version.c,v 1.13 2006/02/10 15:07:17 fzago Exp $
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

/* initial joint version */
static XBVersion jointVersion = { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
};

/* constant versions */
const XBVersion Ver_Local = { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH };
const XBVersion Ver_None = { 0, 0, 0 };
const XBVersion Ver_2_10_1 = { 2, 10, 1 };
const XBVersion Ver_2_10_2 = { 2, 10, 2 };

/* version table for hosts */
static XBVersion hostVersion[MAX_HOSTS];

/*************************************
 * general tools for version structs *
 *************************************/

/*
 * return temporary string for a version
 */
char *
Version_ToString (const XBVersion * ver)
{
	static char tmp[20];
	sprintf (tmp, "V%i.%i.%i", ver->major, ver->minor, ver->patch);
	return tmp;
}								/* Version_ToString */

/*
 * clear a version struct
 */
void
Version_Clear (XBVersion * ver)
{
	assert (NULL != ver);
	memcpy (ver, &Ver_None, sizeof (XBVersion));
}								/* Version_Clear */

/*
 * check if version is set
 */
XBBool
Version_isDefined (const XBVersion * ver)
{
	assert (NULL != ver);
	return (memcmp (ver, &Ver_None, sizeof (XBVersion)) != 0);
}								/* Version_isDefined */

/*
 * compare two versions, return sign of v1-v2
 */
int
Version_Compare (const XBVersion * v1, const XBVersion * v2)
{
	assert (NULL != v1);
	assert (NULL != v2);
	if (v1->major > v2->major) {
		return (1);
	}
	else if (v1->major < v2->major) {
		return (-1);
	}
	if (v1->minor > v2->minor) {
		return (1);
	}
	else if (v1->minor < v2->minor) {
		return (-1);
	}
	if (v1->patch > v2->patch) {
		return (1);
	}
	else if (v1->patch < v2->patch) {
		return (-1);
	}
	return (0);
}								/* Version_Compare */

/******************************
 * getting local data version *
 ******************************/

/*
 * return local version data as XBVersion
 */
void
Version_Get (XBVerType type, XBVersion * ver)
{
	assert (NULL != ver);
	switch (type) {
	case VERSION_JOINT:
		memcpy (ver, &jointVersion, sizeof (XBVersion));
		return;
	default:
		assert (type < MAX_HOSTS);
		memcpy (ver, hostVersion + type, sizeof (XBVersion));
		return;
	}
}								/* Version_Get */

/*
 * check if joint version is at least given version
 */
XBBool
Version_AtLeast (XBVerType type, const XBVersion * ver)
{
	assert (NULL != ver);
	switch (type) {
	case VERSION_JOINT:
		return (Version_Compare (&jointVersion, ver) >= 0);
	default:
		assert (type < MAX_HOSTS);
		return (Version_Compare (hostVersion + type, ver) >= 0);
	}
}								/* Version_AtLeast */

/***************************
 * modifying host version *
 ***************************/

/*
 * reset host versions
 */
void
Version_Reset (void)
{
	unsigned char id;
	for (id = 0; id < MAX_HOSTS; id++) {
		Version_Clear (hostVersion + id);
	}
	Dbg_Version ("Resetting host version\n");
	jointVersion = Ver_Local;
}								/* Version_Reset */

/*
 * set a host version and modify to joint version, return change
 */
XBBool
Version_Join (unsigned char id, const XBVersion * ver)
{
	assert (NULL != ver);
	assert (id < MAX_HOSTS);
	if (Version_isDefined (ver)) {
		Dbg_Version ("Current joint version %s\n", Version_ToString (&jointVersion));
		Dbg_Version ("Joining version %s for host #%u\n", Version_ToString (ver), id);
		memcpy (hostVersion + id, ver, sizeof (XBVersion));
		if (Version_Compare (ver, &jointVersion) < 0) {
			jointVersion = *ver;
			Dbg_Version ("Updating joint version\n");
			return XBTrue;
		}
	}
	else {
		Dbg_Version ("Ignoring undefined version for host #%u\n", id);
	}
	return XBFalse;
}								/* Version_Join */

/*
 * remove a single host version, return change
 */
XBBool
Version_Remove (unsigned char id)
{
	unsigned char h;
	assert (id < MAX_HOSTS);
	if (Version_isDefined (hostVersion + id)) {
		Dbg_Version ("Current joint version %s\n", Version_ToString (&jointVersion));

		/*  first calculate joint version without the host version to be removed */
		memcpy (&jointVersion, &Ver_Local, sizeof (XBVersion));
		for (h = 0; h < MAX_HOSTS; h++) {
			if (h != id && Version_isDefined (hostVersion + h)
				&& Version_Compare (hostVersion + h, &jointVersion) < 0) {
				memcpy (&jointVersion, hostVersion + h, sizeof (XBVersion));
			}
		}

		/* now compare with version to be removed, then clear */
		if (Version_Compare (&jointVersion, hostVersion + id) > 0) {
			Dbg_Version ("removing version for host #%u, joint version update to %s", id,
						 Version_ToString (&jointVersion));
			Version_Clear (hostVersion + id);
			return XBTrue;
		}
		else {
			Dbg_Version ("removing version for host #%u\n", id);
			Version_Clear (hostVersion + id);
		}
	}
	return XBFalse;
}								/* Version_Remove */

/*
 * end of file com.c
 */
