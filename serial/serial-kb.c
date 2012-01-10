/* $Id: serial-kb.c,v 1.7 2007/01/21 15:45:01 fredette Exp $ */

/* serial/serial-kb.c - serial keyboard emulation: */

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
_TME_RCSID("$Id: serial-kb.c,v 1.7 2007/01/21 15:45:01 fredette Exp $");

/* includes: */
#include "serial-kb.h"
#include <tme/misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* macros: */
#define TME_SERIAL_KB_BUFFER_SIZE	(1024)

/* globals: */

/* the list of serial keyboards that we emulate: */
const struct {

  /* the keyboard type: */
  const char *tme_serial_kb_list_type;

  /* the keyboard initialization function: */
  int (*tme_serial_kb_list_init) _TME_P((struct tme_serial_kb *));
} _tme_serial_kb_list[] = {
  
  /* the Sun keyboards: */
  { "sun-type-2", _tme_serial_kb_sun_init },
  { "sun-type-3", _tme_serial_kb_sun_init },
  { "sun-type-4-us", _tme_serial_kb_sun_init },
  { "sun-type-5-us", _tme_serial_kb_sun_init },
  { "sun-type-5-unix", _tme_serial_kb_sun_init },
};

/* the serial keyboard callout function.  it must be called with the mutex locked: */
static void
_tme_serial_kb_callout(struct tme_serial_kb *serial_kb, int new_callouts)
{
  struct tme_keyboard_connection *conn_keyboard;
  struct tme_serial_connection *conn_serial;
  int callouts, later_callouts;
  unsigned int ctrl;
  struct tme_serial_config config;
  tme_uint8_t buffer_input[32], data;
  tme_serial_data_flags_t data_flags;
  struct tme_keyboard_event event;
  int old_empty;
  int rc;
  
  /* add in any new callouts: */
  serial_kb->tme_serial_kb_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (serial_kb->tme_serial_kb_callout_flags
      & TME_SERIAL_KB_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  serial_kb->tme_serial_kb_callout_flags
    |= TME_SERIAL_KB_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = serial_kb->tme_serial_kb_callout_flags)
	  & TME_SERIAL_KB_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    serial_kb->tme_serial_kb_callout_flags
      = (callouts
	 & ~TME_SERIAL_KB_CALLOUTS_MASK);
    callouts &= TME_SERIAL_KB_CALLOUTS_MASK;

    /* get this card's connections: */
    conn_keyboard = serial_kb->tme_serial_kb_connection_kb;
    conn_serial = serial_kb->tme_serial_kb_connection_serial;

    /* if we need to call out new serial control information: */
    if (callouts & TME_SERIAL_KB_CALLOUT_SERIAL_CTRL) {

      /* form the new ctrl: */
      ctrl = serial_kb->tme_serial_kb_serial_ctrl;
      if (!tme_serial_buffer_is_empty(&serial_kb->tme_serial_kb_serial_buffer)) {
	ctrl |= TME_SERIAL_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);
      
      /* do the callout: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_ctrl)
	       (conn_serial,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SERIAL_KB_CALLOUT_SERIAL_CTRL;
      }
    }

    /* if we need to call out new serial config information: */
    if (callouts & TME_SERIAL_KB_CALLOUT_SERIAL_CONFIG) {
      
      /* form the new config: */
      config = serial_kb->tme_serial_kb_type_config;

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);
      
      /* do the callout: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_config)
	       (conn_serial,
		&config))
	    : TME_OK);
      
      /* lock the mutex: */
      tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SERIAL_KB_CALLOUT_SERIAL_CONFIG;
      }
    }

    /* if the serial connection is readable: */
    if (callouts & TME_SERIAL_KB_CALLOUT_SERIAL_READ) {

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);

      /* do the read: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_read)
	       (conn_serial,
		buffer_input,
		sizeof(buffer_input),
		&data_flags))
	    : 0);
	  
      /* lock the mutex: */
      tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
	
      /* if the read was successful: */
      if (rc > 0) {
	    
	/* call any type-specific serial input function: */
	if (serial_kb->tme_serial_kb_type_serial_input != NULL) {
	  rc = (*serial_kb->tme_serial_kb_type_serial_input)
	    (serial_kb,
	     buffer_input,
	     rc,
	     data_flags);
	  assert (rc == TME_OK);
	}

	/* mark that we need to loop to callout to read more data: */
	serial_kb->tme_serial_kb_callout_flags |= TME_SERIAL_KB_CALLOUT_SERIAL_READ;
      }
      
      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_SERIAL_READ flag: */
    }

    /* if we need to call out new keyboard control information: */
    if (callouts & TME_SERIAL_KB_CALLOUT_KEYBOARD_CTRL) {

      /* form the new control: */
      ctrl = serial_kb->tme_serial_kb_keyboard_ctrl;

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);
      
      /* do the callout: */
      rc = (conn_keyboard != NULL
	    ? ((*conn_keyboard->tme_keyboard_connection_ctrl)
	       (conn_keyboard,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SERIAL_KB_CALLOUT_KEYBOARD_CTRL;
      }
    }
      
    /* if the keyboard connection is readable: */
    if (callouts & TME_SERIAL_KB_CALLOUT_KEYBOARD_READ) {

      /* unlock the mutex: */
      tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);

      /* do the read: */
      rc = (conn_keyboard != NULL
	    ? ((*conn_keyboard->tme_keyboard_connection_read)
	       (conn_keyboard,
		&event))
	    : ENOENT);
	  
      /* lock the mutex: */
      tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
	
      /* if the read was successful: */
      if (rc == TME_OK) {
	    
	/* copy this event into our keyboard buffer: */
	rc = tme_keyboard_buffer_copyin(serial_kb->tme_serial_kb_keyboard_buffer,
					&event);
	assert (rc == TME_OK);

	/* see if the serial buffer is currently empty: */
	old_empty
	  = tme_serial_buffer_is_empty(&serial_kb->tme_serial_kb_serial_buffer);

	/* while the keyboard buffer is not empty, copy out
	   events and add them to our serial output buffer: */
	for (; !tme_keyboard_buffer_is_empty(serial_kb->tme_serial_kb_keyboard_buffer); ) {

	  /* get the next keyboard event: */
	  rc = tme_keyboard_buffer_copyout(serial_kb->tme_serial_kb_keyboard_buffer,
					   &event);
	  assert (rc == TME_OK);

	  /* call the type-specific event function to get the serial data: */
	  data = (*serial_kb->tme_serial_kb_type_event)
	    (serial_kb, &event);
	  
	  /* add this serial data to our serial buffer: */
	  tme_serial_buffer_copyin(&serial_kb->tme_serial_kb_serial_buffer,
				   &data, 1,
				   TME_SERIAL_DATA_NORMAL,
				   TME_SERIAL_COPY_FULL_IS_OVERRUN);
	}

	/* if our serial buffer was empty before, but it isn't now,
	   and rate-limiting isn't active, call out that we are
	   readable: */
	if (old_empty
	    && !tme_serial_buffer_is_empty(&serial_kb->tme_serial_kb_serial_buffer)
	    && !serial_kb->tme_serial_kb_rate_limited) {
	  serial_kb->tme_serial_kb_callout_flags 
	    |= TME_SERIAL_KB_CALLOUT_SERIAL_CTRL;
	}

	/* mark that we need to loop to callout to read more data: */
	serial_kb->tme_serial_kb_callout_flags |= TME_SERIAL_KB_CALLOUT_KEYBOARD_READ;
      }
      
      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_KEYBOARD_READ flag: */
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  serial_kb->tme_serial_kb_callout_flags = later_callouts;
}

/* the serial keyboard rate-limiting thread: */
static void
_tme_serial_kb_th_rate(struct tme_serial_kb *serial_kb)
{

  /* lock our mutex: */
  tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);

  /* loop forever: */
  for (;;) {

    /* if rate-limiting is active: */
    if (serial_kb->tme_serial_kb_rate_limited) {

      /* if we just woke up from a sleep: */
      if (serial_kb->tme_serial_kb_rate_sleeping) {
	
	/* we are no longer sleeping: */
	serial_kb->tme_serial_kb_rate_sleeping = FALSE;

	/* rate-limiting is now inactive: */
	serial_kb->tme_serial_kb_rate_limited = FALSE;

	/* if the serial buffer is not empty, call out that we are readable: */
 	if (!tme_serial_buffer_is_empty(&serial_kb->tme_serial_kb_serial_buffer)) {
	  _tme_serial_kb_callout(serial_kb,
				 TME_SERIAL_KB_CALLOUT_SERIAL_CTRL);
	}
      }

      /* otherwise, rate-limiting is active and we were woken up from
	 being idle, which means we have to sleep for the
	 rate-limiting time: */
      else {

	/* we are now sleeping: */
	serial_kb->tme_serial_kb_rate_sleeping = TRUE;

	/* unlock our mutex: */
	tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);

	/* sleep for the rate-limiting time: */
	tme_thread_sleep_yield(0, serial_kb->tme_serial_kb_rate_sleep);
	
	/* lock our mutex: */
	tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
      }
    }

    /* otherwise, rate-limiting isn't active, so just go idle: */
    else {
      assert (!serial_kb->tme_serial_kb_rate_sleeping);
      tme_cond_wait_yield(&serial_kb->tme_serial_kb_rate_cond,
			  &serial_kb->tme_serial_kb_mutex);
    }
  }
  /* NOTREACHED */
}

/* the keyboard control function: */
static int
_tme_serial_kb_keyboard_ctrl(struct tme_keyboard_connection *conn_keyboard,
			     unsigned int ctrl)
{
  struct tme_serial_kb *serial_kb;
  int new_callouts;

  /* recover our data structure: */
  serial_kb = conn_keyboard->tme_keyboard_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock our mutex: */
  tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_KEYBOARD_CTRL_OK_READ) {
    new_callouts |= TME_SERIAL_KB_CALLOUT_KEYBOARD_READ;
  }

  /* make any new callouts: */
  _tme_serial_kb_callout(serial_kb, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial configuration function: */
static int
_tme_serial_kb_serial_config(struct tme_serial_connection *conn_serial,
			     struct tme_serial_config *config)
{
  /* nothing to do: */
  return (TME_OK);
}

/* the serial control function: */
static int
_tme_serial_kb_serial_ctrl(struct tme_serial_connection *conn_serial,
			   unsigned int ctrl)
{
  struct tme_serial_kb *serial_kb;
  int new_callouts;

  /* recover our data structure: */
  serial_kb = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock our mutex: */
  tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_SERIAL_CTRL_OK_READ) {
    new_callouts |= TME_SERIAL_KB_CALLOUT_SERIAL_READ;
  }

  /* call any type-specific control function: */
  if (serial_kb->tme_serial_kb_type_serial_ctrl != NULL) {
    (*serial_kb->tme_serial_kb_type_serial_ctrl)
      (serial_kb, ctrl);
  }

  /* make any new callouts: */
  _tme_serial_kb_callout(serial_kb, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial read callin function: */
static int
_tme_serial_kb_serial_read(struct tme_serial_connection *conn_serial, 
			   tme_uint8_t *data, unsigned int count,
			   tme_serial_data_flags_t *_data_flags)
{
  struct tme_serial_kb *serial_kb;
  int rc;

  /* recover our data structures: */
  serial_kb = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);

  /* if rate-limiting is active, return no data for now: */
  if (serial_kb->tme_serial_kb_rate_limited) {
    rc = 0;
  }

  else {

    /* if rate-limiting is enabled: */
    if (serial_kb->tme_serial_kb_rate_sleep > 0) {

      /* return at most one byte: */
      count = TME_MIN(1, count);

      /* rate-limiting is now active: */
      serial_kb->tme_serial_kb_rate_limited = TRUE;
      tme_cond_notify(&serial_kb->tme_serial_kb_rate_cond, FALSE);
    }

    /* copy out data from the serial buffer: */
    rc = tme_serial_buffer_copyout(&serial_kb->tme_serial_kb_serial_buffer,
				   data, count,
				   _data_flags,
				   TME_SERIAL_COPY_NORMAL);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);

  /* done: */
  return (rc);
}

/* this scores a connection: */
static int
_tme_serial_kb_connection_score(struct tme_connection *conn,
				unsigned int *_score)
{
  struct tme_serial_kb *serial_kb;
  struct tme_keyboard_connection *conn_keyboard;

  /* recover our serial: */
  serial_kb = conn->tme_connection_element->tme_element_private;

  /* both sides must be the same type of connection, and either
     TME_CONNECTION_SERIAL or TME_CONNECTION_KEYBOARD: */
  assert((conn->tme_connection_type
	  == TME_CONNECTION_SERIAL)
	  || (conn->tme_connection_type
	      == TME_CONNECTION_KEYBOARD));
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* if this is a keyboard connection, but it's a keyboard connection
     to another keysym sink, we can't connect to it: */
  if (conn->tme_connection_type
      == TME_CONNECTION_KEYBOARD) {
    conn_keyboard
      = ((struct tme_keyboard_connection *) 
	 conn->tme_connection_other);
    if (conn_keyboard->tme_keyboard_connection_read == NULL
	|| conn_keyboard->tme_keyboard_connection_lookup == NULL) {
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
_tme_serial_kb_connection_make(struct tme_connection *conn,
			       unsigned int state)
{
  struct tme_serial_kb *serial_kb;
  struct tme_keyboard_connection *conn_keyboard;
  int kb_macro_i, kb_map_i;
  tme_keyboard_keyval_t *keysyms_lhs, *keysyms_rhs;
  struct tme_keyboard_map map_buffer;
  int rc;

  /* recover our serial keyboard: */
  serial_kb = conn->tme_connection_element->tme_element_private;

  /* both sides must be the same type of connection, and either
     TME_CONNECTION_SERIAL or TME_CONNECTION_KEYBOARD: */
  assert((conn->tme_connection_type
	  == TME_CONNECTION_SERIAL)
	  || (conn->tme_connection_type
	      == TME_CONNECTION_KEYBOARD));
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);

    /* save our connection: */
    if (conn->tme_connection_type == TME_CONNECTION_SERIAL) {

      serial_kb->tme_serial_kb_connection_serial =
	(struct tme_serial_connection *) conn->tme_connection_other;

      /* call out the serial configuration: */
      _tme_serial_kb_callout(serial_kb, TME_SERIAL_KB_CALLOUT_SERIAL_CONFIG);
    }
    
    /* when we get a keyboard connection, we actually have to do more
       work.  since the connection finally provides a keysym lookup
       function, we need to load any keysym macros, and load the map
       entries into the keyboard buffer: */
    else {

      /* first, take the other side of the connection: */
      conn_keyboard
	= ((struct tme_keyboard_connection *) 
	   conn->tme_connection_other);
      serial_kb->tme_serial_kb_connection_kb
	= conn_keyboard;

      /* if there are keysym macros: */
      if (serial_kb->tme_serial_kb_macros != NULL) {
	
	/* loop over the macros: */
	for (kb_macro_i = 0;
	     serial_kb->tme_serial_kb_macros[kb_macro_i] != NULL;
	     kb_macro_i++) {

	  /* reparse this macro, but this time lookup real keysyms.
	     because the keysym lookups will call across the
	     connection, to avoid deadlock we have to make the call
	     with our own mutex unlocked: */
	  tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);	  
	  rc = tme_keyboard_parse_macro(serial_kb->tme_serial_kb_macros[kb_macro_i],
					(tme_keyboard_keysym_lookup_t)
					conn_keyboard->tme_keyboard_connection_lookup,
					conn_keyboard,
					&keysyms_lhs,
					&keysyms_rhs);
	  tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);

	  /* if the reparsing succeeded: */
	  if (rc == TME_OK) {

	    /* add this macro: */
	    rc = tme_keyboard_buffer_in_macro(serial_kb->tme_serial_kb_keyboard_buffer,
					      keysyms_lhs,
					      keysyms_rhs);
	    tme_free(keysyms_lhs);
	    tme_free(keysyms_rhs);

	    /* if adding the macro failed: */
	    if (rc != TME_OK) {
	      /* XXX diagnostic */
	      abort();
	    }
	  }

	  /* otherwise, the reparsing failed: */
	  else {

	    /* log a complaint: */
	    tme_log(&serial_kb->tme_serial_kb_element->tme_element_log_handle, 0, ENOENT,
	      (&serial_kb->tme_serial_kb_element->tme_element_log_handle,
	       _("cannot add macro '%s', one or more keysyms are missing"),
	       serial_kb->tme_serial_kb_macros[kb_macro_i]));
	  }
	}

	/* free the macros: */
	tme_free_string_array(serial_kb->tme_serial_kb_macros, -1);
	serial_kb->tme_serial_kb_macros = NULL;
      }

      /* loop over the map entries: */
      for (kb_map_i = 0;
	   serial_kb->tme_serial_kb_map[kb_map_i] != NULL;
	   kb_map_i++) {
	
	/* reparse this map entry, but this time lookup real
	   keysyms.  because the keysym lookups will call across the
	   connection, to avoid deadlock we have to make the call
	   with our own mutex unlocked: */
	tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);	  
	rc = tme_keyboard_parse_map(serial_kb->tme_serial_kb_map[kb_map_i],
				    (tme_keyboard_keysym_lookup_t)
				    conn_keyboard->tme_keyboard_connection_lookup,
				    conn_keyboard,
				    &map_buffer);
	tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
	
	/* if the reparsing succeeded: */
	if (rc == TME_OK) {
	  
	  /* call any type-specific pre-map-adding function: */
	  if (serial_kb->tme_serial_kb_type_map_add_pre != NULL) {
	    rc = (*serial_kb->tme_serial_kb_type_map_add_pre)
	      (serial_kb,
	       &map_buffer);
	    if (rc != TME_OK) {
	      /* XXX diagnostic */
	      abort();
	    }
	  }
	  
	  /* if the reparsing succeeded, but either the keysym or
	     keycode is actually undefined, skip this entry: */
	  if ((map_buffer.tme_keyboard_map_keysym
	       == TME_KEYBOARD_KEYVAL_UNDEF)
	      || (map_buffer.tme_keyboard_map_keycode
		  == TME_KEYBOARD_KEYVAL_UNDEF)) {
	    continue;
	  }

	  /* add this map entry: */
	  rc = tme_keyboard_buffer_out_map(serial_kb->tme_serial_kb_keyboard_buffer,
					   &map_buffer);
	  
	  /* if adding the map entry failed: */
	  if (rc != TME_OK) {
	    /* XXX diagnostic */
	    abort();
	  }
	  
	  /* call any type-specific post-map-adding function: */
	  if (serial_kb->tme_serial_kb_type_map_add_post != NULL) {
	    rc = (*serial_kb->tme_serial_kb_type_map_add_post)
	      (serial_kb,
	       &map_buffer);
	    if (rc != TME_OK) {
	      /* XXX diagnostic */
	      abort();
	    }
	  }
	}
	
	/* otherwise, the reparsing failed: */
	else {
	  /* XXX diagnostic */
	  abort();
	}
      }

      /* free the map: */
      tme_free_string_array(serial_kb->tme_serial_kb_map, -1);
      serial_kb->tme_serial_kb_map = NULL;

      /* tell the other side of the connection that we're done looking
	 up keysyms.  again, to avoid deadlock we have to make the
	 call with our own mutex unlocked: */
      tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);	  
      (*conn_keyboard->tme_keyboard_connection_lookup)
	(conn_keyboard, NULL);
      tme_mutex_lock(&serial_kb->tme_serial_kb_mutex);
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&serial_kb->tme_serial_kb_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_serial_kb_connection_break(struct tme_connection *conn,
				unsigned int state)
{
  abort();
}

/* this makes a new connection side for a serial keyboard: */
static int
_tme_serial_kb_connections_new(struct tme_element *element,
			       const char * const *args,
			       struct tme_connection **_conns,
			       char **_output)
{
  struct tme_serial_kb *serial_kb;
  struct tme_keyboard_connection *conn_keyboard;
  struct tme_serial_connection *conn_serial;
  struct tme_connection *conn;

  /* recover our serial: */
  serial_kb = (struct tme_serial_kb *) element->tme_element_private;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* if we don't have a keyboard connection yet: */
  if (serial_kb->tme_serial_kb_connection_kb == NULL) {

    /* create our side of a keyboard connection: */
    conn_keyboard = tme_new0(struct tme_keyboard_connection, 1);
    conn = &conn_keyboard->tme_keyboard_connection;

    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_KEYBOARD;
    conn->tme_connection_score = _tme_serial_kb_connection_score;
    conn->tme_connection_make = _tme_serial_kb_connection_make;
    conn->tme_connection_break = _tme_serial_kb_connection_break;

    /* fill in the keyboard connection: */
    conn_keyboard->tme_keyboard_connection_ctrl = _tme_serial_kb_keyboard_ctrl;
    conn_keyboard->tme_keyboard_connection_read = NULL;
    conn_keyboard->tme_keyboard_connection_lookup = NULL;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* if we don't have a serial connection yet: */
  if (serial_kb->tme_serial_kb_connection_serial == NULL) {

    /* create our side of a serial connection: */
    conn_serial = tme_new0(struct tme_serial_connection, 1);
    conn = &conn_serial->tme_serial_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SERIAL;
    conn->tme_connection_score = _tme_serial_kb_connection_score;
    conn->tme_connection_make = _tme_serial_kb_connection_make;
    conn->tme_connection_break = _tme_serial_kb_connection_break;
    
    /* fill in the serial connection: */
    conn_serial->tme_serial_connection_config = _tme_serial_kb_serial_config;
    conn_serial->tme_serial_connection_ctrl = _tme_serial_kb_serial_ctrl;
    conn_serial->tme_serial_connection_read = _tme_serial_kb_serial_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  return (TME_OK);
}

/* this is a dummy keysym lookup function: */
tme_keyboard_keyval_t
_tme_serial_kb_lookup_dummy(void *_keysym,
			    const struct tme_keyboard_lookup *lookup)
{
  return ((*((tme_keyboard_keyval_t *) _keysym))++);
}

/* the new serial keyboard function: */
TME_ELEMENT_X_NEW_DECL(tme_serial_,kb,keyboard) {
  struct tme_serial_kb *serial_kb;
  const char *kb_type;
  int (*kb_init) _TME_P((struct tme_serial_kb *));
  unsigned int kb_list_i;
  const char *kb_macros_filename;
  FILE *kb_macros_file;
  char **kb_macros;
  unsigned int kb_macros_count;
  int kb_macros_bad;
  const char *kb_map_filename;
  FILE *kb_map_file;
  char **kb_map;
  unsigned int kb_map_count;
  int kb_map_bad;
  char **tokens;
  int tokens_count;
  unsigned int line_number;
  char line_buffer[1024], *p1, c;
  tme_keyboard_keyval_t keysym;
  tme_keyboard_keyval_t *keysyms_lhs, *keysyms_rhs;
  int in_map;
  struct tme_keyboard_map map_buffer;
  int rate;
  int usage;
  int arg_i;
  int rc;

  /* initialize: */
  kb_type = NULL;
  kb_macros_filename = NULL;
  kb_map_filename = NULL;
  rate = 0;
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    /* the keyboard type we're emulating: */
    if (TME_ARG_IS(args[arg_i + 0], "type")
	&& args[arg_i + 1] != NULL
	&& kb_type == NULL) {
      kb_type = args[arg_i + 1];
      arg_i += 2;
    }

    /* the macros file: */
    else if (TME_ARG_IS(args[arg_i + 0], "macros")
	     && args[arg_i + 1] != NULL
	     && kb_macros_filename == NULL) {
      kb_macros_filename = args[arg_i + 1];
      arg_i += 2;
    }

    /* the map file: */
    else if (TME_ARG_IS(args[arg_i + 0], "map")
	     && args[arg_i + 1] != NULL
	     && kb_map_filename == NULL) {
      kb_map_filename = args[arg_i + 1];
      arg_i += 2;
    }

    /* a limiting rate: */
    else if (TME_ARG_IS(args[arg_i + 0], "rate")
	     && args[arg_i + 1] != NULL
	     && (rate = atoi(args[arg_i + 1])) > 0) {
      arg_i += 2;
    }
    
    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      /* we must have been given a type and a map file: */
      if (kb_type == NULL
	  || kb_map_filename == NULL) {
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
			    "%s %s type %s [ macros %s ] map %s [ rate %s ]",
			    _("usage:"),
			    args[0],
			    _("KEYBOARD-TYPE"),
			    _("FILENAME"),
			    _("FILENAME"),
			    _("RATE"));
    return (EINVAL);
  }

  /* make sure that this keyboard type is known: */
  kb_init = NULL;
  for (kb_list_i = 0;
       kb_list_i < TME_ARRAY_ELS(_tme_serial_kb_list);
       kb_list_i++) {
    if (!strcmp(_tme_serial_kb_list[kb_list_i].tme_serial_kb_list_type,
		kb_type)) {
      kb_init = _tme_serial_kb_list[kb_list_i].tme_serial_kb_list_init;
      break;
    }
  }
  if (kb_init == NULL) {
    tme_output_append_error(_output, "%s", kb_type);
    return (ENOENT);
  }

  /* read in and store any macros.  we can't really parse them until
     we have a keyboard connection, since before then we don't have
     any string->keysym mapping, but we do sanity check that they
     parse: */
  kb_macros = NULL;
  kb_macros_count = 0;
  if (kb_macros_filename != NULL) {

    /* try to open the macros file: */
    kb_macros_file = fopen(kb_macros_filename, "r");
    if (kb_macros_file == NULL) {
      tme_output_append_error(_output, "%s", kb_macros_filename);
      return (errno);
    }
    
    /* loop over all of the lines in the file: */
    kb_macros_bad = FALSE;
    keysym = 1;
    for (line_number = 1;; line_number++) {

      /* get the next line from the file, skipping blank lines
	 and lines that begin with the comment character: */
      if (fgets(line_buffer, sizeof(line_buffer) - 1, kb_macros_file) == NULL) {
	break;
      }
      line_buffer[sizeof(line_buffer) - 1] = '\0';
      if ((p1 = strchr(line_buffer, '\n')) != NULL) {
	*p1 = '\0';
      }
      for (p1 = line_buffer;
	   ((c = *(p1++)) != '\0'
	    && isspace((unsigned char) c)););
      if (c == '\0'
	  || c == '#') {
	continue;
      }

      /* store this macro.  this string array always has one extra
	 slot, which will be filled with NULL to terminate the array: */
      if (kb_macros_count == 0) {
	kb_macros = tme_new(char *, 2);
      }
      else {
	kb_macros = tme_renew(char *, kb_macros, kb_macros_count + 2);
      }
      kb_macros[kb_macros_count++] = tme_strdup(line_buffer);

      /* check that this macro parses correctly: */
      rc = tme_keyboard_parse_macro(line_buffer,
				    _tme_serial_kb_lookup_dummy,
				    &keysym,
				    &keysyms_lhs,
				    &keysyms_rhs);
      if (rc != TME_OK) {
	tme_output_append_error(_output,
				"%s:%u: %s\n",
				kb_macros_filename,
				line_number,
				strerror(rc));
	kb_macros_bad = TRUE;
      }
      else {
	tme_free(keysyms_lhs);
	tme_free(keysyms_rhs);
      }
    }
    fclose(kb_macros_file);
    if (kb_macros_count > 0) {
      kb_macros[kb_macros_count] = NULL;
    }

    /* fail if one or more keyboard macros were bad: */
    if (kb_macros_bad) {
      if (kb_macros != NULL) {
	tme_free_string_array(kb_macros, -1);
      }
      return (EINVAL);
    }
  }

  /* read in and store the map entries.  we can't really parse them
     until we have a keyboard connection, since before then we don't
     have any string->keysym mapping, but we do sanity check that they
     parse: */
  kb_map = NULL;
  kb_map_count = 0;

  /* try to open the map file: */
  kb_map_file = fopen(kb_map_filename, "r");
  if (kb_map_file == NULL) {
    tme_output_append_error(_output, "%s", kb_map_filename);
    if (kb_macros != NULL) {
      tme_free_string_array(kb_macros, -1);
    }
    return (errno);
  }
    
  /* loop over all of the lines in the file: */
  kb_map_bad = FALSE;
  keysym = 1;
  in_map = FALSE;
  for (line_number = 1;; line_number++) {

    /* get the next line from the file, skipping blank lines
       and lines that begin with the comment character: */
    if (fgets(line_buffer, sizeof(line_buffer) - 1, kb_map_file) == NULL) {
      break;
    }
    line_buffer[sizeof(line_buffer) - 1] = '\0';
    if ((p1 = strchr(line_buffer, '\n')) != NULL) {
      *p1 = '\0';
    }
    for (p1 = line_buffer;
	 ((c = *(p1++)) != '\0'
	  && isspace((unsigned char) c)););
    if (c == '\0'
	|| c == '#') {
      continue;
    }

    /* if we're not in the map we want, wait for it to begin: */
    if (!in_map) {
      tokens = tme_misc_tokenize(line_buffer, '#', &tokens_count);
      if (tokens_count == 3
	  && !strcmp(tokens[0], "map")
	  && !strcmp(tokens[1], kb_type)
	  && !strcmp(tokens[2], "{")) {
	in_map = TRUE;
      }
      tme_free_string_array(tokens, -1);
      continue;
    }

    /* check if this map is ending: */
    tokens = tme_misc_tokenize(line_buffer, '#', &tokens_count);
    in_map = (tokens_count != 1
	      || strcmp(tokens[0], "}"));
    tme_free_string_array(tokens, -1);
    if (!in_map) {
      continue;
    }
    
    /* store this map entry.  this string array always has one extra
       slot, which will be filled with NULL to terminate the array: */
    if (kb_map_count == 0) {
      kb_map = tme_new(char *, 2);
    }
    else {
      kb_map = tme_renew(char *, kb_map, kb_map_count + 2);
    }
    kb_map[kb_map_count++] = tme_strdup(line_buffer);

    /* check that this map entry parses correctly: */
    rc = tme_keyboard_parse_map(line_buffer,
				_tme_serial_kb_lookup_dummy,
				&keysym,
				&map_buffer);
    if (rc != TME_OK) {
      tme_output_append_error(_output,
			      "%s:%u: %s\n",
			      kb_macros_filename,
			      line_number,
			      strerror(rc));
      kb_map_bad = TRUE;
    }
  }
  fclose(kb_map_file);
  if (kb_map_count > 0) {
    kb_map[kb_map_count] = NULL;
  }

  /* fail if there aren't any map entries: */
  else {
    if (kb_macros != NULL) {
      tme_free_string_array(kb_macros, -1);
    }
    tme_output_append_error(_output,
			    "%s: %s\n",
			    kb_macros_filename,
			    kb_type);
    return (ENOENT);
  }

  /* fail if one or more keyboard map entries were bad: */
  if (kb_map_bad) {
    if (kb_macros != NULL) {
      tme_free_string_array(kb_macros, -1);
    }
    if (kb_map != NULL) {
      tme_free_string_array(kb_map, -1);
    }
    return (EINVAL);
  }

  /* start the serial keyboard structure: */
  serial_kb = tme_new0(struct tme_serial_kb, 1);
  serial_kb->tme_serial_kb_element = element;
  tme_mutex_init(&serial_kb->tme_serial_kb_mutex);
  serial_kb->tme_serial_kb_type = kb_type;
  serial_kb->tme_serial_kb_macros = kb_macros;
  serial_kb->tme_serial_kb_map = kb_map;
  serial_kb->tme_serial_kb_connection_kb = NULL;
  serial_kb->tme_serial_kb_connection_serial = NULL;
  serial_kb->tme_serial_kb_callout_flags = 0;
  serial_kb->tme_serial_kb_keyboard_ctrl
    = 0;
  serial_kb->tme_serial_kb_keyboard_buffer
    = tme_keyboard_buffer_new(TME_SERIAL_KB_BUFFER_SIZE);
  serial_kb->tme_serial_kb_keyboard_buffer->tme_keyboard_buffer_log_handle
    = &element->tme_element_log_handle;

  if (rate > 0) {
    serial_kb->tme_serial_kb_rate_sleep = (1000000UL / rate);
    tme_cond_init(&serial_kb->tme_serial_kb_rate_cond);
    tme_thread_create((tme_thread_t) _tme_serial_kb_th_rate, serial_kb);
  }
  serial_kb->tme_serial_kb_serial_ctrl
    = (TME_SERIAL_CTRL_DTR
       | TME_SERIAL_CTRL_DCD);
  tme_serial_buffer_init(&serial_kb->tme_serial_kb_serial_buffer, 
			 TME_SERIAL_KB_BUFFER_SIZE);
  (*kb_init)(serial_kb);

  /* fill the element: */
  element->tme_element_private = serial_kb;
  element->tme_element_connections_new = _tme_serial_kb_connections_new;

  return (TME_OK);
}
