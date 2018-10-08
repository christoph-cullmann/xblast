/*
 * File util.c - file and directory i/o
 *
 * $Id: util.c,v 1.19 2006/06/12 10:55:35 fzago Exp $
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
 * local macros
 */
#define NUM_XBLAST_PATH 4

#ifndef XBLASTDIR
#define XBLASTDIR NULL
#endif

/*
 * local variables
 */
static int initPath = 0;
static char *pathList[NUM_XBLAST_PATH] = {
	NULL,
	NULL,
	".",
	"/",
};

/* shared buffer for complete filename */
static char zeile[1024];

/*
 * set paths for loading and saving
 */
static void
InitPaths (void)
{
	static char userPath[1024];
	char tmp[1024];

	char *home;
	/* set private xblast path */
	home = getenv ("HOME");
	if (NULL == home) {
		strcpy (userPath, "./user");
	}
	else {
		sprintf (userPath, "%s/.xblast_tnt", home);
	}
#if defined(__MINGW32__) || defined(WMS)
	mkdir (userPath);
	/* config subdirs */
	sprintf (tmp, "%s/config", userPath);
	mkdir (tmp);
	sprintf (tmp, "%s/demo", userPath);
	mkdir (tmp);
	sprintf (tmp, "%s/central", userPath);
	mkdir (tmp);
#else
	mkdir (userPath, 0755);
	/* config subdirs */
	sprintf (tmp, "%s/config", userPath);
	mkdir (tmp, 0700);
	sprintf (tmp, "%s/demo", userPath);
	mkdir (tmp, 0700);
	sprintf (tmp, "%s/central", userPath);
	mkdir (tmp, 0700);
#endif
	/* set user path as first path to load from */
	pathList[0] = userPath;
	/* check environment for xblast search path */
	pathList[1] = getenv ("XBLASTDIR");
	/* --- */
	initPath = 1;
}								/* InitPaths */

/*
 *   find and open an xblast data file.
 */
FILE *
FileOpen (const char *path, const char *name, const char *ext, const char *mode)
{
	FILE *fp;
	int i;
	/* sanity check */
	assert (NULL != path);
	assert (NULL != name);
	assert (NULL != ext);
	assert (NULL != mode);
	/* setup paths for loading */
	if (!initPath) {
		InitPaths ();
	}
	/* try to open file, in directories given in path list   */
	for (i = 0; i < NUM_XBLAST_PATH; i++) {
		if (NULL != pathList[i]) {
			/* create full path to file */
			sprintf (zeile, "%s/%s/%s.%s", pathList[i], path, name, ext);
			/* try to open it */
			if (NULL != (fp = fopen (zeile, mode))) {
				/* file opened succesfully, use fclose to close it */
				return fp;
			}
		}
	}
	/* sorry file opening failed */
	fprintf (stderr, "failed to open file \"%s/%s.%s\".\n", path, name, ext);
	for (i = 0; i < NUM_XBLAST_PATH; i++) {
		if (NULL != pathList[i]) {
			fprintf(stderr,"%i : %s\n",i,pathList[i]);
		}
	}
	return NULL;
}								/* FileOpen */

/**********************
 * creating dir lists *
 **********************/

/*
 * add a file to list
 */
static void
AddToFileList (XBDir ** pList, const char *name, size_t len, time_t mtime)
{
	XBDir *ptr;
	XBDir *next;
	XBDir *prev = NULL;
	int cmp;
	char tmp[256];
	assert (pList != NULL);
	assert (name != NULL);
	assert (len < 256 - 1);
	prev = NULL;
	strncpy (tmp, name, len);
	tmp[len] = 0;
	for (ptr = *pList; ptr != NULL; ptr = ptr->next) {
		cmp = strcmp (ptr->name, tmp);
		if (cmp == 0) {
			/* element already exists */
			return;
		}
		else if (cmp > 0) {
			break;
		}
		prev = ptr;
	}
	/* create new element */
	ptr = calloc (1, sizeof (XBDir));
	assert (ptr != NULL);
	ptr->name = DupStringNum (name, len);
	ptr->mtime = mtime;
	/* insert element after prev */
	if (prev == NULL) {
		next = *pList;
		*pList = ptr;
	}
	else {
		next = prev->next;
		prev->next = ptr;
	}
	ptr->next = next;
}								/* AddToFileList */

/*
 * create file list for dir and extension, recurse optional
 */
XBDir *
CreateFileList (const char *path, const char *ext, XBBool rec)
{
	int i;
	XBDir *list = NULL;
	XBDir *list1 = NULL;
	XBDir *ptr;
#if defined(__MINGW32__) || defined(WMS)
	WIN32_FIND_DATA find_data;
	WIN32_FIND_DATA find_data1;
	HANDLE dp = NULL;
	HANDLE dp1 = NULL;
	size_t lenExt;
	size_t lenName;
	struct _stat buf;
	char *pFile;
	static char dirName[1024];
	static char dirName1[1024];
	static char fileName[1024];
	static char fileName1[1024];
	char dirToOpen[256];
	XBBool testFile = XBTrue;
#else
	DIR *dp;
	DIR *dp1;
	struct dirent *dirp;
	size_t lenExt;
	size_t lenName;
	struct stat buf;
	char *pFile;
	char dirName[1024];
	char dirName1[1024];
	char fileName[1024];
	char fileName1[1024];
#endif
	/* sanity check */
	assert (NULL != path);
	assert (NULL != ext);
	assert ('.' != ext[0]);
	/* set load paths if needed */
	if (!initPath) {
		InitPaths ();
	}
	/* search all directories */
	lenExt = strlen (ext);
	for (i = 0; i < NUM_XBLAST_PATH; i++) {
		if (NULL != pathList[i]) {
			Dbg_File ("trying path %s...\n", pathList[i]);
			/* create path to search */
			strcpy (dirName, pathList[i]);
			strcat (dirName, "/");
			strcat (dirName, path);
			/* prepare full path for file */
			strcpy (fileName, dirName);
			strcat (fileName, "/");
			pFile = fileName + strlen (fileName);
			/* open directory to read */
#if defined(__MINGW32__) || defined(WMS)
			/* windows specific */
			strcpy (dirToOpen, dirName);
			strcat (dirToOpen, "/*");
			/* try to open dir */
			if (INVALID_HANDLE_VALUE == (dp = FindFirstFile (dirToOpen, &find_data))) {
				continue;
			}
			Dbg_File ("opening dir %s\n", dirName);
			while (testFile) {
				/* ignore hidden files */
				if (find_data.cFileName[0] == '.') {
					testFile = FindNextFile (dp, &find_data);
					continue;
				}
				/* ignore files with wrong extension */
				lenName = strlen (find_data.cFileName);
				if (lenName <= lenExt) {
					testFile = FindNextFile (dp, &find_data);
					continue;
				}
				/* check for directory */
				if (0 != strcmp (find_data.cFileName + lenName - lenExt, ext)) {
					/* if recursive check if directory */
					if (0 != rec && find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						sprintf (dirName1, "%s/%s/*", dirName, find_data.cFileName);
						/* try to open */
						if (INVALID_HANDLE_VALUE != (dp1 = FindFirstFile (dirName1, &find_data1))) {
							/* cut the \/* */
							dirName1[strlen (dirName1) - 2] = '\0';
							/* create list */
							list1 = CreateFileList (dirName1, ext, rec);
							if (list1 != NULL) {
								for (ptr = list1; NULL != ptr; ptr = ptr->next) {
									sprintf (fileName1, "%s/%s", find_data.cFileName, ptr->name);
									AddToFileList (&list, fileName1, strlen (fileName1),
												   ptr->mtime);
								}
								DeleteFileList (list1);
								list1 = NULL;
							}
							testFile = FindNextFile (dp, &find_data);
							continue;
						}
					}
					testFile = FindNextFile (dp, &find_data);
					continue;
				}
				/* ignore files without modification time */
				strcpy (pFile, find_data.cFileName);
				if (0 != _stat (fileName, &buf)) {
					testFile = FindNextFile (dp, &find_data);
					continue;
				}
				AddToFileList (&list, find_data.cFileName, lenName - lenExt - 1, buf.st_mtime);
				testFile = FindNextFile (dp, &find_data);
			}
			/* close directory */
			FindClose (dp);
#else
			/* try to open */
			if (NULL == (dp = opendir (dirName))) {
				continue;
			}
			Dbg_File ("opening dir %s\n", dirName);
			/* read files */
			while (NULL != (dirp = readdir (dp))) {
				/* ignroe hidden files */
				if (dirp->d_name[0] == '.') {
					continue;
				}
				/* ignore files with wrong extension */
				lenName = strlen (dirp->d_name);
				if (lenName <= lenExt) {
					continue;
				}
				/* check name for sanity */
				if (0 != strcmp (dirp->d_name + lenName - lenExt, ext)) {
					/* if recursive check if directory */
					if (0 != rec) {
						/* it might be a directory */
						sprintf (dirName1, "%s/%s", dirName, dirp->d_name);
						if (NULL != (dp1 = opendir (dirName1))) {
							/* create a list of the dir / subdirs */
							list1 = CreateFileList (dirName1, ext, rec);
							/* list not empty */
							if (list1 != NULL) {
								/* save list in sub list */
								for (ptr = list1; NULL != ptr; ptr = ptr->next) {
									/* list name is filename from the path GAME_DATADIR/level */
									sprintf (fileName1, "%s/%s", dirp->d_name, ptr->name);
									/* add to main list */
									AddToFileList (&list, fileName1, strlen (fileName1),
												   ptr->mtime);
								}
								DeleteFileList (list1);
								list1 = NULL;
							}
							continue;
						}
					}
					continue;
				}
				/* ignore files without modification time */
				strcpy (pFile, dirp->d_name);
				if (0 != stat (fileName, &buf)) {
					continue;
				}
				/* add the file */
				AddToFileList (&list, dirp->d_name, lenName - lenExt - 1, buf.st_mtime);
			}
			/* close directory */
			closedir (dp);
#endif
		}
	}
	return list;
}								/* CreateFileList */

/*
 * delete a file list
 */
void
DeleteFileList (XBDir * list)
{
	XBDir *next;
	while (NULL != list) {
		next = list->next;
		/* delete data */
		if (list->name != NULL) {
			free (list->name);
		}
		free (list);
		/* continue */
		list = next;
	}
}								/* DeleteFileList */

/******************
 * loading images *
 ******************/

/*
 * swaps bit order of pbm data to xbm format
 */
static void
PbmSwapBits (unsigned char *pbm, int width, int height)
{
	int nbytes;
	static XBBool convTableInit = XBFalse;
	static unsigned convTable[256];
	if (!convTableInit) {
		unsigned i, j;
		unsigned *ptr;
		for (i = 0, ptr = convTable; i < 256; i++, ptr++) {
			*ptr = 0;
			for (j = 0; j < 8; j++) {
				if (i & (1 << j)) {
					*ptr |= (128 >> j);
				}
			}
		}
		convTableInit = XBTrue;
	}
	for (nbytes = (width * height + 7) / 8; nbytes > 0; nbytes--, pbm++) {
		*pbm = convTable[*pbm];
	}
}								/* PbmSwapBits */

#ifdef MINI_XBLAST
static void
PbmShrink (unsigned char *pbm, int *width, int *height)
{
	static XBBool mergeTableInit = XBFalse;
	static unsigned loMergeTable[256];
	static unsigned hiMergeTable[256];
	/* --- */
	int x, y;
	int hHeight;
	int hWidth;
	unsigned char *dst;
	unsigned char *pFirst;
	unsigned char *pSecond;
	if (!mergeTableInit) {
		unsigned i, j;
		unsigned *hi;
		unsigned *lo;
		for (i = 0, hi = hiMergeTable, lo = loMergeTable; i < 256; i++, hi++, lo++) {
			*hi = 0;
			*lo = 0;
			for (j = 0; j < 8; j++) {
				if (i & (1 << j)) {
					*lo |= (1 << (j / 2));
				}
			}
			*hi = *lo << 4;
		}
		mergeTableInit = XBTrue;
	}
	/* do the conversion */
	assert (0 == *width % 8);
	hWidth = *width / 2;
	hHeight = *height / 2;
	dst = pbm;
	for (y = 0; y < hHeight; y++) {
		pFirst = pbm + 2 * y * (*width / 8);
		pSecond = pFirst + (*width / 8);
		for (x = 0; x < (hWidth / 8); x++) {
			dst[0] = (loMergeTable[pFirst[0]] |
					  loMergeTable[pSecond[0]] |
					  hiMergeTable[pFirst[1]] | hiMergeTable[pSecond[1]]);
			dst++;
			pFirst += 2;
			pSecond += 2;
		}
		if (0 != hWidth % 8) {
			dst[0] = (loMergeTable[pFirst[0]] | loMergeTable[pSecond[0]]);
			dst++;
		}
	}
	*height = hHeight;
	*width = hWidth;
}								/* PbmShrink */
#endif

/*
 * public function read pbm file
 * load portable bitmap into memory
 */
unsigned char *
ReadPbmFile (const char *path, const char *filename, int *width, int *height)
{
	FILE *fp = NULL;
	unsigned char *pbm = NULL;
	size_t pbmSize;
	assert (width != NULL);
	assert (height != NULL);
	/* try to open file name */
	if (NULL == (fp = FileOpen (path, filename, "pbm", "rb"))) {
		return NULL;
	}
	/* read header */
	if (3 != fscanf (fp, "%s%d%d%*d%*c", zeile, width, height)) {
		fprintf (stderr, "Failed to read ppm header\n");
		goto Error;
	}
	/* set size */
	pbmSize = ((*width) * (*height) + 7) / 8;
	/* alloc data */
	if (NULL == (pbm = malloc (pbmSize * sizeof (char)))) {
		goto Error;
	}
	/* read data */
	if (pbmSize != fread (pbm, sizeof (char), pbmSize, fp)) {
		goto Error;
	}
	/* swap bit order */
	PbmSwapBits (pbm, *width, *height);
#ifdef MINI_XBLAST
	PbmShrink (pbm, width, height);
#endif
	/* close file */
	fclose (fp);
	return pbm;
	/* error handling */
  Error:
	if (fp != NULL) {
		fclose (fp);
	}
	if (pbm != NULL) {
		free (pbm);
	}
	return NULL;
}								/* ReadPbmFile */

#ifdef MINI_XBLAST
/*
 * shrink ppm data to half size
 */
static void
ShrinkPpm (unsigned char *ppm, int *width, int *height)
{
	int x, y;
	int hWidth;
	int hHeight;
	unsigned r, g, b;
	unsigned char *pFirst;
	unsigned char *pSecond;
	unsigned char *dst;
	assert (NULL != width);
	assert (NULL != height);
	hWidth = *width / 2;
	hHeight = *height / 2;
	dst = ppm;
	for (y = 0; y < hHeight; y++) {
		pFirst = ppm + 3 * 2 * y * (*width);
		pSecond = pFirst + 3 * (*width);
		for (x = 0; x < hWidth; x++) {
			/* read data */
			r = 3u + (unsigned)pFirst[0] + (unsigned)pFirst[3] + (unsigned)pSecond[0] +
				(unsigned)pSecond[3];
			g = 3u + (unsigned)pFirst[1] + (unsigned)pFirst[4] + (unsigned)pSecond[1] +
				(unsigned)pSecond[4];
			b = 3u + (unsigned)pFirst[2] + (unsigned)pFirst[5] + (unsigned)pSecond[2] +
				(unsigned)pSecond[5];
			/* store data */
			dst[0] = (r >> 2);
			dst[1] = (g >> 2);
			dst[2] = (b >> 2);
			/* next step */
			pFirst += 6;
			pSecond += 6;
			dst += 3;
		}
	}
	*width = hWidth;
	*height = hHeight;
}								/* ShrinkPpm */
#endif

/*
 * public function read pbm file
 * load portable bitmap into memory
 */
unsigned char *
ReadPpmFile (const char *path, const char *filename, int *width, int *height)
{
	FILE *fp = NULL;
	unsigned char *ppm = NULL;
	size_t nPixel;
	assert (width != NULL);
	assert (height != NULL);
	/* try to open for reading */
	if (NULL == (fp = FileOpen (path, filename, "ppm", "rb"))) {
		goto Error;
	}
	/* read header */
	if (3 != fscanf (fp, "%s%d%d%*d%*c", zeile, width, height)) {
		fprintf (stderr, "Failed to read ppm header\n");
		goto Error;
	}
	nPixel = 3 * (*width) * (*height);
	/* alloc data */
	if (NULL == (ppm = malloc (nPixel * sizeof (char)))) {
		goto Error;
	}
	/* read data */
	if (nPixel != fread (ppm, sizeof (char), nPixel, fp)) {
		goto Error;
	}
	/* close file */
	fclose (fp);
#ifdef MINI_XBLAST
	/* convert to half size if needed */
	ShrinkPpm (ppm, width, height);
#endif
	/* that's all */
	return ppm;
	/* error handling */
  Error:
	if (fp != NULL) {
		fclose (fp);
	}
	if (ppm != NULL) {
		free (ppm);
	}
	return NULL;
}								/* ReadPpmFile */

#ifdef MINI_XBLAST
/*
 * shrink epm to half size
 */
static void
ShrinkEpm (unsigned char *epm, int *width, int *height, int depth)
{
	int x, y;
	int hWidth;
	int hHeight;
	unsigned v;
	unsigned char *pFirst;
	unsigned char *pSecond;
	unsigned char *dst;
	assert (NULL != width);
	assert (NULL != height);
	hWidth = *width / 2;
	hHeight = *height / 2;
	dst = epm;
	for (y = 0; y < depth * hHeight; y++) {
		pFirst = epm + 2 * y * (*width);
		pSecond = pFirst + (*width);
		for (x = 0; x < hWidth; x++) {
			/* read data */
			v = 3u + (unsigned)pFirst[0] + (unsigned)pFirst[1] + (unsigned)pSecond[0] +
				(unsigned)pSecond[1];
			/* store data */
			dst[0] = (v >> 2);
			/* next step */
			pFirst += 2;
			pSecond += 2;
			dst += 1;
		}
	}
	*width = hWidth;
	*height = hHeight;
}								/* ShrinkEpm */
#endif

/*
 * load  Extended Pixmap (.epm) into memory
 */
unsigned char *
ReadEpmFile (const char *path, const char *filename, int *width, int *height, int *depth)
{
	FILE *fp = NULL;
	unsigned char *epm = NULL;
	unsigned char *buf = NULL;
	size_t nPixel;
	assert (width != NULL);
	assert (height != NULL);
	assert (depth != NULL);
	/* try to open for reading */
	if (NULL == (fp = FileOpen (path, filename, "epm", "rb"))) {
		fprintf (stderr, "failed to open file \"%s/%s.%s\".\n", path, filename, "epm");
		return NULL;
	}
	/* read header */
	if (4 != fscanf (fp, "%s%d%d%*d%d%*c", zeile, width, height, depth)) {
		fprintf (stderr, "Failed to read epm header\n");
		goto Error;
	}
	/* calc number of pixels */
	nPixel = (*depth) * (*width) * (*height);
	/* alloc data */
	if (NULL == (epm = malloc (nPixel * sizeof (char)))) {
		goto Error;
	}
	/* check magic */
	if (0 == strcmp (zeile, "PX")) {
		/* uncompressed data */
		/* read data */
		if (nPixel != fread (epm, sizeof (char), nPixel, fp)) {
			goto Error;
		}
	}
	else if (0 == strcmp (zeile, "PZ")) {
		/* compressed data */
		int i, j, n_bytes, zero_count;
		/* alloc input buffer */
		if (NULL == (buf = malloc (2 * nPixel * sizeof (char)))) {
			goto Error;
		}
		if (0 == (n_bytes = fread (buf, sizeof (char), 2 * nPixel, fp))) {
			goto Error;
		}
		for (i = 0, j = 0; i < n_bytes; i++) {
			if (buf[i]) {
				epm[j] = buf[i];
				j++;
			}
			else {
				zero_count = 0;
				do {
					i++;
					zero_count += buf[i];
				} while (buf[i] == 255);
				memset (epm + j, 0, zero_count);
				j += zero_count;
			}
		}
		free (buf);
	}
	else {
		/* wrong magic word */
		fprintf (stderr, "Wrong magic word \"%s\".\n", zeile);
		goto Error;
	}
#ifdef MINI_XBLAST
	ShrinkEpm (epm, width, height, *depth);
#endif
	/* close file */
	fclose (fp);
	return epm;
	/* Error handling */
  Error:
	if (fp != NULL) {
		fclose (fp);
	}
	if (epm != NULL) {
		free (epm);
	}
	if (buf != NULL) {
		free (buf);
	}
	return NULL;
}								/* ReadEpmFile */

/*
 * load raw sound file into memory
 */
char *
ReadRawFile (const char *path, const char *filename, size_t * len)
{
	FILE *fp = NULL;
	char *buf = NULL;
	struct stat statBuf;
	size_t toRead;
	size_t lastRead;
	char *ptr;
	size_t i;
	/* sanity check */
	assert (NULL != path);
	assert (NULL != filename);
	assert (NULL != len);
	/* open file for reading */
	if (NULL == (fp = FileOpen (path, filename, "raw", "rb"))) {
		fprintf (stderr, "failed to open file \"%s/%s.%s\".\n", path, filename, "raw");
		return NULL;
	}
	/* determine length */
	if (-1 == fstat (fileno (fp), &statBuf)) {
		fprintf (stderr, "failed to get length of file \"%s/%s.%s\".\n", path, filename, "raw");
		goto Error;
	}
	*len = statBuf.st_size;
#ifdef DEBUG
	fprintf (stderr, "length of file \"%s/%s.%s\" is %lu\n", path, filename, "raw", (unsigned long)*len);
#endif
	/* allocate buffer */
	if (NULL == (buf = malloc (*len))) {
		fprintf (stderr, "failed to alloc buffer for file \"%s/%s.%s\".\n", path, filename, "raw");
		goto Error;
	}
	/* read data */
	toRead = *len;
	ptr = buf;
	while (toRead > 0) {
		if (0 >= (lastRead = fread (ptr, 1, toRead, fp))) {
			fprintf (stderr, "error while reading file \"%s/%s.%s\".\n", path, filename, "raw");
			goto Error;
		}
		toRead -= lastRead;
		ptr += lastRead;
	}
	fclose (fp);
	/* convert data from 0..255 to -128..127 */
	for (i = 0; i < *len; i++) {
		buf[i] += 0x80;
	}
	/* that's all */
	return buf;
  Error:
	if (NULL != buf) {
		free (buf);
	}
	if (NULL != fp) {
		fclose (fp);
	}
	return NULL;
}								/* ReadRawFile */

/*
 * end of file util.h
 */
