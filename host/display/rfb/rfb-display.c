/* $Id: rfb-screen.c,v 1.11 2009/08/30 21:39:03 fredette Exp $ */

/* host/rfb/rfb-display.c - VNC support: */

/*
 * Copyright (c) 2003 Matt Fredette
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
#include <rfb/rfb.h>
#ifdef _TME_HAVE_GLIB
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#else
#include <SDL.h>
#endif
#include <stdlib.h>

static const int bpp=4;

/* TODO: odd maxx doesn't work (vncviewer bug) */

static void
_tme_rfb_display_bell(struct tme_display *display) {
  rfbSendBell(display->tme_screen_data);
}

static int
_tme_rfb_display_update(struct tme_display *display) {
  long usec;
  rfbScreenInfoPtr server = display->tme_screen_data;
  
  if(!rfbIsActive(server)) return 1;

  usec = server->deferUpdateTime*1000;
  rfbProcessEvents(server, usec);
  return TME_OK;
}

/* this is called before the screen's display is updated: */
static void
_tme_rfb_screen_redraw(struct tme_screen *screen, int x, int y, int w, int h)
{
  rfbScreenInfoPtr server = screen->tme_screen_display->tme_screen_data;
  
  if((char *)screen->tme_screen_fb->tme_fb_connection_buffer == server->frameBuffer)
    rfbMarkRectAsModified(server, x, y, w, h);
}

/* switch to new framebuffer contents */
static void _tme_rfb_screen_resize(struct tme_screen *screen)
{
  unsigned char *oldfb, *newfb;
  struct tme_fb_connection *conn_fb = screen->tme_screen_fb;
  int width = conn_fb->tme_fb_connection_width;
  int height = conn_fb->tme_fb_connection_height;
  rfbScreenInfoPtr server = screen->tme_screen_display->tme_screen_data;

  conn_fb->tme_fb_connection_buffsz = width * height * bpp;   
  conn_fb->tme_fb_connection_scanline_pad = _tme_scanline_pad(width * bpp);
  newfb = (unsigned char*)tme_malloc(conn_fb->tme_fb_connection_buffsz);
  if(!conn_fb->tme_fb_connection_buffer ||
     (char *)conn_fb->tme_fb_connection_buffer == server->frameBuffer) {
    rfbNewFramebuffer(server, (char*)newfb, width, height, 8, 3, bpp);
  }
  free(conn_fb->tme_fb_connection_buffer);
  conn_fb->tme_fb_connection_buffer = newfb;  
  /*** FIXME: Re-install cursor. ***/
}

/* this makes a new screen: */
struct tme_screen *
_tme_rfb_screen_new(struct tme_display *display,
		    struct tme_connection *conn)
{
  struct tme_screen *screen;
  struct tme_fb_connection *conn_fb;
  rfbScreenInfoPtr server = display->tme_screen_data;
  rfbPixelFormat* format=&server->serverFormat;

  server->desktopName = display->tme_display_title;

  screen = tme_screen_new(display, struct tme_screen, conn);

  conn_fb = screen->tme_screen_fb;

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_skipx = 0;
  conn_fb->tme_fb_connection_order = (format->bigEndian) ? (TME_ENDIAN_BIG) : (TME_ENDIAN_LITTLE);
  conn_fb->tme_fb_connection_bits_per_pixel = format->bitsPerPixel;
  conn_fb->tme_fb_connection_depth = format->depth;
  conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
  conn_fb->tme_fb_connection_mask_g = format->greenMax << format->greenShift;
  conn_fb->tme_fb_connection_mask_b = format->blueMax << format->blueShift;
  conn_fb->tme_fb_connection_mask_r = format->redMax << format->redShift;
  
  /* We've handled the configure event, no need for further processing. */
  return (screen);
}

static struct tme_display *display;

static void _tme_rfb_clientgone(rfbClientPtr cl)
{
  cl->clientData = NULL;
}

static enum rfbNewClientAction _tme_rfb_newclient(rfbClientPtr cl)
{
  cl->clientData = display;
  cl->clientGoneHook = _tme_rfb_clientgone;
  return RFB_CLIENT_ACCEPT;
}

static void _tme_mouse_ev(int buttons, int x, int y, rfbClientPtr cl) { 
  struct tme_display *display = cl->clientData;

  /* make the buttons mask: */
  display->tme_screen_mouse_buttons_last = buttons;
  _tme_mouse_event(0,x,y,display);
}

static void _tme_keyboard_key_ev(int down, tme_keyboard_keyval_t key, rfbClientPtr cl) {
  _tme_keyboard_key_event(down, key, cl->clientData);
}

#ifndef _TME_HAVE_GLIB
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

static char *_tme_sdl_keyval_name(tme_keyboard_keyval_t sym) {
  return SDL_GetKeyName(tme_to_sdl_keysym(sym));
}

static tme_keyboard_keyval_t _tme_sdl_keyval_from_name(const char *name) {
  return sdl_to_tme_keysym(SDL_GetKeyFromName(name));
}
#endif

/* the new RFB display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_rfb,display) {
  rfbScreenInfoPtr server;
  int arg_i = 0;

  while(args[++arg_i] != NULL);
  
  /* start our data structure: */
  display = tme_new0(struct tme_display, 1);
#ifdef _TME_HAVE_GLIB
  display->tme_display_keyval_name = gdk_keyval_name;
  display->tme_display_keyval_from_name = gdk_keyval_from_name;
  display->tme_display_keyval_convert_case = gdk_keyval_convert_case;
  display->tme_display_key_void_symbol = GDK_KEY_VoidSymbol;
#else
  display->tme_display_keyval_name = _tme_sdl_keyval_name;
  display->tme_display_keyval_from_name = _tme_sdl_keyval_from_name;
  display->tme_display_key_void_symbol = SDLK_UNKNOWN;
#endif

  /* start our data structure: */
  tme_display_init(element, display);

  /* recover our data structure: */
  display = element->tme_element_private;

  /* allocate initial screen structure of the given size: */
  //  rfbProcessSizeArguments(&maxx, &maxy, &bpp, &arg_i, args);
  server=rfbGetScreen(&arg_i,args,
		      display->tme_screen_width,
		      display->tme_screen_height,
		      8,3,bpp);
  if(!server)
    return 1;
  server->frameBuffer = (char*)tme_malloc(display->tme_screen_width *
					  display->tme_screen_height *
					  bpp);
  server->alwaysShared = TRUE;
  server->ptrAddEvent = _tme_mouse_ev;
  server->kbdAddEvent = _tme_keyboard_key_ev;
  server->newClientHook = _tme_rfb_newclient;
  //  server->httpDir = "../webclients";
  //  server->httpEnableProxyConnect = TRUE;
  rfbInitServer(server);
  
  display->tme_screen_data = server;

  /* set the display-specific functions: */
  display->tme_display_bell = _tme_rfb_display_bell;
  display->tme_display_update = _tme_rfb_display_update;
  display->tme_screen_add = _tme_rfb_screen_new;
  display->tme_screen_resize = _tme_rfb_screen_resize;
  display->tme_screen_redraw = _tme_rfb_screen_redraw;

#ifdef _TME_HAVE_GLIB
  display->tme_display_keyval_name = gdk_keyval_name;
  display->tme_display_keyval_from_name = gdk_keyval_from_name;
  display->tme_display_keyval_convert_case = gdk_keyval_convert_case;
  display->tme_display_key_void_symbol = GDK_KEY_VoidSymbol;
#else
  display->tme_display_keyval_name = _tme_sdl_keyval_name;
  display->tme_display_keyval_from_name = _tme_sdl_keyval_from_name;
  display->tme_display_key_void_symbol = SDLK_UNKNOWN;
#endif

  return (TME_OK);
}
