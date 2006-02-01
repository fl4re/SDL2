/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* This is the XFree86 Xv extension implementation of YUV video overlays */

#include "SDL_video.h"
#include "SDL_cgxvideo.h"

#ifdef XFREE86_XV

extern SDL_Overlay *X11_CreateYUVOverlay(_THIS, int width, int height, Uint32 format, SDL_Surface *display);

extern int X11_LockYUVOverlay(_THIS, SDL_Overlay *overlay);

extern void X11_UnlockYUVOverlay(_THIS, SDL_Overlay *overlay);

extern int X11_DisplayYUVOverlay(_THIS, SDL_Overlay *overlay, SDL_Rect *dstrect);

extern void X11_FreeYUVOverlay(_THIS, SDL_Overlay *overlay);

#endif /* XFREE86_XV */
