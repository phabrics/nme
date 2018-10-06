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
