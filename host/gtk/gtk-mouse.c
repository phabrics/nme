/* $Id: gtk-mouse.c,v 1.3 2007/03/03 15:33:22 fredette Exp $ */

/* host/gtk/gtk-mouse.c - GTK mouse support: */

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
_TME_RCSID("$Id: gtk-mouse.c,v 1.3 2007/03/03 15:33:22 fredette Exp $");

/* includes: */
#include "gtk-display.h"
#include <gdk/gdkkeysyms.h>

/* macros: */
#define TME_GTK_MOUSE_CURSOR_WIDTH	(16)
#define TME_GTK_MOUSE_CURSOR_HEIGHT	(16)

/* types: */

/* globals: */

/* our invisible cursor: */
static const gchar _tme_gtk_mouse_cursor_source[(TME_GTK_MOUSE_CURSOR_WIDTH
						 * TME_GTK_MOUSE_CURSOR_HEIGHT
						 / 8)] = 
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const gchar _tme_gtk_mouse_cursor_mask[(TME_GTK_MOUSE_CURSOR_WIDTH
					       * TME_GTK_MOUSE_CURSOR_HEIGHT
					       / 8)] = 
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static GdkColor _tme_gtk_mouse_cursor_color = { 0, 0, 0, 0 };

/* if X11 support is present: */
#ifndef X_DISPLAY_MISSING

/* includes: */
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#if TME_MOUSE_EVENT_TIME_UNDEF != CurrentTime
#error "TME_MOUSE_EVENT_TIME_UNDEF and CurrentTime disagree"
#endif

/* this warps the pointer to the middle of the GtkImage: */
static void
_tme_gtk_mouse_warp_pointer(struct tme_gtk_screen *screen)
{
  XWarpPointer(GDK_DISPLAY(),
	       None, 
	       GDK_WINDOW_XWINDOW(screen->tme_gtk_screen_gtkimage->window),
	       0, 0, 0, 0,
	       screen->tme_gtk_screen_mouse_warp_x,
	       screen->tme_gtk_screen_mouse_warp_y);
}

#endif /* !X_DISPLAY_MISSING */

/* this is for debugging only: */
#if 0
#include <stdio.h>
void
_tme_gtk_mouse_debug(const struct tme_mouse_event *event)
{
  fprintf(stderr,
	  "buttons = 0x%02x dx=%d dy=%d\n",
	  event->tme_mouse_event_buttons,
	  event->tme_mouse_event_delta_x,
	  event->tme_mouse_event_delta_y);
}
#else
#define _tme_gtk_mouse_debug(e) do { } while (/* CONSTCOND */ 0)
#endif

/* this is a GTK callback for a mouse event in the framebuffer event box: */
static int
_tme_gtk_mouse_mouse_event(GtkWidget *widget,
			   GdkEvent *gdk_event_raw,
			   struct tme_gtk_screen *screen)
{
  struct tme_gtk_display *display;
  struct tme_mouse_event tme_event;
  gint x;
  gint y;
  guint state, button;
  unsigned int buttons;
  int was_empty;
  int new_callouts;
  int rc;

  /* start the tme event: */
  tme_event.tme_mouse_event_delta_units
    = TME_MOUSE_UNITS_UNKNOWN;

  /* recover our data structure: */
  display = screen->tme_gtk_screen_display;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* if this is motion: */
  if (gdk_event_raw->type == GDK_MOTION_NOTIFY) {

    /* this event must have happened in the gtkimage: */
    assert (gdk_event_raw->motion.window
	    == screen->tme_gtk_screen_gtkimage->window);

    /* set the event time: */
    tme_event.tme_mouse_event_time
      = gdk_event_raw->motion.time;

    /* the buttons haven't changed: */
    tme_event.tme_mouse_event_buttons =
      screen->tme_gtk_screen_mouse_buttons_last;

    /* if the pointer position hasn't changed either, return now.
       every time we warp the pointer we will get a motion event, and
       this should ignore those events: */
    x = gdk_event_raw->motion.x;
    y = gdk_event_raw->motion.y;
    if (x == screen->tme_gtk_screen_mouse_warp_x
	&& y == screen->tme_gtk_screen_mouse_warp_y) {
      
      /* unlock the mutex: */
      tme_mutex_unlock(&display->tme_gtk_display_mutex);

      /* stop propagating this event: */
      return (TRUE);
    }

    /* warp the pointer back to center: */
    _tme_gtk_mouse_warp_pointer(screen);
  }

  /* otherwise, if this is a double- or triple-click: */
  else if (gdk_event_raw->type == GDK_2BUTTON_PRESS
	   || gdk_event_raw->type == GDK_3BUTTON_PRESS) {

    /* we ignore double- and triple-click events, since normal button
       press and release events are always generated also: */
      
    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);

    /* stop propagating this event: */
    return (TRUE);
  }

  /* otherwise, this must be a button press or a release: */
  else {
    assert (gdk_event_raw->type == GDK_BUTTON_PRESS
	    || gdk_event_raw->type == GDK_BUTTON_RELEASE);

    /* this event must have happened in the gtkimage: */
    assert (gdk_event_raw->button.window
	    == screen->tme_gtk_screen_gtkimage->window);

    /* set the event time: */
    tme_event.tme_mouse_event_time
      = gdk_event_raw->button.time;

    /* get the pointer position: */
    x = gdk_event_raw->button.x;
    y = gdk_event_raw->button.y;

    /* make the buttons mask: */
    buttons = 0;
    button = gdk_event_raw->button.button;
    state = gdk_event_raw->button.state;
#define _TME_GTK_MOUSE_BUTTON(i, gdk, tme)		\
  if ((button == i)					\
      ? (gdk_event_raw->type == GDK_BUTTON_PRESS)	\
      : (state & gdk))					\
      buttons |= tme
    _TME_GTK_MOUSE_BUTTON(1, GDK_BUTTON1_MASK, TME_MOUSE_BUTTON_0);
    _TME_GTK_MOUSE_BUTTON(2, GDK_BUTTON2_MASK, TME_MOUSE_BUTTON_1);
    _TME_GTK_MOUSE_BUTTON(3, GDK_BUTTON3_MASK, TME_MOUSE_BUTTON_2);
    _TME_GTK_MOUSE_BUTTON(4, GDK_BUTTON4_MASK, TME_MOUSE_BUTTON_3);
    _TME_GTK_MOUSE_BUTTON(5, GDK_BUTTON5_MASK, TME_MOUSE_BUTTON_4);
#undef _TME_GTK_MOUSE_BUTTON
    tme_event.tme_mouse_event_buttons = buttons;
    screen->tme_gtk_screen_mouse_buttons_last = buttons;
  }

  /* make the deltas: */
  tme_event.tme_mouse_event_delta_x = 
    (((int) x)
     - ((int) screen->tme_gtk_screen_mouse_warp_x));
  tme_event.tme_mouse_event_delta_y = 
    (((int) y)
     - ((int) screen->tme_gtk_screen_mouse_warp_y));

  /* assume that we won't need any new callouts: */
  new_callouts = 0;
  
  /* remember if the mouse buffer was empty: */
  was_empty
    = tme_mouse_buffer_is_empty(display->tme_gtk_display_mouse_buffer);

  /* add this tme event to the mouse buffer: */
  _tme_gtk_mouse_debug(&tme_event);
  rc = tme_mouse_buffer_copyin(display->tme_gtk_display_mouse_buffer,
			       &tme_event);
  assert (rc == TME_OK);

  /* if the mouse buffer was empty and now it isn't,
     call out the mouse controls: */
  if (was_empty
      && !tme_mouse_buffer_is_empty(display->tme_gtk_display_mouse_buffer)) {
    new_callouts |= TME_GTK_DISPLAY_CALLOUT_MOUSE_CTRL;
  }

  /* run any callouts: */
  _tme_gtk_display_callout(display, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  /* stop propagating this event: */
  return (TRUE);
}

/* this is a GTK callback for an event in the event box containing the
   mouse label: */
static int
_tme_gtk_mouse_ebox_event(GtkWidget *widget,
			  GdkEvent *gdk_event_raw,
			  struct tme_gtk_screen *screen)
{
  struct tme_gtk_display *display;
  int rc;
  gint junk;
  char *status;

  /* if this is an enter notify event, grab the focus and continue
     propagating the event: */
  if (gdk_event_raw->type == GDK_ENTER_NOTIFY) {
    gtk_widget_grab_focus(widget);
    return (FALSE);
  }

  /* if this is not a key press event, continue propagating it now: */
  if (gdk_event_raw->type != GDK_KEY_PRESS) {
    return (FALSE);
  }

  /* recover our data structure: */
  display = screen->tme_gtk_screen_display;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* the mouse must not be on already: */
  assert (screen->tme_gtk_screen_mouse_keyval
	  == GDK_VoidSymbol);

  /* this keyval must not be GDK_VoidSymbol: */
  assert (gdk_event_raw->key.keyval
	  != GDK_VoidSymbol);
  
  /* set the text on the mouse label: */
  gtk_label_set_text(GTK_LABEL(screen->tme_gtk_screen_mouse_label),
		     _("Mouse is on"));
  
  /* push the mouse status onto the statusbar: */
  status = NULL;
  tme_output_append(&status,
		    _("Press the %s key to turn the mouse off"),
		    gdk_keyval_name(gdk_event_raw->key.keyval));
  gtk_statusbar_push(GTK_STATUSBAR(screen->tme_gtk_screen_mouse_statusbar),
		     screen->tme_gtk_screen_mouse_statusbar_cid,
		     status);
  tme_free(status);
  
  /* if the original events mask on the framebuffer event box have
     never been saved, save them now, and add the mouse events: */
  if (screen->tme_gtk_screen_mouse_events_old == 0) {
    screen->tme_gtk_screen_mouse_events_old
      = gdk_window_get_events(screen->tme_gtk_screen_event_box->window);
    gtk_widget_add_events(screen->tme_gtk_screen_event_box,
			  GDK_POINTER_MOTION_MASK
			  | GDK_BUTTON_PRESS_MASK
			  | GDK_BUTTON_RELEASE_MASK);
  }

  /* get the current width and height of the framebuffer gtkimage, and
     halve them to get the warp center: */
  gdk_window_get_geometry(screen->tme_gtk_screen_gtkimage->window,
			  &junk,
			  &junk,
			  &screen->tme_gtk_screen_mouse_warp_x,
			  &screen->tme_gtk_screen_mouse_warp_y,
			  &junk);
  screen->tme_gtk_screen_mouse_warp_x >>= 1;
  screen->tme_gtk_screen_mouse_warp_y >>= 1;
  
  /* warp the pointer to center: */
  _tme_gtk_mouse_warp_pointer(screen);
  
  /* grab the pointer: */
  rc
    = gdk_pointer_grab(screen->tme_gtk_screen_gtkimage->window,
		       TRUE,
		       GDK_POINTER_MOTION_MASK
		       | GDK_BUTTON_PRESS_MASK
		       | GDK_BUTTON_RELEASE_MASK,
		       screen->tme_gtk_screen_gtkimage->window,
		       display->tme_gtk_display_mouse_cursor,
		       gdk_event_raw->key.time);
  assert (rc == 0);
  
  /* we are now in mouse mode: */
  screen->tme_gtk_screen_mouse_keyval
    = gdk_event_raw->key.keyval;

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  /* stop propagating this event: */
  return (TRUE);
}

/* this turns mouse mode off.  it is called with the mutex locked: */
void
_tme_gtk_mouse_mode_off(struct tme_gtk_screen *screen,
			guint32 time)
{
  /* the mouse must be on: */
  assert (screen->tme_gtk_screen_mouse_keyval
	  != GDK_VoidSymbol);

  /* ungrab the pointer: */
  gdk_pointer_ungrab(time);

  /* restore the old events mask on the event box: */
  gdk_window_set_events(screen->tme_gtk_screen_event_box->window,
			screen->tme_gtk_screen_mouse_events_old);

  /* pop our message off of the statusbar: */
  gtk_statusbar_pop(GTK_STATUSBAR(screen->tme_gtk_screen_mouse_statusbar),
		    screen->tme_gtk_screen_mouse_statusbar_cid);

  /* restore the text on the mouse label: */
  gtk_label_set_text(GTK_LABEL(screen->tme_gtk_screen_mouse_label),
		     _("Mouse is off"));

  /* the mouse is now off: */
  screen->tme_gtk_screen_mouse_keyval = GDK_VoidSymbol;
}

/* this is called when the mouse controls change: */
static int
_tme_gtk_mouse_ctrl(struct tme_mouse_connection *conn_mouse, 
		    unsigned int ctrl)
{
  struct tme_gtk_display *display;

  /* recover our data structure: */
  display = conn_mouse
    ->tme_mouse_connection.tme_connection_element->tme_element_private;

  /* XXX TBD */
  abort();

  return (TME_OK);
}

/* this is called to read the mouse: */
static int
_tme_gtk_mouse_read(struct tme_mouse_connection *conn_mouse, 
		    struct tme_mouse_event *event,
		    unsigned int count)
{
  struct tme_gtk_display *display;
  int rc;

  /* recover our data structure: */
  display = conn_mouse
    ->tme_mouse_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* copy an event out of the mouse buffer: */
  rc = tme_mouse_buffer_copyout(display->tme_gtk_display_mouse_buffer,
				event,
				count);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  return (rc);
}

/* this breaks a mouse connection: */
static int
_tme_gtk_mouse_connection_break(struct tme_connection *conn,
				unsigned int state)
{
  abort();
}

/* this makes a new mouse connection: */
static int
_tme_gtk_mouse_connection_make(struct tme_connection *conn,
			       unsigned int state)
{
  struct tme_gtk_display *display;

  /* recover our data structure: */
  display = conn->tme_connection_element->tme_element_private;

  /* both sides must be mouse connections: */
  assert(conn->tme_connection_type
	 == TME_CONNECTION_MOUSE);
  assert(conn->tme_connection_other->tme_connection_type
	 == TME_CONNECTION_MOUSE);

  /* we are always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* save our connection: */
    tme_mutex_lock(&display->tme_gtk_display_mutex);
    display->tme_gtk_display_mouse_connection
      = (struct tme_mouse_connection *) conn->tme_connection_other;
    tme_mutex_unlock(&display->tme_gtk_display_mutex);
  }

  return (TME_OK);
}

/* this scores a mouse connection: */
static int
_tme_gtk_mouse_connection_score(struct tme_connection *conn,
				unsigned int *_score)
{
  struct tme_mouse_connection *conn_mouse;

  /* both sides must be mouse connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_MOUSE);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_MOUSE);

  /* the other side cannot be a real mouse: */
  conn_mouse
    = (struct tme_mouse_connection *) conn->tme_connection_other;
  *_score = (conn_mouse->tme_mouse_connection_read == NULL);
  return (TME_OK);
}

/* this makes a new connection side for a GTK mouse: */
int
_tme_gtk_mouse_connections_new(struct tme_gtk_display *display, 
			       struct tme_connection **_conns)
{
  struct tme_mouse_connection *conn_mouse;
  struct tme_connection *conn;

  /* if we don't have a mouse connection yet: */
  if (display->tme_gtk_display_mouse_connection == NULL) {

    /* create our side of a mouse connection: */
    conn_mouse = tme_new0(struct tme_mouse_connection, 1);
    conn = &conn_mouse->tme_mouse_connection;

    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_MOUSE;
    conn->tme_connection_score = _tme_gtk_mouse_connection_score;
    conn->tme_connection_make = _tme_gtk_mouse_connection_make;
    conn->tme_connection_break = _tme_gtk_mouse_connection_break;

    /* fill in the mouse connection: */
    conn_mouse->tme_mouse_connection_ctrl = _tme_gtk_mouse_ctrl;
    conn_mouse->tme_mouse_connection_read = _tme_gtk_mouse_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* this attaches the GTK mouse to a new screen: */
void
_tme_gtk_mouse_attach(struct tme_gtk_screen *screen)
{
  struct tme_gtk_display *display;
  GtkWidget *hbox0;
  GtkWidget *ebox;

  /* get the display: */
  display = screen->tme_gtk_screen_display;

  /* create the horizontal packing box for the mouse controls: */
  hbox0 = gtk_hbox_new(FALSE, 0);

  /* pack the horizontal packing box into the outer vertical packing box: */
  gtk_box_pack_start(GTK_BOX(screen->tme_gtk_screen_vbox0), 
		     hbox0,
		     FALSE, FALSE, 0);

  /* show the horizontal packing box: */
  gtk_widget_show(hbox0);

  /* create the event box for the mouse on label: */
  ebox = gtk_event_box_new();

  /* pack the event box into the horizontal packing box: */
  gtk_box_pack_start(GTK_BOX(hbox0), 
		     ebox,
		     FALSE, FALSE, 0);

  /* set the tip on the event box, which will eventually contain the mouse on label: */
  gtk_tooltips_set_tip(GTK_TOOLTIPS(display->tme_gtk_display_tooltips),
		       ebox,
		       "Press a key here to turn the mouse on.  The same key " \
		       "will turn the mouse off.",
		       NULL);

  /* make sure the event box gets key_press events: */
  gtk_widget_add_events(ebox, 
			GDK_KEY_PRESS_MASK);

  /* set a signal handler for the event box events: */
  gtk_signal_connect(GTK_OBJECT(ebox),
		     "event",
		     GTK_SIGNAL_FUNC(_tme_gtk_mouse_ebox_event),
		     screen);

  /* the event box can focus: */
  GTK_WIDGET_SET_FLAGS(ebox, GTK_CAN_FOCUS);

  /* show the event box: */
  gtk_widget_show(ebox);

  /* create the mouse on label: */
  screen->tme_gtk_screen_mouse_label
    = gtk_label_new(_("Mouse is off"));

  /* add the mouse on label to the event box: */
  gtk_container_add(GTK_CONTAINER(ebox), 
		    screen->tme_gtk_screen_mouse_label);

  /* show the mouse on label: */
  gtk_widget_show(screen->tme_gtk_screen_mouse_label);

  /* create the mouse statusbar: */
  screen->tme_gtk_screen_mouse_statusbar
    = gtk_statusbar_new();

  /* pack the mouse statusbar into the horizontal packing box: */
  gtk_box_pack_start(GTK_BOX(hbox0), 
		     screen->tme_gtk_screen_mouse_statusbar,
		     TRUE, TRUE, 10);

  /* show the mouse statusbar: */
  gtk_widget_show(screen->tme_gtk_screen_mouse_statusbar);

  /* push an initial message onto the statusbar: */
  screen->tme_gtk_screen_mouse_statusbar_cid
    = gtk_statusbar_get_context_id(GTK_STATUSBAR(screen->tme_gtk_screen_mouse_statusbar),
				   "mouse context");
  gtk_statusbar_push(GTK_STATUSBAR(screen->tme_gtk_screen_mouse_statusbar),
		     screen->tme_gtk_screen_mouse_statusbar_cid,
		     _("The Machine Emulator"));

  /* although the event mask doesn't include these events yet,
     set a signal handler for the mouse events: */
  gtk_signal_connect(GTK_OBJECT(screen->tme_gtk_screen_event_box),
		     "motion_notify_event",
		     GTK_SIGNAL_FUNC(_tme_gtk_mouse_mouse_event), 
		     screen);
  gtk_signal_connect(GTK_OBJECT(screen->tme_gtk_screen_event_box),
		     "button_press_event",
		     GTK_SIGNAL_FUNC(_tme_gtk_mouse_mouse_event), 
		     screen);
  gtk_signal_connect(GTK_OBJECT(screen->tme_gtk_screen_event_box),
		     "button_release_event",
		     GTK_SIGNAL_FUNC(_tme_gtk_mouse_mouse_event), 
		     screen);

  /* mouse mode is off: */
  screen->tme_gtk_screen_mouse_keyval = GDK_VoidSymbol;
}

/* this initializes mouse part of the display: */
void
_tme_gtk_mouse_new(struct tme_gtk_display *display)
{
  GdkPixmap *source, *mask;

  /* we have no mouse connection: */
  display->tme_gtk_display_mouse_connection = NULL;
  
  /* allocate the mouse buffer: */
  display->tme_gtk_display_mouse_buffer
    = tme_mouse_buffer_new(1024);

  /* create the mouse cursor: */
  source
    = gdk_bitmap_create_from_data(NULL,
				  _tme_gtk_mouse_cursor_source,
				  TME_GTK_MOUSE_CURSOR_WIDTH,
				  TME_GTK_MOUSE_CURSOR_HEIGHT);
  mask
    = gdk_bitmap_create_from_data (NULL,
				   _tme_gtk_mouse_cursor_mask,
				   TME_GTK_MOUSE_CURSOR_WIDTH,
				   TME_GTK_MOUSE_CURSOR_HEIGHT);
  display->tme_gtk_display_mouse_cursor
    = gdk_cursor_new_from_pixmap(source,
				 mask,
				 &_tme_gtk_mouse_cursor_color,
				 &_tme_gtk_mouse_cursor_color,
				 0,
				 0);
  gdk_pixmap_unref(source);
  gdk_pixmap_unref(mask);
}
