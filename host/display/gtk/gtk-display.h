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

#ifndef _HOST_GTK_DISPLAY_H
#define _HOST_GTK_DISPLAY_H

_TME_RCSID("$Id: gtk-display.h,v 1.10 2009/08/28 01:29:47 fredette Exp $");

/* includes: */
#include "display.h"
#ifndef G_ENABLE_DEBUG
#define G_ENABLE_DEBUG (0)
#endif /* !G_ENABLE_DEBUG */
#include <gtk/gtk.h>

/* macros: */

/* types: */

/* a display: */
typedef struct tme_gtk_display {
  /* the generic display structure */
  struct tme_display display;

  /* the GTK application structure */
  GtkApplication *tme_gtk_application;
  
  GdkDisplay *tme_gdk_display;
  
  GdkCursor *tme_gdk_display_cursor;

  GdkSeat *tme_gdk_display_seat;

#if GTK_MAJOR_VERSION == 3
  GdkMonitor *tme_gdk_display_monitor;
#endif
} _tme_gtk_display, *tme_gtk_display;
  
/* a screen: */
typedef struct tme_gtk_screen {

  /* the generic screen structure */
  struct tme_screen screen;

  /* the top-level window: */
  GtkWidget *tme_gtk_screen_window;
  
  /* the header bar: */
  GtkWidget *tme_gtk_screen_header;

  /* the drawing area & surface for the framebuffer: */
  GtkWidget *tme_gtk_screen_draw;
  cairo_surface_t *tme_gtk_screen_surface;

  /* the mouse label: */
  GtkWidget *tme_gtk_screen_mouse_label;
  /* the mouse toggle button: */
  GtkWidget *tme_gtk_screen_mouse_button;
  /* the name of the mouse toggle key: */
  GtkWidget *tme_gtk_screen_mouse_key;

  /* if GDK_VoidSymbol, mouse mode is off.  otherwise,
     mouse mode is on, and this is the keyval that will
     turn mouse mode off: */
  guint tme_gtk_screen_mouse_keyval;

#if GTK_MAJOR_VERSION == 3
  cairo_format_t tme_gtk_screen_format;
  GtkEventController *key, *mouse, *motion, *event;
  GtkGesture *press;

  /* when mouse mode is on, this is the previous events mask
     for the framebuffer event box: */
  //GdkEventMask tme_gtk_screen_mouse_events_old;

} _tme_gtk_screen, *tme_gtk_screen;

/* a menu item: */
struct tme_display_menu_item {
  /* the string for the menu item label: */
  const char *name;

  GCallback menu_func;
};

/* prototypes: */
GtkWidget *_tme_display_menu_radio _TME_P((struct tme_gtk_screen *, struct tme_display_menu_item *, int num_items));
#else
} _tme_gtk_screen, *tme_gtk_screen;
#endif
void _tme_gtk_keyboard_attach _TME_P((struct tme_gtk_screen *));
void _tme_gtk_mouse_attach _TME_P((struct tme_gtk_screen *));

#endif /* _HOST_GTK_DISPLAY_H */

