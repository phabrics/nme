/* $Id: gtk-screen.c,v 1.11 2009/08/30 21:39:03 fredette Exp $ */

/* host/gtk/gtk-screen.c - GTK screen support: */

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
_TME_RCSID("$Id: gtk-screen.c,v 1.11 2009/08/30 21:39:03 fredette Exp $");

/* includes: */
#include "gtk-display.h"
#include <stdlib.h>
#if GTK_MAJOR_VERSION == 4
#define _tme_gtk_init gtk_init_check
#else
static gboolean _tme_gtk_init(void) {
  char **argv;
  char *argv_buffer[3];
  int argc;

  /* conjure up an argv.  this is pretty bad: */
  argv = argv_buffer;
  argc = 0;
  argv[argc++] = "tmesh";
#if 0
  argv[argc++] = "--gtk-debug=signals";
#endif
  argv[argc] = NULL;
  return gtk_init_check(&argc, &argv);
}
#endif

static void
_tme_gtk_display_bell(_tme_gtk_display *display) {
  gdk_display_beep(display->tme_gdk_display);
}

static int
_tme_gtk_display_update(struct tme_display *display) {
  int rc;
  
  for(rc=TRUE;rc && g_main_context_pending(NULL);
      rc = g_main_context_iteration(NULL, TRUE));
  
  if(rc) rc = (gtk_window_list_toplevels() != NULL);
  return !rc;
}

#if GTK_MAJOR_VERSION == 3

/* this sets the screen scaling to that indicated by the Scale menu: */
static void
_tme_gtk_screen_scale_default(GtkWidget *widget,
			      _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_scale_set(screen, -TME_FB_XLAT_SCALE_NONE );
}

/* this sets the screen scaling to that indicated by the Scale menu: */
static void
_tme_gtk_screen_scale_half(GtkWidget *widget,
			   _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_scale_set(screen, TME_FB_XLAT_SCALE_HALF );
}

/* this sets the screen scaling to that indicated by the Scale menu: */
static void
_tme_gtk_screen_scale_full(GtkWidget *widget,
			   _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_scale_set(screen, TME_FB_XLAT_SCALE_NONE );
}

/* this sets the screen scaling to that indicated by the Scale menu: */
static void
_tme_gtk_screen_scale_double(GtkWidget *widget,
			     _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_scale_set(screen, TME_FB_XLAT_SCALE_DOUBLE );
}

/* this sets the screen size: */
static inline void
_tme_screen_format_set(_tme_gtk_screen *screen,
		       cairo_format_t format)
{
  screen->tme_gtk_screen_format = format;
  _tme_screen_configure(screen);
}

/* this sets the screen format to that indicated by the Format menu: */
static void
_tme_gtk_screen_format_argb32(GtkWidget *widget,
			      _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_format_set(screen, CAIRO_FORMAT_ARGB32);
}

/* this sets the screen format to that indicated by the Format menu: */
static void
_tme_gtk_screen_format_rgb24(GtkWidget *widget,
			     _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_format_set(screen, CAIRO_FORMAT_RGB24);
}

/* this sets the screen format to that indicated by the Format menu: */
static void
_tme_gtk_screen_format_a8(GtkWidget *widget,
			  _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_format_set(screen, CAIRO_FORMAT_A8);
}

/* this sets the screen format to that indicated by the Format menu: */
static void
_tme_gtk_screen_format_a1(GtkWidget *widget,
			  _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_format_set(screen, CAIRO_FORMAT_A1);
}

/* this sets the screen format to that indicated by the Format menu: */
static void
_tme_gtk_screen_format_rgb16_565(GtkWidget *widget,
				 _tme_gtk_screen *screen)
{
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    return;

  _tme_screen_format_set(screen, CAIRO_FORMAT_RGB16_565);
}

static struct tme_display_menu_item format_items[] =
  {
   { _("ARGB32"), G_CALLBACK(_tme_gtk_screen_format_argb32) },
   { _("RGB24"), G_CALLBACK(_tme_gtk_screen_format_rgb24) },
   { _("A8"), G_CALLBACK(_tme_gtk_screen_format_a8) },
   { _("A1"), G_CALLBACK(_tme_gtk_screen_format_a1) },
   { _("RGB16_565"), G_CALLBACK(_tme_gtk_screen_format_rgb16_565) }
  };

static struct tme_display_menu_item scale_items[] =
  {
   { _("Default"), G_CALLBACK(_tme_gtk_screen_scale_default) },
   { _("Half"), G_CALLBACK(_tme_gtk_screen_scale_half) },
   { _("Full"), G_CALLBACK(_tme_gtk_screen_scale_full) },
   { _("Double"), G_CALLBACK(_tme_gtk_screen_scale_double) }
  };

#endif

/* Screen-specific size request */
static void _tme_gtk_screen_resize(_tme_gtk_screen *screen) {
  struct tme_fb_connection *conn_fb = screen->screen.tme_screen_fb;
  
  /* set a minimum size */
  gtk_widget_set_size_request(screen->tme_gtk_screen_draw,
			      conn_fb->tme_fb_connection_width,
			      conn_fb->tme_fb_connection_height);
}

/* Create a new surface of the appropriate size to store our scribbles */
static void
_tme_gtk_screen_configure(GtkWidget         *widget,
#if GTK_MAJOR_VERSION == 4
			  int width,
			  int height,
#elif GTK_MAJOR_VERSION == 3
			  GdkEventConfigure *event,
#endif
			  gpointer          _screen)
{
  _tme_gtk_screen *screen;
  struct tme_display *display;
  struct tme_fb_connection *conn_fb;
#if GTK_MAJOR_VERSION == 4
  GdkSurface
#elif GTK_MAJOR_VERSION == 3
  GdkWindow 
#endif
    *window;
  
  screen = (_tme_gtk_screen *) _screen;

  /* get the display: */
  display = screen->screen.tme_screen_display;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  if(screen->tme_gtk_screen_surface) {
    cairo_surface_destroy(screen->tme_gtk_screen_surface);
    screen->tme_gtk_screen_surface = NULL;
  }

#if GTK_MAJOR_VERSION == 4
  window = gtk_native_get_surface(gtk_widget_get_native(widget));

  //  screen->screen.tme_screen_scale = gdk_surface_get_scale_factor(window);
  
  if(window) {
      screen->tme_gtk_screen_surface =
	gdk_surface_create_similar_surface(window,
					   CAIRO_CONTENT_COLOR,
					   gtk_widget_get_width(widget) /* * screen->screen.tme_screen_scale */,
					   gtk_widget_get_height(widget) // * screen->screen.tme_screen_scale
					   );
  }
#elif GTK_MAJOR_VERSION == 3
  window = gtk_widget_get_window(widget);
  
  screen->screen.tme_screen_scale = gdk_window_get_scale_factor(window);
  
  screen->tme_gtk_screen_surface
    = gdk_window_create_similar_image_surface(window,
					      screen->tme_gtk_screen_format,
					      gdk_window_get_width(window) * screen->screen.tme_screen_scale,
					      gdk_window_get_height(window) * screen->screen.tme_screen_scale,
					      screen->screen.tme_screen_scale);
#endif

  conn_fb = screen->screen.tme_screen_fb;

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_width = cairo_image_surface_get_width(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_height = cairo_image_surface_get_height(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_skipx = 0;
  conn_fb->tme_fb_connection_scanline_pad = _tme_scanline_pad(cairo_image_surface_get_stride(screen->tme_gtk_screen_surface));
  conn_fb->tme_fb_connection_order = TME_ENDIAN_NATIVE;
  conn_fb->tme_fb_connection_buffer = cairo_image_surface_get_data(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_buffsz = cairo_image_surface_get_stride(screen->tme_gtk_screen_surface) * conn_fb->tme_fb_connection_height;
  conn_fb->tme_fb_connection_bits_per_pixel = 32;
  conn_fb->tme_fb_connection_depth = 24;
  conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
  conn_fb->tme_fb_connection_mask_g = 0x00ff00;
  conn_fb->tme_fb_connection_mask_b = 0x0000ff;
  conn_fb->tme_fb_connection_mask_r = 0xff0000;

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

  /* We've handled the configure event, no need for further processing. */
  //  return TRUE;
}

/* this is called before the screen's display is updated: */
static void
_tme_gtk_screen_redraw(_tme_gtk_screen *screen, int x, int y, int w, int h)
{
  cairo_surface_flush(screen->tme_gtk_screen_surface);
  cairo_surface_mark_dirty(screen->tme_gtk_screen_surface);
  gtk_widget_queue_draw(screen->tme_gtk_screen_draw);
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
_tme_gtk_screen_draw(GtkWidget *widget,
		     cairo_t   *cr,
#if GTK_MAJOR_VERSION == 4
		     int width,
		     int height,
#endif
		     gpointer   _screen)
{
  struct tme_display *display;
  _tme_gtk_screen *screen;

  screen = (_tme_gtk_screen *) _screen;

  /* get the display: */
  display = screen->screen.tme_screen_display;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  cairo_set_source_surface(cr, screen->tme_gtk_screen_surface, 0, 0);
  cairo_paint(cr);

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

  return FALSE;
}

static void
_tme_gtk_screen_close(_tme_gtk_screen *screen)
{
  if (screen->tme_gtk_screen_surface)
    cairo_surface_destroy (screen->tme_gtk_screen_surface);
}

/* this makes a new screen: */
_tme_gtk_screen *
_tme_gtk_screen_new(_tme_gtk_display *display,
		    struct tme_connection *conn)
{
  _tme_gtk_screen *screen;
  GtkWidget *vbox;
#if GTK_MAJOR_VERSION == 3
  GtkWidget *menu_bar;
  GtkWidget *menu;
  GtkWidget *submenu;
  GtkWidget *menu_item;
#endif
  
  screen = tme_screen_new(display, _tme_gtk_screen, conn);

  /* create the header bar: */
  screen->tme_gtk_screen_header = gtk_header_bar_new();

  /* create the outer vertical packing box: */
  vbox
    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  /* create the Draw for the framebuffer area: */
  screen->tme_gtk_screen_draw = gtk_drawing_area_new();
  
  /* create the top-level window, and allow it to shrink, grow,
     and auto-shrink: */
  screen->tme_gtk_screen_window = 
#if GTK_MAJOR_VERSION == 4
     gtk_window_new();

  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(screen->tme_gtk_screen_draw),
				 _tme_gtk_screen_draw, screen, NULL);
  g_signal_connect_after(screen->tme_gtk_screen_draw, "resize",
			 G_CALLBACK(_tme_gtk_screen_configure), screen);

  /* add the outer vertical packing box to the window: */
  gtk_window_set_child(GTK_WINDOW(screen->tme_gtk_screen_window), vbox);
  
  /* pack the Draw into the outer vertical packing box: */
  gtk_box_prepend(GTK_BOX(vbox), 
		  screen->tme_gtk_screen_draw);

#elif GTK_MAJOR_VERSION == 3
  gtk_window_new(GTK_WINDOW_TOPLEVEL);
  screen->screen.tme_screen_scale = gdk_monitor_get_scale_factor(display->tme_gdk_display_monitor);

  /* add the outer vertical packing box to the window: */
  gtk_container_add(GTK_CONTAINER(screen->tme_gtk_screen_window),
		    vbox);

  /* create the menu bar and pack it into the outer vertical packing
     box: */
  menu_bar = gtk_menu_bar_new ();
  gtk_box_pack_start(GTK_BOX(vbox), 
		     menu_bar,
		     FALSE, FALSE, 0);
  //  gtk_widget_show(menu_bar);

  /* create the Screen menu: */
  menu = gtk_menu_new();

  /* create the Screen scaling submenu: */
  submenu = _tme_display_menu_radio(screen, scale_items, TME_ARRAY_ELS(scale_items));

  /* create the Screen scaling submenu item: */
  menu_item = gtk_menu_item_new_with_label(_("Scale"));
  //  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  /* create the Screen colormap submenu: */
  submenu = _tme_display_menu_radio(screen, format_items, TME_ARRAY_ELS(format_items));

  /* create the Screen scaling submenu item: */
  menu_item = gtk_menu_item_new_with_label(_("Format"));
  //  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  _tme_screen_format_set(screen, CAIRO_FORMAT_RGB24);

  /* create the Screen menu bar item, attach the menu to it, and 
     attach the menu bar item to the menu bar: */
  menu_item = gtk_menu_item_new_with_label("Screen");
  //  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

  /* pack the Draw into the outer vertical packing box: */
  gtk_box_pack_start(GTK_BOX(vbox), 
		     screen->tme_gtk_screen_draw,
		     FALSE, FALSE, 0);

  g_signal_connect(screen->tme_gtk_screen_draw, "draw",
		   G_CALLBACK(_tme_gtk_screen_draw), screen);
  g_signal_connect(screen->tme_gtk_screen_draw, "configure-event",
		   G_CALLBACK(_tme_gtk_screen_configure), screen);
#endif

  gtk_window_set_resizable(GTK_WINDOW(screen->tme_gtk_screen_window), FALSE);

  /* attach the mouse to this screen: */
  _tme_gtk_mouse_attach(screen);

  /* attach the keyboard to this screen: */
  _tme_gtk_keyboard_attach(screen);

  gtk_window_set_titlebar(GTK_WINDOW(screen->tme_gtk_screen_window),
			  screen->tme_gtk_screen_header);
  
  gtk_window_set_title(GTK_WINDOW(screen->tme_gtk_screen_window),
		       display->display.tme_display_title);

  g_signal_connect(screen->tme_gtk_screen_window, "destroy",
		   G_CALLBACK(_tme_gtk_screen_close), screen);
  
  /* unlock our mutex: */
  tme_mutex_unlock(&display->display.tme_display_mutex);

  /* show the top-level window: */
  gtk_widget_show_all(screen->tme_gtk_screen_window);

  /* lock our mutex: */
  tme_mutex_lock(&display->display.tme_display_mutex);

  return (screen);
}

#if GTK_MAJOR_VERSION == 3
/* this creates a menu of radio buttons: */
GtkWidget *
_tme_display_menu_radio(_tme_gtk_screen *screen,
			struct tme_display_menu_item menu_items[],
			int num_items)
{
  GtkWidget *menu;
  GSList *menu_group;
  GtkWidget *menu_item;
  int i;
  
  /* create the menu: */
  menu = gtk_menu_new();

  /* create the menu items: */
  menu_group = NULL;
  for (i=0;i<num_items;i++) {
    menu_item
      = gtk_radio_menu_item_new_with_label(menu_group,
					   menu_items[i].name);
    menu_group
      = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
    g_signal_connect(menu_item, 
		     "activate",
		     menu_items[i].menu_func,
		     (gpointer) screen);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    //    gtk_widget_show(menu_item);
  }

  /* return the menu: */
  return (menu);
}
#endif

/* the new GTK display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_gtk,display) {
  _tme_gtk_display *display;
#if GTK_MAJOR_VERSION == 3
  GdkRectangle workarea;
#endif
  int rc;
  
  /* GTK requires program to be running non-setuid */
#ifdef HAVE_SETUID
  setuid(getuid());
#endif
  
  if(rc = !_tme_gtk_init()) return rc;
  
  /* start our data structure: */
  display = tme_new0(_tme_gtk_display, 1);
  tme_display_init(element, display);

  /* recover our data structure: */
  display = element->tme_element_private;

  //  display->tme_gtk_application = gtk_application_new("org.phabrics.tme", G_APPLICATION_DEFAULT_FLAGS);
  
  display->tme_gdk_display = gdk_display_get_default();

  display->tme_gdk_display_seat = gdk_display_get_default_seat(display->tme_gdk_display);

  display->tme_gdk_display_cursor =
#if GTK_MAJOR_VERSION == 4
    GDK_BLANK_CURSOR;
    //    = gdk_cursor_new_from_name("none", NULL);
#elif GTK_MAJOR_VERSION == 3
    gdk_cursor_new_for_display(display->tme_gdk_display, GDK_BLANK_CURSOR);

  display->tme_gdk_display_monitor = gdk_display_get_primary_monitor(display->tme_gdk_display);

  gdk_monitor_get_workarea(display->tme_gdk_display_monitor, &workarea);

  if(GDK_IS_MONITOR(display->tme_gdk_display_monitor) &&
     workarea.width &&
     workarea.height) {
    display->display.tme_screen_width = workarea.width;    
    display->display.tme_screen_height = workarea.height;
  }
#endif

  /* set the display-specific functions: */
  display->display.tme_display_bell = _tme_gtk_display_bell;
  display->display.tme_display_update = _tme_gtk_display_update;
  display->display.tme_screen_add = _tme_gtk_screen_new;
  display->display.tme_screen_resize = _tme_gtk_screen_resize;
  display->display.tme_screen_redraw = _tme_gtk_screen_redraw;

  /* setup the thread loop function: */
  //tme_threads_init(NULL, gtk_main);

  /* unlock mutex once gtk main thread is running: */
  //  g_idle_add(_tme_gtk_screens_update, display);

  return rc;
}
