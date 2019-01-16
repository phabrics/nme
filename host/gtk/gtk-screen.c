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

int tme_sjlj_threads_main _TME_P((void *unused));

static _tme_thret
_tme_gtk_display_th_update(void *disp) {
  struct tme_display *display;
  struct tme_fb_connection *conn_fb;
  struct tme_gtk_screen *screen;  

  display = (struct tme_display *)disp;
  
#ifdef TME_THREADS_SJLJ
  tme_thread_create(&display->tme_display_thread, tme_sjlj_threads_main, NULL);
#endif
  tme_thread_enter(NULL);

  //_tme_thread_suspended();
  
  for(;;) {
    while (gtk_events_pending ())
      gtk_main_iteration ();

    /*      if(gtk_grab_get_current() == screen->tme_gtk_screen_gtkframe)
	_tme_gtk_mouse_warp_pointer(screen);
    */
    /* lock the mutex: */
    if(tme_mutex_trylock(&display->tme_display_mutex)) continue;

    //_tme_thread_resumed();

#ifndef TME_THREADS_SJLJ
    _tme_display_callout(display, 0);
#endif    

    /* loop over all screens: */
    for (screen = display->tme_display_screens;
	 screen != NULL;
	 screen = screen->screen.tme_screen_next) {
#ifndef TME_THREADS_SJLJ
      _tme_screen_update(screen);
#endif
      /* if those contents changed, update the screen: */
      if (screen->screen.tme_screen_update) {
	cairo_surface_flush(screen->tme_gtk_screen_surface);
	cairo_surface_mark_dirty(screen->tme_gtk_screen_surface);
	gtk_widget_queue_draw(screen->tme_gtk_screen_gtkframe);
      }
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&display->tme_display_mutex);

    tme_thread_yield();

    //    _tme_thread_suspended();
  }

  /* NOTREACHED */
  tme_thread_exit();

}

/* this recovers the scanline-pad value for an image buffer: */
static unsigned int
_tme_gtk_scanline_pad(int bpl)
{
  if ((bpl % sizeof(tme_uint32_t)) == 0) {
    return (32);
  }
  if ((bpl % sizeof(tme_uint16_t)) == 0) {
    return (16);
  }
  return (8);
}

/* this sets the screen scaling to default: */
static void
_tme_gtk_screen_scale_default(GtkWidget *widget,
			      struct tme_gtk_screen *screen)
{
  /* return now if the menu item isn't active: */
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    _tme_screen_scale_set(screen,
			  -TME_FB_XLAT_SCALE_NONE);
}

/* this sets the screen scaling to half: */
static void
_tme_gtk_screen_scale_half(GtkWidget *widget,
			   struct tme_gtk_screen *screen)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    _tme_screen_scale_set(screen,
			  TME_FB_XLAT_SCALE_HALF);
}

/* this sets the screen scaling to none: */
static void
_tme_gtk_screen_scale_none(GtkWidget *widget,
			   struct tme_gtk_screen *screen)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    _tme_screen_scale_set(screen,
			  TME_FB_XLAT_SCALE_NONE);
}

/* this sets the screen scaling to double: */
static void
_tme_gtk_screen_scale_double(GtkWidget *widget,
			     struct tme_gtk_screen *screen)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))))
    _tme_screen_scale_set(screen,
			  TME_FB_XLAT_SCALE_DOUBLE);
}

/* this creates the Screen scaling submenu: */
static GCallback
_tme_gtk_screen_submenu_scaling(void *_screen,
				struct tme_display_menu_item *menu_item)
{
  struct tme_gtk_screen *screen;

  screen = (struct tme_gtk_screen *) _screen;
  menu_item->tme_display_menu_item_widget = NULL;
  switch (menu_item->tme_display_menu_item_which) {
  case 0:
    menu_item->tme_display_menu_item_string = _("Default");
    menu_item->tme_display_menu_item_widget = &screen->tme_gtk_screen_scale_default;
    return (G_CALLBACK(_tme_gtk_screen_scale_default));
  case 1:
    menu_item->tme_display_menu_item_string = _("Half");
    menu_item->tme_display_menu_item_widget = &screen->tme_gtk_screen_scale_half;
    return (G_CALLBACK(_tme_gtk_screen_scale_half));
  case 2:
    menu_item->tme_display_menu_item_string = _("Full");
    return (G_CALLBACK(_tme_gtk_screen_scale_none));
  case 3:
    menu_item->tme_display_menu_item_string = _("Double");
    return (G_CALLBACK(_tme_gtk_screen_scale_double));
  default:
    break;
  }
  return (NULL);
}

/* Screen-specific size request */
static int _tme_gtk_screen_set_size(struct tme_gtk_screen *screen,
				    int width,
				    int height) {
  int config;
  struct tme_fb_connection *conn_fb = screen->screen.tme_screen_fb;

  /* update our framebuffer connection: */
  config = (conn_fb->tme_fb_connection_width != width ||
	    conn_fb->tme_fb_connection_height != height);

  if(config)
    /* set a minimum size */
    gtk_widget_set_size_request(screen->tme_gtk_screen_gtkframe, width, height);
  
  return config;
}

/* Create a similar image surface to the screen's target surface (i.e., backing store) */
static inline cairo_surface_t *
_tme_gtk_screen_create_similar_image(GdkWindow *window,
				     cairo_format_t format,
				     int width,
				     int height) 
{
  cairo_surface_t *surface;

#if GTK_MINOR_VERSION < 10
  surface = 
    cairo_image_surface_create(format,
			       width,
			       height);
#else
  surface = 
    gdk_window_create_similar_image_surface(window,
					    format,
					    width,
					    height,
					    0);
#endif
    return surface;
}

/* Create a new surface of the appropriate size to store our scribbles */
static void
_tme_gtk_screen_init(GtkWidget         *widget,
		     struct tme_gtk_screen *screen)
{
  tme_uint8_t *bitmap_data;
  int bitmap_width, bitmap_height;
  unsigned int y;

  screen->tme_gtk_screen_surface = 
    _tme_gtk_screen_create_similar_image(gtk_widget_get_window(widget),
					 CAIRO_FORMAT_A1,
					 gtk_widget_get_allocated_width(widget),
					 gtk_widget_get_allocated_height(widget));

  /* create an image surface of an alternating-bits area. */
  bitmap_data = cairo_image_surface_get_data(screen->tme_gtk_screen_surface);
  assert(bitmap_data != NULL);
  bitmap_width = cairo_image_surface_get_width(screen->tme_gtk_screen_surface) / 8;
  bitmap_height = cairo_image_surface_get_height(screen->tme_gtk_screen_surface);
  cairo_surface_flush(screen->tme_gtk_screen_surface);
  for (y = 0;
       y < bitmap_height;
       y++) {
    memset(bitmap_data + y * bitmap_width,
	   (y & 1
	    ? 0x33
	    : 0xcc),
	   bitmap_width);
  }
  cairo_surface_mark_dirty(screen->tme_gtk_screen_surface);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
_tme_gtk_screen_configure(GtkWidget         *widget,
			  GdkEventConfigure *event,
			  gpointer          _screen)
{
  struct tme_gtk_screen *screen;
  struct tme_display *display;
  struct tme_fb_connection *conn_fb;
  GdkWindow *window;
  int scale;
  
  screen = (struct tme_gtk_screen *) _screen;

  /* get the display: */
  display = screen->screen.tme_screen_display;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  cairo_surface_destroy(screen->tme_gtk_screen_surface);

  window = gtk_widget_get_window(screen->tme_gtk_screen_gtkframe);
  
  screen->screen.tme_screen_scale = gdk_window_get_scale_factor(window);
  
  screen->tme_gtk_screen_surface
    = gdk_window_create_similar_image_surface(window,
					      CAIRO_FORMAT_ARGB32,
					      gdk_window_get_width(window) * screen->screen.tme_screen_scale,
					      gdk_window_get_height(window) * screen->screen.tme_screen_scale,
					      screen->screen.tme_screen_scale);

  conn_fb = screen->screen.tme_screen_fb;

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_width = cairo_image_surface_get_width(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_height = cairo_image_surface_get_height(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_skipx = cairo_image_surface_get_stride(screen->tme_gtk_screen_surface) - conn_fb->tme_fb_connection_width * sizeof(tme_uint32_t);
  conn_fb->tme_fb_connection_scanline_pad = _tme_gtk_scanline_pad(cairo_image_surface_get_stride(screen->tme_gtk_screen_surface));
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
  return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
_tme_gtk_screen_draw(GtkWidget *widget,
		     cairo_t   *cr,
		     gpointer   _screen)
{
  struct tme_display *display;
  struct tme_gtk_screen *screen;

  screen = (struct tme_gtk_screen *) _screen;

  /* get the display: */
  display = screen->screen.tme_screen_display;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  cairo_set_source_surface(cr, screen->tme_gtk_screen_surface, 0, 0);
  cairo_paint(cr);
  screen->screen.tme_screen_update = FALSE;    

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

  return FALSE;
}

/* this makes a new screen: */
struct tme_gtk_screen *
_tme_gtk_screen_new(struct tme_gdk_display *display,
		    struct tme_connection *conn)
{
  struct tme_gtk_screen *screen;
  GdkDisplay *gdkdisplay;
  GdkDeviceManager *devices;
  GdkRectangle workarea;
  GtkWidget *menu_bar;
  GtkWidget *menu;
  GtkWidget *submenu;
  GtkWidget *menu_item;
  char title[sizeof(PACKAGE_STRING) + 16];

  /* lock our mutex: */
  tme_mutex_lock(&display->display.tme_display_mutex);

  screen = tme_screen_new(display, struct tme_gtk_screen, conn);

  display->tme_gdk_display = gdk_display_get_default();

  display->tme_gdk_display_cursor
    = gdk_cursor_new_for_display(display->tme_gdk_display, GDK_BLANK_CURSOR);

  display->tme_gdk_display_seat = gdk_display_get_default_seat(display->tme_gdk_display);

  display->tme_gdk_display_monitor = gdk_display_get_primary_monitor(display->tme_gdk_display);

  gdk_monitor_get_workarea(display->tme_gdk_display_monitor, &workarea);

  display->display.tme_screen_width = workarea.width;
  display->display.tme_screen_height = workarea.height;
  screen->screen.tme_screen_scale = gdk_monitor_get_scale_factor(display->tme_gdk_display_monitor);
  
  /* create the top-level window, and allow it to shrink, grow,
     and auto-shrink: */
  screen->tme_gtk_screen_window
    = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_resizable(GTK_WINDOW(screen->tme_gtk_screen_window), FALSE);

  /* create the outer vertical packing box: */
  screen->tme_gtk_screen_vbox0
    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  /* add the outer vertical packing box to the window: */
  gtk_container_add(GTK_CONTAINER(screen->tme_gtk_screen_window),
		    screen->tme_gtk_screen_vbox0);

  /* create the menu bar and pack it into the outer vertical packing
     box: */
  menu_bar = gtk_menu_bar_new ();
  gtk_box_pack_start(GTK_BOX(screen->tme_gtk_screen_vbox0), 
		     menu_bar,
		     FALSE, FALSE, 0);
  //  gtk_widget_show(menu_bar);

  /* create the Screen menu: */
  menu = gtk_menu_new();

  /* create the Screen scaling submenu: */
  submenu
    = _tme_display_menu_radio(screen,
				  _tme_gtk_screen_submenu_scaling);

  /* create the Screen scaling submenu item: */
  menu_item = gtk_menu_item_new_with_label(_("Scale"));
  //  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  /* create the Screen menu bar item, attach the menu to it, and 
     attach the menu bar item to the menu bar: */
  menu_item = gtk_menu_item_new_with_label("Screen");
  //  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

  /* create the Gtkframe for the framebuffer area: */
  screen->tme_gtk_screen_gtkframe = gtk_drawing_area_new();
  
  /* new a minimum size */
  //_tme_gtk_screen_set_size(screen, BLANK_SIDE, BLANK_SIDE);
  //  _tme_gtk_screen_init(screen->tme_gtk_screen_gtkframe, screen);
  
  _tme_screen_configure(screen);

  /* pack the Gtkframe into the outer vertical packing box: */
  gtk_box_pack_start(GTK_BOX(screen->tme_gtk_screen_vbox0), 
		     screen->tme_gtk_screen_gtkframe,
		     FALSE, FALSE, 0);

  g_signal_connect(screen->tme_gtk_screen_gtkframe, "draw",
		   G_CALLBACK(_tme_gtk_screen_draw), screen);

  g_signal_connect(screen->tme_gtk_screen_gtkframe, "configure-event",
		   G_CALLBACK(_tme_gtk_screen_configure), screen);

  /* attach the mouse to this screen: */
  _tme_gtk_mouse_attach(screen);

  /* attach the keyboard to this screen: */
  _tme_gtk_keyboard_attach(screen);

  snprintf(title, sizeof(title), "%s (%s)", PACKAGE_STRING, conn->tme_connection_other->tme_connection_element->tme_element_args[0]);
  gtk_window_set_title(GTK_WINDOW(screen->tme_gtk_screen_window), title);
  
  /* unlock our mutex: */
  tme_mutex_unlock(&display->display.tme_display_mutex);

  /* show the top-level window: */
  gtk_widget_show_all(screen->tme_gtk_screen_window);

  return (screen);
}

static void _tme_gtk_init(void) {
  char **argv;
  char *argv_buffer[3];
  int argc;

  /* GTK requires program to be running non-setuid */
#ifdef HAVE_SETUID
  setuid(getuid());
#endif
  
  /* conjure up an argv.  this is pretty bad: */
  argv = argv_buffer;
  argc = 0;
  argv[argc++] = "tmesh";
#if 1
  argv[argc++] = "--gtk-debug=signals";
#endif
  argv[argc] = NULL;
  gtk_init(&argc, &argv);
}

/* this is a GTK callback for an enter notify event, that has the
   widget grab focus and then continue normal event processing: */
gint
_tme_display_enter_focus(GtkWidget *widget,
			 GdkEvent *gdk_event_raw,
			 gpointer junk)
{

  /* grab the focus: */
  gtk_widget_grab_focus(widget);

  /* continue normal event processing: */
  return (FALSE);
}

/* this creates a menu of radio buttons: */
GtkWidget *
_tme_display_menu_radio(void *state,
			tme_display_menu_items_t menu_items)
{
  GtkWidget *menu;
  GSList *menu_group;
  struct tme_display_menu_item menu_item_buffer;
  GCallback menu_func;
  GtkWidget *menu_item;

  /* create the menu: */
  menu = gtk_menu_new();

  /* create the menu items: */
  menu_group = NULL;
  for (menu_item_buffer.tme_display_menu_item_which = 0;
       ;
       menu_item_buffer.tme_display_menu_item_which++) {
    menu_func = (*menu_items)(state, &menu_item_buffer);
    if (menu_func == G_CALLBACK(NULL)) {
      break;
    }
    menu_item
      = gtk_radio_menu_item_new_with_label(menu_group,
					   menu_item_buffer.tme_display_menu_item_string);
    if (menu_item_buffer.tme_display_menu_item_widget != NULL) {
      *menu_item_buffer.tme_display_menu_item_widget = menu_item;
    }
    menu_group
      = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
    g_signal_connect(menu_item, 
		     "activate",
		     menu_func,
		     (gpointer) state);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    //    gtk_widget_show(menu_item);
  }

  /* return the menu: */
  return (menu);
}

/* the new GTK display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_gtk,display) {
  struct tme_display *display;

  /* start our data structure: */
  display = tme_new0(struct tme_gdk_display, 1);
  tme_display_init(element, display);

  /* recover our data structure: */
  display = element->tme_element_private;

  _tme_gtk_init();
  
  /* set the display-specific functions: */
  display->tme_screen_add = _tme_gtk_screen_new;
  display->tme_screen_set_size = _tme_gtk_screen_set_size;

  /* setup the thread loop function: */
  //  tme_threads_init(NULL, gtk_main);

  /* unlock mutex once gtk main thread is running: */
  //  g_idle_add(_tme_gtk_screens_update, display);

  tme_threads_init(_tme_gtk_display_th_update, display);
  
  return (TME_OK);
}
