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
#include "SDL_config.h"

#ifndef __SDL_PH_MOUSE_H__
#define __SDL_PH_MOUSE_H__

#include "SDL_ph_video.h"

/* Functions to be exported */
extern void ph_FreeWMCursor (_THIS, WMcursor * cursor);
extern WMcursor *ph_CreateWMCursor (_THIS,
                                    Uint8 * data, Uint8 * mask, int w, int h,
                                    int hot_x, int hot_y);
extern PhCursorDef_t ph_GetWMPhCursor (WMcursor * cursor);
extern int ph_ShowWMCursor (_THIS, WMcursor * cursor);
extern void ph_WarpWMCursor (_THIS, Uint16 x, Uint16 y);
extern void ph_CheckMouseMode (_THIS);
extern void ph_UpdateMouse (_THIS);

#endif /* __SDL_PH_MOUSE_H__ */
/* vi: set ts=4 sw=4 expandtab: */
