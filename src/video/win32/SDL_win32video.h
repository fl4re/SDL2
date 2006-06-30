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

#ifndef _SDL_win32video_h
#define _SDL_win32video_h

#include "../SDL_sysvideo.h"

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>

#include "SDL_win32events.h"
#include "SDL_win32keyboard.h"
#include "SDL_win32mouse.h"
#include "SDL_win32window.h"

#ifdef UNICODE
#define WIN_StringToUTF8(S) SDL_iconv_string("UTF-8", "UCS-2", (char *)S, (wcslen(S)+1)*sizeof(WCHAR))
#define WIN_UTF8ToString(S) (WCHAR *)SDL_iconv_string("UCS-2", "UTF-8", (char *)S, SDL_strlen(S)+1)
#else
#define WIN_StringToUTF8(S) SDL_iconv_string("UTF-8", "ASCII", (char *)S, (strlen(S)+1))
#define WIN_UTF8ToString(S) SDL_iconv_string("ASCII", "UTF-8", (char *)S, SDL_strlen(S)+1)
#endif

/* Private display data */

typedef struct
{
    int mouse;
    int keyboard;
} SDL_VideoData;

#endif /* _SDL_win32video_h */

/* vi: set ts=4 sw=4 expandtab: */
