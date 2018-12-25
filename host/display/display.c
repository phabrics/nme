/* host/disp/display.c - generic display support: */

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

#include <tme/common.h>

/* includes: */
#include "display.h"


/* the display main thread: */
#ifdef TME_THREADS_SJLJ
static _tme_thret
_tme_display_th_main(void *fn)
{
  tme_thread_enter(NULL);

  for(;tme_sjlj_threads_main_iter(fn););

    /* NOTREACHED */
  tme_thread_exit();
}
#endif

/* the screens update loop: */
static _tme_thret
_tme_display_th_update(void *disp)
{
  struct tme_display *display;
  struct tme_screen *screen;
  struct tme_fb_connection *conn_fb;
  struct tme_fb_connection *conn_fb_other;
  int rc;

  display = (struct tme_display *)disp;
  
  tme_thread_enter(NULL);
  
  for(;;
#ifdef TME_THREADS_SJLJ
      tme_sjlj_yield()
#else
    tme_thread_yield()
#endif
      ) {
    
    /* lock the mutex: */
    tme_mutex_lock(&display->tme_display_mutex);
    
    _tme_display_callout(display, 0);
    
    /* loop over all screens: */
    for (screen = display->tme_display_screens;
	 screen != NULL;
	 screen = screen->tme_screen_next) {

      /* skip this screen if it's unconnected: */
      if (!screen->tme_screen_update &&
	  (conn_fb = screen->tme_screen_fb) &&
	  conn_fb->tme_fb_connection_buffer &&
	  (conn_fb_other = (struct tme_fb_connection *)conn_fb->tme_fb_connection.tme_connection_other) &&
	  conn_fb_other->tme_fb_connection_update) {

	/* if the framebuffer has an update function, call it: */
	rc = (*conn_fb_other->tme_fb_connection_update)(conn_fb_other);
	assert (rc == TME_OK);
      
	if (!screen->tme_screen_fb_xlat) {
	  _tme_screen_xlat_set(screen);
	  /* force the next translation to retranslate the entire buffer: */
	  tme_fb_xlat_redraw(conn_fb_other);
	  conn_fb_other->tme_fb_connection_offset_updated_first = 0;
	  conn_fb_other->tme_fb_connection_offset_updated_last = 0 - (tme_uint32_t) 1;
	}

	/* translate this framebuffer's contents: */
	screen->tme_screen_update = (*screen->tme_screen_fb_xlat)(conn_fb_other, conn_fb);
#ifdef DEBUG_FB
	tme_uint32_t *i;
	tme_uint32_t last_i, j;

	i = (tme_uint32_t *)conn_fb->tme_fb_connection_buffer;
	last_i = *i;
	printf("%8x: %8x\n", i, last_i);
	for(j=0;j<conn_fb->tme_fb_connection_buffsz;j+=sizeof(tme_uint32_t)) {
	  if(last_i != *i) { last_i = *i; printf("%8x: %8x\n", i, last_i); }
	  i++;
	}
#endif
      }
    }
    
    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_display_mutex);

  }
  
  /* NOTREACHED */
  tme_thread_exit();

}

/* the generic display callout function.  it must be called with the mutex locked: */
void
_tme_display_callout(struct tme_display *display,
		     int new_callouts)
{
  struct tme_keyboard_connection *conn_keyboard;
  struct tme_mouse_connection *conn_mouse;
  int callouts, later_callouts;
  unsigned int ctrl;
  int rc;
  
  /* add in any new callouts: */
  display->tme_display_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (display->tme_display_callout_flags
      & TME_DISPLAY_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  display->tme_display_callout_flags
    |= TME_DISPLAY_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = display->tme_display_callout_flags)
	  & TME_DISPLAY_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    display->tme_display_callout_flags
      = (callouts
	 & ~TME_DISPLAY_CALLOUTS_MASK);
    callouts
      &= TME_DISPLAY_CALLOUTS_MASK;

    /* get our keyboard connection: */
    conn_keyboard = display->tme_display_keyboard_connection;

    /* if we need to call out new keyboard control information: */
    if (callouts & TME_DISPLAY_CALLOUT_KEYBOARD_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (!tme_keyboard_buffer_is_empty(display->tme_display_keyboard_buffer)) {
	ctrl |= TME_KEYBOARD_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&display->tme_display_mutex);
      
      /* do the callout: */
      rc = (conn_keyboard != NULL
	    ? ((*conn_keyboard->tme_keyboard_connection_ctrl)
	       (conn_keyboard,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&display->tme_display_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_DISPLAY_CALLOUT_KEYBOARD_CTRL;
      }
    }

    /* get our mouse connection: */
    conn_mouse = display->tme_display_mouse_connection;

    /* if we need to call out new mouse control information: */
    if (callouts & TME_DISPLAY_CALLOUT_MOUSE_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (!tme_mouse_buffer_is_empty(display->tme_display_mouse_buffer)) {
	ctrl |= TME_MOUSE_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&display->tme_display_mutex);
      
      /* do the callout: */
      rc = (conn_mouse != NULL
	    ? ((*conn_mouse->tme_mouse_connection_ctrl)
	       (conn_mouse,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&display->tme_display_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_DISPLAY_CALLOUT_MOUSE_CTRL;
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  display->tme_display_callout_flags = later_callouts;

}

/* set the translation function to use for this screen */
void
_tme_screen_xlat_set(struct tme_screen *screen) {
  struct tme_fb_connection *conn_fb;
  const struct tme_fb_connection *conn_fb_other;
  struct tme_fb_xlat fb_xlat_q;
  const struct tme_fb_xlat *fb_xlat_a;
  int scale;
  struct tme_display *display;
  const void *map_g_old;
  const void *map_r_old;
  const void *map_b_old;
  const tme_uint32_t *map_pixel_old;
  tme_uint32_t map_pixel_count_old;  
  tme_uint32_t colorset;
  tme_uint32_t color_count;
  struct tme_fb_color *colors_tme;
  
  scale = screen->tme_screen_fb_scale;
  if (scale < 0) scale = -scale;
  conn_fb = screen->tme_screen_fb;
  conn_fb_other = (const struct tme_fb_connection *) conn_fb->tme_fb_connection.tme_connection_other;
  
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

  /* get the needed colors: */
  colorset = tme_fb_xlat_colors_get(conn_fb_other, scale, conn_fb, &colors_tme);
  color_count = conn_fb->tme_fb_connection_map_pixel_count;

  /* if we need to allocate colors, but the colorset is not tied to
     the source framebuffer characteristics, and is identical to the
     currently allocated colorset, we can reuse the previously
     allocated maps and colors: */
  if (color_count > 0
      && colorset != TME_FB_COLORSET_NONE
      && colorset == screen->tme_screen_colorset) {

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
    screen->tme_screen_colorset = colorset;

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

  }

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

  display = screen->tme_screen_display;

  /* if this translation isn't optimal, log a note: */
  if (!tme_fb_xlat_is_optimal(fb_xlat_a)) {
    tme_log(&display->tme_display_element->tme_element_log_handle, 0, TME_OK,
	    (&display->tme_display_element->tme_element_log_handle,
	     _("no optimal framebuffer translation function available")));
  }

  /* save the translation function: */
  screen->tme_screen_fb_xlat = fb_xlat_a->tme_fb_xlat_func;
}

/* this is called for a configuration request: */
int
_tme_screen_configure(struct tme_screen *screen)
{
  struct tme_display *display;
  struct tme_fb_connection *conn_fb_other, *conn_fb;
  unsigned long fb_area, percentage;
  int width, height;
  int height_extra, scale;

  /* recover our data structures: */
  display = screen->tme_screen_display;
  conn_fb = screen->tme_screen_fb;
  conn_fb_other = (struct tme_fb_connection *) conn_fb->tme_fb_connection.tme_connection_other;

  /* if the user hasn't specified a scaling, pick one: */
  scale = screen->tme_screen_fb_scale;
  if (scale < 0) {

    /* calulate the areas, in square pixels, of the emulated
       framebuffer and the host's screen: */
    fb_area = (conn_fb_other->tme_fb_connection_width
	       * conn_fb_other->tme_fb_connection_height);

    /* see what percentage of the host's screen would be taken up by
       an unscaled emulated framebuffer: */
    percentage = (fb_area * 100) / display->tme_screen_area;

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

    screen->tme_screen_fb_scale = -scale;
  }

  /* get the required dimensions for the frame: */
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

  /* set the size & translation function */
  screen->tme_screen_fb_xlat = NULL;  
  if(display->tme_screen_set_size(screen, width, height)) {
    conn_fb->tme_fb_connection_buffer = NULL;  
    screen->tme_screen_update = FALSE;
  }
}

/* this is called for a mode change: */
static int
_tme_screen_mode_change(struct tme_fb_connection *conn_fb)
{
  struct tme_display *display;
  struct tme_screen *screen;

  /* recover our data structures: */
  display = conn_fb->tme_fb_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  /* find the screen that this framebuffer connection references: */
  for (screen = display->tme_display_screens;
       (screen != NULL
	&& screen->tme_screen_fb != conn_fb);
       screen = screen->tme_screen_next);
  assert (screen != NULL);

  /* request configuration on actual screen: */
  _tme_screen_configure(screen);
  
  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

  /* done: */
  return (TME_OK);
}

/* this sets the screen size: */
void
_tme_screen_scale_set(struct tme_screen *screen,
		      int scale_new)
{
  struct tme_display *display;
  int scale_old;
  int rc;

  /* get the display: */
  display = screen->tme_screen_display;

  /* lock our mutex: */
  _tme_mutex_lock(&display->tme_display_mutex);

  /* get the old scaling and set the new scaling: */
  scale_old = screen->tme_screen_fb_scale;
  if (scale_old < 0
      && scale_new < 0) {
    scale_new = scale_old;
  }
  screen->tme_screen_fb_scale = scale_new;

  /* call the configuration function if the scaling has changed: */
  if (scale_new != scale_old) {
    rc = _tme_screen_configure(screen);
    assert (rc == TME_OK);
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

}

/* this adds a new screen: */
struct tme_screen *
_tme_screen_add(struct tme_display *display,
		size_t sz,
		struct tme_connection *conn)
{
  struct tme_screen *screen;

  /* allocate screen structure of the given size: */
  screen = tme_malloc0(sz);
  
  /* create the new screen and link it in: */
  screen->tme_screen_next = display->tme_display_screens;
  display->tme_display_screens = screen;

  /* the backpointer to the display: */
  screen->tme_screen_display = display;
  
  /* save our connection: */
  screen->tme_screen_fb = conn;

  /* the user hasn't specified a scaling yet: */
  screen->tme_screen_fb_scale
    = -TME_FB_XLAT_SCALE_NONE;

  /* we have no colorset: */
  screen->tme_screen_colorset = TME_FB_COLORSET_NONE;

  /* there is no translation function: */
  screen->tme_screen_fb_xlat = NULL;

  return (screen);
}

/* this breaks a framebuffer connection: */
static int
_tme_display_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new framebuffer connection: */
static int
_tme_display_connection_make(struct tme_connection *conn,
			    unsigned int state)
{
  struct tme_display *display;
  struct tme_screen *screen;

  /* recover our data structures: */
  display = (struct tme_display *) conn->tme_connection_element->tme_element_private;

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
    //tme_mutex_lock(&display->tme_display_mutex);

    /* if our initial screen is already connected, make a new screen: */
    screen = display->tme_screen_add(display, conn);
    /* unlock our mutex: */
    //tme_mutex_unlock(&display->tme_display_mutex);

    /* call our mode change function: */
    //    _tme_screen_mode_change((struct tme_fb_connection *) conn);
  }

  return (TME_OK);
}

/* this makes a new connection side for a display: */
static int
_tme_display_connections_new(struct tme_element *element, 
			     const char * const *args, 
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_display *display;
  struct tme_fb_connection *conn_fb;
  struct tme_connection *conn;

  /* recover our data structure: */
  display = (struct tme_display *) element->tme_element_private;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* make any new keyboard connections: */
  _tme_keyboard_connections_new(display, _conns);

  /* make any new mouse connections: */
  _tme_mouse_connections_new(display, _conns);

  /* allocate a new framebuffer connection: */
  conn_fb = tme_new0(struct tme_fb_connection, 1);
  conn = &conn_fb->tme_fb_connection;
  
  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_FRAMEBUFFER;
  conn->tme_connection_score = tme_fb_connection_score;
  conn->tme_connection_make = _tme_display_connection_make;
  conn->tme_connection_break = _tme_display_connection_break;

  /* fill in the framebuffer connection: */
  conn_fb->tme_fb_connection_mode_change = _tme_screen_mode_change;

  /* return the connection side possibility: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

/* the new generic display function: */
int tme_display_init(struct tme_element *element) {
  struct tme_display *display;

  /* start our data structure: */
  display = tme_new0(struct tme_display, 1);
  display->tme_display_element = element;

  /* create the keyboard: */
  _tme_keyboard_new(display);

  /* create the mouse: */
  _tme_mouse_new(display);

  /* start the threads: */
  tme_mutex_init(&display->tme_display_mutex);

  /* setup the thread loop function: */
#ifdef TME_THREADS_SJLJ
  tme_sjlj_thread_create(&display->tme_display_sjlj_thread, _tme_display_th_update, display);
  tme_thread_create(&display->tme_display_thread, _tme_display_th_main, NULL);
#else
  tme_thread_create(&display->tme_display_thread, _tme_display_th_update, display);
#endif  
  /* fill the element: */
  element->tme_element_private = display;
  element->tme_element_connections_new = _tme_display_connections_new;

  return (TME_OK);
}
