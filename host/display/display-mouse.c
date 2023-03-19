/* host/disp/mouse.c - generic mouse support: */

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

/* this is for debugging only: */
#if 0
#include <stdio.h>
void
_tme_mouse_debug(const struct tme_mouse_event *event)
{
  fprintf(stderr,
	  "buttons = 0x%02x dx=%d dy=%d\n",
	  event->tme_mouse_event_buttons,
	  event->tme_mouse_event_delta_x,
	  event->tme_mouse_event_delta_y);
}
#else
#define _tme_mouse_debug(e) do { } while (/* CONSTCOND */ 0)
#endif

/* this is a generic callback for a mouse event: */
void
_tme_mouse_event(int button, int x, int y, struct tme_display *display)
{
  struct tme_mouse_event tme_event;
  int was_empty;
  int new_callouts, rc;
  
  /* make the buttons mask: */
  int buttons = display->tme_screen_mouse_buttons_last;

  if(button>0)
    buttons |= TME_BIT(button-1);
  else if(button) {
    button = -button;
    buttons &= ~TME_BIT(button-1);
  }

  /* start the tme event: */
  tme_event.tme_mouse_event_delta_units
    = TME_MOUSE_UNITS_UNKNOWN;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  /* set the event time: */
  tme_event.tme_mouse_event_time = tme_thread_get_time();

  /* if the button mask and pointer position haven't changed, return now.
     every time we warp the pointer we will get a motion event, and
     this should ignore those events: */

  if (buttons == display->tme_screen_mouse_buttons_last
      && x == display->tme_screen_mouse_warp_x
      && y == display->tme_screen_mouse_warp_y) {
    
    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_display_mutex);
    
    /* stop propagating this event: */
    return;
  }
  
  tme_event.tme_mouse_event_buttons =
    display->tme_screen_mouse_buttons_last = buttons;
  
  /* make the deltas: */
  tme_event.tme_mouse_event_delta_x = 
    (((int) x)
     - ((int) display->tme_screen_mouse_warp_x));
  tme_event.tme_mouse_event_delta_y = 
    (((int) y)
     - ((int) display->tme_screen_mouse_warp_y));

  display->tme_screen_mouse_warp_x = x;
  display->tme_screen_mouse_warp_y = y;
  
  /* assume that we won't need any new callouts: */
  new_callouts = 0;
  
  /* remember if the mouse buffer was empty: */
  was_empty
    = tme_mouse_buffer_is_empty(display->tme_display_mouse_buffer);

  /* add this tme event to the mouse buffer: */
  _tme_mouse_debug(&tme_event);
  rc = tme_mouse_buffer_copyin(display->tme_display_mouse_buffer,
			       &tme_event);
  assert (rc == TME_OK);

  /* if the mouse buffer was empty and now it isn't,
     call out the mouse controls: */
  if (was_empty
      && !tme_mouse_buffer_is_empty(display->tme_display_mouse_buffer)) {
    new_callouts |= TME_DISPLAY_CALLOUT_MOUSE_CTRL;
  }

  /* add in any new callouts: */
  display->tme_display_callout_flags |= new_callouts;

  /* run any callouts: */
  //_tme_display_callout(display, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);
}

/* this is called when the mouse controls change: */
static int
_tme_mouse_ctrl(struct tme_mouse_connection *conn_mouse, 
		    unsigned int ctrl)
{
  struct tme_display *display;

  /* recover our data structure: */
  display = conn_mouse
    ->tme_mouse_connection.tme_connection_element->tme_element_private;

  /* XXX TBD */
  abort();

  return (TME_OK);
}

/* this is called to read the mouse: */
static int
_tme_mouse_read(struct tme_mouse_connection *conn_mouse, 
		struct tme_mouse_event *event,
		unsigned int count)
{
  struct tme_display *display;
  int rc;

  /* recover our data structure: */
  display = conn_mouse
    ->tme_mouse_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  /* copy an event out of the mouse buffer: */
  rc = tme_mouse_buffer_copyout(display->tme_display_mouse_buffer,
				event,
				count);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

  return (rc);
}

/* this breaks a mouse connection: */
static int
_tme_mouse_connection_break(struct tme_connection *conn,
				unsigned int state)
{
  abort();
}

/* this makes a new mouse connection: */
static int
_tme_mouse_connection_make(struct tme_connection *conn,
			       unsigned int state)
{
  struct tme_display *display;

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
    tme_mutex_lock(&display->tme_display_mutex);
    display->tme_display_mouse_connection
      = (struct tme_mouse_connection *) conn->tme_connection_other;
    tme_mutex_unlock(&display->tme_display_mutex);
  }

  return (TME_OK);
}

/* this scores a mouse connection: */
static int
_tme_mouse_connection_score(struct tme_connection *conn,
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

/* this makes a new connection side for a generic mouse: */
int
_tme_mouse_connections_new(struct tme_display *display, 
			       struct tme_connection **_conns)
{
  struct tme_mouse_connection *conn_mouse;
  struct tme_connection *conn;

  /* if we don't have a mouse connection yet: */
  if (display->tme_display_mouse_connection == NULL) {

    /* create our side of a mouse connection: */
    conn_mouse = tme_new0(struct tme_mouse_connection, 1);
    conn = &conn_mouse->tme_mouse_connection;

    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_MOUSE;
    conn->tme_connection_score = _tme_mouse_connection_score;
    conn->tme_connection_make = _tme_mouse_connection_make;
    conn->tme_connection_break = _tme_mouse_connection_break;

    /* fill in the mouse connection: */
    conn_mouse->tme_mouse_connection_ctrl = _tme_mouse_ctrl;
    conn_mouse->tme_mouse_connection_read = _tme_mouse_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* this initializes mouse part of the display: */
void
_tme_mouse_new(struct tme_display *display)
{
  /* we have no mouse connection: */
  display->tme_display_mouse_connection = NULL;
  
  /* allocate the mouse buffer: */
  display->tme_display_mouse_buffer
    = tme_mouse_buffer_new(1024);
}
