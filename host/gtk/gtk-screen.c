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

/* we are aware of the problems with gdk_image_new_bitmap, and we cope
   with them, so we define GDK_ENABLE_BROKEN to get its prototype
   under GTK 2: */
#define GDK_ENABLE_BROKEN

/* includes: */
#include "gtk-display.h"
#include <stdlib.h>

/* macros: */

/* the GTK 1.x gtk_image_new function is the GTK 2.x
   gtk_image_new_from_image function: */
#if GTK_MAJOR_VERSION == 1
#define gtk_image_new_from_image gtk_image_new
#endif /* GTK_MAJOR_VERSION == 1 */

/* the GTK screens update thread: */
void
_tme_gtk_screen_th_update(struct tme_gtk_display *display)
{
  struct tme_gtk_screen *screen;
  struct tme_fb_connection *conn_fb_other;
  int changed;
  int rc;
  
  /* loop forever: */
  for (;;) {

    /* lock the mutex: */
    tme_mutex_lock(&display->tme_gtk_display_mutex);

    /* loop over all screens: */
    for (screen = display->tme_gtk_display_screens;
	 screen != NULL;
	 screen = screen->tme_gtk_screen_next) {

      /* skip this screen if it's unconnected: */
      if (screen->tme_gtk_screen_fb == NULL) {
	continue;
      }

      /* get the other side of this connection: */
      conn_fb_other
	= ((struct tme_fb_connection *) 
	   screen->tme_gtk_screen_fb->tme_fb_connection.tme_connection_other);

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

      /* translate this framebuffer's contents: */
      changed = (*screen->tme_gtk_screen_fb_xlat)
	(((struct tme_fb_connection *) 
	  screen->tme_gtk_screen_fb->tme_fb_connection.tme_connection_other),
	 screen->tme_gtk_screen_fb);

      /* if those contents changed, redraw the widget: */
      if (changed) {
	gtk_widget_queue_draw(screen->tme_gtk_screen_gtkimage);
      }
    }

    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);

    /* update again in .5 seconds: */
    tme_thread_sleep_yield(0, 500000);
  }
  /* NOTREACHED */
}

/* this recovers the bits-per-pixel value for a GdkImage: */
static unsigned int
_tme_gtk_gdkimage_bipp(GdkImage *image)
{
  unsigned int bipp, total_bits;

  /* if the bytes per pixel value is greater than one, or if the image
     depth is 8 or greater, just convert the bytes per pixel value to
     bits per pixel: */
  if (image->bpp > 1
      || image->depth >= 8) {
    return (image->bpp * 8);
  }

  /* otherwise, we know that the depth of the image is less than
     eight, and the number of bits per pixel is eight or less: */
  total_bits = image->bpl;
  total_bits *= 8;
  for (bipp = 8;
       bipp > image->depth && (bipp * image->width) > total_bits;
       bipp >>= 1);
  return (bipp);
}

/* this recovers the scanline-pad value for a GdkImage: */
static unsigned int
_tme_gtk_gdkimage_scanline_pad(GdkImage *image)
{

  if ((image->bpl % sizeof(tme_uint32_t)) == 0) {
    return (32);
  }
  if ((image->bpl % sizeof(tme_uint16_t)) == 0) {
    return (16);
  }
  return (8);
}

/* this is called for a mode change: */
int
_tme_gtk_screen_mode_change(struct tme_fb_connection *conn_fb)
{
  struct tme_gtk_display *display;
  struct tme_gtk_screen *screen;
  struct tme_fb_connection *conn_fb_other;
  struct tme_fb_xlat fb_xlat_q;
  const struct tme_fb_xlat *fb_xlat_a;
  int scale;
  unsigned long fb_area, avail_area, percentage;
  gint width, height;
  gint height_extra;
  const void *map_g_old;
  const void *map_r_old;
  const void *map_b_old;
  const tme_uint32_t *map_pixel_old;
  tme_uint32_t map_pixel_count_old;  
  tme_uint32_t colorset;
  GdkImage *gdkimage;
  GdkVisual *visual;
  tme_uint32_t color_count, color_i;
  tme_uint32_t color_count_distinct;
  tme_uint32_t color_j;
  struct tme_fb_color *colors_tme;
  GdkColor *colors_gdk;
  gboolean *success;
  gboolean warned_color_alloc;

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

  /* get the system's default visual: */
  visual = gdk_visual_get_system();

  /* get the required dimensions for the GdkImage: */
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

  /* if the previous gdkimage isn't the right size: */
  gdkimage = screen->tme_gtk_screen_gdkimage;
  if (gdkimage->width != width
      || gdkimage->height != (height + height_extra)) {

    /* allocate a new gdkimage: */
    gdkimage = gdk_image_new(GDK_IMAGE_FASTEST,
			     visual,
			     width,
			     height
			     + height_extra);

    /* set the new image on the image widget: */
    gtk_image_set(GTK_IMAGE(screen->tme_gtk_screen_gtkimage),
		  gdkimage,
		  NULL);

    /* destroy the previous gdkimage and remember the new one: */
    gdk_image_destroy(screen->tme_gtk_screen_gdkimage);
    screen->tme_gtk_screen_gdkimage = gdkimage;
  }

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

  /* update our framebuffer connection: */
  conn_fb->tme_fb_connection_width = width;
  conn_fb->tme_fb_connection_height = height;
  conn_fb->tme_fb_connection_depth = gdkimage->depth;
  conn_fb->tme_fb_connection_bits_per_pixel = _tme_gtk_gdkimage_bipp(gdkimage);
  conn_fb->tme_fb_connection_skipx = 0;
  conn_fb->tme_fb_connection_scanline_pad = _tme_gtk_gdkimage_scanline_pad(gdkimage);
  conn_fb->tme_fb_connection_order = (gdkimage->byte_order == GDK_LSB_FIRST
				      ? TME_ENDIAN_LITTLE
				      : TME_ENDIAN_BIG);
  conn_fb->tme_fb_connection_buffer = gdkimage->mem;
  switch (visual->type) {
  case GDK_VISUAL_STATIC_GRAY: 
  case GDK_VISUAL_GRAYSCALE:
    conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_MONOCHROME;
    break;
  default:
    assert(FALSE);
    /* FALLTHROUGH */
  case GDK_VISUAL_STATIC_COLOR:
  case GDK_VISUAL_PSEUDO_COLOR:
  case GDK_VISUAL_DIRECT_COLOR:
  case GDK_VISUAL_TRUE_COLOR:
    conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
    break;
  }
  switch (visual->type) {
  case GDK_VISUAL_DIRECT_COLOR:
    /* we set the primary maps to anything non-NULL, to indicate that
       primaries are index mapped: */
    conn_fb->tme_fb_connection_map_g = conn_fb;
    conn_fb->tme_fb_connection_map_r = conn_fb;
    conn_fb->tme_fb_connection_map_b = conn_fb;
    /* FALLTHROUGH */
  case GDK_VISUAL_TRUE_COLOR:
    conn_fb->tme_fb_connection_mask_g = visual->green_mask;
    conn_fb->tme_fb_connection_mask_r = visual->red_mask;
    conn_fb->tme_fb_connection_mask_b = visual->blue_mask;
    break;
  default:
    conn_fb->tme_fb_connection_mask_g = 0;
    conn_fb->tme_fb_connection_mask_r = 0;
    conn_fb->tme_fb_connection_mask_b = 0;
    break;
  }

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

      /* recreate the array of GdkColor: */
      colors_gdk = tme_new(GdkColor, map_pixel_count_old);
      color_i = 0;
      do {
	colors_gdk[color_i].pixel = map_pixel_old[color_i];
      } while (++color_i < map_pixel_count_old);

      /* free the colors: */
      gdk_colormap_free_colors(gdk_colormap_get_system(),
			       colors_gdk,
			       map_pixel_count_old);
      tme_free(colors_gdk);
      tme_free((void *) map_pixel_old);
    }

    /* if we need to allocate colors: */
    if (color_count > 0) {

      /* make the GdkColor array, and count the number of distinct colors: */
      colors_gdk = tme_new(GdkColor, color_count * 2);
      color_count_distinct = 0;
      for (color_i = 0; color_i < color_count; color_i++) {
	color_j = colors_tme[color_i].tme_fb_color_pixel;
	colors_gdk[color_j].green = colors_tme[color_i].tme_fb_color_value_g;
	colors_gdk[color_j].red   = colors_tme[color_i].tme_fb_color_value_r;
	colors_gdk[color_j].blue  = colors_tme[color_i].tme_fb_color_value_b;
	if (color_j >= color_count_distinct) {
	  color_count_distinct = color_j + 1;
	}
      }
      success = tme_new(gboolean, color_count_distinct);

      /* allocate exact matches for as many colors as possible: */
      gdk_colormap_alloc_colors(gdk_colormap_get_system(),
				colors_gdk,
				color_count_distinct,
				FALSE,
				FALSE,
				success);

      /* allocate read-only best matches for any colors we failed to
	 allocate exactly: */
      warned_color_alloc = FALSE;
      for (color_i = 0; color_i < color_count; color_i++) {
	color_j = colors_tme[color_i].tme_fb_color_pixel;
	if (!success[color_j]) {
	  if (!gdk_colormap_alloc_color(gdk_colormap_get_system(),
					&colors_gdk[color_j],
					FALSE,
					TRUE)) {
	    if (!warned_color_alloc) {
	      warned_color_alloc = TRUE;
	      tme_log(&display->tme_gtk_display_element->tme_element_log_handle, 0, ENOMEM,
		      (&display->tme_gtk_display_element->tme_element_log_handle,
		       _("could not allocate all colors")));
	    }
	  }
	}
	colors_tme[color_i].tme_fb_color_pixel = colors_gdk[color_j].pixel;
      }

      /* free the arrays used with gdk_colormap_alloc_colors(): */
      tme_free(success);
      tme_free(colors_gdk);

      /* set the needed colors: */
      tme_fb_xlat_colors_set(conn_fb_other, scale, conn_fb, colors_tme);
    }
  }

  /* compose the framebuffer translation question: */
  fb_xlat_q.tme_fb_xlat_width			= conn_fb_other->tme_fb_connection_width;
  fb_xlat_q.tme_fb_xlat_height			= conn_fb_other->tme_fb_connection_height;
  fb_xlat_q.tme_fb_xlat_scale			= (unsigned int) scale;
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

  /* if this translation isn't optimal, log a note: */
  if (!tme_fb_xlat_is_optimal(fb_xlat_a)) {
    tme_log(&display->tme_gtk_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_gtk_display_element->tme_element_log_handle,
	     _("no optimal framebuffer translation function available")));
  }

  /* save the translation function: */
  screen->tme_gtk_screen_fb_xlat = fb_xlat_a->tme_fb_xlat_func;

  /* force the next translation to do a complete redraw: */
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
  if (!GTK_CHECK_MENU_ITEM(GTK_MENU_ITEM(widget))->active) {
    return;
  }

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
static GtkSignalFunc
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
    return (GTK_SIGNAL_FUNC(_tme_gtk_screen_scale_default));
  case 1:
    menu_item->tme_gtk_display_menu_item_string = _("Half");
    menu_item->tme_gtk_display_menu_item_widget = &screen->tme_gtk_screen_scale_half;
    return (GTK_SIGNAL_FUNC(_tme_gtk_screen_scale_half));
  case 2:
    menu_item->tme_gtk_display_menu_item_string = _("Full");
    return (GTK_SIGNAL_FUNC(_tme_gtk_screen_scale_none));
  case 3:
    menu_item->tme_gtk_display_menu_item_string = _("Double");
    return (GTK_SIGNAL_FUNC(_tme_gtk_screen_scale_double));
  default:
    break;
  }
  return (NULL);
}

/* this makes a new screen: */
struct tme_gtk_screen *
_tme_gtk_screen_new(struct tme_gtk_display *display)
{
  struct tme_gtk_screen *screen, **_prev;
  GtkWidget *menu_bar;
  GtkWidget *menu;
  GtkWidget *submenu;
  GtkWidget *menu_item;
  tme_uint8_t *bitmap_data;
  unsigned int y;
#define BLANK_SIDE (16 * 8)

  /* create the new screen and link it in: */
  for (_prev = &display->tme_gtk_display_screens;
       (screen = *_prev) != NULL;
       _prev = &screen->tme_gtk_screen_next);
  screen = *_prev = tme_new0(struct tme_gtk_screen, 1);

  /* the backpointer to the display: */
  screen->tme_gtk_screen_display = display;
  
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
  gtk_window_set_policy(GTK_WINDOW(screen->tme_gtk_screen_window),
			FALSE, FALSE, TRUE);

  /* create the outer vertical packing box: */
  screen->tme_gtk_screen_vbox0
    = gtk_vbox_new(FALSE, 0);

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
  gtk_menu_append(GTK_MENU(menu), menu_item);

  /* create the Screen menu bar item, attach the menu to it, and 
     attach the menu bar item to the menu bar: */
  menu_item = gtk_menu_item_new_with_label("Screen");
  gtk_widget_show(menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);

  /* create an event box for the framebuffer area: */
  screen->tme_gtk_screen_event_box
    = gtk_event_box_new();

  /* pack the event box into the outer vertical packing box: */
  gtk_box_pack_start(GTK_BOX(screen->tme_gtk_screen_vbox0), 
		     screen->tme_gtk_screen_event_box,
		     FALSE, FALSE, 0);

  /* show the event box: */
  gtk_widget_show(screen->tme_gtk_screen_event_box);

  /* create a GdkImage of an alternating-bits area.  we must use
     malloc() here since this memory will end up as part of an XImage,
     and X will call free() on it: */
  bitmap_data = (tme_uint8_t *)
    malloc((BLANK_SIDE * BLANK_SIDE) / 8);
  assert(bitmap_data != NULL);
  for (y = 0;
       y < BLANK_SIDE;
       y++) {
    memset(bitmap_data
	   + (y * BLANK_SIDE / 8),
	   (y & 1
	    ? 0x33
	    : 0xcc),
	   (BLANK_SIDE / 8));
  }
  screen->tme_gtk_screen_gdkimage
    = gdk_image_new_bitmap(gdk_visual_get_system(),
			   bitmap_data,
			   BLANK_SIDE,
			   BLANK_SIDE);

  /* create the GtkImage for the framebuffer area: */
  screen->tme_gtk_screen_gtkimage
    = gtk_image_new_from_image(screen->tme_gtk_screen_gdkimage, NULL);

  /* add the GtkImage to the event box: */
  gtk_container_add(GTK_CONTAINER(screen->tme_gtk_screen_event_box), 
		    screen->tme_gtk_screen_gtkimage);

  /* show the GtkImage: */
  gtk_widget_show(screen->tme_gtk_screen_gtkimage);

  /* show the outer vertical packing box: */
  gtk_widget_show(screen->tme_gtk_screen_vbox0);

  /* show the top-level window: */
  gtk_widget_show(screen->tme_gtk_screen_window);

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

  /* recover our data structures: */
  display = (struct tme_gtk_display *) conn->tme_connection_element->tme_element_private;
  conn_fb = (struct tme_fb_connection *) conn;
  conn_fb_other = (struct tme_fb_connection *) conn->tme_connection_other;

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
    tme_mutex_lock(&display->tme_gtk_display_mutex);

    /* if our initial screen is already connected, make a new screen: */
    screen = display->tme_gtk_display_screens;
    if (screen->tme_gtk_screen_fb != NULL) {
      screen = _tme_gtk_screen_new(display);
    }

    /* save our connection: */
    screen->tme_gtk_screen_fb = conn_fb;

    /* unlock our mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);

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
