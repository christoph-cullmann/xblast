
/* Test out the window manager interaction functions */

#include "xblast.h"

#include "sdl_common.h"
#include "sdl_image.h"
#include "sdl_keysym.h"
#include "sdl_event.h"
#include "sdl_pixmap.h"
#include "sdl_sprite.h"
#include "sdl_text.h"
#include "sdl_tile.h"

static Uint8 video_bpp;
static Uint32 video_flags = 0;

#ifdef unused
static SDL_Surface *
LoadIconSurface (char *file, Uint8 ** maskp)
{
	SDL_Surface *icon;
	Uint8 *pixels;
	Uint8 *mask;
	int mlen, i, j;

	*maskp = NULL;

	/* Load the icon surface */
	icon = SDL_LoadBMP (file);
	if (icon == NULL) {
		fprintf (stderr, "Couldn't load %s: %s\n", file, SDL_GetError ());
		return (NULL);
	}

	/* Check width and height 
	   if ( (icon->w%8) != 0 ) {
	   fprintf(stderr, "Icon width must be a multiple of 8!\n");
	   SDL_FreeSurface(icon);
	   return(NULL);
	   }
	 */

	if (icon->format->palette == NULL) {
		fprintf (stderr, "Icon must have a palette!\n");
		SDL_FreeSurface (icon);
		return (NULL);
	}

	/* Set the colorkey */
	SDL_SetColorKey (icon, SDL_SRCCOLORKEY, *((Uint8 *) icon->pixels));

	/* Create the mask */
	pixels = (Uint8 *) icon->pixels;
	Dbg_Out ("Transparent pixel: (%d,%d,%d)\n",
			icon->format->palette->colors[*pixels].r,
			icon->format->palette->colors[*pixels].g, icon->format->palette->colors[*pixels].b);
	mlen = (icon->w * icon->h + 7) / 8;
	mask = (Uint8 *) malloc (mlen);
	if (mask == NULL) {
		fprintf (stderr, "Out of memory!\n");
		SDL_FreeSurface (icon);
		return (NULL);
	}
	memset (mask, 0, mlen);
	for (i = 0; i < icon->h; i++)
		for (j = 0; j < icon->w; j++) {
			int pindex = i * icon->pitch + j;
			int mindex = i * icon->w + j;
			if (pixels[pindex] != *pixels)
				mask[mindex >> 3] |= 1 << (7 - (mindex & 7));
		}
	*maskp = mask;
	return (icon);
}
#endif

void
SetupVideo (CFGVideoSetup * video)
{
	SDL_Surface *surface;

	surface = SDL_GetVideoSurface ();

	if (((surface->flags & SDL_FULLSCREEN) && video->mode == XBVM_Windowed) ||
		((surface->flags & SDL_FULLSCREEN) == 0 && video->mode == XBVM_Full))
		SDL_WM_ToggleFullScreen (screen);
}

static void
HotKey_ToggleFullScreen (void)
{
	SDL_Surface *screen;

	screen = SDL_GetVideoSurface ();
	if (SDL_WM_ToggleFullScreen (screen)) {
		Dbg_Out ("Toggled fullscreen mode - now %s\n",
				(screen->flags & SDL_FULLSCREEN) ? "fullscreen" : "windowed");
	}
	else {
		Dbg_Out ("Unable to toggle fullscreen mode\n");
	}
}

static void
HotKey_ToggleGrab (void)
{
	SDL_GrabMode mode;

	Dbg_Out ("Ctrl-G: toggling input grab!\n");
	mode = SDL_WM_GrabInput (SDL_GRAB_QUERY);
	if (mode == SDL_GRAB_ON) {
		Dbg_Out ("Grab was on\n");
	}
	else {
		Dbg_Out ("Grab was off\n");
	}
	mode = SDL_WM_GrabInput (mode ? SDL_GRAB_OFF : SDL_GRAB_ON);
	if (mode == SDL_GRAB_ON) {
		Dbg_Out ("Grab is now on\n");
	}
	else {
		Dbg_Out ("Grab is now off\n");
	}
}

static void
HotKey_Iconify (void)
{
	Dbg_Out ("Ctrl-Z: iconifying window!\n");
	SDL_WM_IconifyWindow ();
}

static void
HotKey_Quit (void)
{
	SDL_Event event;

	Dbg_Out ("Posting internal quit request\n");
	event.type = SDL_USEREVENT;
	SDL_PushEvent (&event);
}

static int
FilterEvents (const SDL_Event * event)
{
	switch (event->type) {

	case SDL_ACTIVEEVENT:
		/* See what happened */
		Dbg_Out ("App %s ", event->active.gain ? "gained" : "lost");
		if (event->active.state & SDL_APPACTIVE)
			Dbg_Out ("active ");
		if (event->active.state & SDL_APPMOUSEFOCUS)
			Dbg_Out ("mouse ");
		if (event->active.state & SDL_APPINPUTFOCUS)
			Dbg_Out ("input ");
		Dbg_Out ("focus\n");

		/* See if we are iconified or restored */
		if (event->active.state & SDL_APPACTIVE) {
			Dbg_Out ("App has been %s\n", event->active.gain ? "restored" : "iconified");
		}
		return (0);

		/* Pass various input events */
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_KEYUP:
		return (1);

	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_ESCAPE) {
			HotKey_Quit ();
			return(1);
		}
		if ((event->key.keysym.sym == SDLK_g) && (event->key.keysym.mod & KMOD_CTRL)) {
			HotKey_ToggleGrab ();
			return(0);
		}
		if ((event->key.keysym.sym == SDLK_z) && (event->key.keysym.mod & KMOD_CTRL)) {
			HotKey_Iconify ();
			return(0);
		}
		if ((event->key.keysym.sym == SDLK_RETURN) && (event->key.keysym.mod & KMOD_ALT)) {
			HotKey_ToggleFullScreen ();
			return(0);
		}
		return (1);

		/* Pass the video resize event through .. */
	case SDL_VIDEORESIZE:
		return (1);

		/* This is important!  Queue it if we want to quit. */
	case SDL_QUIT:
		Dbg_Out ("Quit demanded\n");
		return (1);

		/* Drop all other events */
	default:
		return (0);
	}
}

static int
SetVideoMode (int w, int h)
{
	/* screen is a global variable defined in sdl_common.c */
	screen = SDL_SetVideoMode (w, h, video_bpp, video_flags);
	if (screen == NULL) {
		Dbg_Out (stderr, "Couldn't set %dx%dx%d video mode: %s\n",
				 w, h, video_bpp, SDL_GetError ());
		return (-1);
	}

	if (!screen->flags) {
		Dbg_Out ("No hardware support.\n");
	}

	Dbg_Out ("Running in %s mode\n", screen->flags & SDL_FULLSCREEN ? "fullscreen" : "windowed");

	return (0);
}

static XBBool
InitDisplay (void)
{
	char *title;
	int parsed;
	int w, h;
	const SDL_VideoInfo *videoInfo;
	CFGVideoSetup video;

	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf (stderr, "Couldn't initialize SDL: %s\n", SDL_GetError ());
		exit (1);
	}
	atexit (SDL_Quit);

	videoInfo = SDL_GetVideoInfo ();
	Dbg_Out ("Hardware available:...............................%s\n",
			videoInfo->hw_available ? "yes" : "no");
	Dbg_Out ("Window manager_available:.........................%s\n",
			videoInfo->wm_available ? "yes" : "no");
	Dbg_Out ("Hardware to hardware blits accelerated?:..........%s\n",
			videoInfo->blit_hw ? "yes" : "no");
	Dbg_Out ("Hardware to hardware colorkey blits accelerated?:.%s\n",
			videoInfo->blit_hw_CC ? "yes" : "no");
	Dbg_Out ("Hardware to hardware alpha blits accelerated?:....%s\n",
			videoInfo->blit_hw_A ? "yes" : "no");
	Dbg_Out ("Software to hardware blits accelerated?:..........%s\n",
			videoInfo->blit_sw ? "yes" : "no");
	Dbg_Out ("Software to hardware colorkey blits accelerated?:.%s\n",
			videoInfo->blit_sw_CC ? "yes" : "no");
	Dbg_Out ("Software to hardware alpha blits accelerated?:....%s\n",
			videoInfo->blit_sw_A ? "yes" : "no");
	Dbg_Out ("Color fills accelerated?:.........................%s\n",
			videoInfo->blit_fill ? "yes" : "no");
	Dbg_Out ("Total amount of video memory in Kilobytes:........%d\n", videoInfo->video_mem);

	Dbg_Out ("Display pixel format:\n");
	Dbg_Out ("  Bits per pixel:.%d\n", videoInfo->vfmt->BitsPerPixel);
	Dbg_Out ("  Rmask:..........%08x\n", videoInfo->vfmt->Rmask);
	Dbg_Out ("  Gmask:..........%08x\n", videoInfo->vfmt->Gmask);
	Dbg_Out ("  Bmask:..........%08x\n", videoInfo->vfmt->Bmask);
	Dbg_Out ("  Amask:..........%08x\n", videoInfo->vfmt->Amask);
	Dbg_Out ("  Colorkey:.......%08x\n", videoInfo->vfmt->colorkey);
	Dbg_Out ("  Alpha:..........%08x\n", videoInfo->vfmt->alpha);

	/* Try Hardware surfaces if available. */
	video_flags |= (videoInfo->hw_available ? SDL_HWSURFACE : SDL_SWSURFACE);
	video_flags |= (videoInfo->hw_available ? SDL_DOUBLEBUF : 0);
	/* RLE encoding increases speed when blitting transparent surfaces. */
	video_flags |= SDL_RLEACCEL;
	/* Use current videomode for resolution. This is the idel format. */
	video_bpp = videoInfo->vfmt->BitsPerPixel;

	/* Check command line arguments */
	w = PIXW;
	h = PIXH + SCOREH;
	parsed = 1;

	/* Set the icon -- this must be done before the first mode set */
	/*  icon = LoadIconSurface("icon.bmp", &icon_mask);
	   if ( icon != NULL ) {
	   SDL_WM_SetIcon(icon, icon_mask);
	   }
	   if ( icon_mask != NULL )
	   free(icon_mask);
	 */
	/* Set the title bar */
	title = "XBlast TNT " VERSION_STRING;
	SDL_WM_SetCaption (title, "XBlast TNT");

	/* See if it's really set */
	SDL_WM_GetCaption (&title, NULL);

	if (RetrieveVideoSetup (&video)) {
		if (video.mode == XBVM_Full) {
			video_flags |= SDL_FULLSCREEN;
		}
	}

	/* Initialize the display */
	if (SetVideoMode (w, h) < 0) {
		return (1);
	}

	/* Set the color key. Experimental. */
	/* TODO: Test effects of SDL_SRCCOLORKEY flag */
	SDL_SetColorKey (screen, SDL_RLEACCEL, SDL_MapRGB (screen->format, 0xFF, 0xFF, 0xFF));

	/* Set an event filter. */
	SDL_SetEventFilter(FilterEvents);

	return XBTrue;
}

XBBool
GUI_Init (int argc, char *argv[])
{
	if (!InitDisplay ()) {
		return XBFalse;
	}
	/* now setup image loading */
	/* if (! InitImage () ) {
	   return XBFalse;
	   } */
	if (!InitKeysym ()) {
		return XBFalse;
	}
	if (!InitEvent ()) {
		return XBFalse;
	}
	/* now create pixmap for double bufferung */
	if (!InitPixmap ()) {
		return XBFalse;
	}
	/* now create pixmap for double bufferung */
	if (!InitSprites ()) {
		return XBFalse;
	}
	if (!InitFonts ()) {
		return XBFalse;
	}
	/* initalisize tile drawing and scoreboard */
	if (!InitTiles ()) {
		return XBFalse;
	}

	return XBTrue;
}

void
GUI_Finish (void)
{

}								/* GUI_Finish */
void
GUI_OnQuit (XBQuitFunction _quitFunc)
{

}								/* GUI_OnQuit */
