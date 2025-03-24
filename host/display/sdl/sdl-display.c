/* host/sdl/sdl-display.c - SDL2 support: */

/*
 * Copyright (c) 2022 Ruben Agin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Matt Fredette.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tme/common.h>

/* includes: */
#include "display.h"
#ifdef main
#include <SDL.h>
#else
#include <SDL3/SDL.h>
#endif
#include <signal.h>
#include <stdlib.h>
#ifdef HAVE_X11_KEYSYM_H
#include <X11/keysym.h>
#elif HAVE_RFB_KEYSYM_H
#include <rfb/keysym.h>
#elif !defined(_TME_HAVE_GTK)
#error "No keysym header file on system."
#endif
#ifdef _TME_HAVE_GTK
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#endif

static int _tme_sdl_keymods[] = {
#ifdef main
  KMOD_SHIFT,
  KMOD_NUM | KMOD_CAPS,
  KMOD_CTRL,
  KMOD_ALT,
  KMOD_GUI,
#else
  SDL_KMOD_SHIFT,
  SDL_KMOD_NUM | SDL_KMOD_CAPS,
  SDL_KMOD_CTRL,
  SDL_KMOD_ALT,
  SDL_KMOD_GUI,
#endif  
  0,
  0,
  0
};

static struct { char mask; int bits_stored; } utf8Mapping[]= {
        {0b00111111, 6},
        {0b01111111, 7},
        {0b00011111, 5},
        {0b00001111, 4},
        {0b00000111, 3},
        {0,0}
};

/* TODO: odd maxx doesn't work (vncviewer bug) */

static int enableResizable = 0, viewOnly, listenLoop;
struct tme_sdl_screen {
  /* the generic screen structure */
  struct tme_screen screen;
  int sdlFlags;
  SDL_Surface* sdl;
  SDL_Texture *sdlTexture;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;
};

/* client's pointer position */
static int x,y;
static int rightAltKeyDown, leftAltKeyDown;

static void
_tme_sdl_display_bell(struct tme_display *display) {
  tme_beep();
}

/* switch to new framebuffer contents */
static int _tme_sdl_screen_resize(struct tme_sdl_screen *screen)
{
  unsigned char *oldfb, *newfb;
  struct tme_fb_connection *conn_fb = screen->screen.tme_screen_fb;
  int width = conn_fb->tme_fb_connection_width;
  int height = conn_fb->tme_fb_connection_height;
  int depth = SDL_BITSPERPIXEL(SDL_PIXELFORMAT_RGBA32);
  float scaleX, scaleY;
  struct tme_display *display;
#ifdef main
  SDL_PixelFormat *format;
#else
  SDL_PixelFormatDetails *format;
#endif
  
  if (enableResizable)
    screen->sdlFlags |= SDL_WINDOW_RESIZABLE;
#ifdef main
  else
    screen->sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
  /* (re)create the surface used as the client's framebuffer */
  if(screen->sdl)
#ifdef main
    SDL_FreeSurface(screen->sdl);
  screen->sdl = SDL_CreateRGBSurface(0,
				     width,
				     height,
				     depth,
				     0,0,0,0);
#else
  SDL_DestroySurface(screen->sdl);
  screen->sdl=SDL_CreateSurface(width,
				height,
				SDL_PIXELFORMAT_ARGB8888);
#endif  
  
  /* get the display: */
  display = screen->screen.tme_screen_display;
  if(!screen->sdl)
        tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("resize: error creating surface: %s\n"), SDL_GetError()));

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_width = screen->sdl->pitch / (depth / 8);
  conn_fb->tme_fb_connection_buffer = screen->sdl->pixels;
  conn_fb->tme_fb_connection_buffsz = screen->sdl->pitch * height;   
  conn_fb->tme_fb_connection_skipx = 0;
  conn_fb->tme_fb_connection_scanline_pad = _tme_scanline_pad(screen->sdl->pitch);
  conn_fb->tme_fb_connection_order = TME_ENDIAN_LITTLE;
  conn_fb->tme_fb_connection_bits_per_pixel = depth;
  conn_fb->tme_fb_connection_depth = 24;
  conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
#ifdef main
  format = screen->sdl->format;
#else
  format = SDL_GetPixelFormatDetails(screen->sdl->format);
#endif
  conn_fb->tme_fb_connection_mask_g = format->Gmask;
  conn_fb->tme_fb_connection_mask_b = format->Bmask;
  conn_fb->tme_fb_connection_mask_r = format->Rmask;
  /* create or resize the window */
  if(!screen->sdlWindow) {
    screen->sdlWindow = SDL_CreateWindow(display->tme_display_title,
#ifdef main
					 SDL_WINDOWPOS_UNDEFINED,
					 SDL_WINDOWPOS_UNDEFINED,
#endif
					 width,
					 height,
					 screen->sdlFlags);
    if(!screen->sdlWindow)
          tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("resize: error creating window: %s\n"), SDL_GetError()));
  } else {
    SDL_SetWindowSize(screen->sdlWindow, width, height);
  }
  /* create the renderer if it does not already exist */
  if(!screen->sdlRenderer) {
    screen->sdlRenderer = SDL_CreateRenderer(screen->sdlWindow, NULL);
    if(!screen->sdlRenderer)
          tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("resize: error creating renderer: %s\n"), SDL_GetError()));
#ifdef main
    SDL_SetRelativeMouseMode(true);
#else
    SDL_SetWindowRelativeMouseMode(screen->sdlWindow, true);
#endif
  }
#ifdef main
  SDL_RenderSetLogicalSize(screen->sdlRenderer, width, height);  /* this is a departure from the SDL1.2-based version, but more in the sense of a VNC viewer in keeeping aspect ratio */
  SDL_RenderGetScale(screen->sdlRenderer, &scaleX, &scaleY);
#else
  SDL_SetRenderLogicalPresentation(screen->sdlRenderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);  /* this is a departure from the SDL1.2-based version, but more in the sense of a VNC viewer in keeeping aspect ratio */
  SDL_GetRenderScale(screen->sdlRenderer, &scaleX, &scaleY);
#endif
  
  tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	  (&display->tme_display_element->tme_element_log_handle,
	   _("resize: renderer scale: %d %d\n"), width, height));
  
  /* (re)create the texture that sits in between the surface->pixels and the renderer */
  if(screen->sdlTexture)
    SDL_DestroyTexture(screen->sdlTexture);
  screen->sdlTexture = SDL_CreateTexture(screen->sdlRenderer,
					 SDL_PIXELFORMAT_ARGB8888,
					 SDL_TEXTUREACCESS_STREAMING,
					 width, height);
  if(!screen->sdlTexture)
        tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("resize: error creating texture: %s\n"), SDL_GetError()));
  return TRUE;
}

static tme_keyboard_keyval_t sdl_to_tme_keysym(SDL_Keycode sym) {
  tme_keyboard_keyval_t k = 0;

  switch (sym) {
  case SDLK_BACKSPACE: k = XK_BackSpace; break;
  case SDLK_TAB: k = XK_Tab; break;
  case SDLK_CLEAR: k = XK_Clear; break;
  case SDLK_RETURN: k = XK_Return; break;
  case SDLK_PAUSE: k = XK_Pause; break;
  case SDLK_ESCAPE: k = XK_Escape; break;
  case SDLK_DELETE: k = XK_Delete; break;
  case SDLK_KP_0: k = XK_KP_0; break;
  case SDLK_KP_1: k = XK_KP_1; break;
  case SDLK_KP_2: k = XK_KP_2; break;
  case SDLK_KP_3: k = XK_KP_3; break;
  case SDLK_KP_4: k = XK_KP_4; break;
  case SDLK_KP_5: k = XK_KP_5; break;
  case SDLK_KP_6: k = XK_KP_6; break;
  case SDLK_KP_7: k = XK_KP_7; break;
  case SDLK_KP_8: k = XK_KP_8; break;
  case SDLK_KP_9: k = XK_KP_9; break;
  case SDLK_KP_PERIOD: k = XK_KP_Decimal; break;
  case SDLK_KP_DIVIDE: k = XK_KP_Divide; break;
  case SDLK_KP_MULTIPLY: k = XK_KP_Multiply; break;
  case SDLK_KP_MINUS: k = XK_KP_Subtract; break;
  case SDLK_KP_PLUS: k = XK_KP_Add; break;
  case SDLK_KP_ENTER: k = XK_KP_Enter; break;
  case SDLK_KP_EQUALS: k = XK_KP_Equal; break;
  case SDLK_UP: k = XK_Up; break;
  case SDLK_DOWN: k = XK_Down; break;
  case SDLK_RIGHT: k = XK_Right; break;
  case SDLK_LEFT: k = XK_Left; break;
  case SDLK_INSERT: k = XK_Insert; break;
  case SDLK_HOME: k = XK_Home; break;
  case SDLK_END: k = XK_End; break;
  case SDLK_PAGEUP: k = XK_Page_Up; break;
  case SDLK_PAGEDOWN: k = XK_Page_Down; break;
  case SDLK_F1: k = XK_F1; break;
  case SDLK_F2: k = XK_F2; break;
  case SDLK_F3: k = XK_F3; break;
  case SDLK_F4: k = XK_F4; break;
  case SDLK_F5: k = XK_F5; break;
  case SDLK_F6: k = XK_F6; break;
  case SDLK_F7: k = XK_F7; break;
  case SDLK_F8: k = XK_F8; break;
  case SDLK_F9: k = XK_F9; break;
  case SDLK_F10: k = XK_F10; break;
  case SDLK_F11: k = XK_F11; break;
  case SDLK_F12: k = XK_F12; break;
  case SDLK_F13: k = XK_F13; break;
  case SDLK_F14: k = XK_F14; break;
  case SDLK_F15: k = XK_F15; break;
  case SDLK_NUMLOCKCLEAR: k = XK_Num_Lock; break;
  case SDLK_CAPSLOCK: k = XK_Caps_Lock; break;
  case SDLK_SCROLLLOCK: k = XK_Scroll_Lock; break;
  case SDLK_RSHIFT: k = XK_Shift_R; break;
  case SDLK_LSHIFT: k = XK_Shift_L; break;
  case SDLK_RCTRL: k = XK_Control_R; break;
  case SDLK_LCTRL: k = XK_Control_L; break;
  case SDLK_RALT: k = XK_Alt_R; break;
  case SDLK_LALT: k = XK_Alt_L; break;
  case SDLK_LGUI: k = XK_Super_L; break;
  case SDLK_RGUI: k = XK_Super_R; break;
#if 0
  case SDLK_COMPOSE: k = XK_Compose; break;
#endif
  case SDLK_MODE: k = XK_Mode_switch; break;
  case SDLK_HELP: k = XK_Help; break;
  case SDLK_PRINTSCREEN: k = XK_Print; break;
  case SDLK_SYSREQ: k = XK_Sys_Req; break;
  default: break;
  }
  /* SDL_TEXTINPUT does not generate characters if ctrl is down, so handle those here */
  if (k == 0 && sym > 0x0 && sym < 0x100) // && s->mod & KMOD_CTRL)
    k = sym;
  return k;
}

#ifndef _TME_HAVE_GTK
static SDL_Keycode tme_to_sdl_keysym(tme_keyboard_keyval_t sym) {
  SDL_Keycode k = 0;

  switch (sym) {
  case XK_BackSpace: k = SDLK_BACKSPACE; break;
  case XK_Tab: k = SDLK_TAB; break;
  case XK_Clear: k = SDLK_CLEAR; break;
  case XK_Return: k = SDLK_RETURN; break;
  case XK_Pause: k = SDLK_PAUSE; break;
  case XK_Escape: k = SDLK_ESCAPE; break;
  case XK_Delete: k = SDLK_DELETE; break;
  case XK_KP_0: k = SDLK_KP_0; break;
  case XK_KP_1: k = SDLK_KP_1; break;
  case XK_KP_2: k = SDLK_KP_2; break;
  case XK_KP_3: k = SDLK_KP_3; break;
  case XK_KP_4: k = SDLK_KP_4; break;
  case XK_KP_5: k = SDLK_KP_5; break;
  case XK_KP_6: k = SDLK_KP_6; break;
  case XK_KP_7: k = SDLK_KP_7; break;
  case XK_KP_8: k = SDLK_KP_8; break;
  case XK_KP_9: k = SDLK_KP_9; break;
  case XK_KP_Decimal: k = SDLK_KP_PERIOD; break;
  case XK_KP_Divide: k = SDLK_KP_DIVIDE; break;
  case XK_KP_Multiply: k = SDLK_KP_MULTIPLY; break;
  case XK_KP_Subtract: k = SDLK_KP_MINUS; break;
  case XK_KP_Add: k = SDLK_KP_PLUS; break;
  case XK_KP_Enter: k = SDLK_KP_ENTER; break;
  case XK_KP_Equal: k = SDLK_KP_EQUALS; break;
  case XK_Up: k = SDLK_UP; break;
  case XK_Down: k = SDLK_DOWN; break;
  case XK_Right: k = SDLK_RIGHT; break;
  case XK_Left: k = SDLK_LEFT; break;
  case XK_Insert: k = SDLK_INSERT; break;
  case XK_Home: k = SDLK_HOME; break;
  case XK_End: k = SDLK_END; break;
  case XK_Page_Up: k = SDLK_PAGEUP; break;
  case XK_Page_Down: k = SDLK_PAGEDOWN; break;
  case XK_F1: k = SDLK_F1; break;
  case XK_F2: k = SDLK_F2; break;
  case XK_F3: k = SDLK_F3; break;
  case XK_F4: k = SDLK_F4; break;
  case XK_F5: k = SDLK_F5; break;
  case XK_F6: k = SDLK_F6; break;
  case XK_F7: k = SDLK_F7; break;
  case XK_F8: k = SDLK_F8; break;
  case XK_F9: k = SDLK_F9; break;
  case XK_F10: k = SDLK_F10; break;
  case XK_F11: k = SDLK_F11; break;
  case XK_F12: k = SDLK_F12; break;
  case XK_F13: k = SDLK_F13; break;
  case XK_F14: k = SDLK_F14; break;
  case XK_F15: k = SDLK_F15; break;
  case XK_Num_Lock: k = SDLK_NUMLOCKCLEAR; break;
  case XK_Caps_Lock: k = SDLK_CAPSLOCK; break;
  case XK_Scroll_Lock: k = SDLK_SCROLLLOCK; break;
  case XK_Shift_R: k = SDLK_RSHIFT; break;
  case XK_Shift_L: k = SDLK_LSHIFT; break;
  case XK_Control_R: k = SDLK_RCTRL; break;
  case XK_Control_L: k = SDLK_LCTRL; break;
  case XK_Alt_R: k = SDLK_RALT; break;
  case XK_Alt_L: k = SDLK_LALT; break;
  case XK_Super_L: k = SDLK_LGUI; break;
  case XK_Super_R: k = SDLK_RGUI; break;
#if 0
  case XK_Compose: k = SDLK_COMPOSE; break;
#endif
  case XK_Mode_switch: k = SDLK_MODE; break;
  case XK_Help: k = SDLK_HELP; break;
  case XK_Print: k = SDLK_PRINTSCREEN; break;
  case XK_Sys_Req: k = SDLK_SYSREQ; break;
  default: break;
  }
  /* SDL_TEXTINPUT does not generate characters if ctrl is down, so handle those here */
  if (k == 0 && sym > 0x0 && sym < 0x100) // && s->mod & KMOD_CTRL)
    k = sym;
  return k;
}

/* UTF-8 decoding is from https://rosettacode.org/wiki/UTF-8_encode_and_decode which is under GFDL 1.2 */
static tme_keyboard_keyval_t utf8char2rfbKeySym(const char chr[4]) {
  int bytes = strlen(chr);
  int shift = utf8Mapping[0].bits_stored * (bytes - 1);
  tme_keyboard_keyval_t codep = (*chr++ & utf8Mapping[bytes].mask) << shift;
  int i;
  for(i = 1; i < bytes; ++i, ++chr) {
    shift -= utf8Mapping[0].bits_stored;
    codep |= ((char)*chr & utf8Mapping[0].mask) << shift;
  }
  return codep;
}

static char *_tme_sdl_keyval_name(tme_keyboard_keyval_t sym) {
  return SDL_GetKeyName(tme_to_sdl_keysym(sym));
}

static tme_keyboard_keyval_t _tme_sdl_keyval_from_name(const char *name) {
  return sdl_to_tme_keysym(SDL_GetKeyFromName(name));
}
#endif

static void
_tme_sdl_screen_redraw(struct tme_sdl_screen *screen, int x, int y, int w, int h)
{
  SDL_Surface *sdl = screen->sdl;
  /* update texture from surface->pixels */
  SDL_Rect r = {x,y,w,h};
  struct tme_display *display = screen->screen.tme_screen_display;
  
  if(SDL_UpdateTexture(screen->sdlTexture, &r, sdl->pixels + y*sdl->pitch + x*4, sdl->pitch)
#ifdef main
     < 0
#else
     == false
#endif
     )
    tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("update: failed to update texture: %s\n"), SDL_GetError()));
  /* copy texture to renderer and show */
  if(SDL_RenderClear(screen->sdlRenderer)
#ifdef main
     < 0
#else
     == false
#endif
     )
    tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("update: failed to clear renderer: %s\n"), SDL_GetError()));
#ifdef main
  if(SDL_RenderCopy(screen->sdlRenderer, screen->sdlTexture, NULL, NULL) < 0)
#else
  if(!SDL_RenderTexture(screen->sdlRenderer, screen->sdlTexture, NULL, NULL))
#endif
    tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("update: failed to copy texture to renderer: %s\n"), SDL_GetError()));
  SDL_RenderPresent(screen->sdlRenderer);
}
static int
_tme_sdl_display_update(struct tme_display *display) {
  SDL_Event e;
  int keydown = 0;
  if(SDL_PollEvent(&e)) {

    switch(e.type) {
#ifdef main
    case SDL_WINDOWEVENT:
      switch (e.window.event) {
      case SDL_WINDOWEVENT_FOCUS_LOST:
#else
      case SDL_EVENT_WINDOW_FOCUS_LOST:
#endif
      
	if (rightAltKeyDown) {
	  _tme_keyboard_key_event(FALSE, SDLK_RALT, display);
	  rightAltKeyDown = FALSE;
	  tme_log(&display->tme_display_element->tme_element_log_handle, 10, TME_OK,
		  (&display->tme_display_element->tme_element_log_handle,
		   _("released right Alt key\n")));
	}
	if (leftAltKeyDown) {
	  _tme_keyboard_key_event(FALSE, SDLK_LALT, display);
	  leftAltKeyDown = FALSE;
	  tme_log(&display->tme_display_element->tme_element_log_handle, 10, TME_OK,
		  (&display->tme_display_element->tme_element_log_handle,
		   _("released left Alt key\n")));
	}
	break;
#ifdef main
      }
      break;
#endif
#ifdef main
    case SDL_MOUSEWHEEL:
#else
    case SDL_EVENT_MOUSE_WHEEL:
#endif      
      {
	int steps;
	if (viewOnly)
	  break;
	if(e.wheel.y > 0)
	  for(steps = 0; steps < e.wheel.y; ++steps) {
	    _tme_mouse_event(4, x, y, display);
	    _tme_mouse_event(-4, x, y, display);
	  }
	if(e.wheel.y < 0)
	  for(steps = 0; steps > e.wheel.y; --steps) {
	    _tme_mouse_event(5, x, y, display);
	    _tme_mouse_event(-5, x, y, display);
	  }
	if(e.wheel.x > 0)
	  for(steps = 0; steps < e.wheel.x; ++steps) {
	    _tme_mouse_event(7, x, y, display);
	    _tme_mouse_event(-7, x, y, display);
	  }
	if(e.wheel.x < 0)
	  for(steps = 0; steps > e.wheel.x; --steps) {
	    _tme_mouse_event(6, x, y, display);
	    _tme_mouse_event(-6, x, y, display);
	  }
	break;
      }
#ifdef main
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEMOTION:
#else
    case SDL_EVENT_MOUSE_BUTTON_UP:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_MOTION:
#endif
      {
	int button, i;
	if (viewOnly)
	  break;
#ifdef main
	if (e.type == SDL_MOUSEMOTION)
#else
	if (e.type == SDL_EVENT_MOUSE_MOTION)
#endif
	  {
	  x = e.motion.xrel;
	  y = e.motion.yrel;
	  button = 0; // e.motion.state;
	}
	else {
	  x = 0; // e.button.x;
	  y = 0; // e.button.y;
	  button = e.button.button;
#ifdef main
	  if (e.type == SDL_MOUSEBUTTONUP)
#else
	  if (e.type == SDL_EVENT_MOUSE_BUTTON_UP)
#endif
	    button = -button;
	}
	_tme_mouse_event(button, x, y, display);
	break;
      }
#ifdef main
    case SDL_KEYUP:
      keydown = 1;
    case SDL_KEYDOWN:
#else
    case SDL_EVENT_KEY_DOWN:
      keydown = 1;
    case SDL_EVENT_KEY_UP:
#endif
      if (viewOnly)
	break;
#ifdef main
      SDL_Keycode sym = e.key.keysym.sym;
      SDL_Keymod mod = e.key.keysym.mod;
#else
      SDL_Keycode sym = e.key.key;
      SDL_Keymod mod = e.key.mod;
#endif
      _tme_keyboard_key_event(keydown | mod<<1,
			      sdl_to_tme_keysym(sym), display);
      if (sym == SDLK_RALT)
	rightAltKeyDown = keydown;
      if (sym == SDLK_LALT)
	leftAltKeyDown = keydown;
      break;
#ifdef main
    case SDL_TEXTINPUT:
#else
    case SDL_EVENT_TEXT_INPUT:
#endif
#if 0
      if (viewOnly)
	break;
      tme_keyboard_keyval_t sym = utf8char2rfbKeySym(e.text.text);
      _tme_keyboard_key_event(TRUE, sym, display);
      _tme_keyboard_key_event(FALSE, sym, display);
#endif
      break;
    default:
      tme_log(&display->tme_display_element->tme_element_log_handle, 10, TME_OK,
	      (&display->tme_display_element->tme_element_log_handle,
	       _("ignore SDL event: 0x%x\n"), e.type));
    }
  }
  return TRUE;
}

/* the new SDL display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_sdl,display) {
  struct tme_display *display;
  int arg_i = 0;

  for(arg_i++;args[arg_i];arg_i+=2)
    SDL_SetHint(args[arg_i], args[arg_i+1]);
  
  /* start our data structure: */
  display = tme_new0(struct tme_display, 1);
#ifdef _TME_HAVE_GTK
  display->tme_display_keyval_name = gdk_keyval_name;
  display->tme_display_keyval_from_name = gdk_keyval_from_name;
  display->tme_display_keyval_convert_case = gdk_keyval_convert_case;
  display->tme_display_key_void_symbol = GDK_KEY_VoidSymbol;
#else
  display->tme_display_keyval_name = _tme_sdl_keyval_name;
  display->tme_display_keyval_from_name = _tme_sdl_keyval_from_name;
  display->tme_display_key_void_symbol = SDLK_UNKNOWN;
#endif
  display->tme_display_keymods = _tme_sdl_keymods;
  tme_display_init(element, display);

  /* recover our data structure: */
  display = element->tme_element_private;

  SDL_Init(SDL_INIT_VIDEO
#ifdef main
	   | SDL_INIT_NOPARACHUTE
#endif
	   );
  atexit(SDL_Quit);
  signal(SIGINT, exit);

  /* set the display-specific functions: */
  display->tme_display_bell = _tme_sdl_display_bell;
  display->tme_display_update = _tme_sdl_display_update;
  display->tme_screen_add = (void *)sizeof(struct tme_sdl_screen);
  display->tme_screen_resize = _tme_sdl_screen_resize;
  display->tme_screen_redraw = _tme_sdl_screen_redraw;
  //  tme_thread_create(&display->tme_display_thread, tme_display_th_update, display);  
  //  tme_mutex_unlock(&display->tme_display_mutex);
  return (TME_OK);
}
