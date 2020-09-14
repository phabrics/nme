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
#include <stdlib.h>

static const int bpp=4;
static int maxx=1920, maxy=1080;
static struct tme_display *_display;
/* TODO: odd maxx doesn't work (vncviewer bug) */

typedef struct tme_rfb_screen {
  int tme_rfb_screen_mouse_buttons_last,
    tme_rfb_screen_mouse_warp_x,
    tme_rfb_screen_mouse_warp_y;
} tme_rfb_screen;

typedef struct tme_rfb_display {
  /* the generic display structure */
  struct tme_display display;

  rfbScreenInfoPtr server;
} tme_rfb_display;

static void _tme_rfb_clientgone(rfbClientPtr cl)
{
  free(cl->clientData);
  cl->clientData = NULL;
}

static enum rfbNewClientAction _tme_rfb_newclient(rfbClientPtr cl)
{
  cl->clientData = tme_new0(struct tme_rfb_screen, 1);
  cl->clientGoneHook = _tme_rfb_clientgone;
  return RFB_CLIENT_ACCEPT;
}

static void
_tme_rfb_display_bell(struct tme_rfb_display *display) {
  rfbSendBell(display->server);
}

static int
_tme_rfb_display_update(struct tme_display *display) {
  long usec;
  rfbScreenInfoPtr server = ((tme_rfb_display *)display)->server;
  
  if(!rfbIsActive(server)) return 1;

  usec = server->deferUpdateTime*1000;
  rfbProcessEvents(server, usec);
  return TME_OK;
}

/* this is called before the screen's display is updated: */
static void
_tme_rfb_screen_redraw(struct tme_screen *screen)
{
  struct tme_fb_connection *conn_fb = screen->tme_screen_fb;
  rfbScreenInfoPtr server = ((tme_rfb_display *)screen->tme_screen_display)->server;
  
  if((char *)conn_fb->tme_fb_connection_buffer == server->frameBuffer)
    rfbMarkRectAsModified(server, 0, 0,
			  conn_fb->tme_fb_connection_width,
			  conn_fb->tme_fb_connection_height);
}

/* switch to new framebuffer contents */
static void _tme_rfb_screen_resize(struct tme_screen *screen)
{
  unsigned char *oldfb, *newfb;
  struct tme_fb_connection *conn_fb = screen->tme_screen_fb;
  int width = conn_fb->tme_fb_connection_width;
  int height = conn_fb->tme_fb_connection_height;
  rfbScreenInfoPtr server = ((tme_rfb_display *)screen->tme_screen_display)->server;

  conn_fb->tme_fb_connection_buffsz = width * height * bpp;   
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
_tme_rfb_screen_new(struct tme_rfb_display *display,
		    struct tme_connection *conn)
{
  struct tme_screen *screen;
  struct tme_fb_connection *conn_fb;
  rfbPixelFormat* format=&display->server->serverFormat;

  /* lock our mutex: */
  tme_mutex_lock(&display->display.tme_display_mutex);

  screen = tme_screen_new(display, struct tme_screen, conn);

  screen->tme_screen_scale = 1;
  
  conn_fb = screen->tme_screen_fb;

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_skipx = 0;
  conn_fb->tme_fb_connection_scanline_pad = 4 * bpp;
  conn_fb->tme_fb_connection_order = (format->bigEndian) ? (TME_ENDIAN_BIG) : (TME_ENDIAN_LITTLE);
  conn_fb->tme_fb_connection_bits_per_pixel = format->bitsPerPixel;
  conn_fb->tme_fb_connection_depth = format->depth;
  conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
  conn_fb->tme_fb_connection_mask_g = format->greenMax << format->greenShift;
  conn_fb->tme_fb_connection_mask_b = format->blueMax << format->blueShift;
  conn_fb->tme_fb_connection_mask_r = format->redMax << format->redShift;
  
  _tme_screen_configure(screen);

  /* unlock our mutex: */
  tme_mutex_unlock(&display->display.tme_display_mutex);

  /* We've handled the configure event, no need for further processing. */
  return (screen);
}

/* this is a VNC callback for a key press or release event: */
static void
_tme_rfb_key_event(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
  struct tme_keyboard_event tme_event;

  /* make a tme event from this rfb event: */
  tme_event.tme_keyboard_event_type
    = (down
       ? TME_KEYBOARD_EVENT_PRESS
       : TME_KEYBOARD_EVENT_RELEASE);
  tme_event.tme_keyboard_event_modifiers
    = TME_KEYBOARD_MODIFIER_NONE;
  tme_event.tme_keyboard_event_keyval
    = key;
  tme_get_time(&tme_event.tme_keyboard_event_time);

  /* lock the mutex: */
  tme_mutex_lock(&_display->tme_display_mutex);

  _tme_keyboard_key_event(&tme_event, _display);
  
  /* unlock the mutex: */
  tme_mutex_unlock(&_display->tme_display_mutex);

}

/* this is a VNC callback for a mouse event: */
static int
_tme_rfb_mouse_event(int buttonMask, int x, int y, rfbClientPtr cl)
{
  struct tme_mouse_event tme_event;
  struct tme_rfb_screen *screen = (tme_rfb_screen *)cl->clientData;
  
  /* start the tme event: */
  tme_event.tme_mouse_event_delta_units
    = TME_MOUSE_UNITS_UNKNOWN;

  /* lock the mutex: */
  tme_mutex_lock(&_display->tme_display_mutex);

  /* set the event time: */
  tme_get_time(&tme_event.tme_mouse_event_time);

  /* if the button mask and pointer position haven't changed, return now.
     every time we warp the pointer we will get a motion event, and
     this should ignore those events: */

  if (buttonMask == screen->tme_rfb_screen_mouse_buttons_last
      && x == screen->tme_rfb_screen_mouse_warp_x
      && y == screen->tme_rfb_screen_mouse_warp_y) {
    
    /* unlock the mutex: */
    tme_mutex_unlock(&_display->tme_display_mutex);
    
    /* stop propagating this event: */
    return (TRUE);
  }
  
  /* make the buttons mask: */
  tme_event.tme_mouse_event_buttons =
    screen->tme_rfb_screen_mouse_buttons_last = buttonMask;
  
  /* make the deltas: */
  tme_event.tme_mouse_event_delta_x = 
    (((int) x)
     - ((int) screen->tme_rfb_screen_mouse_warp_x));
  tme_event.tme_mouse_event_delta_y = 
    (((int) y)
     - ((int) screen->tme_rfb_screen_mouse_warp_y));

  screen->tme_rfb_screen_mouse_warp_x = x;
  screen->tme_rfb_screen_mouse_warp_y = y;
  
  _tme_mouse_mouse_event(&tme_event, _display);
  
  /* unlock the mutex: */
  tme_mutex_unlock(&_display->tme_display_mutex);

  rfbDefaultPtrAddEvent(buttonMask, x, y, cl);
 
  /* stop propagating this event: */
  return (TRUE);
}

/* the new RFB display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_rfb,display) {
  rfbScreenInfoPtr server;
  struct tme_rfb_display *display;
  int arg_i = 0;

  while(args[++arg_i] != NULL);
  
  /* start our data structure: */
  display = tme_new0(struct tme_rfb_display, 1);
  tme_display_init(element, display);

  /* recover our data structure: */
  display = element->tme_element_private;

  /* allocate initial screen structure of the given size: */
  //  rfbProcessSizeArguments(&maxx, &maxy, &bpp, &arg_i, args);
  server=rfbGetScreen(&arg_i,args,maxx,maxy,8,3,bpp);
  if(!server)
    return 1;
  server->desktopName = "The Machine Emulator";
  server->frameBuffer = (char*)tme_malloc(maxx*maxy*bpp);
  server->alwaysShared = TRUE;
  server->ptrAddEvent = _tme_rfb_mouse_event;
  server->kbdAddEvent = _tme_rfb_key_event;
  server->newClientHook = _tme_rfb_newclient;
  //  server->httpDir = "../webclients";
  // server->httpEnableProxyConnect = TRUE;
  rfbInitServer(server);
  
  display->server = server;
  display->display.tme_screen_width = maxx;
  display->display.tme_screen_height = maxy;

  /* set the display-specific functions: */
  display->display.tme_display_bell = _tme_rfb_display_bell;
  display->display.tme_display_update = _tme_rfb_display_update;
  display->display.tme_screen_add = _tme_rfb_screen_new;
  display->display.tme_screen_resize = _tme_rfb_screen_resize;
  display->display.tme_screen_redraw = _tme_rfb_screen_redraw;

  _display = display;
  return (TME_OK);
}
