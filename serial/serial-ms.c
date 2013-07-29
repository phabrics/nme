/* $Id: serial-ms.c,v 1.4 2010/02/14 00:48:04 fredette Exp $ */

/* serial/serial-ms.c - serial mouse emulation: */

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
_TME_RCSID("$Id: serial-ms.c,v 1.4 2010/02/14 00:48:04 fredette Exp $");

/* includes: */
#include "serial-ms.h"

/* macros: */
#define TME_SERIAL_MS_BUFFER_SIZE	(1024)

/* globals: */

/* the list of serial mice that we emulate: */
const struct {

  /* the mouse type: */
  const char *tme_serial_ms_list_type;

  /* the mouse initialization function: */
  int (*tme_serial_ms_list_init) _TME_P((struct tme_serial_ms *));
} _tme_serial_ms_list[] = {
  
  /* the mousesystems 5-byte protocol: */
  { "mousesystems-5", _tme_serial_ms_mssystems5_init },
};

/* the serial mouse callout function.  it must be called with the mutex locked: */
static void
_tme_serial_ms_callout(struct tme_serial_ms *serial_ms, int new_callouts)
{
  struct tme_mouse_connection *conn_mouse;
  struct tme_serial_connection *conn_serial;
  int callouts, later_callouts;
  unsigned int ctrl;
  struct tme_serial_config config;
  tme_uint8_t buffer_input[32];
  tme_serial_data_flags_t data_flags;
  struct timeval now;
  unsigned long rate_sleep_usec;
  unsigned long passed_sec;
  long passed_usec;
  struct tme_mouse_event buffer_events[32];
  int old_empty;
  int rc;
  
  /* add in any new callouts: */
  serial_ms->tme_serial_ms_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (serial_ms->tme_serial_ms_callout_flags
      & TME_SERIAL_MS_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  serial_ms->tme_serial_ms_callout_flags
    |= TME_SERIAL_MS_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = serial_ms->tme_serial_ms_callout_flags)
	  & TME_SERIAL_MS_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    serial_ms->tme_serial_ms_callout_flags
      = (callouts
	 & ~TME_SERIAL_MS_CALLOUTS_MASK);
    callouts &= TME_SERIAL_MS_CALLOUTS_MASK;

    /* get this card's connections: */
    conn_mouse = serial_ms->tme_serial_ms_connection_ms;
    conn_serial = serial_ms->tme_serial_ms_connection_serial;

    /* if we need to call out new serial control information: */
    if (callouts & TME_SERIAL_MS_CALLOUT_SERIAL_CTRL) {

      /* form the new ctrl: */
      ctrl = serial_ms->tme_serial_ms_serial_ctrl;
      if (!tme_serial_buffer_is_empty(&serial_ms->tme_serial_ms_serial_buffer)) {
	ctrl |= TME_SERIAL_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);
      
      /* do the callout: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_ctrl)
	       (conn_serial,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SERIAL_MS_CALLOUT_SERIAL_CTRL;
      }
    }

    /* if we need to call out new serial config information: */
    if (callouts & TME_SERIAL_MS_CALLOUT_SERIAL_CONFIG) {
      
      /* form the new config: */
      config = serial_ms->tme_serial_ms_type_config;

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);
      
      /* do the callout: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_config)
	       (conn_serial,
		&config))
	    : TME_OK);
      
      /* lock the mutex: */
      tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SERIAL_MS_CALLOUT_SERIAL_CONFIG;
      }
    }

    /* if the serial connection is readable: */
    if (callouts & TME_SERIAL_MS_CALLOUT_SERIAL_READ) {

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);

      /* do the read: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_read)
	       (conn_serial,
		buffer_input,
		sizeof(buffer_input),
		&data_flags))
	    : 0);
	  
      /* lock the mutex: */
      tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);
	
      /* if the read was successful: */
      if (rc > 0) {
	    
	/* call any type-specific serial input function: */
	if (serial_ms->tme_serial_ms_type_serial_input != NULL) {
	  rc = (*serial_ms->tme_serial_ms_type_serial_input)
	    (serial_ms,
	     buffer_input,
	     rc,
	     data_flags);
	  assert (rc == TME_OK);
	}

	/* mark that we need to loop to callout to read more data: */
	serial_ms->tme_serial_ms_callout_flags
	  |= TME_SERIAL_MS_CALLOUT_SERIAL_READ;
      }
      
      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_SERIAL_READ flag: */
    }

    /* if we need to call out new mouse control information: */
    if (callouts & TME_SERIAL_MS_CALLOUT_MOUSE_CTRL) {

      /* form the new control: */
      ctrl = serial_ms->tme_serial_ms_mouse_ctrl;

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);
      
      /* do the callout: */
      rc = (conn_mouse != NULL
	    ? ((*conn_mouse->tme_mouse_connection_ctrl)
	       (conn_mouse,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SERIAL_MS_CALLOUT_MOUSE_CTRL;
      }
    }
      
    /* if the mouse connection is readable: */
    if (callouts & TME_SERIAL_MS_CALLOUT_MOUSE_READ) {

      /* get the current time: */
      gettimeofday(&now, NULL);

      /* if we're not already doing a rate-limiting sleep: */
      rate_sleep_usec = serial_ms->tme_serial_ms_rate_sleep_usec;
      if (rate_sleep_usec == 0) {

	/* see how many microseconds have passed since the last event
	   was read, clipping it at one second: */
	passed_sec = now.tv_sec;
	passed_usec = now.tv_usec;
	passed_usec -= serial_ms->tme_serial_ms_event_read_last.tv_usec;
	if (passed_usec < 0) {
	  passed_sec -= 1;
	  passed_usec += 1000000;
	}
	assert (passed_usec > 0 && passed_usec < 1000000);
	passed_sec -= serial_ms->tme_serial_ms_event_read_last.tv_sec;
	if (passed_sec > 0) {
	  passed_usec = 1000000;
	}

	/* if not enough microseconds have passed since the last event
	   was read: */
	rate_sleep_usec = serial_ms->tme_serial_ms_rate_usec;
	if (passed_usec < (long) rate_sleep_usec) {

	  /* set the sleep time and wake up the rate thread: */
	  serial_ms->tme_serial_ms_rate_sleep_usec = rate_sleep_usec - passed_usec;
	  tme_cond_notify(&serial_ms->tme_serial_ms_rate_cond, FALSE);
	}

	/* reload any sleep time: */
	rate_sleep_usec = serial_ms->tme_serial_ms_rate_sleep_usec;
      }

      /* if we're doing a rate-limiting sleep: */
      if (rate_sleep_usec != 0) {

	/* read no events: */
	rc = 0;
      }

      /* otherwise, we're not doing a rate-limiting sleep: */
      else {

	/* set the time that the last event was read: */
	serial_ms->tme_serial_ms_event_read_last = now;

	/* unlock the mutex: */
	tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);

	/* if we're rate-limiting, only read one event, otherwise
	   read as many as we can: */
	rc = (serial_ms->tme_serial_ms_rate_usec ? 1 : TME_ARRAY_ELS(buffer_events));

	/* do the read: */
	rc = (conn_mouse != NULL
	      ? ((*conn_mouse->tme_mouse_connection_read)
		 (conn_mouse,
		  buffer_events,
		  rc))
	      : ENOENT);
	  
	/* lock the mutex: */
	tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);
      }
	
      /* if the read was successful: */
      if (rc > 0) {
	    
	/* note if our serial buffer is currently empty: */
	old_empty
	  = tme_serial_buffer_is_empty(&serial_ms->tme_serial_ms_serial_buffer);

	/* call the type-specific events function to turn these
	   events into serial data: */
	rc = (*serial_ms->tme_serial_ms_type_events)
	  (serial_ms, buffer_events, rc);
	assert (rc == TME_OK);

	/* if our serial buffer was empty before, call out
	   that we are readable: */
	if (old_empty) {
	  serial_ms->tme_serial_ms_callout_flags 
	    |= TME_SERIAL_MS_CALLOUT_SERIAL_CTRL;
	}

	/* mark that we need to loop to callout to read more data: */
	serial_ms->tme_serial_ms_callout_flags |= TME_SERIAL_MS_CALLOUT_MOUSE_READ;
      }
      
      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_MOUSE_READ flag: */
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  serial_ms->tme_serial_ms_callout_flags = later_callouts;
}

/* the serial mouse rate-limiting thread: */
static void
_tme_serial_ms_th_rate(void *_serial_ms)
{
  struct tme_serial_ms *serial_ms;

  /* recover our data structure: */
  serial_ms = _serial_ms;

  /* lock our mutex: */
  tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);

  /* loop forever: */
  for (;;) {

    /* if we need to call out a mouse event read: */
    if (serial_ms->tme_serial_ms_rate_do_callout) {

      /* we no longer need to call out a mouse event read or sleep: */
      serial_ms->tme_serial_ms_rate_do_callout = FALSE;
      serial_ms->tme_serial_ms_rate_sleep_usec = 0;

      /* call out the mouse event read: */
      _tme_serial_ms_callout(serial_ms,
			     TME_SERIAL_MS_CALLOUT_MOUSE_READ);
    }

    /* if we need to sleep: */
    if (serial_ms->tme_serial_ms_rate_sleep_usec != 0) {

      /* after we sleep, we need to call out a mouse event read: */
      serial_ms->tme_serial_ms_rate_do_callout = TRUE;

      /* unlock our mutex: */
      tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);

      /* sleep: */
      tme_thread_sleep_yield(0, serial_ms->tme_serial_ms_rate_sleep_usec);
	
      /* lock our mutex: */
      tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);
    }

    /* otherwise, we need to wait on the condition: */
    else {
      tme_cond_wait_yield(&serial_ms->tme_serial_ms_rate_cond,
			  &serial_ms->tme_serial_ms_mutex);
    }
  }
  /* NOTREACHED */
}

/* the mouse control function: */
static int
_tme_serial_ms_mouse_ctrl(struct tme_mouse_connection *conn_mouse,
			  unsigned int ctrl)
{
  struct tme_serial_ms *serial_ms;
  int new_callouts;

  /* recover our data structure: */
  serial_ms = conn_mouse->tme_mouse_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock our mutex: */
  tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_MOUSE_CTRL_OK_READ) {
    new_callouts |= TME_SERIAL_MS_CALLOUT_MOUSE_READ;
  }

  /* make any new callouts: */
  _tme_serial_ms_callout(serial_ms, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial configuration function: */
static int
_tme_serial_ms_serial_config(struct tme_serial_connection *conn_serial,
			     struct tme_serial_config *config)
{
  struct tme_serial_ms *serial_ms;

  /* recover our data structure: */
  serial_ms = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* save the configuration: */
  serial_ms->tme_serial_ms_peer_config = *config;

  /* nothing to do: */
  return (TME_OK);
}

/* the serial control function: */
static int
_tme_serial_ms_serial_ctrl(struct tme_serial_connection *conn_serial,
			   unsigned int ctrl)
{
  struct tme_serial_ms *serial_ms;
  int new_callouts;

  /* recover our data structure: */
  serial_ms = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock our mutex: */
  tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_SERIAL_CTRL_OK_READ) {
    new_callouts |= TME_SERIAL_MS_CALLOUT_SERIAL_READ;
  }

  /* call any type-specific control function: */
  if (serial_ms->tme_serial_ms_type_serial_ctrl != NULL) {
    (*serial_ms->tme_serial_ms_type_serial_ctrl)
      (serial_ms, ctrl);
  }

  /* make any new callouts: */
  _tme_serial_ms_callout(serial_ms, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial read callin function: */
static int
_tme_serial_ms_serial_read(struct tme_serial_connection *conn_serial, 
			   tme_uint8_t *data, unsigned int count,
			   tme_serial_data_flags_t *_data_flags)
{
  struct tme_serial_ms *serial_ms;
  int new_callouts;
  int rc;

  /* recover our data structures: */
  serial_ms = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);

  /* copy out data from the serial buffer: */
  rc = tme_serial_buffer_copyout(&serial_ms->tme_serial_ms_serial_buffer,
				 data, count,
				 _data_flags,
				 TME_SERIAL_COPY_NORMAL);

  /* make any new callouts: */
  _tme_serial_ms_callout(serial_ms, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);

  /* done: */
  return (rc);
}

/* this scores a connection: */
static int
_tme_serial_ms_connection_score(struct tme_connection *conn,
				unsigned int *_score)
{
  struct tme_serial_ms *serial_ms;
  struct tme_mouse_connection *conn_mouse;

  /* recover our serial: */
  serial_ms = conn->tme_connection_element->tme_element_private;

  /* both sides must be the same type of connection, and either
     TME_CONNECTION_SERIAL or TME_CONNECTION_MOUSE: */
  assert((conn->tme_connection_type
	  == TME_CONNECTION_SERIAL)
	  || (conn->tme_connection_type
	      == TME_CONNECTION_MOUSE));
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* if this is a mouse connection, but it's a mouse connection
     to another mouse sink, we can't connect to it: */
  if (conn->tme_connection_type
      == TME_CONNECTION_MOUSE) {
    conn_mouse
      = ((struct tme_mouse_connection *) 
	 conn->tme_connection_other);
    if (conn_mouse->tme_mouse_connection_read == NULL) {
      *_score = 0;
      return (TME_OK);
    }
  }

  /* otherwise, any connection is always good: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_serial_ms_connection_make(struct tme_connection *conn,
			       unsigned int state)
{
  struct tme_serial_ms *serial_ms;

  /* recover our serial mouse: */
  serial_ms = conn->tme_connection_element->tme_element_private;

  /* both sides must be the same type of connection, and either
     TME_CONNECTION_SERIAL or TME_CONNECTION_MOUSE: */
  assert((conn->tme_connection_type
	  == TME_CONNECTION_SERIAL)
	  || (conn->tme_connection_type
	      == TME_CONNECTION_MOUSE));
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&serial_ms->tme_serial_ms_mutex);

    /* save a serial connection: */
    if (conn->tme_connection_type == TME_CONNECTION_SERIAL) {

      serial_ms->tme_serial_ms_connection_serial
	= ((struct tme_serial_connection *)
	   conn->tme_connection_other);

      /* call out the serial configuration: */
      _tme_serial_ms_callout(serial_ms, TME_SERIAL_MS_CALLOUT_SERIAL_CONFIG);
    }
    
    /* otherwise, save a mouse connection: */
    else {
      serial_ms->tme_serial_ms_connection_ms
	= ((struct tme_mouse_connection *)
	   conn->tme_connection_other);
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&serial_ms->tme_serial_ms_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_serial_ms_connection_break(struct tme_connection *conn,
				unsigned int state)
{
  abort();
}

/* this makes a new connection side for a serial mouse: */
static int
_tme_serial_ms_connections_new(struct tme_element *element,
			       const char * const *args,
			       struct tme_connection **_conns,
			       char **_output)
{
  struct tme_serial_ms *serial_ms;
  struct tme_mouse_connection *conn_mouse;
  struct tme_serial_connection *conn_serial;
  struct tme_connection *conn;

  /* recover our serial: */
  serial_ms = (struct tme_serial_ms *) element->tme_element_private;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* if we don't have a mouse connection yet: */
  if (serial_ms->tme_serial_ms_connection_ms == NULL) {

    /* create our side of a mouse connection: */
    conn_mouse = tme_new0(struct tme_mouse_connection, 1);
    conn = &conn_mouse->tme_mouse_connection;

    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_MOUSE;
    conn->tme_connection_score = _tme_serial_ms_connection_score;
    conn->tme_connection_make = _tme_serial_ms_connection_make;
    conn->tme_connection_break = _tme_serial_ms_connection_break;

    /* fill in the mouse connection: */
    conn_mouse->tme_mouse_connection_ctrl = _tme_serial_ms_mouse_ctrl;
    conn_mouse->tme_mouse_connection_read = NULL;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* if we don't have a serial connection yet: */
  if (serial_ms->tme_serial_ms_connection_serial == NULL) {

    /* create our side of a serial connection: */
    conn_serial = tme_new0(struct tme_serial_connection, 1);
    conn = &conn_serial->tme_serial_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SERIAL;
    conn->tme_connection_score = _tme_serial_ms_connection_score;
    conn->tme_connection_make = _tme_serial_ms_connection_make;
    conn->tme_connection_break = _tme_serial_ms_connection_break;
    
    /* fill in the serial connection: */
    conn_serial->tme_serial_connection_config = _tme_serial_ms_serial_config;
    conn_serial->tme_serial_connection_ctrl = _tme_serial_ms_serial_ctrl;
    conn_serial->tme_serial_connection_read = _tme_serial_ms_serial_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  return (TME_OK);
}

/* the new serial mouse function: */
TME_ELEMENT_X_NEW_DECL(tme_serial_,kb,mouse) {
  struct tme_serial_ms *serial_ms;
  const char *ms_type;
  int (*ms_init) _TME_P((struct tme_serial_ms *));
  unsigned int ms_list_i;
  int usage;
  int arg_i;

  /* initialize: */
  ms_type = NULL;
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    /* the mouse type we're emulating: */
    if (TME_ARG_IS(args[arg_i + 0], "type")
	&& args[arg_i + 1] != NULL
	&& ms_type == NULL) {
      ms_type = args[arg_i + 1];
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      /* we must have been given a type: */
      if (ms_type == NULL) {
	usage = TRUE;
      }
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
			    "%s %s type %s",
			    _("usage:"),
			    args[0],
			    _("MOUSE-TYPE"));
    return (EINVAL);
  }

  /* make sure that this mouse type is known: */
  ms_init = NULL;
  for (ms_list_i = 0;
       ms_list_i < TME_ARRAY_ELS(_tme_serial_ms_list);
       ms_list_i++) {
    if (!strcmp(_tme_serial_ms_list[ms_list_i].tme_serial_ms_list_type,
		ms_type)) {
      ms_init = _tme_serial_ms_list[ms_list_i].tme_serial_ms_list_init;
      break;
    }
  }
  if (ms_init == NULL) {
    tme_output_append_error(_output, "%s", ms_type);
    return (ENOENT);
  }

  /* start the serial mouse structure: */
  serial_ms = tme_new0(struct tme_serial_ms, 1);
  serial_ms->tme_serial_ms_element = element;
  tme_mutex_init(&serial_ms->tme_serial_ms_mutex);
  serial_ms->tme_serial_ms_type = ms_type;
  serial_ms->tme_serial_ms_connection_ms = NULL;
  serial_ms->tme_serial_ms_connection_serial = NULL;
  serial_ms->tme_serial_ms_callout_flags = 0;
  serial_ms->tme_serial_ms_mouse_ctrl
    = 0;
  serial_ms->tme_serial_ms_serial_ctrl
    = (TME_SERIAL_CTRL_DTR
       | TME_SERIAL_CTRL_DCD);
  tme_serial_buffer_init(&serial_ms->tme_serial_ms_serial_buffer, 
			 TME_SERIAL_MS_BUFFER_SIZE);
  (*ms_init)(serial_ms);

  /* start any rate-limiting thread: */
  if (serial_ms->tme_serial_ms_rate_usec > 0) {
    tme_cond_init(&serial_ms->tme_serial_ms_rate_cond);
    tme_thread_create(_tme_serial_ms_th_rate, serial_ms);
  }

  /* fill the element: */
  element->tme_element_private = serial_ms;
  element->tme_element_connections_new = _tme_serial_ms_connections_new;

  return (TME_OK);
}
