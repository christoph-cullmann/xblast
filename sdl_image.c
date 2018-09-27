/*
 * file SDL_image.c - image conversion (rgb to pixel)
 *
 * $Id: sdl_image.c,v 1.3 2004/09/13 22:32:35 tenderflake Exp $
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
#include "util.h"
#include "color.h"
#include "sdl_common.h"
#include <string.h>
#include <SDL/SDL_image.h>


/*
 * library function: InitImages
 * description:      initializes data structure neede for image conversion
 * parameters:       none
 * return value:     0 on success, -1 on failure 
 */
XBBool 
InitImages (void)
{
  // Nothing to do!
  return XBTrue;
} /* InitImages */


/*
 *
 */
void
FinishImages (void)
{
  // Nothing to do!
} /* FinishImages */


/*
 * library function: CatPathAndFilename
 * description:      Concatenate path and filename in single string.
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 * return value:     Absolute filename.
 */
char *
CatPathAndFilename(const char *path, 
                   const char *filename)
{
  char *absPath;
  size_t pathLength, filenameLength;
  
  /* Check if last character is a path separator "/" */
  pathLength     = strlen(path);
  filenameLength = strlen(filename);
  
  /* 1 seperator "/" char + 1 string terminating NULL = 2 */
  absPath = (char *)calloc(pathLength + filenameLength + 2, sizeof(char));
  if ( !absPath ) {
    fprintf(stderr, "Could not allocate memory in CatPathAndFile(%s, %s)", path, filename);
    return NULL;
  };
  strcat(absPath, path);
  strcat(absPath, "/");
  strcat(absPath, filename);
  return absPath;
};


/*
 * library function: AddExtension
 * description:      Add specified extension to filename.
 * parameters:       - Filename (without extension)
 *                   - Extension (duh!)
 * return value:     Filename + Extension.
 */
char *
AddExtension(const char *Filename,
             const char *Extension)
{
  char *result;
  size_t FilenameLength, ExtensionLength;
  
  /* Check if last character is a path separator "/" */
  FilenameLength     = strlen(Filename);
  ExtensionLength = strlen(Extension);
  
  /* 1 "." char + 1 string terminating NULL = 2 */
  result = (unsigned char *)calloc(FilenameLength + ExtensionLength + 2, sizeof(char));
  if ( !result ) {
    fprintf(stderr, "Could not allocate memory in AddExtension(%s, %s)", Filename, Extension);
    return NULL;
  };
  strcat(result, Filename);
  strcat(result, ".");
  strcat(result, Extension);
  return result;
};


/*
 * library function: ConvertToScreenSurface
 * description:      Convert a given surface to the same
 *                   pixel format used by the screen.
 * parameters:       - Surface to convert.
 * return value:     Converted surface.
 */
static SDL_Surface *
ConvertToScreenSurface(SDL_Surface *src) {
  SDL_Surface *bitmap = NULL;
  bitmap = SDL_ConvertSurface(src, screen->format, SDL_HWSURFACE);
  if(!bitmap) {
    fprintf(stderr, "Could not create bitmap (%s)", SDL_GetError());
    return NULL;
  };
  return bitmap;
};



/*
 * local function: BitmapFromRGBPixel
 * description:    creates bitmap from  pixel data in  24 bit RGB format
 * parameters:     data   - 24 bit pixel data (b,g,r!)
 *                 width  - width of bitmap
 *                 height - height of bitmap
 * return value:   handle of bitmap, or NULL on failure
 */
static SDL_Surface *
BitmapFromRGBPixel (unsigned char *data, int width, int height)
{

  SDL_Surface *temp   = NULL; 
  SDL_Surface *bitmap = NULL;
  unsigned char *pixels = NULL;

  // Create new empty surface with correct dimensions
  temp = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 24,
                                    RMASK, GMASK, BMASK, 0);

  // Copy all pixels from data to surface.
  SDL_LockSurface(temp);
  pixels = (unsigned char *)temp->pixels;
  int i = 0;
  for(i = 0; i<(3*width*height); i++){
    *(pixels+i) = *(data+i);
  };
  SDL_UnlockSurface(temp);

  bitmap = ConvertToScreenSurface(temp);
  SDL_FreeSurface(temp);
  return bitmap;

} /* BitmapFromRGBPixel */




/*
 * library function: ReadPbmBitmap
 * description:      create a bitmap from a given pbm-file
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 * return value:     handle of bitmap, or NULL on failure
 */
SDL_Surface *
ReadPbmBitmap (const char *path, const char *filename)
{
  SDL_Surface   *temp, *bitmap;

  char *tempAbsFilename, *absFilename;
  tempAbsFilename = CatPathAndFilename(path, filename);
  absFilename = AddExtension(tempAbsFilename, "pbm");

  temp = IMG_Load(absFilename);
  if(!temp) {
    fprintf(stderr, "Could not create bitmap (%s)", SDL_GetError());
    return NULL;
  };
  
  bitmap = ConvertToScreenSurface(temp);
  SDL_FreeSurface(temp);
  return bitmap;

} /* ReadPbmBitmap */


/*
 * library function: ReadRgbPixmap
 * description:      create a bitmap from a ppm file (using rgb values)
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 * return value:     handle of bitmap, or NULL on failure
 */
SDL_Surface * 
ReadRgbPixmap (const char *path, const char *filename)
{

  SDL_Surface   *temp, *bitmap;

  char *tempAbsFilename, *absFilename;
  tempAbsFilename = CatPathAndFilename(path, filename);
  absFilename = AddExtension(tempAbsFilename, "ppm");

  temp = IMG_Load(absFilename);
  if(!temp) {
    fprintf(stderr, "Could not create bitmap (%s)", SDL_GetError());
    return NULL;
  };
  
  bitmap = ConvertToScreenSurface(temp);
  SDL_FreeSurface(temp);
  return bitmap;

} /* ReadRgbPixmap */


/*
 * library function: ReadCchPixmap
 * description:      create a bitmap from a ppm file (using red as bg, green as add 
 *                   and white as highlight)
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 *                   fg     - base color (black most of the time)
 *                   bg     - first color (for red pixels)
 *                   add    - seconed color (for green pixels)
 * return value:     handle of bitmap, or NULL on failure
 */
SDL_Surface *
ReadCchPixmap (const char *path, const char *filename, XBColor fg, XBColor bg, XBColor add)
{
  int            width;
  int            height;
  unsigned char *ppm;
  SDL_Surface   *bitmap;

  /* load ppm file */
  if (NULL == (ppm = ReadPpmFile (path, filename, &width, &height) ) ) {
    fprintf(stderr, "ReadPpmFile(&s, %s) failed", path, width);
    return NULL;
  }
  /* convert color */
  CchToPpm (ppm, width, height, fg, bg, add);
  /* now create bitmap */

  bitmap = BitmapFromRGBPixel(ppm, width, height);
  if(!bitmap) {
    fprintf(stderr, "Could not create bitmap (%s)", SDL_GetError());
    return NULL;
  };

  free (ppm);
  return bitmap;
} /* ReadCchPixmap */


/*
 * library function: ReadEpmPixmap
 * description:      create a bitmap from a ppm file (using red as bg, green as add 
 *                   and white as highlight)
 * parameters:       path     - relative path for image
 *                   filename - name of image file
 *                   n_colors - number of color layers
 *                   color    - arrays with colors foreach layer
 * return value:     handle of bitmap, or NULL on failure
 */
SDL_Surface *
ReadEpmPixmap (const char *path, const char *filename, int n_colors, const XBColor *color)
{
  int            width;
  int            height;
  int            depth;
  unsigned char *epm;
  unsigned char *ppm;
  SDL_Surface   *bitmap, *mask;
  char *tempAbsFilename, *absFilename;

/*   assert (NULL != color); */
  assert (NULL != path);
  assert (NULL != filename);
  /* load ppm file */
  if (NULL == (epm = ReadEpmFile (path, filename, &width, &height, &depth) ) ) {
    fprintf(stderr, "ReadEpmFile(%s, %s) failed.", path, filename);
    return NULL;
  }

  tempAbsFilename = CatPathAndFilename(path, filename);
  absFilename = AddExtension(tempAbsFilename, "pbm");

  mask  = IMG_Load(absFilename);
  SDL_SetColorKey(mask, SDL_SRCCOLORKEY, 1);
  
  /* check depth */
  if (depth < n_colors) {
    n_colors = depth;
  }
  /* create ppm array */
  ppm = malloc (width * height * 3);
  assert (ppm != NULL);
  /* convert color */
  EpmToPpm (epm, ppm, width, height, n_colors, color);

  bitmap = BitmapFromRGBPixel(ppm, width, height);

  SDL_BlitSurface(mask, NULL, bitmap, NULL);
  SDL_SetColorKey(bitmap, SDL_SRCCOLORKEY, SDL_MapRGB(bitmap->format, 0xFF, 0xFF, 0xFF));
  SDL_FreeSurface(mask);

  if(!bitmap) {
    fprintf(stderr, "Could not create bitmap (%s)", SDL_GetError());
    return NULL;
  };
  
  return bitmap;
} /* ReadEpmPixmap */



/*
 * convert colorname to value (not supported for win32)
 */
XBColor
GUI_ParseColor (const char *name)
{
  return COLOR_INVALID;
} /* GUI_ParseColor */

/*
 * end of file SDL_image.c
 */
