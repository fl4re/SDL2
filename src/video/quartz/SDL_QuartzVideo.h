/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/*    
    @file   SDL_QuartzVideo.h
    @author Darrell Walisser
    
    @abstract SDL video driver for MacOS X.
    
    @discussion
    
    TODO
        - Hardware Cursor support with NSCursor instead of Carbon
        - Keyboard repeat/mouse speed adjust (if needed)
        - Multiple monitor support (currently only main display)
        - Accelerated blitting support
        - Fix white OpenGL window on minimize (fixed)
        - Find out what events should be sent/ignored if window is mimimized
        - Find a way to deal with external resolution/depth switch while app is running
        - Resizeable windows
        - Check accuracy of QZ_SetGamma()
    Problems:
        - OGL not working in full screen with software renderer
        - SetColors sets palette correctly but clears framebuffer
        - Crash in CG after several mode switches (I think this has been fixed)
        - Retained windows don't draw their title bar quite right (OS Bug) (not using retained windows)
        - Cursor in 8 bit modes is screwy (might just be Radeon PCI bug)
        - Warping cursor delays mouse events for a fraction of a second,
          there is a hack around this that helps a bit
*/

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include "SDL_video.h"
#include "SDL_error.h"
#include "SDL_timer.h"
#include "SDL_syswm.h"
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"
#include "SDL_events_c.h"

/* This is a workaround to directly access NSOpenGLContext's CGL context */
/* We need to do this in order to check for errors */
@interface NSOpenGLContext (CGLContextAccess)
- (CGLContextObj) cglContext;
@end

@implementation NSOpenGLContext (CGLContextAccess)
- (CGLContextObj) cglContext;
{
    return _contextAuxiliary;
}
@end

/* Structure for rez switch gamma fades */
/* We can hide the monitor flicker by setting the gamma tables to 0 */
#define QZ_GAMMA_TABLE_SIZE 256

typedef struct {

    CGGammaValue red[QZ_GAMMA_TABLE_SIZE];
    CGGammaValue green[QZ_GAMMA_TABLE_SIZE];
    CGGammaValue blue[QZ_GAMMA_TABLE_SIZE];

} SDL_QuartzGammaTable;

/* Main driver structure to store required state information */
typedef struct SDL_PrivateVideoData {

    CGDirectDisplayID  display;            /* 0 == main display (only support single display) */
    CFDictionaryRef    mode;               /* current mode of the display */
    CFDictionaryRef    save_mode;          /* original mode of the display */
    CFArrayRef         mode_list;          /* list of available fullscreen modes */
    CGDirectPaletteRef palette;            /* palette of an 8-bit display */
    NSOpenGLContext    *gl_context;        /* object that represents an OpenGL rendering context */
    Uint32             width, height, bpp; /* frequently used data about the display */
    Uint32             flags;              /* flags for mode, for teardown purposes */
    Uint32             video_set;          /* boolean; indicates if video was set correctly */
    Uint32             warp_flag;          /* boolean; notify to event loop that a warp just occured */
    Uint32             warp_ticks;         /* timestamp when the warp occured */
    NSWindow           *window;            /* Cocoa window to implement the SDL window */
    NSQuickDrawView    *view;              /* the window's view; draw 2D into this view */
    ImageDescriptionHandle yuv_idh;
    MatrixRecordPtr        yuv_matrix;
    DecompressorComponent  yuv_codec;
    ImageSequence          yuv_seq;
    PlanarPixmapInfoYUV420 *yuv_pixmap;
    Sint16                  yuv_width, yuv_height;
    CGrafPtr                yuv_port;

} SDL_PrivateVideoData ;

#define _THIS    SDL_VideoDevice *this
#define display_id (this->hidden->display)
#define mode (this->hidden->mode)
#define save_mode (this->hidden->save_mode)
#define mode_list (this->hidden->mode_list)
#define palette (this->hidden->palette)
#define gl_context (this->hidden->gl_context)
#define device_width (this->hidden->width)
#define device_height (this->hidden->height)
#define device_bpp (this->hidden->bpp)
#define mode_flags (this->hidden->flags)
#define qz_window (this->hidden->window)
#define window_view (this->hidden->view)
#define video_set (this->hidden->video_set)
#define warp_ticks (this->hidden->warp_ticks)
#define warp_flag (this->hidden->warp_flag)
#define yuv_idh (this->hidden->yuv_idh)
#define yuv_matrix (this->hidden->yuv_matrix)
#define yuv_codec (this->hidden->yuv_codec)
#define yuv_seq (this->hidden->yuv_seq)
#define yuv_pixmap (this->hidden->yuv_pixmap)
#define yuv_data (this->hidden->yuv_data)
#define yuv_width (this->hidden->yuv_width)
#define yuv_height (this->hidden->yuv_height)
#define yuv_port (this->hidden->yuv_port)

/* Obscuring code: maximum number of windows above ours (inclusive) */
#define kMaxWindows 256

/* Some of the Core Graphics Server API for obscuring code */
#define kCGSWindowLevelTop          2147483632
#define kCGSWindowLevelDockIconDrag 500
#define kCGSWindowLevelDockMenu     101
#define kCGSWindowLevelMenuIgnore    21
#define kCGSWindowLevelMenu          20
#define kCGSWindowLevelDockLabel     12
#define kCGSWindowLevelDockIcon      11
#define kCGSWindowLevelDock          10
#define kCGSWindowLevelUtility        3
#define kCGSWindowLevelNormal         0

/* For completeness; We never use these window levels, they are always below us
#define kCGSWindowLevelMBarShadow -20
#define kCGSWindowLevelDesktopPicture -2147483647
#define kCGSWindowLevelDesktop        -2147483648
*/

typedef CGError       CGSError;
typedef long          CGSWindowCount;
typedef void *        CGSConnectionID;
typedef int           CGSWindowID;
typedef CGSWindowID*  CGSWindowIDList;
typedef CGWindowLevel CGSWindowLevel;
typedef NSRect        CGSRect;

extern CGSConnectionID _CGSDefaultConnection ();

extern CGSError CGSGetOnScreenWindowList (CGSConnectionID cid,
                                          CGSConnectionID owner,
                                          CGSWindowCount listCapacity,
                                          CGSWindowIDList list,
                                          CGSWindowCount *listCount);

extern CGSError CGSGetScreenRectForWindow (CGSConnectionID cid,
                                           CGSWindowID wid,
                                           CGSRect *rect);

extern CGWindowLevel CGSGetWindowLevel (CGSConnectionID cid,
                                        CGSWindowID wid,
                                        CGSWindowLevel *level);

extern CGSError CGSDisplayHWFill (CGDirectDisplayID id, unsigned int x, unsigned int y,
                                  unsigned int w, unsigned int h, unsigned int color);

extern CGSError CGSDisplayCanHWFill (CGDirectDisplayID id);

extern CGSError CGSGetMouseEnabledFlags (CGSConnectionID cid, CGSWindowID wid, int *flags);

/* Bootstrap functions */
static int              QZ_Available ();
static SDL_VideoDevice* QZ_CreateDevice (int device_index);
static void             QZ_DeleteDevice (SDL_VideoDevice *device);

/* Initialization, Query, Setup, and Redrawing functions */
static int          QZ_VideoInit        (_THIS, SDL_PixelFormat *video_format);

static SDL_Rect**   QZ_ListModes        (_THIS, SDL_PixelFormat *format,
                                         Uint32 flags);
static void         QZ_UnsetVideoMode   (_THIS);

static SDL_Surface* QZ_SetVideoMode     (_THIS, SDL_Surface *current,
                                         int width, int height, int bpp,
                                         Uint32 flags);
static int          QZ_ToggleFullScreen (_THIS, int on);
static int          QZ_SetColors        (_THIS, int first_color,
                                         int num_colors, SDL_Color *colors);
static void         QZ_DirectUpdate     (_THIS, int num_rects, SDL_Rect *rects);
static void         QZ_UpdateRects      (_THIS, int num_rects, SDL_Rect *rects);
static void         QZ_VideoQuit        (_THIS);

/* Hardware surface functions (for fullscreen mode only) */
static int  QZ_FillHWRect (_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color);
static int  QZ_LockHWSurface(_THIS, SDL_Surface *surface);
static void QZ_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void QZ_FreeHWSurface (_THIS, SDL_Surface *surface);
/* static int  QZ_FlipHWSurface (_THIS, SDL_Surface *surface); */

/* Gamma Functions */
static int QZ_SetGamma     (_THIS, float red, float green, float blue);
static int QZ_GetGamma     (_THIS, float *red, float *green, float *blue);
static int QZ_SetGammaRamp (_THIS, Uint16 *ramp);
static int QZ_GetGammaRamp (_THIS, Uint16 *ramp);

/* OpenGL functions */
static int    QZ_SetupOpenGL (_THIS, int bpp, Uint32 flags);
static void   QZ_TearDownOpenGL (_THIS);
static void*  QZ_GL_GetProcAddress (_THIS, const char *proc);
static int    QZ_GL_GetAttribute   (_THIS, SDL_GLattr attrib, int* value);
static int    QZ_GL_MakeCurrent    (_THIS);
static void   QZ_GL_SwapBuffers    (_THIS);
static int    QZ_GL_LoadLibrary    (_THIS, const char *location);

/* Private function to warp the cursor (used internally) */
static void  QZ_PrivateWarpCursor (_THIS, int x, int y);

/* Cursor and Mouse functions */
static void         QZ_FreeWMCursor     (_THIS, WMcursor *cursor);
static WMcursor*    QZ_CreateWMCursor   (_THIS, Uint8 *data, Uint8 *mask,
                                         int w, int h, int hot_x, int hot_y);
static int          QZ_ShowWMCursor     (_THIS, WMcursor *cursor);
static void         QZ_WarpWMCursor     (_THIS, Uint16 x, Uint16 y);
static void         QZ_MoveWMCursor     (_THIS, int x, int y);
static void         QZ_CheckMouseMode   (_THIS);

/* Event functions */
static void         QZ_InitOSKeymap     (_THIS);
static void         QZ_PumpEvents       (_THIS);

/* Window Manager functions */
static void QZ_SetCaption    (_THIS, const char *title, const char *icon);
static void QZ_SetIcon       (_THIS, SDL_Surface *icon, Uint8 *mask);
static int  QZ_IconifyWindow (_THIS);
static SDL_GrabMode QZ_GrabInput (_THIS, SDL_GrabMode grab_mode);
/*static int  QZ_GetWMInfo     (_THIS, SDL_SysWMinfo *info);*/

/* YUV functions */
static SDL_Overlay* QZ_CreateYUVOverlay (_THIS, int width, int height,
                                         Uint32 format, SDL_Surface *display);

