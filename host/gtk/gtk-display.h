/* $Id: gtk-display.h,v 1.10 2009/08/28 01:29:47 fredette Exp $ */

/* host/gtk/gtk-display.h - header file for GTK display support: */

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

#ifndef _HOST_GTK_GTK_DISPLAY_H
#define _HOST_GTK_GTK_DISPLAY_H

#include <tme/common.h>
_TME_RCSID("$Id: gtk-display.h,v 1.10 2009/08/28 01:29:47 fredette Exp $");

/* includes: */
#include <tme/generic/fb.h>
#include <tme/generic/keyboard.h>
#include <tme/generic/mouse.h>
#include <tme/threads.h>
#include <tme/hash.h>
#ifndef G_ENABLE_DEBUG
#define G_ENABLE_DEBUG (0)
#endif /* !G_ENABLE_DEBUG */
#include <gtk/gtk.h>

/* macros: */

/* the callout flags: */
#define TME_GTK_DISPLAY_CALLOUT_CHECK		(0)
#define TME_GTK_DISPLAY_CALLOUT_RUNNING		TME_BIT(0)
#define TME_GTK_DISPLAY_CALLOUTS_MASK		(-2)
#define  TME_GTK_DISPLAY_CALLOUT_KEYBOARD_CTRL	TME_BIT(1)
#define  TME_GTK_DISPLAY_CALLOUT_MOUSE_CTRL	TME_BIT(2)

/* types: */

struct tme_gtk_display;

/* a screen: */
struct tme_gtk_screen {

  /* the next screen: */
  struct tme_gtk_screen *tme_gtk_screen_next;

  /* a backpointer to the display: */
  struct tme_gtk_display *tme_gtk_screen_display;

  /* the framebuffer connection.  unlike many other elements, this is
     *our* side of the framebuffer connection, not the peer's side: */
  struct tme_fb_connection *tme_gtk_screen_fb;

  /* the current scaling.  if this is < 0, the user has not forced a
     given scaling yet: */
  int tme_gtk_screen_fb_scale;

  /* any colorset signature: */
  tme_uint32_t tme_gtk_screen_colorset;

  /* the top-level window: */
  GtkWidget *tme_gtk_screen_window;
  
  /* the outer vertical packing box: */
  GtkWidget *tme_gtk_screen_vbox0;

  /* various menu item widgets: */
  GtkWidget *tme_gtk_screen_scale_default;
  GtkWidget *tme_gtk_screen_scale_half;

  /* the GtkEventBox, GdkImage and GtkImage for the framebuffer: */
  GtkWidget *tme_gtk_screen_event_box;
  GdkImage *tme_gtk_screen_gdkimage;
  GtkWidget *tme_gtk_screen_gtkimage;

  /* the translation function: */
  int (*tme_gtk_screen_fb_xlat) _TME_P((struct tme_fb_connection *, 
					struct tme_fb_connection *));

  /* the mouse on label: */
  GtkWidget *tme_gtk_screen_mouse_label;

  /* the status bar, and the context ID: */
  GtkWidget *tme_gtk_screen_mouse_statusbar;
  guint tme_gtk_screen_mouse_statusbar_cid;

  /* if GDK_VoidSymbol, mouse mode is off.  otherwise,
     mouse mode is on, and this is the keyval that will
     turn mouse mode off: */
  guint tme_gtk_screen_mouse_keyval;

  /* when mouse mode is on, this is the previous events mask
     for the framebuffer event box: */
  GdkEventMask tme_gtk_screen_mouse_events_old;

  /* when mouse mode is on, this is the warp center: */
  gint tme_gtk_screen_mouse_warp_x;
  gint tme_gtk_screen_mouse_warp_y;

  /* when mouse mode is on, the last tme buttons state: */
  unsigned int tme_gtk_screen_mouse_buttons_last;

  /* if nonzero, the screen needs a full redraw: */
  int tme_gtk_screen_full_redraw;
};

/* a GTK bad keysym: */
struct tme_gtk_keysym_bad {

  /* these are kept on a singly linked list: */
  struct tme_gtk_keysym_bad *tme_gtk_keysym_bad_next;

  /* the bad keysym string: */
  char *tme_gtk_keysym_bad_string;

  /* the flags and context used in the lookup: */
  unsigned int tme_keysym_bad_flags;
  unsigned int tme_gtk_keysym_bad_context_length;
  tme_uint8_t *tme_gtk_keysym_bad_context;
};

/* a display: */
struct tme_gtk_display {

  /* backpointer to our element: */
  struct tme_element *tme_gtk_display_element;

  /* our mutex: */
  tme_mutex_t tme_gtk_display_mutex;

  /* our keyboard connection: */
  struct tme_keyboard_connection *tme_gtk_display_keyboard_connection;

  /* our keyboard buffer: */
  struct tme_keyboard_buffer *tme_gtk_display_keyboard_buffer;

  /* our keysyms hash: */
  tme_hash_t tme_gtk_display_keyboard_keysyms;

  /* the bad keysym records: */
  struct tme_gtk_keysym_bad *tme_gtk_display_keyboard_keysyms_bad;

  /* our keysym to keycode hash: */
  tme_hash_t tme_gtk_display_keyboard_keysym_to_keycode;

  /* the next keysym to allocate for an unknown keysym string: */
  guint tme_gtk_display_keyboard_keysym_alloc_next;

  /* our mouse connection: */
  struct tme_mouse_connection *tme_gtk_display_mouse_connection;

  /* our mouse buffer: */
  struct tme_mouse_buffer *tme_gtk_display_mouse_buffer;

  /* our mouse cursor: */
  GdkCursor *tme_gtk_display_mouse_cursor;

  /* our screens: */
  struct tme_gtk_screen *tme_gtk_display_screens;

  /* the callout flags: */
  unsigned int tme_gtk_display_callout_flags;

  /* the tooltips group: */
  GtkTooltips *tme_gtk_display_tooltips;
};

/* a menu item: */
struct tme_gtk_display_menu_item {

  /* which menu item this is: */
  unsigned int tme_gtk_display_menu_item_which;

  /* where to save the menu item widget: */
  GtkWidget **tme_gtk_display_menu_item_widget;

  /* the string for the menu item label: */
  const char *tme_gtk_display_menu_item_string;
};

/* this generates menu items: */
typedef GtkSignalFunc (*tme_gtk_display_menu_items_t) _TME_P((void *, struct tme_gtk_display_menu_item *));

/* prototypes: */
struct tme_gtk_screen *_tme_gtk_screen_new _TME_P((struct tme_gtk_display *));
int _tme_gtk_screen_connections_new _TME_P((struct tme_gtk_display *, 
					    struct tme_connection **));
void _tme_gtk_keyboard_new _TME_P((struct tme_gtk_display *));
void _tme_gtk_keyboard_attach _TME_P((struct tme_gtk_screen *));
int _tme_gtk_keyboard_connections_new _TME_P((struct tme_gtk_display *,
					      struct tme_connection **));
void _tme_gtk_mouse_new _TME_P((struct tme_gtk_display *));
void _tme_gtk_mouse_mode_off _TME_P((struct tme_gtk_screen *, guint32));
void _tme_gtk_mouse_attach _TME_P((struct tme_gtk_screen *));
int _tme_gtk_mouse_connections_new _TME_P((struct tme_gtk_display *,
					   struct tme_connection **));
void _tme_gtk_screen_th_update _TME_P((struct tme_gtk_display *));
void _tme_gtk_display_callout _TME_P((struct tme_gtk_display *,
				      int));
gint _tme_gtk_display_enter_focus _TME_P((GtkWidget *, GdkEvent *, gpointer));
GtkWidget *_tme_gtk_display_menu_radio _TME_P((void *, tme_gtk_display_menu_items_t));

#endif /* _HOST_GTK_GTK_DISPLAY_H */

