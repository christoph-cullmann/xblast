/*
 * file x11c_init.c - initialite x11 graphics engine
 *
 * $Id: x11c_init.c,v 1.5 2005/01/04 03:02:42 iskywalker Exp $
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
#include "gui.h"

#include "x11_common.h"
#include "x11_event.h"
#include "x11c_image.h"
#include "x11c_text.h"
#include "x11c_tile.h"
#include "x11c_sprite.h"
#include "x11c_pixmap.h"
#include "x11_joystick.h"

#include "image.h"
#include "geom.h"
#include "version.h"

/*
 * local variables
 */
static XBQuitFunction quitFunc = NULL;

/*
 * local function: GetBitsPerPixel
 * description:    get memory bits for color depth
 * return value:   bits per pixel (in memroy)
 * parameters:     depth - color depth bits bits 
 */
static int
GetBitsPerPixel (int depth)
{
  XPixmapFormatValues *xpfv;
  int npfv, result;
  
  result = depth;
  if (NULL != (xpfv = XListPixmapFormats (dpy, &npfv) ) ) {
    int i;
    for (i=0; i<npfv; i++) {
      if (depth == xpfv[i].depth) {
	result = xpfv[i].bits_per_pixel;
      }
    }
    XFree (xpfv);
  }
  return result;
} /* GetBitsPerPixel */

/*
 * local function: alloc color
 * description:    allocate any given color by name (we need this to get black an white into our colormap
 * result:         colormap index of allocated color
 * parameters:     colorName - literal name of color
 *                 subst     - colormap index to use if alloc fails
 */ 
static int
AllocColor (char *colorName, int subst)
{
  XColor colorUsed, colorExact;

  /* get color from name */
  if ( (NULL == colorName) || 
       (! XParseColor (dpy, cmap, colorName, &colorExact) ) ) {
    fprintf (stderr, "unknown color %s\n", colorName ? colorName : "(null)");
    /* use substitute instead */
    return subst;
  }

  /* try to alloc color cell */
  colorUsed = colorExact;
  if (XAllocColor(dpy, cmap, &colorUsed) ) {
    return colorUsed.pixel;
  }
  /* if private colormap is in use return white */
  if (cmap != DefaultColormap(dpy, DefaultScreen(dpy) ) ) {
    return subst;
  }
  /* create private color map */
  cmap = XCopyColormapAndFree(dpy, cmap);
  XSetWindowColormap(dpy, win, cmap);
  /* alloc again */
  colorUsed = colorExact;
  if (XAllocColor(dpy, cmap, &colorUsed) ) {
    return colorUsed.pixel;
  }
  return subst;
} /* AllocColor */

/*
 * local function: InitDisplay
 * description:    Contac X11 displays and set some standard values
 * return value:   XBTrue if successful
 * parameters:     display     - string with display name
 *                 visualClass - pointer to return visual class of display
 */
static XBBool
InitDisplay (const char *display, int *visualClass)
{
  XVisualInfo visual_info;

#ifdef DEBUG
  fprintf (stderr, "Display is \"%s\"\n", XDisplayName (display) );
#endif
  /* open display */
  if ( !(dpy = XOpenDisplay(display) ) ) {
    fprintf (stderr, "could not open display %s\n", XDisplayName (display));
    return XBFalse;
  }
  /* set some information of visual */
  defDepth     = DefaultDepth(dpy, DefaultScreen(dpy));
  bitsPerPixel = GetBitsPerPixel (defDepth);
  /* set default visual */
  defVisual    = DefaultVisual(dpy, DefaultScreen(dpy) ),
  /* set colormap to default */
  cmap         = DefaultColormap(dpy, DefaultScreen(dpy));
  /* alloc black and white, so we still have them if we a use private colormap */
  whitePixel   = AllocColor ("White", WhitePixel(dpy, DefaultScreen(dpy) ) );
  blackPixel   = AllocColor ("Black", BlackPixel(dpy, DefaultScreen(dpy) ) );
  /* get visual class */
  assert (visualClass != NULL);
  *visualClass = DirectColor;
  while (! XMatchVisualInfo (dpy, DefaultScreen(dpy), defDepth, *visualClass, &visual_info) ) {
    *visualClass -= 1;
  }
  if (*visualClass < TrueColor) {
    return XBFalse;
  }
  return XBTrue;
} /* InitDisplay */

/* 
 * local function: InitWindow 
 * description:    create xblast window and icon
 * return value:   XBTrue on success
 * parameters:     winTitle - window title for window manager
 *                 iconTile - icon title for window manager
 */
static int 
InitWindow (char *winTitle, char *iconTitle)
{
  XWindowAttributes     xwa;
  XSetWindowAttributes  xswa;
  XSizeHints           *xsh;
  XClassHint           *xch;
  XEvent                xev;
  XGCValues             xgcv;
  /* Set Window Attributes */
  xswa.event_mask        = EVENT_MASK_NORMAL;
  xswa.background_pixel  = blackPixel;
  xswa.border_pixel      = blackPixel;
  xswa.override_redirect = False;
  xswa.colormap          = cmap;
  /* Open the Window */
  win = XCreateWindow(dpy, DefaultRootWindow (dpy), 0, 0, PIXW, PIXH+SCOREH, 0,
		      defDepth, InputOutput, defVisual,
		      CWEventMask | CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWColormap,
		      &xswa );

  /* Change Window and icon Title */
  XChangeProperty(dpy, win, XA_WM_NAME, XA_STRING, 8, PropModeReplace, 
		  (unsigned char *) winTitle, strlen(winTitle) );
  XChangeProperty(dpy, win, XA_WM_ICON_NAME, XA_STRING, 8, PropModeReplace, 
		  (unsigned char *) iconTitle, strlen(iconTitle) );
  /* set window class */
  if (NULL == (xch = XAllocClassHint () ) ) {
    fprintf (stderr, "alloc failed\n");
    return XBFalse;
  }
  xch->res_name  = xblastResName;
  xch->res_class = xblastResClass;

  XSetClassHint(dpy, win, xch);
  XFree(xch);
  /* set min and max geometry */
  if (NULL == (xsh = XAllocSizeHints())) {
    fprintf (stderr, "alloc failed\n");
    return XBFalse;
  }
  xsh->flags      = PPosition | PSize | PMinSize | PMaxSize;
  xsh->min_width  = PIXW;
  xsh->max_width  = PIXW;
  xsh->min_height = PIXH+SCOREH;
  xsh->max_height = PIXH+SCOREH;
  XSetWMSizeHints (dpy, win, xsh, XA_WM_NORMAL_HINTS);
  XFree(xsh);
  /* create graphics context for window */
  xgcv.foreground = blackPixel;
  xgcv.background = whitePixel;
  gcWindow        = XCreateGC (dpy, win, GCForeground | GCBackground, &xgcv);
  /* Set Cursor */
  XDefineCursor (dpy, win, XCreateFontCursor(dpy, XC_trek) );
  /* Map the Window */
  XMapRaised (dpy, win);
  /* wait for an expose event */
  do {
    XNextEvent(dpy, &xev);
  } while (xev.type != Expose );
  /* get actual window size */
  if (! XGetWindowAttributes(dpy, win , &xwa)) {
    fprintf (stderr, "could not get window size\n");
    return XBFalse;
  }
  if ( (xwa.width < PIXW) || (xwa.height < PIXH) ) {
    fprintf (stderr, "display is to small for window\n");
    return XBFalse;
  }
  return XBTrue;
} /* InitWindow */

/*
 * local function: InitIcon
 * description:    Setup icon for XBlast window
 * return value:   none
 * parameters:     none
 */
static void
SetIcon (void)
{
  XWMHints *wmh = XAllocWMHints ();
  assert (NULL != wmh);
  /* set icon pixmap and mask */
  wmh->flags       = IconPixmapHint | IconMaskHint;
  wmh->icon_pixmap = ReadRgbPixmap (imgPathMisc, "xblast");
  wmh->icon_mask   = ReadPbmBitmap (imgPathMisc, "xblast");
  XSetWMHints (dpy, win, wmh);
  /* clean up */
  XFree (wmh);
} /* InitIcon */

/*
 * global function: GUI_Init 
 * description:     Initialize the X11 graphics engine
 * return value:    XBTrue on success
 * parameters:      argc - number commandline arguments
 *                  argv - array with commandline arguments
 */
XBBool
GUI_Init (int argc, char *argv[])
{
  int visualClass;

  /* init x11 display */
  if (! InitDisplay (NULL, &visualClass)) {
    return XBFalse;
  }
  /* now initialize the window */
  if (! InitWindow ("XBlast TNT " VERSION_STRING, "XBlast TNT") ) {
    return XBFalse;
  }
  /* now setup image loading */
  if (! InitImage (visualClass) ) {
    return XBFalse;
  }
  /* create our own icon */
  SetIcon ();
  /* now create pixmap for double bufferung */
  if (! InitPixmap () ) {
    return XBFalse;
  }
  /* initalisize fonst for text output */
  if (! InitFonts ()) {
    return XBFalse;
  }
  /* initalisize tile drawing and scoreboard */
  if (! InitTiles () ) {
    return XBFalse;
  }
  /* Initialsize Sprites */
  if (! InitSprites ()) {
    return XBFalse;
  }
  /* Setup Event handler */
  if (! InitEvent () ) {
    return XBFalse;
  }
  /* init joystick support */
  if (! InitJoystick ()) {
    return XBFalse;
  }
  /* that's all */
  return XBTrue;
} /* init_display */

/*
 * global function: GUI_Finish
 * description:     Shutdown x11 graphics engine
 * return value:    none
 * parameters:      none
 */
void
GUI_Finish (void)
{
  /* finish joystick support */
  FinishJoystick ();
  /* some cleaning up */
  FinishEvent ();
  /* shutdown connection to x-server */
  if (dpy != NULL) {
    XCloseDisplay (dpy);
  }
} /* GUI_Finish */

/*
 *
 */
static int
IoErrorHandler (Display *_dpy)
{
  assert (NULL != quitFunc);

  Dbg_Out ("connection to display %s lost.\n"
	   "shutting down xblast.\n", 
	   DisplayString (_dpy));
  (*quitFunc) ();

  return 0;
} /* IoErrorHandler */

/*
 *
 */
void
GUI_OnQuit (XBQuitFunction _quitFunc) 
{
  assert (NULL != _quitFunc);

  quitFunc = _quitFunc;
  XSetIOErrorHandler (IoErrorHandler);
} /* GUI_OnQuit */

/*
 * end of file x11c_init.c
 */
