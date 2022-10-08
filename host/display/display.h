/* host/disp/display.h - header file for generic display support: */

/*
 * Copyright (c) 2017 Ruben Agin
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

#ifndef _HOST_DISPLAY_H
#define _HOST_DISPLAY_H

#include <tme/common.h>

/* includes: */
#include <tme/generic/fb.h>
#include <tme/generic/keyboard.h>
#include <tme/generic/mouse.h>
#include <tme/threads.h>
#include <tme/hash.h>

/* macros: */

/* the callout flags: */
#define TME_DISPLAY_CALLOUT_CHECK		(0)
#define TME_DISPLAY_CALLOUT_RUNNING		TME_BIT(0)
#define TME_DISPLAY_CALLOUTS_MASK		(-2)
#define  TME_DISPLAY_CALLOUT_KEYBOARD_CTRL	TME_BIT(1)
#define  TME_DISPLAY_CALLOUT_MOUSE_CTRL	TME_BIT(2)
#define BLANK_SIDE (16 * 8)
#define TME_SCREEN_UPDATE_NONE                  (0)
#define TME_SCREEN_UPDATE_REDRAW                (1)
#define TME_SCREEN_UPDATE_RESIZE                (2)
#define TME_SCREEN_MAXSIZE                   (1024)
/* types: */

struct tme_display;

/* a screen: */
struct tme_screen {

  /* the next screen: */
  struct tme_screen *tme_screen_next;

  /* a backpointer to the display: */
  struct tme_display *tme_screen_display;

  /* the framebuffer connection.  unlike many other elements, this is
   *our* side of the framebuffer connection, not the peer's side: */
  struct tme_fb_connection *tme_screen_fb;

  /* the current scaling.  if this is < 0, the user has not forced a
     given scaling yet: */
  int tme_screen_fb_scale;
  // This is the device scale (if any) for the host platform itself
  int tme_screen_scale;
  
  /* any colorset signature: */
  tme_uint32_t tme_screen_colorset;

  /* if nonzero, the screen has changed and should be updated in the display: */
  int tme_screen_update;

  /* the translation function: */
  int (*tme_screen_fb_xlat) _TME_P((struct tme_fb_connection *, 
				    struct tme_fb_connection *));

};

/* a bad keysym: */
struct tme_keysym_bad {

  /* these are kept on a singly linked list: */
  struct tme_keysym_bad *tme_keysym_bad_next;

  /* the bad keysym string: */
  char *tme_keysym_bad_string;

  /* the flags and context used in the lookup: */
  unsigned int tme_keysym_bad_flags;
  unsigned int tme_keysym_bad_context_length;
  tme_uint8_t *tme_keysym_bad_context;
};

/* a display: */
struct tme_display {

  /* backpointer to our element: */
  struct tme_element *tme_display_element;

  /* our mutex: */
  tme_mutex_t tme_display_mutex;

  /* our thread: */
  tme_threadid_t tme_display_thread;

  /* our keyboard connection: */
  struct tme_keyboard_connection *tme_display_keyboard_connection;

  /* our keyboard buffer: */
  struct tme_keyboard_buffer *tme_display_keyboard_buffer;

  /* our keysyms hash: */
  tme_hash_t tme_display_keyboard_keysyms;

  /* the bad keysym records: */
  struct tme_keysym_bad *tme_display_keyboard_keysyms_bad;

  /* our keysym to keycode hash: */
  tme_hash_t tme_display_keyboard_keysym_to_keycode;

  /* the next keysym to allocate for an unknown keysym string: */
  unsigned int tme_display_keyboard_keysym_alloc_next;

  /* our mouse connection: */
  struct tme_mouse_connection *tme_display_mouse_connection;

  /* our mouse buffer: */
  struct tme_mouse_buffer *tme_display_mouse_buffer;

  /* the title to be displayed by the front-end: */
  char tme_display_title[sizeof(PACKAGE_STRING) + 16];

  /* our screens: */
  struct tme_screen *tme_display_screens;

  /* the callout flags: */
  unsigned int tme_display_callout_flags;

  /* available screen dimensions: */
  int tme_screen_width;
  int tme_screen_height;
  
  /* screen mouse data: */
  int tme_screen_mouse_buttons_last,
    tme_screen_mouse_warp_x,
    tme_screen_mouse_warp_y;

  /* a convenience pointer for any platform-specific host data: */
  void *tme_screen_data;
  
  /* implementation-specific callback functions: */
  int (*tme_display_bell) _TME_P((struct tme_display *));
  int (*tme_display_update) _TME_P((struct tme_display *));
  struct tme_screen *(*tme_screen_add) _TME_P((struct tme_display *, struct tme_connection *));
  int (*tme_screen_resize) _TME_P((struct tme_screen *));
  int (*tme_screen_redraw) _TME_P((struct tme_screen *, int x, int y, int w, int h));
  int (*tme_screen_update) _TME_P((struct tme_screen *));
};

/* prototypes: */
/* this recovers the scanline-pad value for an image buffer: */
inline unsigned int
_tme_scanline_pad(int bpl)
{
  if ((bpl % sizeof(tme_uint32_t)) == 0) {
    return (32);
  }
  if ((bpl % sizeof(tme_uint16_t)) == 0) {
    return (16);
  }
  return (8);
}

extern struct tme_display *(*_tme_display_get) _TME_P((void *));
struct tme_screen *_tme_screen_add _TME_P((struct tme_display *,
					   size_t,
					   struct tme_connection *));
#define tme_screen_new(display, screen, conn) ((screen *)_tme_screen_add(display,sizeof(screen),conn))
void _tme_screen_scale_set _TME_P((struct tme_screen *screen,
				   int scale_new));
void _tme_screen_xlat_set _TME_P((struct tme_screen *screen));
void _tme_keyboard_new _TME_P((struct tme_display *));
int _tme_keyboard_connections_new _TME_P((struct tme_display *,
					  struct tme_connection **));
int _tme_keyboard_key_press _TME_P((int down, tme_keyboard_keyval_t key, void *disp));
void _tme_mouse_new _TME_P((struct tme_display *));
void _tme_mouse_mode_off _TME_P((struct tme_screen *, tme_uint32_t));
int _tme_mouse_connections_new _TME_P((struct tme_display *,
				       struct tme_connection **));
int _tme_mouse_buttons_event _TME_P((int buttons, int x, int y, void *disp));

#endif /* _HOST_DISPLAY_H */

