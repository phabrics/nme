/* $Id: posix-serial.c,v 1.11 2007/08/24 00:57:01 fredette Exp $ */

/* host/posix/posix-serial.c - implementation of serial ports on a POSIX system: */

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
_TME_RCSID("$Id: posix-serial.c,v 1.11 2007/08/24 00:57:01 fredette Exp $");

/* includes: */
#include <tme/generic/serial.h>
#include <tme/threads.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

/* macros: */
#define TME_POSIX_SERIAL_BUFFER_SIZE	(4096)

/* structures: */
struct tme_posix_serial {

  /* our mutex: */
  tme_mutex_t tme_posix_serial_mutex;

  /* backpointer to our element: */
  struct tme_element *tme_posix_serial_element;

  /* our connection: */
  struct tme_serial_connection *tme_posix_serial_connection;

  /* our writer thread condition: */
  tme_cond_t tme_posix_serial_cond_writer;

  /* this is nonzero iff callouts are running: */
  int tme_posix_serial_callouts_running;

  /* our input file descriptor: */
  int tme_posix_serial_fd_in;

  /* our output file descriptor: */
  int tme_posix_serial_fd_out;

  /* if we're emulating break: */
  int tme_posix_serial_emulate_break;

  /* our current control inputs: */
  unsigned int tme_posix_serial_ctrl_callin;

  /* the number of control output cycles to assert BREAK: */
  int tme_posix_serial_ctrl_callout_break;

  /* our current control outputs: */
  unsigned int tme_posix_serial_ctrl_callout;

  /* the last control outputs we successfully called out: */
  unsigned int tme_posix_serial_ctrl_callout_last;
  
  /* our input and output buffers: */
  struct tme_serial_buffer tme_posix_serial_buffer_in;
  struct tme_serial_buffer tme_posix_serial_buffer_out;

  /* our input scanner state: */
  int tme_posix_serial_input_scanner_state;
};

/* the serial callout function.  it must be called with the mutex locked: */
static void
_tme_posix_serial_callout(struct tme_posix_serial *serial)
{
  struct tme_serial_connection *conn_serial;
  unsigned int ctrl;
  tme_uint8_t buffer_input[1024];
  unsigned int buffer_input_size;
  tme_serial_data_flags_t data_flags;
  int rc;
  int again;

  /* if this function is already running in another thread, return
     now.  the other thread will do our work: */
  if (serial->tme_posix_serial_callouts_running) {
    return;
  }

  /* callouts are now running: */
  serial->tme_posix_serial_callouts_running = TRUE;

  /* loop running callouts until there is nothing to do: */
  for (again = TRUE; again;) {
    again = FALSE;

    /* if we're connected: */
    conn_serial = serial->tme_posix_serial_connection;
    if (conn_serial != NULL) {

      /* if we need to notify our connection of new ctrl bits: */
      ctrl = serial->tme_posix_serial_ctrl_callout;
      if (ctrl != serial->tme_posix_serial_ctrl_callout_last) {

	/* unlock the mutex: */
	tme_mutex_unlock(&serial->tme_posix_serial_mutex);

	/* try to do the notify: */
	rc = (*conn_serial->tme_serial_connection_ctrl)(conn_serial, ctrl);

	/* lock the mutex: */
	tme_mutex_lock(&serial->tme_posix_serial_mutex);

	/* if the notify was successful, note what we sent, and allow
	   the outermost loop to run again: */
	if (rc == TME_OK) {
	  serial->tme_posix_serial_ctrl_callout_last = ctrl;
	  again = TRUE;
	}
      }

      /* if our connection is readable, and our output buffer isn't full,
	 read the connection: */
      if ((serial->tme_posix_serial_ctrl_callin & TME_SERIAL_CTRL_OK_READ)
	  && !tme_serial_buffer_is_full(&serial->tme_posix_serial_buffer_out)) {

	/* get the minimum of the free space in the output buffer and
	   the size of our stack buffer: */
	buffer_input_size = tme_serial_buffer_space_free(&serial->tme_posix_serial_buffer_out);
	buffer_input_size = TME_MIN(buffer_input_size, sizeof(buffer_input));

	/* unlock the mutex: */
	tme_mutex_unlock(&serial->tme_posix_serial_mutex);

	/* do the read: */
	rc = (*conn_serial->tme_serial_connection_read)
	  (conn_serial,
	   buffer_input, 
	   buffer_input_size,
	   &data_flags);

	/* lock the mutex: */
	tme_mutex_lock(&serial->tme_posix_serial_mutex);

	/* if the read was successful, add what we got and notify the
	   writer so that it may try to write: */
	if (rc > 0) {
	  (void) tme_serial_buffer_copyin(&serial->tme_posix_serial_buffer_out,
					  buffer_input,
					  rc,
					  data_flags,
					  TME_SERIAL_COPY_NORMAL);
	  tme_cond_notify(&serial->tme_posix_serial_cond_writer, TRUE);

	  /* allow the outermost loop to run again: */
	  again = TRUE;
	}

	/* otherwise, the read failed, and the convention dictates
	   that we forget this connection was readable: */
	else {
	  serial->tme_posix_serial_ctrl_callin &= ~TME_SERIAL_CTRL_OK_READ;
	}
      }
    }
  }

  /* there are no more callouts to make: */
  serial->tme_posix_serial_callouts_running = FALSE;
}

/* the serial control thread: */
static void
_tme_posix_serial_th_ctrl(struct tme_posix_serial *serial)
{
  int modem_state, modem_state_out;
  unsigned int ctrl;

  /* loop forever: */
  for (;;) {
   
    /* get the modem state of the input device: */
    if (ioctl(serial->tme_posix_serial_fd_in, TIOCMGET, &modem_state) < 0) {
      modem_state = 0;
    }

    /* if the output device is different, get the modem state of the
       output device and merge it in: */
    if (serial->tme_posix_serial_fd_out
	!= serial->tme_posix_serial_fd_in) {
      if (ioctl(serial->tme_posix_serial_fd_in, TIOCMGET, &modem_state_out) < 0) {
	modem_state_out = 0;
      }
      modem_state &= ~(TIOCM_DTR | TIOCM_RTS | TIOCM_CTS);
      modem_state |= modem_state_out & ~(TIOCM_CD | TIOCM_RI | TIOCM_DSR);
    }
    
    /* lock the mutex: */
    tme_mutex_lock(&serial->tme_posix_serial_mutex);
    
    /* update the control outputs: */
    ctrl = (serial->tme_posix_serial_ctrl_callout
	    & ~(TME_SERIAL_CTRL_CTS
		| TME_SERIAL_CTRL_DCD
		| TME_SERIAL_CTRL_RI
		| TME_SERIAL_CTRL_BREAK));
    if (modem_state & TIOCM_CTS) {
      ctrl |= TME_SERIAL_CTRL_CTS;
    }
    if (modem_state & TIOCM_CD) {
      ctrl |= TME_SERIAL_CTRL_DCD;
    }
    if (modem_state & TIOCM_RI) {
      ctrl |= TME_SERIAL_CTRL_RI;
    }
    if (serial->tme_posix_serial_ctrl_callout_break > 0) {
      ctrl |= TME_SERIAL_CTRL_BREAK;
      serial->tme_posix_serial_ctrl_callout_break--;
    }
    
    /* if the control outputs have changed, call out the change: */
    if (ctrl != serial->tme_posix_serial_ctrl_callout) {
      serial->tme_posix_serial_ctrl_callout = ctrl;
      _tme_posix_serial_callout(serial);
    }
    
    /* unlock the mutex: */
    tme_mutex_unlock(&serial->tme_posix_serial_mutex);

    /* check the controls again in .5 seconds: */
    tme_thread_sleep_yield(0, 500000);
  }
  /* NOTREACHED */
}

/* the serial writer thread: */
static void
_tme_posix_serial_th_writer(struct tme_posix_serial *serial)
{
  tme_uint8_t buffer_output[1024];
  unsigned int buffer_output_size;
  int rc;

  /* lock the mutex: */
  tme_mutex_lock(&serial->tme_posix_serial_mutex);

  /* loop forever: */
  for (;;) {

    /* if there is no data to write, wait on the writer condition,
       after which there must be data to write: */
    if (tme_serial_buffer_is_empty(&serial->tme_posix_serial_buffer_out)) {
      tme_cond_wait_yield(&serial->tme_posix_serial_cond_writer,
			  &serial->tme_posix_serial_mutex);
    }

    /* get the data to write: */
    buffer_output_size = 
      tme_serial_buffer_copyout(&serial->tme_posix_serial_buffer_out,
				buffer_output,
				sizeof(buffer_output),
				NULL,
				TME_SERIAL_COPY_PEEK);
    assert(buffer_output_size > 0);

    /* unlock the mutex: */
    tme_mutex_unlock(&serial->tme_posix_serial_mutex);

    /* try to write the device: */
    rc = tme_thread_write_yield(serial->tme_posix_serial_fd_out,
				buffer_output,
				buffer_output_size);

    /* lock the mutex: */
    tme_mutex_lock(&serial->tme_posix_serial_mutex);

    /* if the write was successful: */
    if (rc > 0) {

      /* remove the written data from the output buffer: */
      tme_serial_buffer_copyout(&serial->tme_posix_serial_buffer_out,
				NULL,
				rc,
				NULL,
				TME_SERIAL_COPY_NORMAL);

      /* call out for more data: */
      _tme_posix_serial_callout(serial);
    }
  }
  /* NOTREACHED */
}
    
/* the serial reader thread: */
static void
_tme_posix_serial_th_reader(struct tme_posix_serial *serial)
{
  tme_uint8_t buffer_input[1024];
  tme_uint8_t buffer_slack[10];
  tme_uint8_t byte, *byte_head, *byte_tail;
  int scanner_state;
  int buffer_was_empty;
  int rc;

  /* loop forever: */
  for (;;) {

    /* try to read the device: */
    rc = tme_thread_read_yield(serial->tme_posix_serial_fd_in,
			       buffer_input,
			       sizeof(buffer_input));

    /* if the read failed: */
    if (rc < 0) {
      /* XXX diagnostic */
      continue;
    }

    /* if we hit EOF: */
    if (rc == 0) {
      return;
    }

    /* lock the mutex: */
    tme_mutex_lock(&serial->tme_posix_serial_mutex);

    /* remember if this buffer was empty: */
    buffer_was_empty =
      tme_serial_buffer_is_empty(&serial->tme_posix_serial_buffer_in);

    /* scan the input: */
    byte_head = buffer_input;
    scanner_state = serial->tme_posix_serial_input_scanner_state;
    for (; rc > 0; ) {
	  
      /* in state zero, we haven't seen anything interesting: */
      if (scanner_state == 0) {
	
	/* scan quickly, stopping only when we see an escape: */
	byte_tail = byte_head;
	if (serial->tme_posix_serial_emulate_break) {
	  do {
	    byte = *(byte_head++);
	    if (byte == 0xff
		|| byte == '^') {
	      break;
	    }
	  } while (--rc > 0);
	}
	else {
	  do {
	    byte = *(byte_head++);
	    if (byte == 0xff) {
	      break;
	    }
	  } while (--rc > 0);
	}
	
	/* if we stopped scanning because we saw an escape, back up: */
	if (rc > 0) {
	  byte_head--;
	}

	/* add in the normal data we scanned quickly: */
	if (byte_head > byte_tail) {
	  (void) tme_serial_buffer_copyin(&serial->tme_posix_serial_buffer_in,
					  byte_tail,
					  (byte_head - byte_tail),
					  TME_SERIAL_DATA_NORMAL,
					  TME_SERIAL_COPY_FULL_IS_OVERRUN);
	}

	/* if this byte is an 0xff, move to state one: */
	if (byte == 0xff) {
	  byte_head++;
	  --rc;
	  scanner_state = 1;
	}

	/* if we're emulating breaks, and this is a carat, move to
	   state eight: */
	else if (serial->tme_posix_serial_emulate_break
		 && byte == '^') {
	  byte_head++;
	  --rc;
	  scanner_state = 8;
	}
	
	/* otherwise, we must have drained everything: */
	else {
	  assert(rc == 0);
	}
      }

      /* in state one, we have seen 0xff: */
      else if (scanner_state == 1) {

	/* get the next byte: */
	byte = *(byte_head++);
	--rc;
	
	/* if this is an 0x00, move to state two: */
	if (byte == 0x00) {
	  scanner_state = 2;
	}
	
	/* otherwise, receive 0xff, and if this is not an 0xff, return
	   it to the buffer.  move to state zero: */
	else {
	  buffer_slack[0] = 0xff;
	  (void) tme_serial_buffer_copyin(&serial->tme_posix_serial_buffer_in,
					  buffer_slack,
					  1,
					  TME_SERIAL_DATA_NORMAL,
					  TME_SERIAL_COPY_FULL_IS_OVERRUN);
	  if (byte != 0xff) {
	    byte_head--;
	    ++rc;
	  }
	  scanner_state = 0;
	}
      }

      /* in state two, we have seen 0xff 0x00: */
      else if (scanner_state == 2) {

	/* get the next byte: */
	byte = *(byte_head++);
	--rc;
	
	/* if this is an 0x00, this is a break: */
	if (byte == 0x00) {

	  /* if break isn't already being asserted, assert it and call
	     out the change.  always reset the assertion time: */
	  if (!(serial->tme_posix_serial_ctrl_callout & TME_SERIAL_CTRL_BREAK)) {
	    serial->tme_posix_serial_ctrl_callout |= TME_SERIAL_CTRL_BREAK;
	    _tme_posix_serial_callout(serial);
	  }
	  serial->tme_posix_serial_ctrl_callout_break = 2;
	}

	/* otherwise, this byte was received with bad framing or
	   parity.  we can't tell which, so call it bad parity: */
	else {
	  (void) tme_serial_buffer_copyin(&serial->tme_posix_serial_buffer_in,
					  byte_head - 1,
					  1,
					  TME_SERIAL_DATA_BAD_PARITY,
					  TME_SERIAL_COPY_FULL_IS_OVERRUN);
	}
	
	/* move to state zero: */
	scanner_state = 0;
      }

      /* in states eight and nine, we have seen one or two break escapes,
	 respectively: */
      else if (scanner_state == 8
	       || scanner_state == 9) {
	assert(serial->tme_posix_serial_emulate_break);
	
	/* get the next byte: */
	byte = *(byte_head++);
	--rc;
	
	/* if this isn't also a break escape, return it to the
	   buffer, receive the break escapes, and move to state
	   zero: */
	if (byte != '^') {
	  byte_head--;
	  ++rc;
	  buffer_slack[0] = buffer_slack[1] = '^';
	  (void) tme_serial_buffer_copyin(&serial->tme_posix_serial_buffer_in,
					  buffer_slack,
					  scanner_state - 7,
					  TME_SERIAL_DATA_NORMAL,
					  TME_SERIAL_COPY_FULL_IS_OVERRUN);
	  scanner_state = 0;
	}
	
	/* otherwise, this is a break escape.  if we have now seen
	   three total, this is a break, and move to state zero: */
	else if (++scanner_state == 10) {

	  /* if break isn't already being asserted, assert it and call
	     out the change.  always reset the assertion time: */
	  if (!(serial->tme_posix_serial_ctrl_callout & TME_SERIAL_CTRL_BREAK)) {
	    serial->tme_posix_serial_ctrl_callout |= TME_SERIAL_CTRL_BREAK;
	    _tme_posix_serial_callout(serial);
	  }
	  serial->tme_posix_serial_ctrl_callout_break = 2;
	  scanner_state = 0;
	}
      }
      
      /* any other state is impossible: */
      else {
	assert(FALSE);
      }
    }
	      
    /* update the scanner state: */
    serial->tme_posix_serial_input_scanner_state = scanner_state;

    /* if the input buffer was previously empty, and it is now not
       empty, call out that we can be read again: */
    if (buffer_was_empty
	&& !tme_serial_buffer_is_empty(&serial->tme_posix_serial_buffer_in)) {
      serial->tme_posix_serial_ctrl_callout |= TME_SERIAL_CTRL_OK_READ;
      _tme_posix_serial_callout(serial);
    }

    /* unlock the mutex: */
    tme_mutex_unlock(&serial->tme_posix_serial_mutex);
  }
  /* NOTREACHED */
}

/* the serial configuration callin function: */
static int
_tme_posix_serial_config(struct tme_serial_connection *conn_serial, struct tme_serial_config *config)
{
  struct tme_posix_serial *serial;
  struct termios serial_termios;
  tme_uint32_t config_baud;
  speed_t termios_baud;
  int is_input, rc;

  /* recover our data structure: */
  serial = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&serial->tme_posix_serial_mutex);

  /* loop over the input and output devices: */
  for (is_input = 2; is_input-- > 0; ) {

    /* get the current configuration of the device: */
    rc = tcgetattr((is_input
		    ? serial->tme_posix_serial_fd_in
		    : serial->tme_posix_serial_fd_out),
		   &serial_termios);
    
    /* update the configuration: */
    
    /* baud rate: */
    config_baud = config->tme_serial_config_baud;
    termios_baud = (config_baud == 0 ? B0
		    : config_baud <= 50 ? B50
		    : config_baud <= 75 ? B75
		    : config_baud <= 110 ? B110
		    : config_baud <= 134 ? B134
		    : config_baud <= 150 ? B150
		    : config_baud <= 200 ? B200
		    : config_baud <= 300 ? B300
		    : config_baud <= 600 ? B600
		    : config_baud <= 1200 ? B1200
		    : config_baud <= 1800 ? B1800
		    : config_baud <= 2400 ? B2400
		    : config_baud <= 4800 ? B4800
		    : config_baud <= 9600 ? B9600
		    : config_baud <= 19200 ? B19200
		    : config_baud <= 38400 ? B38400
		    : (speed_t) -1);
    if (termios_baud == (speed_t) -1) {
      /* XXX diagnostic */
      termios_baud = B38400;
    }
    rc = cfsetspeed(&serial_termios, termios_baud);

    /* input mode or output mode: */
    if (is_input) {
      serial_termios.c_iflag = PARMRK;
      if (config->tme_serial_config_flags & TME_SERIAL_FLAGS_CHECK_PARITY) {
	serial_termios.c_iflag = INPCK;
      }
    }
    else {
      serial_termios.c_oflag = 0;
    }

    /* control mode: */
    serial_termios.c_cflag = CREAD | CLOCAL;
    switch (config->tme_serial_config_bits_data) {
    case 5: serial_termios.c_cflag |= CS5; break;
    case 6: serial_termios.c_cflag |= CS6; break;
    case 7: serial_termios.c_cflag |= CS7; break;
    default: assert(FALSE);
    case 8: serial_termios.c_cflag |= CS8; break;
    }
    switch (config->tme_serial_config_bits_stop) {
    case 1: break;
    default: assert(FALSE);
    case 2: serial_termios.c_cflag |= CSTOPB; break;
    }
    switch (config->tme_serial_config_parity) {
    default: assert(FALSE);
    case TME_SERIAL_PARITY_NONE: break;
    case TME_SERIAL_PARITY_ODD: serial_termios.c_cflag |= PARENB | PARODD; break;
    case TME_SERIAL_PARITY_EVEN: serial_termios.c_cflag |= PARENB; break;
    }
  
    /* local mode: */
    serial_termios.c_lflag = 0;

    /* set the configuration on the devices: */
    rc = tcsetattr((is_input
		    ? serial->tme_posix_serial_fd_in
		    : serial->tme_posix_serial_fd_out),
		   TCSADRAIN,
		   &serial_termios);
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&serial->tme_posix_serial_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial control callin function: */
static int
_tme_posix_serial_ctrl(struct tme_serial_connection *conn_serial, unsigned int control)
{
  struct tme_posix_serial *serial;
  int modem_state;
  int rc;

  /* recover our data structure: */
  serial = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&serial->tme_posix_serial_mutex);

  /* get the current output device modem state: */
  rc = ioctl(serial->tme_posix_serial_fd_out, TIOCMGET, &modem_state);

  /* update the modem state: */
  if (control & TME_SERIAL_CTRL_DTR) {
    modem_state |= TIOCM_DTR;
  }
  else {
    modem_state &= TIOCM_DTR;
  }
  if (control & TME_SERIAL_CTRL_RTS) {
    modem_state |= TIOCM_RTS;
  }
  else {
    modem_state &= TIOCM_RTS;
  }

  /* set the new modem state: */
  rc = ioctl(serial->tme_posix_serial_fd_out, TIOCMSET, &modem_state);

  /* send a break: */
  if (control & TME_SERIAL_CTRL_BREAK) {
    tcsendbreak(serial->tme_posix_serial_fd_out, 0);
  }

  /* remember these controls.  if the OK_READ is set, call out a read: */
  serial->tme_posix_serial_ctrl_callin = control;
  if (control & TME_SERIAL_CTRL_OK_READ) {
    _tme_posix_serial_callout(serial);
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&serial->tme_posix_serial_mutex);

  /* done: */
  return (TME_OK);
}

/* the serial read callin function: */
static int
_tme_posix_serial_read(struct tme_serial_connection *conn_serial, 
		       tme_uint8_t *data, unsigned int count,
		       tme_serial_data_flags_t *_data_flags)
{
  struct tme_posix_serial *serial;
  unsigned int rc;

  /* recover our data structure: */
  serial = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&serial->tme_posix_serial_mutex);

  /* copy data out of the input buffer: */
  rc = tme_serial_buffer_copyout(&serial->tme_posix_serial_buffer_in,
				 data, count,
				 _data_flags,
				 TME_SERIAL_COPY_NORMAL);

  /* if the input buffer is now empty, our connection shouldn't read
     again until we have filled some of the buffer.  we don't call
     this transition out, because the convention dictates that
     our connection forget that we're readable: */
  if (rc < count) {
    serial->tme_posix_serial_ctrl_callout &= ~TME_SERIAL_CTRL_OK_READ;
    serial->tme_posix_serial_ctrl_callout_last &= ~TME_SERIAL_CTRL_OK_READ;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&serial->tme_posix_serial_mutex);

  /* done: */
  return (rc);
}

/* this scores a connection: */
static int
_tme_posix_serial_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_posix_serial *serial;

  /* recover our serial: */
  serial = conn->tme_connection_element->tme_element_private;

  /* both sides must be serial connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SERIAL);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SERIAL);

  /* serial connections are always good: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_posix_serial_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_posix_serial *serial;

  /* recover our serial: */
  serial = conn->tme_connection_element->tme_element_private;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SERIAL);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SERIAL);

  /* serials are always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&serial->tme_posix_serial_mutex);

    /* save our connection: */
    serial->tme_posix_serial_connection =
      (struct tme_serial_connection *) conn->tme_connection_other;

    /* do any pending callouts: */
    _tme_posix_serial_callout(serial);

    /* unlock our mutex: */
    tme_mutex_unlock(&serial->tme_posix_serial_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_posix_serial_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a serial: */
static int
_tme_posix_serial_connections_new(struct tme_element *element,
				  const char * const *args,
				  struct tme_connection **_conns,
				  char **_output)
{
  struct tme_posix_serial *serial;
  struct tme_serial_connection *conn_serial;
  struct tme_connection *conn;

  /* recover our serial: */
  serial = (struct tme_posix_serial *) element->tme_element_private;

  /* if this serial is already connected, we can do nothing: */
  if (serial->tme_posix_serial_connection != NULL) {
    return (EISCONN);
  }

  /* create our side of a serial connection: */
  conn_serial = tme_new0(struct tme_serial_connection, 1);
  conn = &conn_serial->tme_serial_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_SERIAL;
  conn->tme_connection_score = _tme_posix_serial_connection_score;
  conn->tme_connection_make = _tme_posix_serial_connection_make;
  conn->tme_connection_break = _tme_posix_serial_connection_break;

  /* fill in the serial connection: */
  conn_serial->tme_serial_connection_config = _tme_posix_serial_config;
  conn_serial->tme_serial_connection_ctrl = _tme_posix_serial_ctrl;
  conn_serial->tme_serial_connection_read = _tme_posix_serial_read;

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new serial function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_posix,serial) {
  struct tme_posix_serial *serial;
  const char *filename_in;
  const char *filename_out;
  int fd_in, fd_out;
  int usage;
  int arg_i;
  int saved_errno;
  int emulate_break;

  /* initialize: */
  filename_in = NULL;
  filename_out = NULL;
  emulate_break = FALSE;
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    /* the device we're supposed to use for input: */
    if (TME_ARG_IS(args[arg_i + 0], "device-input")
	&& args[arg_i + 1] != NULL
	&& filename_in == NULL) {
      filename_in = args[arg_i + 1];
      arg_i += 2;
    }

    /* the device we're supposed to use for output: */
    else if (TME_ARG_IS(args[arg_i + 0], "device-output")
	     && args[arg_i + 1] != NULL
	     && filename_out == NULL) {
      filename_out = args[arg_i + 1];
      arg_i += 2;
    }

    /* the device we're supposed to use for input and output: */
    else if (TME_ARG_IS(args[arg_i + 0], "device")
	     && args[arg_i + 1] != NULL
	     && filename_in == NULL
	     && filename_out == NULL) {
      filename_in = filename_out = args[arg_i + 1];
      arg_i += 2;
    }

    /* if we're supposed to emulate break: */
    else if (TME_ARG_IS(args[arg_i + 0], "break-carats")) {
      emulate_break = TRUE;
      arg_i++;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      /* we must have been given input and output devices: */
      if (filename_in == NULL
	  || filename_out == NULL) {
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
			    "%s %s { device %s | { device-input %s device-output %s } } [break-carats]",
			    _("usage:"),
			    args[0],
			    _("DEVICE"),
			    _("DEVICE"),
			    _("DEVICE"));
    return (EINVAL);
  }

  /* open the devices: */
  fd_in = fd_out = -1;
  if (fd_in < 0
      && !strcmp(filename_in, "-")) {
    fd_in = STDIN_FILENO;
  }
  if (fd_out < 0
      && !strcmp(filename_out, "-")) {
    fd_out = STDOUT_FILENO;
  }
  if (fd_in < 0) {
    if (strcmp(filename_in, filename_out) == 0) {
      fd_in = fd_out = open(filename_in, O_RDWR | O_NONBLOCK);
    }
    else {
      fd_in = open(filename_in, O_RDONLY | O_NONBLOCK);
    }
    if (fd_in < 0) {
      tme_output_append_error(_output, "%s", filename_in);
      return (errno);
    }
  }
  if (fd_out < 0) {
    fd_out = open(filename_out, O_WRONLY | O_NONBLOCK);
    if (fd_out < 0) {
      saved_errno = errno;
      close(fd_in);
      tme_output_append_error(_output, "%s", filename_out);
      return (saved_errno);
    }
  }

  /* start the serial structure: */
  serial = tme_new0(struct tme_posix_serial, 1);
  serial->tme_posix_serial_element = element;
  serial->tme_posix_serial_fd_in = fd_in;
  serial->tme_posix_serial_fd_out = fd_out;
  serial->tme_posix_serial_emulate_break = emulate_break;
  serial->tme_posix_serial_ctrl_callout = 0;
  serial->tme_posix_serial_ctrl_callout_last = 0;
  tme_serial_buffer_init(&serial->tme_posix_serial_buffer_in, 
			 TME_POSIX_SERIAL_BUFFER_SIZE);
  tme_serial_buffer_init(&serial->tme_posix_serial_buffer_out, 
			 TME_POSIX_SERIAL_BUFFER_SIZE);

  /* start the threads: */
  tme_mutex_init(&serial->tme_posix_serial_mutex);
  tme_cond_init(&serial->tme_posix_serial_cond_writer);
  tme_thread_create((tme_thread_t) _tme_posix_serial_th_writer, serial);
  tme_thread_create((tme_thread_t) _tme_posix_serial_th_reader, serial);
  tme_thread_create((tme_thread_t) _tme_posix_serial_th_ctrl, serial);

  /* fill the element: */
  element->tme_element_private = serial;
  element->tme_element_connections_new = _tme_posix_serial_connections_new;

  return (TME_OK);
}
