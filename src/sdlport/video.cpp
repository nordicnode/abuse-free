/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 2001 Anthony Kruize <trandor@labyrinth.net.au>
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 */

#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#ifdef HAVE_OPENGL
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* __APPLE__ */
#endif /* HAVE_OPENGL */

#include "common.h"

#include "filter.h"
#include "image.h"
#include "setup.h"
#include "video.h"

SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;
SDL_Texture *sdl_texture = NULL;
SDL_Surface *surface = NULL;
image *screen = NULL;
int win_xscale, win_yscale, mouse_xscale, mouse_yscale;
int xres, yres;

extern palette *lastl;
extern flags_struct flags;
#ifdef HAVE_OPENGL
GLfloat texcoord[4];
GLuint texid;
SDL_Surface *texture = NULL;
#endif

static void update_window_part(SDL_Rect *rect);

//
// power_of_two()
// Get the nearest power of two
//
static int power_of_two(int input) {
  int value;
  for (value = 1; value < input; value <<= 1)
    ;
  return value;
}

//
// set_mode()
// Set the video mode
//
void set_mode(int mode, int argc, char **argv) {
  int vidFlags = 0;

  // Calculate the window scale
  win_xscale = mouse_xscale = (flags.xres << 16) / xres;
  win_yscale = mouse_yscale = (flags.yres << 16) / yres;

  // Try using opengl hw accell
  if (flags.gl) {
#ifdef HAVE_OPENGL
    printf("Video : OpenGL enabled\n");
    // allow doublebuffering in with gl too
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, flags.doublebuf);
    // set video gl capability
    vidFlags |= SDL_WINDOW_OPENGL;
    // force no scaling, let the hw do it
    win_xscale = win_yscale = 1 << 16;
#else
    // ignore the option if not available
    printf("Video : OpenGL disabled (Support missing in executable)\n");
    flags.gl = 0;
#endif
  }

  if (flags.fullscreen)
    vidFlags |= SDL_WINDOW_FULLSCREEN;

  // Set the icon for this window.  Looks nice on taskbars etc.
  sdl_window = SDL_CreateWindow("Abuse", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, flags.xres, flags.yres,
                                vidFlags);

  if (sdl_window == NULL) {
    printf("Video : Unable to set video mode : %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Surface *icon = SDL_LoadBMP("abuse.bmp");
  if (icon) {
    SDL_SetWindowIcon(sdl_window, icon);
    SDL_FreeSurface(icon);
  }

  sdl_renderer =
      SDL_CreateRenderer(sdl_window, -1,
                         SDL_RENDERER_ACCELERATED |
                             (flags.doublebuf ? SDL_RENDERER_PRESENTVSYNC : 0));
  if (sdl_renderer == NULL) {
    printf("Video : Unable to create renderer : %s\n", SDL_GetError());
    exit(1);
  }

  // Create the screen image
  screen = new image(vec2i(xres, yres), NULL, 2);
  if (screen == NULL) {
    // Our screen image is no good, we have to bail.
    printf("Video : Unable to create screen image.\n");
    exit(1);
  }
  screen->clear();

  // Create our 8-bit surface
  surface = SDL_CreateRGBSurface(0, xres, yres, 8, 0, 0, 0, 0);
  if (surface == NULL) {
    // Our surface is no good, we have to bail.
    printf("Video : Unable to create 8-bit surface.\n");
    exit(1);
  }

  // Create a texture to render the surface
  sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING, xres, yres);

  printf("Video : %dx%d\n", xres, yres);

  // Grab and hide the mouse cursor
  SDL_ShowCursor(0);
  if (flags.grabmouse)
    SDL_SetWindowGrab(sdl_window, SDL_TRUE);

  update_dirty(screen);
}

//
// close_graphics()
// Shutdown the video mode
//
void close_graphics() {
  if (lastl)
    delete lastl;
  lastl = NULL;
  // Free our 8-bit surface
  if (surface)
    SDL_FreeSurface(surface);

  if (sdl_texture)
    SDL_DestroyTexture(sdl_texture);

  if (sdl_renderer)
    SDL_DestroyRenderer(sdl_renderer);

  if (sdl_window)
    SDL_DestroyWindow(sdl_window);

  delete screen;
}

// put_part_image()
// Draw only dirty parts of the image
//
void put_part_image(image *im, int x, int y, int x1, int y1, int x2, int y2) {
  int xe, ye;
  SDL_Rect srcrect, dstrect;
  int ii;
  Uint8 *dpixel;

  if (y > yres || x > xres)
    return;

  CHECK(x1 >= 0 && x2 >= x1 && y1 >= 0 && y2 >= y1);

  // Adjust if we are trying to draw off the screen
  if (x < 0) {
    x1 += -x;
    x = 0;
  }
  srcrect.x = x1;
  if (x + (x2 - x1) >= xres)
    xe = xres - x + x1 - 1;
  else
    xe = x2;

  if (y < 0) {
    y1 += -y;
    y = 0;
  }
  srcrect.y = y1;
  if (y + (y2 - y1) >= yres)
    ye = yres - y + y1 - 1;
  else
    ye = y2;

  if (srcrect.x >= xe || srcrect.y >= ye)
    return;

  srcrect.w = xe - srcrect.x;
  srcrect.h = ye - srcrect.y;

  // In SDL 2, we draw 1:1 to our 8-bit surface
  dpixel = ((Uint8 *)surface->pixels) + y * surface->pitch + x;

  for (ii = 0; ii < srcrect.h; ii++) {
    memcpy(dpixel, im->scan_line(srcrect.y + ii) + srcrect.x, srcrect.w);
    dpixel += surface->pitch;
  }
}

//
// load()
// Set the palette
//
void palette::load() {
  if (lastl)
    delete lastl;
  lastl = copy();

  // Force to only 256 colours.
  // Shouldn't be needed, but best to be safe.
  if (ncolors > 256)
    ncolors = 256;

  SDL_Color colors[256];
  for (int ii = 0; ii < ncolors; ii++) {
    colors[ii].r = red(ii);
    colors[ii].g = green(ii);
    colors[ii].b = blue(ii);
    colors[ii].a = 255;
  }
  SDL_SetPaletteColors(surface->format->palette, colors, 0, ncolors);

  // Now redraw the surface
  update_window_done();
}

//
// load_nice()
//
void palette::load_nice() { load(); }

// ---- support functions ----

void update_window_done() {
  // Update the texture from the surface
  // We need to convert from 8-bit palette to 32-bit RGBA for the texture
  SDL_Surface *temp_surface =
      SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
  if (temp_surface) {
    SDL_UpdateTexture(sdl_texture, NULL, temp_surface->pixels,
                      temp_surface->pitch);
    SDL_FreeSurface(temp_surface);
  }

  SDL_RenderClear(sdl_renderer);
  SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
  SDL_RenderPresent(sdl_renderer);
}

static void update_window_part(SDL_Rect *rect) {
  // Part updates are handled by update_window_done in the unified SDL 2 path
}
