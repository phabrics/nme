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

/* a screen: */
struct tme_gtk_screen {

  /* the generic screen structure */
  struct tme_screen screen;
  
  /* any colorset signature: */
  tme_uint32_t tme_gtk_screen_colorset;

  /* the top-level window: */
  GtkWidget *tme_gtk_screen_window;
  
  /* the outer vertical packing box: */
  GtkWidget *tme_gtk_screen_vbox0;

  /* various menu item widgets: */
  GtkWidget *tme_gtk_screen_scale_default;
  GtkWidget *tme_gtk_screen_scale_half;

  /* the Gtkframe & cairo_surface for the framebuffer: */
  GtkWidget *tme_gtk_screen_gtkframe;
  cairo_surface_t *tme_gtk_screen_surface;
  GdkDevice *tme_gtk_screen_pointer;

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

/* a menu item: */
struct tme_display_menu_item {

  /* which menu item this is: */
  unsigned int tme_display_menu_item_which;

  /* where to save the menu item widget: */
  GtkWidget **tme_display_menu_item_widget;

  /* the string for the menu item label: */
  const char *tme_display_menu_item_string;
};

/* this generates menu items: */
typedef GCallback (*tme_display_menu_items_t) _TME_P((void *, struct tme_display_menu_item *));

/* prototypes: */
void _tme_gtk_keyboard_attach _TME_P((struct tme_gtk_screen *));
void _tme_gtk_mouse_attach _TME_P((struct tme_gtk_screen *));
int _tme_gtk_screen_update _TME_P((void *));
gint _tme_display_enter_focus _TME_P((GtkWidget *, GdkEvent *, gpointer));
GtkWidget *_tme_display_menu_radio _TME_P((void *, tme_display_menu_items_t));

#endif /* _HOST_GTK_DISPLAY_H */

