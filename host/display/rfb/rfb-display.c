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

/* the new RFB display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_rfb,display) {
  rfbScreenInfoPtr server;
  int arg_i = 0;

  while(args[++arg_i] != NULL);
  
  /* start our data structure: */
  tme_display_init(element, 0);

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

  return (TME_OK);
}
