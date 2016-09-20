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

static _tme_inline void _tme_gtk_main_iter(void) {
  while (gtk_events_pending ())
    gtk_main_iteration ();
  //  gtk_main_iteration_do(FALSE);
}

/* the GTK screens update thread: */
int
_tme_gtk_screen_update(void *disp)
{
  struct tme_gtk_display *display;
  struct tme_gtk_screen *screen;
  struct tme_fb_connection *conn_fb;
  struct tme_fb_connection *conn_fb_other;
  int changed;
  int rc;
#ifdef DEBUG_CAIRO
  tme_uint32_t *i;
  tme_uint32_t last_i, j;
#endif

  _tme_threads_main_iter(_tme_gtk_main_iter);

  display = (struct tme_gtk_display *)disp;
  
  _tme_thread_resumed();

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* loop over all screens: */
  for (screen = display->tme_gtk_display_screens;
       screen != NULL;
       screen = screen->tme_gtk_screen_next) {

    /* skip this screen if it's unconnected: */
    if ((conn_fb = screen->tme_gtk_screen_fb) == NULL) {
      continue;
    }

    /* get the other side of this connection: */
    conn_fb_other = (struct tme_fb_connection *)conn_fb->tme_fb_connection.tme_connection_other;

    /* if the framebuffer has an update function, call it: */
    if (conn_fb_other->tme_fb_connection_update != NULL) {
      rc = (*conn_fb_other->tme_fb_connection_update)(conn_fb_other);
      assert (rc == TME_OK);
    }

    /* if this framebuffer needs a full redraw: */
    if (screen->tme_gtk_screen_full_redraw) {

      /* force the next translation to retranslate the entire buffer: */
      tme_fb_xlat_redraw(conn_fb_other);
      conn_fb_other->tme_fb_connection_offset_updated_first = 0;
      conn_fb_other->tme_fb_connection_offset_updated_last = 0 - (tme_uint32_t) 1;

      /* clear the full redraw flag: */
      screen->tme_gtk_screen_full_redraw = FALSE;
    }

    changed = FALSE;
    if (screen->tme_gtk_screen_fb_xlat) {
      cairo_surface_flush(screen->tme_gtk_screen_surface);

      /* translate this framebuffer's contents: */
      changed = (*screen->tme_gtk_screen_fb_xlat)(conn_fb_other, conn_fb);
    } 

    /* if those contents changed, redraw the widget: */
    if (changed) {
#ifdef DEBUG_CAIRO
      i = (tme_uint32_t *)conn_fb->tme_fb_connection_buffer;
      last_i = *i;
      printf("%8x: %8x\n", i, last_i);
      for(j=0;j<conn_fb->tme_fb_connection_buffsz;j+=sizeof(tme_uint32_t)) {
	if(last_i != *i) { last_i = *i; printf("%8x: %8x\n", i, last_i); }
	i++;
      }
#endif
      cairo_surface_mark_dirty(screen->tme_gtk_screen_surface);
      gtk_widget_queue_draw(screen->tme_gtk_screen_gtkframe);
    }
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  _tme_thread_suspended();
  return (TME_OK);
}

/* the (default) GTK screens update thread: */
_tme_thret
_tme_gtk_screen_th_update(struct tme_gtk_display *display)
{
  /* loop forever: */
  for (;;) {
    _tme_gtk_screen_update(display);

    /* update again in .5 seconds:
    _tme_thread_resumed();
    tme_thread_sleep(0, 500000);
    _tme_thread_suspended();
    */
  }
  _tme_thread_resumed();
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

/* set the translation function to use for this screen */
static void
_tme_gtk_screen_xlat_set(const struct tme_fb_connection *conn_fb, 
			 struct tme_gtk_screen *screen) {
  const struct tme_fb_connection *conn_fb_other;
  struct tme_fb_xlat fb_xlat_q;
  const struct tme_fb_xlat *fb_xlat_a;
  int scale;
  struct tme_gtk_display *display;
  
  scale = screen->tme_gtk_screen_fb_scale;
  if (scale < 0) scale = -scale;
  conn_fb_other = (const struct tme_fb_connection *) conn_fb->tme_fb_connection.tme_connection_other;
  
  /* compose the framebuffer translation question: */
  fb_xlat_q.tme_fb_xlat_width			= conn_fb_other->tme_fb_connection_width;
  fb_xlat_q.tme_fb_xlat_height			= conn_fb_other->tme_fb_connection_height;
  fb_xlat_q.tme_fb_xlat_scale			= scale;
  fb_xlat_q.tme_fb_xlat_src_depth		= conn_fb_other->tme_fb_connection_depth;
  fb_xlat_q.tme_fb_xlat_src_bits_per_pixel	= conn_fb_other->tme_fb_connection_bits_per_pixel;
  fb_xlat_q.tme_fb_xlat_src_skipx		= conn_fb_other->tme_fb_connection_skipx;
  fb_xlat_q.tme_fb_xlat_src_scanline_pad	= conn_fb_other->tme_fb_connection_scanline_pad;
  fb_xlat_q.tme_fb_xlat_src_order		= conn_fb_other->tme_fb_connection_order;
  fb_xlat_q.tme_fb_xlat_src_class		= conn_fb_other->tme_fb_connection_class;
  fb_xlat_q.tme_fb_xlat_src_map			= (conn_fb_other->tme_fb_connection_map_g != NULL
						   ? TME_FB_XLAT_MAP_INDEX
						   : TME_FB_XLAT_MAP_LINEAR);
  fb_xlat_q.tme_fb_xlat_src_map_bits		= conn_fb_other->tme_fb_connection_map_bits;
  fb_xlat_q.tme_fb_xlat_src_mask_g		= conn_fb_other->tme_fb_connection_mask_g;
  fb_xlat_q.tme_fb_xlat_src_mask_r		= conn_fb_other->tme_fb_connection_mask_r;
  fb_xlat_q.tme_fb_xlat_src_mask_b		= conn_fb_other->tme_fb_connection_mask_b;
  fb_xlat_q.tme_fb_xlat_dst_depth		= conn_fb->tme_fb_connection_depth;
  fb_xlat_q.tme_fb_xlat_dst_bits_per_pixel	= conn_fb->tme_fb_connection_bits_per_pixel;
  fb_xlat_q.tme_fb_xlat_dst_skipx		= conn_fb->tme_fb_connection_skipx;
  fb_xlat_q.tme_fb_xlat_dst_scanline_pad	= conn_fb->tme_fb_connection_scanline_pad;
  fb_xlat_q.tme_fb_xlat_dst_order		= conn_fb->tme_fb_connection_order;
  fb_xlat_q.tme_fb_xlat_dst_map			= (conn_fb->tme_fb_connection_map_g != NULL
						   ? TME_FB_XLAT_MAP_INDEX
						   : TME_FB_XLAT_MAP_LINEAR);
  fb_xlat_q.tme_fb_xlat_dst_mask_g		= conn_fb->tme_fb_connection_mask_g;
  fb_xlat_q.tme_fb_xlat_dst_mask_r		= conn_fb->tme_fb_connection_mask_r;
  fb_xlat_q.tme_fb_xlat_dst_mask_b		= conn_fb->tme_fb_connection_mask_b;

  /* ask the framebuffer translation question: */
  fb_xlat_a = tme_fb_xlat_best(&fb_xlat_q);

  display = screen->tme_gtk_screen_display;

  /* if this translation isn't optimal, log a note: */
  if (!tme_fb_xlat_is_optimal(fb_xlat_a)) {
    tme_log(&display->tme_gtk_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_gtk_display_element->tme_element_log_handle,
	     _("no optimal framebuffer translation function available")));
  }

  /* save the translation function: */
  screen->tme_gtk_screen_fb_xlat = fb_xlat_a->tme_fb_xlat_func;
}

/* this is called for a mode change: */
int
_tme_gtk_screen_mode_change(struct tme_fb_connection *conn_fb)
{
  struct tme_gtk_display *display;
  struct tme_gtk_screen *screen;
  struct tme_fb_connection *conn_fb_other;
  unsigned long fb_area, avail_area, percentage;
  int width, height;
  int height_extra, scale;
  const void *map_g_old;
  const void *map_r_old;
  const void *map_b_old;
  const tme_uint32_t *map_pixel_old;
  tme_uint32_t map_pixel_count_old;  
  tme_uint32_t colorset;
  tme_uint32_t color_count;
  struct tme_fb_color *colors_tme;
  gboolean config;

  /* recover our data structures: */
  display = conn_fb->tme_fb_connection.tme_connection_element->tme_element_private;
  conn_fb_other = (struct tme_fb_connection *) conn_fb->tme_fb_connection.tme_connection_other;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* find the screen that this framebuffer connection references: */
  for (screen = display->tme_gtk_display_screens;
       (screen != NULL
	&& screen->tme_gtk_screen_fb != conn_fb);
       screen = screen->tme_gtk_screen_next);
  assert (screen != NULL);

  /* if the user hasn't specified a scaling, pick one: */
  scale = screen->tme_gtk_screen_fb_scale;
  if (scale < 0) {

    /* calulate the areas, in square pixels, of the emulated
       framebuffer and the host's screen: */
    fb_area = (conn_fb_other->tme_fb_connection_width
	       * conn_fb_other->tme_fb_connection_height);
    avail_area = (gdk_screen_width()
		  * gdk_screen_height());

    /* see what percentage of the host's screen would be taken up by
       an unscaled emulated framebuffer: */
    percentage = (fb_area * 100) / avail_area;

    /* if this is at least 70%, halve the emulated framebuffer, else
       if this is 30% or less, double the emulated framebuffer: */
    if (percentage >= 70) {
      scale = TME_FB_XLAT_SCALE_HALF;
    }
    else if (percentage <= 30) {
      scale = TME_FB_XLAT_SCALE_DOUBLE;
    }
    else {
      scale = TME_FB_XLAT_SCALE_NONE;
    }

    screen->tme_gtk_screen_fb_scale = -scale;
  }

  /* get the required dimensions for the Gtkframe: */
  width = ((conn_fb_other->tme_fb_connection_width
	    * scale)
	   / TME_FB_XLAT_SCALE_NONE);
  height = ((conn_fb_other->tme_fb_connection_height
	     * scale)
	    / TME_FB_XLAT_SCALE_NONE);
  /* NB: we need to allocate an extra scanline's worth (or, if we're
     doubling, an extra two scanlines' worth) of image, because the
     framebuffer translation functions can sometimes overtranslate
     (see the explanation of TME_FB_XLAT_RUN in fb-xlat-auto.sh): */
  height_extra
    = (scale == TME_FB_XLAT_SCALE_DOUBLE
       ? 2
       : 1);
  
  height += height_extra;

  if((config = 
      (gtk_widget_get_allocated_width(screen->tme_gtk_screen_gtkframe) != width
       || gtk_widget_get_allocated_height(screen->tme_gtk_screen_gtkframe) != height)))
    /* set a minimum size */
    gtk_widget_set_size_request(screen->tme_gtk_screen_gtkframe, width, height);

  /* remember all previously allocated maps and colors, but otherwise
     remove them from our framebuffer structure: */
  map_g_old = conn_fb->tme_fb_connection_map_g;
  map_r_old = conn_fb->tme_fb_connection_map_r;
  map_b_old = conn_fb->tme_fb_connection_map_b;
  map_pixel_old = conn_fb->tme_fb_connection_map_pixel;
  map_pixel_count_old = conn_fb->tme_fb_connection_map_pixel_count;
  conn_fb->tme_fb_connection_map_g = NULL;
  conn_fb->tme_fb_connection_map_r = NULL;
  conn_fb->tme_fb_connection_map_b = NULL;
  conn_fb->tme_fb_connection_map_pixel = NULL;
  conn_fb->tme_fb_connection_map_pixel_count = 0;

  conn_fb->tme_fb_connection_bits_per_pixel = 32;
  conn_fb->tme_fb_connection_depth = 24;
  conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
  conn_fb->tme_fb_connection_mask_g = 0x00ff00;
  conn_fb->tme_fb_connection_mask_b = 0x0000ff;
  conn_fb->tme_fb_connection_mask_r = 0xff0000;
  
  /* get the needed colors: */
  colorset = tme_fb_xlat_colors_get(conn_fb_other, scale, conn_fb, &colors_tme);
  color_count = conn_fb->tme_fb_connection_map_pixel_count;

  /* if we need to allocate colors, but the colorset is not tied to
     the source framebuffer characteristics, and is identical to the
     currently allocated colorset, we can reuse the previously
     allocated maps and colors: */
  if (color_count > 0
      && colorset != TME_FB_COLORSET_NONE
      && colorset == screen->tme_gtk_screen_colorset) {

    /* free the requested color array: */
    tme_free(colors_tme);

    /* restore the previously allocated maps and colors: */
    conn_fb->tme_fb_connection_map_g = map_g_old;
    conn_fb->tme_fb_connection_map_r = map_r_old;
    conn_fb->tme_fb_connection_map_b = map_b_old;
    conn_fb->tme_fb_connection_map_pixel = map_pixel_old;
    conn_fb->tme_fb_connection_map_pixel_count = map_pixel_count_old;
  }


  /* otherwise, we may need to free and/or allocate colors: */
  else {

    /* save the colorset signature: */
    screen->tme_gtk_screen_colorset = colorset;

    /* free any previously allocated maps and colors: */
    if (map_g_old != NULL) {
      tme_free((void *) map_g_old);
    }
    if (map_r_old != NULL) {
      tme_free((void *) map_r_old);
    }
    if (map_b_old != NULL) {
      tme_free((void *) map_b_old);
    }
    if (map_pixel_old != NULL) {
      tme_free((void *) map_pixel_old);
    }

    /* if we need to allocate colors: */
    if (color_count > 0) {
      /* set the needed colors: */
      tme_fb_xlat_colors_set(conn_fb_other, scale, conn_fb, colors_tme);
    }

    /* set the translation function */
    if(!config) _tme_gtk_screen_xlat_set(conn_fb, screen);
  }

  /* force the next translation to do a complete redraw (if not already doing so): */
  screen->tme_gtk_screen_full_redraw = TRUE;

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  /* done: */
  return (TME_OK);
}

/* this sets the screen size: */
static void
_tme_gtk_screen_scale_set(GtkWidget *widget,
			  struct tme_gtk_screen *screen,
			  int scale_new)
{
  struct tme_gtk_display *display;
  int scale_old;
  int rc;

  /* return now if the menu item isn't active: */
  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget)))) {
    return;
  }

  _tme_thread_resumed();

  /* get the display: */
  display = screen->tme_gtk_screen_display;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* get the old scaling and set the new scaling: */
  scale_old = screen->tme_gtk_screen_fb_scale;
  if (scale_old < 0
      && scale_new < 0) {
    scale_new = scale_old;
  }
  screen->tme_gtk_screen_fb_scale = scale_new;

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  /* call the mode change function if the scaling has changed: */
  if (scale_new != scale_old) {
    rc = _tme_gtk_screen_mode_change(screen->tme_gtk_screen_fb);
    assert (rc == TME_OK);
  }
  _tme_thread_suspended();
}

/* this sets the screen scaling to default: */
static void
_tme_gtk_screen_scale_default(GtkWidget *widget,
			      struct tme_gtk_screen *screen)
{
  _tme_gtk_screen_scale_set(widget,
			    screen,
			    -TME_FB_XLAT_SCALE_NONE);
}

/* this sets the screen scaling to half: */
static void
_tme_gtk_screen_scale_half(GtkWidget *widget,
			   struct tme_gtk_screen *screen)
{
  _tme_gtk_screen_scale_set(widget,
			    screen,
			    TME_FB_XLAT_SCALE_HALF);
}

/* this sets the screen scaling to none: */
static void
_tme_gtk_screen_scale_none(GtkWidget *widget,
			   struct tme_gtk_screen *screen)
{
  _tme_gtk_screen_scale_set(widget,
			    screen,
			    TME_FB_XLAT_SCALE_NONE);
}

/* this sets the screen scaling to double: */
static void
_tme_gtk_screen_scale_double(GtkWidget *widget,
			     struct tme_gtk_screen *screen)
{
  _tme_gtk_screen_scale_set(widget,
			    screen,
			    TME_FB_XLAT_SCALE_DOUBLE);
}

/* this creates the Screen scaling submenu: */
static GCallback
_tme_gtk_screen_submenu_scaling(void *_screen,
				struct tme_gtk_display_menu_item *menu_item)
{
  struct tme_gtk_screen *screen;

  screen = (struct tme_gtk_screen *) _screen;
  menu_item->tme_gtk_display_menu_item_widget = NULL;
  switch (menu_item->tme_gtk_display_menu_item_which) {
  case 0:
    menu_item->tme_gtk_display_menu_item_string = _("Default");
    menu_item->tme_gtk_display_menu_item_widget = &screen->tme_gtk_screen_scale_default;
    return (G_CALLBACK(_tme_gtk_screen_scale_default));
  case 1:
    menu_item->tme_gtk_display_menu_item_string = _("Half");
    menu_item->tme_gtk_display_menu_item_widget = &screen->tme_gtk_screen_scale_half;
    return (G_CALLBACK(_tme_gtk_screen_scale_half));
  case 2:
    menu_item->tme_gtk_display_menu_item_string = _("Full");
    return (G_CALLBACK(_tme_gtk_screen_scale_none));
  case 3:
    menu_item->tme_gtk_display_menu_item_string = _("Double");
    return (G_CALLBACK(_tme_gtk_screen_scale_double));
  default:
    break;
  }
  return (NULL);
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
  struct tme_gtk_display *display;
  struct tme_fb_connection *conn_fb;

  _tme_thread_resumed();

  screen = (struct tme_gtk_screen *) _screen;

  /* get the display: */
  display = screen->tme_gtk_screen_display;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  if(screen->tme_gtk_screen_surface == NULL) {
    _tme_gtk_screen_init(widget, screen);

    /* unlock our mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);
    
    _tme_thread_suspended();
    return TRUE;
  }
  
  cairo_surface_destroy(screen->tme_gtk_screen_surface);

  screen->tme_gtk_screen_surface
    = _tme_gtk_screen_create_similar_image(gtk_widget_get_window(screen->tme_gtk_screen_gtkframe),
					   CAIRO_FORMAT_ARGB32,
					   gtk_widget_get_allocated_width(screen->tme_gtk_screen_gtkframe),
					   gtk_widget_get_allocated_height(screen->tme_gtk_screen_gtkframe));

  conn_fb = screen->tme_gtk_screen_fb;

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_width = cairo_image_surface_get_width(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_height = cairo_image_surface_get_height(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_skipx = cairo_image_surface_get_stride(screen->tme_gtk_screen_surface) - conn_fb->tme_fb_connection_width * sizeof(tme_uint32_t);
  conn_fb->tme_fb_connection_scanline_pad = _tme_gtk_scanline_pad(cairo_image_surface_get_stride(screen->tme_gtk_screen_surface));
  conn_fb->tme_fb_connection_order = TME_ENDIAN_NATIVE;
  conn_fb->tme_fb_connection_buffer = cairo_image_surface_get_data(screen->tme_gtk_screen_surface);
  conn_fb->tme_fb_connection_buffsz = cairo_image_surface_get_stride(screen->tme_gtk_screen_surface) * conn_fb->tme_fb_connection_height;
  
  /* set the translation function */
  _tme_gtk_screen_xlat_set(conn_fb, screen);

  /* force the next translation to do a complete redraw: */
  screen->tme_gtk_screen_full_redraw = TRUE;

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  _tme_thread_suspended();

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
  struct tme_gtk_screen *screen;

  screen = (struct tme_gtk_screen *) _screen;
  cairo_set_source_surface(cr, screen->tme_gtk_screen_surface, 0, 0);
  cairo_paint(cr);

  return FALSE;
}

/* this makes a new screen: */
struct tme_gtk_screen *
_tme_gtk_screen_new(struct tme_gtk_display *display)
{
  struct tme_gtk_screen *screen, **_prev;
  GdkDisplay *gdkdisplay;
  GdkDeviceManager *devices;
  GtkWidget *menu_bar;
  GtkWidget *menu;
  GtkWidget *submenu;
  GtkWidget *menu_item;

#define BLANK_SIDE (16 * 8)

  /* create the new screen and link it in: */
  for (_prev = &display->tme_gtk_display_screens;
       (screen = *_prev) != NULL;
       _prev = &screen->tme_gtk_screen_next);
  screen = *_prev = tme_new0(struct tme_gtk_screen, 1);

  /* the backpointer to the display: */
  screen->tme_gtk_screen_display = display;
  
  gdkdisplay = gdk_display_get_default();

  devices = gdk_display_get_device_manager(gdkdisplay);

  screen->tme_gtk_screen_pointer = gdk_device_manager_get_client_pointer(devices);

  /* there is no framebuffer connection yet: */
  screen->tme_gtk_screen_fb = NULL;

  /* the user hasn't specified a scaling yet: */
  screen->tme_gtk_screen_fb_scale
    = -TME_FB_XLAT_SCALE_NONE;

  /* we have no colorset: */
  screen->tme_gtk_screen_colorset = TME_FB_COLORSET_NONE;

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
  gtk_widget_show(menu_bar);

  /* create the Screen menu: */
  menu = gtk_menu_new();

  /* create the Screen scaling submenu: */
  submenu
    = _tme_gtk_display_menu_radio(screen,
				  _tme_gtk_screen_submenu_scaling);

  /* create the Screen scaling submenu item: */
  menu_item = gtk_menu_item_new_with_label(_("Scale"));
  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  /* create the Screen menu bar item, attach the menu to it, and 
     attach the menu bar item to the menu bar: */
  menu_item = gtk_menu_item_new_with_label("Screen");
  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);

  /* create the Gtkframe for the framebuffer area: */
  screen->tme_gtk_screen_gtkframe = gtk_drawing_area_new();

  /* set a minimum size */
  gtk_widget_set_size_request(screen->tme_gtk_screen_gtkframe, BLANK_SIDE, BLANK_SIDE);

  /* pack the Gtkframe into the outer vertical packing box: */
  gtk_box_pack_start(GTK_BOX(screen->tme_gtk_screen_vbox0), 
		     screen->tme_gtk_screen_gtkframe,
		     FALSE, FALSE, 0);

  g_signal_connect(screen->tme_gtk_screen_gtkframe, "draw",
		   G_CALLBACK(_tme_gtk_screen_draw), screen);

  g_signal_connect(screen->tme_gtk_screen_gtkframe, "configure-event",
		   G_CALLBACK(_tme_gtk_screen_configure), screen);

  /* show the top-level window: */
  gtk_widget_show_all(screen->tme_gtk_screen_window);

  /* there is no translation function: */
  screen->tme_gtk_screen_fb_xlat = NULL;

  /* attach the mouse to this screen: */
  _tme_gtk_mouse_attach(screen);

  /* attach the keyboard to this screen: */
  _tme_gtk_keyboard_attach(screen);

  return (screen);
}

/* this breaks a framebuffer connection: */
static int
_tme_gtk_screen_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new framebuffer connection: */
static int
_tme_gtk_screen_connection_make(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_gtk_display *display;
  struct tme_gtk_screen *screen;
  struct tme_fb_connection *conn_fb;
  struct tme_fb_connection *conn_fb_other;
  char title[sizeof(PACKAGE_STRING) + 16];

  /* recover our data structures: */
  display = (struct tme_gtk_display *) conn->tme_connection_element->tme_element_private;
  conn_fb = (struct tme_fb_connection *) conn;

  /* both sides must be framebuffer connections: */
  assert(conn->tme_connection_type
	 == TME_CONNECTION_FRAMEBUFFER);
  assert(conn->tme_connection_other->tme_connection_type
	 == TME_CONNECTION_FRAMEBUFFER);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    //tme_mutex_lock(&display->tme_gtk_display_mutex);

    /* if our initial screen is already connected, make a new screen: */
    screen = display->tme_gtk_display_screens;
    if (screen->tme_gtk_screen_fb != NULL) {
      screen = _tme_gtk_screen_new(display);
    }

    snprintf(title, sizeof(title), "%s (%s)", PACKAGE_STRING, conn->tme_connection_other->tme_connection_element->tme_element_args[0]);
    gtk_window_set_title(GTK_WINDOW(screen->tme_gtk_screen_window), title);

    /* save our connection: */
    screen->tme_gtk_screen_fb = conn_fb;

    /* unlock our mutex: */
    //tme_mutex_unlock(&display->tme_gtk_display_mutex);

    /* call our mode change function: */
    _tme_gtk_screen_mode_change(conn_fb);
  }

  return (TME_OK);
}

/* this makes a new connection side for a GTK screen: */
int
_tme_gtk_screen_connections_new(struct tme_gtk_display *display, 
				struct tme_connection **_conns)
{
  struct tme_fb_connection *conn_fb;
  struct tme_connection *conn;

  /* allocate a new framebuffer connection: */
  conn_fb = tme_new0(struct tme_fb_connection, 1);
  conn = &conn_fb->tme_fb_connection;
  
  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_FRAMEBUFFER;
  conn->tme_connection_score = tme_fb_connection_score;
  conn->tme_connection_make = _tme_gtk_screen_connection_make;
  conn->tme_connection_break = _tme_gtk_screen_connection_break;

  /* fill in the framebuffer connection: */
  conn_fb->tme_fb_connection_mode_change = _tme_gtk_screen_mode_change;

  /* return the connection side possibility: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}
