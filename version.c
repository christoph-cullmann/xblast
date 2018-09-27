/*
 * file version.c - tools for version data
 *
 * $Id: version.c,v 1.4 2005/01/18 15:33:19 lodott Exp $
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

#include "version.h"
#include "atom.h"

static const XBVersion localVersion = {
  VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
};
static XBVersion jointVersion = {
  VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
};

const XBVersion Ver_2_10_1= {2,10,1};

/*
 * return local version data as XBVersion
 */
void
Version_GetLocal(XBVersion *ver) {
  assert(NULL != ver);
  *ver = localVersion;
} /* Version_GetLocal */

/*
 * return local version data as XBVersion
 */
void
Version_GetJoint(XBVersion *ver) {
  assert(NULL != ver);
  *ver = jointVersion;
} /* Version_GetJoint */

/*
 * return temporary version string
 */
char *
Version_ToString(const XBVersion *ver) {
  static char tmp[20];
  sprintf(tmp,"%i.%i.%i",ver->major,ver->minor,ver->patch);
  return tmp;
} /* Version_ToString */

/*
 * clear a version string
 */
void
Version_Clear(XBVersion *ver)
{
  assert (NULL != ver);
  ver->major = 0;
  ver->minor = 0;
  ver->patch = 0;
} /* Version_Clear */

/*
 * clear a version string
 */
XBBool
Version_isDefined(const XBVersion *ver)
{
  assert (NULL != ver);
  return ( ver->major != 0 || ver->minor!=0 || ver->patch!=0);
} /* Version_isDefined */

/*
 * reset joint version to local
 */
void
Version_Reset()
{
  Dbg_Version("Resetting joint version\n");
  jointVersion = localVersion;
} /* Version_Reset */

/*
 * compare two versions
 */
int
Version_Compare(const XBVersion *v1, const XBVersion *v2) {
  assert(NULL != v1);
  assert(NULL != v2);
  if (v1->major > v2->major) {
    return(1);
  } else if (v1->major < v2->major) {
    return(-1);
  }
  if (v1->minor > v2->minor) {
    return(1);
  } else if (v1->minor < v2->minor) {
    return(-1);
  }
  if (v1->patch > v2->patch) {
    return(1);
  } else if (v1->patch < v2->patch) {
    return(-1);
  }
  return(0);
} /* Version_Compare */

/*
 * join another version number (if defined) to jointVersion
 */
void
Version_Join(const XBVersion *ver)
{
  assert(NULL != ver);
  if (Version_isDefined(ver)) {
    Dbg_Version("Joining version %s\n", Version_ToString(ver));
    if (Version_Compare(ver,&jointVersion)<0) {
      jointVersion = *ver;
    }
  }
} /* Version_Join */

/*
 * show version data
 */
void
Version_ShowData()
{
  Dbg_Out("local version = %s\n", Version_ToString(&localVersion));
  Dbg_Out("joint version = %s\n", Version_ToString(&jointVersion));
} /* Version_ShowData */

/*
 * check if joint version is at least given version
 */
XBBool
Version_AtLeast(const XBVersion *ver)
{
  return (Version_Compare(&jointVersion, ver) >= 0);
} /* Version_AtLeast */

/*
 * end of file com.c
 */
