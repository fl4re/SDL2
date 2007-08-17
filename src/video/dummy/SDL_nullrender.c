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

#include "SDL_video.h"
#include "../SDL_sysvideo.h"
#include "../SDL_yuv_sw_c.h"
#include "../SDL_renderer_sw.h"
#include "../SDL_rendercopy.h"


/* SDL surface based renderer implementation */

static SDL_Renderer *SDL_DUMMY_CreateRenderer(SDL_Window * window,
                                              Uint32 flags);
static int SDL_DUMMY_RenderFill(SDL_Renderer * renderer, Uint8 r, Uint8 g,
                                Uint8 b, Uint8 a, const SDL_Rect * rect);
static int SDL_DUMMY_RenderCopy(SDL_Renderer * renderer,
                                SDL_Texture * texture,
                                const SDL_Rect * srcrect,
                                const SDL_Rect * dstrect);
static void SDL_DUMMY_RenderPresent(SDL_Renderer * renderer);
static void SDL_DUMMY_DestroyRenderer(SDL_Renderer * renderer);


SDL_RenderDriver SDL_DUMMY_RenderDriver = {
    SDL_DUMMY_CreateRenderer,
    {
     "dummy",
     (SDL_RENDERER_SINGLEBUFFER | SDL_RENDERER_PRESENTCOPY |
      SDL_RENDERER_PRESENTFLIP2 | SDL_RENDERER_PRESENTFLIP3 |
      SDL_RENDERER_PRESENTDISCARD),
     }
};

typedef struct
{
    int current_screen;
    SDL_Surface *screens[3];
} SDL_DUMMY_RenderData;

SDL_Renderer *
SDL_DUMMY_CreateRenderer(SDL_Window * window, Uint32 flags)
{
    SDL_VideoDisplay *display = SDL_GetDisplayFromWindow(window);
    SDL_DisplayMode *displayMode = &display->current_mode;
    SDL_Renderer *renderer;
    SDL_DUMMY_RenderData *data;
    int i, n;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    if (!SDL_PixelFormatEnumToMasks
        (displayMode->format, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
        SDL_SetError("Unknown display format");
        return NULL;
    }

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (SDL_DUMMY_RenderData *) SDL_malloc(sizeof(*data));
    if (!data) {
        SDL_DUMMY_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }
    SDL_zerop(data);

    renderer->RenderFill = SDL_DUMMY_RenderFill;
    renderer->RenderCopy = SDL_DUMMY_RenderCopy;
    renderer->RenderPresent = SDL_DUMMY_RenderPresent;
    renderer->DestroyRenderer = SDL_DUMMY_DestroyRenderer;
    renderer->info.name = SDL_DUMMY_RenderDriver.info.name;
    renderer->info.flags = 0;
    renderer->window = window->id;
    renderer->driverdata = data;
    Setup_SoftwareRenderer(renderer);

    if (flags & SDL_RENDERER_PRESENTFLIP2) {
        renderer->info.flags |= SDL_RENDERER_PRESENTFLIP2;
        n = 2;
    } else if (flags & SDL_RENDERER_PRESENTFLIP3) {
        renderer->info.flags |= SDL_RENDERER_PRESENTFLIP3;
        n = 3;
    } else {
        renderer->info.flags |= SDL_RENDERER_PRESENTCOPY;
        n = 1;
    }
    for (i = 0; i < n; ++i) {
        data->screens[i] =
            SDL_CreateRGBSurface(0, window->w, window->h, bpp, Rmask, Gmask,
                                 Bmask, Amask);
        if (!data->screens[i]) {
            SDL_DUMMY_DestroyRenderer(renderer);
            return NULL;
        }
        SDL_SetSurfacePalette(data->screens[i], display->palette);
    }
    data->current_screen = 0;

    return renderer;
}

static int
SDL_DUMMY_RenderFill(SDL_Renderer * renderer, Uint8 r, Uint8 g, Uint8 b,
                     Uint8 a, const SDL_Rect * rect)
{
    SDL_DUMMY_RenderData *data =
        (SDL_DUMMY_RenderData *) renderer->driverdata;
    SDL_Surface *target = data->screens[data->current_screen];
    Uint32 color;
    SDL_Rect real_rect = *rect;

    color = SDL_MapRGBA(target->format, r, g, b, a);

    return SDL_FillRect(target, &real_rect, color);
}

static int
SDL_DUMMY_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                     const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{
    SDL_DUMMY_RenderData *data =
        (SDL_DUMMY_RenderData *) renderer->driverdata;
    SDL_Window *window = SDL_GetWindowFromID(renderer->window);
    SDL_VideoDisplay *display = SDL_GetDisplayFromWindow(window);

    if (SDL_ISPIXELFORMAT_FOURCC(texture->format)) {
        SDL_Surface *target = data->screens[data->current_screen];
        void *pixels =
            (Uint8 *) target->pixels + dstrect->y * target->pitch +
            dstrect->x * target->format->BytesPerPixel;
        return SDL_SW_CopyYUVToRGB((SDL_SW_YUVTexture *) texture->driverdata,
                                   srcrect, display->current_mode.format,
                                   dstrect->w, dstrect->h, pixels,
                                   target->pitch);
    } else {
        SDL_Surface *surface = (SDL_Surface *) texture->driverdata;
        SDL_Surface *target = data->screens[data->current_screen];
        SDL_RenderCopyFunc copyfunc = (SDL_RenderCopyFunc) surface->userdata;

        if (copyfunc) {
            SDL_RenderCopyData copydata;

            copydata.src =
                (Uint8 *) surface->pixels + srcrect->y * surface->pitch +
                srcrect->x * surface->format->BytesPerPixel;
            copydata.src_w = srcrect->w;
            copydata.src_h = srcrect->h;
            copydata.src_pitch = surface->pitch;
            copydata.dst =
                (Uint8 *) target->pixels + dstrect->y * target->pitch +
                dstrect->x * target->format->BytesPerPixel;
            copydata.dst_w = dstrect->w;
            copydata.dst_h = dstrect->h;
            copydata.dst_pitch = target->pitch;
            copydata.flags = 0;
            if (texture->modMode & SDL_TEXTUREMODULATE_COLOR) {
                copydata.flags |= SDL_RENDERCOPY_MODULATE_COLOR;
                copydata.r = texture->r;
                copydata.g = texture->g;
                copydata.b = texture->b;
            }
            if (texture->modMode & SDL_TEXTUREMODULATE_ALPHA) {
                copydata.flags |= SDL_RENDERCOPY_MODULATE_ALPHA;
                copydata.a = texture->a;
            }
            if (texture->blendMode & SDL_TEXTUREBLENDMODE_MASK) {
                copydata.flags |= SDL_RENDERCOPY_MASK;
            } else if (texture->blendMode & SDL_TEXTUREBLENDMODE_BLEND) {
                copydata.flags |= SDL_RENDERCOPY_BLEND;
            } else if (texture->blendMode & SDL_TEXTUREBLENDMODE_ADD) {
                copydata.flags |= SDL_RENDERCOPY_ADD;
            } else if (texture->blendMode & SDL_TEXTUREBLENDMODE_MOD) {
                copydata.flags |= SDL_RENDERCOPY_MOD;
            }
            if (texture->scaleMode) {
                copydata.flags |= SDL_RENDERCOPY_NEAREST;
            }
            copyfunc(&copydata);
            return 0;
        } else {
            SDL_Rect real_srcrect = *srcrect;
            SDL_Rect real_dstrect = *dstrect;

            return SDL_LowerBlit(surface, &real_srcrect, target,
                                 &real_dstrect);
        }
    }
}

static void
SDL_DUMMY_RenderPresent(SDL_Renderer * renderer)
{
    static int frame_number;
    SDL_DUMMY_RenderData *data =
        (SDL_DUMMY_RenderData *) renderer->driverdata;

    /* Send the data to the display */
    if (SDL_getenv("SDL_VIDEO_DUMMY_SAVE_FRAMES")) {
        char file[128];
        SDL_snprintf(file, sizeof(file), "SDL_window%d-%8.8d.bmp",
                     renderer->window, ++frame_number);
        SDL_SaveBMP(data->screens[data->current_screen], file);
    }

    /* Update the flipping chain, if any */
    if (renderer->info.flags & SDL_RENDERER_PRESENTFLIP2) {
        data->current_screen = (data->current_screen + 1) % 2;
    } else if (renderer->info.flags & SDL_RENDERER_PRESENTFLIP3) {
        data->current_screen = (data->current_screen + 1) % 3;
    }
}

static void
SDL_DUMMY_DestroyRenderer(SDL_Renderer * renderer)
{
    SDL_DUMMY_RenderData *data =
        (SDL_DUMMY_RenderData *) renderer->driverdata;
    int i;

    if (data) {
        for (i = 0; i < SDL_arraysize(data->screens); ++i) {
            if (data->screens[i]) {
                SDL_FreeSurface(data->screens[i]);
            }
        }
        SDL_free(data);
    }
    SDL_free(renderer);
}

/* vi: set ts=4 sw=4 expandtab: */
