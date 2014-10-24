/* $Id: serial-sink.c,v 1.1 2006/09/30 12:56:01 fredette Exp $ */

/* serial/serial-sink.c - a serial sink: */

/*
 * Copyright (c) 2006 Matt Fredette
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
_TME_RCSID("$Id: serial-sink.c,v 1.1 2006/09/30 12:56:01 fredette Exp $");

/* includes: */
#include <tme/common.h>
#include <tme/threads.h>
#include <tme/generic/serial.h>

/* macros: */

/* the callout flags: */
#define TME_SERIAL_SINK_CALLOUT_CHECK		(0)
#define TME_SERIAL_SINK_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SERIAL_SINK_CALLOUTS_MASK		(-2)
#define  TME_SERIAL_SINK_CALLOUT_SERIAL_READ	TME_BIT(1)

/* types: */

/* a serial sink: */
struct tme_serial_sink {

  /* backpointer to our element: */
  struct tme_element *tme_serial_sink_element;

  /* our mutex: */
  tme_mutex_t tme_serial_sink_mutex;

  /* our serial connection: */
  struct tme_serial_connection *tme_serial_sink_connection_serial;

  /* the callout flags: */
  int tme_serial_sink_callout_flags;
};

/* the serial sink callout function.  it must be called with the mutex locked: */
static void
_tme_serial_sink_callout(struct tme_serial_sink *serial_sink, int new_callouts)
{
  struct tme_serial_connection *conn_serial;
  int callouts, later_callouts;
  tme_uint8_t buffer_input[32];
  tme_serial_data_flags_t data_flags;
  int rc;
  
  /* add in any new callouts: */
  serial_sink->tme_serial_sink_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (serial_sink->tme_serial_sink_callout_flags
      & TME_SERIAL_SINK_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  serial_sink->tme_serial_sink_callout_flags
    |= TME_SERIAL_SINK_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = serial_sink->tme_serial_sink_callout_flags)
	  & TME_SERIAL_SINK_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    serial_sink->tme_serial_sink_callout_flags
      = (callouts
	 & ~TME_SERIAL_SINK_CALLOUTS_MASK);
    callouts &= TME_SERIAL_SINK_CALLOUTS_MASK;

    /* get this sink's connection: */
    conn_serial = serial_sink->tme_serial_sink_connection_serial;

    /* if the serial connection is readable: */
    if (callouts & TME_SERIAL_SINK_CALLOUT_SERIAL_READ) {

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_sink->tme_serial_sink_mutex);

      /* do the read: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_read)
	       (conn_serial,
		buffer_input,
		sizeof(buffer_input),
		&data_flags))
	    : 0);
	  
      /* lock the mutex: */
      tme_mutex_lock(&serial_sink->tme_serial_sink_mutex);
	
      /* if the read was successful: */
      if (rc > 0) {
	    
	/* mark that we need to loop to callout to read more data: */
	serial_sink->tme_serial_sink_callout_flags
	  |= TME_SERIAL_SINK_CALLOUT_SERIAL_READ;
      }
      
      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_SERIAL_READ flag: */
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  serial_sink->tme_serial_sink_callout_flags = later_callouts;
}

/* the serial configuration function: */
static int
_tme_serial_sink_serial_config(struct tme_serial_connection *conn_serial,
			       struct tme_serial_config *config)
{
  /* nothing to do: */
  return (TME_OK);

  /* unused: */
  conn_serial = 0;
  config = 0;
}

/* the serial control function: */
static int
_tme_serial_sink_serial_ctrl(struct tme_serial_connection *conn_serial,
			     unsigned int ctrl)
{
  struct tme_serial_sink *serial_sink;
  int new_callouts;

  /* recover our data structure: */
  serial_sink = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock our mutex: */
  tme_mutex_lock(&serial_sink->tme_serial_sink_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_SERIAL_CTRL_OK_READ) {
    new_callouts |= TME_SERIAL_SINK_CALLOUT_SERIAL_READ;
  }

  /* make any new callouts: */
  _tme_serial_sink_callout(serial_sink, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&serial_sink->tme_serial_sink_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial read callin function: */
static int
_tme_serial_sink_serial_read(struct tme_serial_connection *conn_serial, 
			     tme_uint8_t *data, unsigned int count,
			     tme_serial_data_flags_t *_data_flags)
{
  /* we never have any data to read: */
  return (0);

  /* unused: */
  conn_serial = 0;
  data = 0;
  count = 0;
  _data_flags = 0;
}

/* this scores a connection: */
static int
_tme_serial_sink_connection_score(struct tme_connection *conn,
				unsigned int *_score)
{

  /* any connection is always good: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_serial_sink_connection_make(struct tme_connection *conn,
				 unsigned int state)
{
  struct tme_serial_sink *serial_sink;

  /* recover our serial sink: */
  serial_sink = conn->tme_connection_element->tme_element_private;

  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&serial_sink->tme_serial_sink_mutex);

    serial_sink->tme_serial_sink_connection_serial
      = ((struct tme_serial_connection *)
	 conn->tme_connection_other);

    /* unlock our mutex: */
    tme_mutex_unlock(&serial_sink->tme_serial_sink_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_serial_sink_connection_break(struct tme_connection *conn,
				unsigned int state)
{
  abort();
}

/* this makes a new connection side for a serial sink: */
static int
_tme_serial_sink_connections_new(struct tme_element *element,
				 const char * const *args,
				 struct tme_connection **_conns,
				 char **_output)
{
  struct tme_serial_sink *serial_sink;
  struct tme_serial_connection *conn_serial;
  struct tme_connection *conn;

  /* recover our serial: */
  serial_sink = (struct tme_serial_sink *) element->tme_element_private;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* if we don't have a serial connection yet: */
  if (serial_sink->tme_serial_sink_connection_serial == NULL) {

    /* create our side of a serial connection: */
    conn_serial = tme_new0(struct tme_serial_connection, 1);
    conn = &conn_serial->tme_serial_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SERIAL;
    conn->tme_connection_score = _tme_serial_sink_connection_score;
    conn->tme_connection_make = _tme_serial_sink_connection_make;
    conn->tme_connection_break = _tme_serial_sink_connection_break;
    
    /* fill in the serial connection: */
    conn_serial->tme_serial_connection_config = _tme_serial_sink_serial_config;
    conn_serial->tme_serial_connection_ctrl = _tme_serial_sink_serial_ctrl;
    conn_serial->tme_serial_connection_read = _tme_serial_sink_serial_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  return (TME_OK);
}

/* the new serial sink function: */
TME_ELEMENT_X_NEW_DECL(tme_serial_,kb,sink) {
  struct tme_serial_sink *serial_sink;
  int usage;
  int arg_i;

  /* initialize: */
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    if (0) {
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      break;
    }

    /* this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the serial sink structure: */
  serial_sink = tme_new0(struct tme_serial_sink, 1);
  serial_sink->tme_serial_sink_element = element;
  tme_mutex_init(&serial_sink->tme_serial_sink_mutex);
  serial_sink->tme_serial_sink_connection_serial = NULL;
  serial_sink->tme_serial_sink_callout_flags = 0;

  /* fill the element: */
  element->tme_element_private = serial_sink;
  element->tme_element_connections_new = _tme_serial_sink_connections_new;

  return (TME_OK);
}
