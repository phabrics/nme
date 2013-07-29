/* $Id: serial-ms.h,v 1.4 2010/02/14 00:36:11 fredette Exp $ */

/* serial/serial-ms.h - implementation header file for serial mouse
   emulation: */

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

#ifndef _SERIAL_SERIAL_MS_H
#define _SERIAL_SERIAL_MS_H

#include <tme/common.h>
_TME_RCSID("$Id: serial-ms.h,v 1.4 2010/02/14 00:36:11 fredette Exp $");

/* includes: */
#include <tme/generic/mouse.h>
#include <tme/generic/serial.h>
#include <tme/element.h>
#include <tme/threads.h>

/* macros: */

/* the callout flags: */
#define TME_SERIAL_MS_CALLOUT_CHECK		(0)
#define TME_SERIAL_MS_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SERIAL_MS_CALLOUTS_MASK		(-2)
#define  TME_SERIAL_MS_CALLOUT_SERIAL_CTRL	TME_BIT(1)
#define  TME_SERIAL_MS_CALLOUT_SERIAL_CONFIG	TME_BIT(2)
#define  TME_SERIAL_MS_CALLOUT_SERIAL_READ	TME_BIT(3)
#define  TME_SERIAL_MS_CALLOUT_MOUSE_CTRL	TME_BIT(4)
#define  TME_SERIAL_MS_CALLOUT_MOUSE_READ	TME_BIT(5)

/* types: */

/* a serial mouse: */
struct tme_serial_ms {

  /* backpointer to our element: */
  struct tme_element *tme_serial_ms_element;

  /* our mutex: */
  tme_mutex_t tme_serial_ms_mutex;

  /* our type: */
  _tme_const char *tme_serial_ms_type;

  /* our type-specific state: */
  void *tme_serial_ms_type_state;

  /* our type-specific event function: */
  int (*tme_serial_ms_type_events) _TME_P((struct tme_serial_ms *,
					   _tme_const struct tme_mouse_event *,
					   unsigned int));

  /* our type-specific serial control function: */
  int (*tme_serial_ms_type_serial_ctrl) _TME_P((struct tme_serial_ms *,
						unsigned int));

  /* our type-specific serial input function: */
  int (*tme_serial_ms_type_serial_input) _TME_P((struct tme_serial_ms *,
						 tme_uint8_t *,
						 unsigned int,
						 tme_serial_data_flags_t));

  /* our type-specific serial configuration: */
  struct tme_serial_config tme_serial_ms_type_config;

  /* the serial configuration from our serial connection: */
  struct tme_serial_config tme_serial_ms_peer_config;

  /* our mouse connection: */
  struct tme_mouse_connection *tme_serial_ms_connection_ms;

  /* our serial connection: */
  struct tme_serial_connection *tme_serial_ms_connection_serial;

  /* the callout flags: */
  int tme_serial_ms_callout_flags;

  /* our serial output buffer: */
  struct tme_serial_buffer tme_serial_ms_serial_buffer;

  /* our current mouse control outputs: */
  unsigned int tme_serial_ms_mouse_ctrl;

  /* our current serial control outputs: */
  unsigned int tme_serial_ms_serial_ctrl;

  /* the time when the last events were read: */
  struct timeval tme_serial_ms_event_read_last;

  /* if nonzero, the rate-limiting microseconds per event: */
  unsigned long tme_serial_ms_rate_usec;

  /* if nonzero, the rate-limiting current sleep time: */
  unsigned long tme_serial_ms_rate_sleep_usec;

  /* if nonzero, the rate-limiting thread needs to call out an event
     read: */
  int tme_serial_ms_rate_do_callout;

  /* our rate-limiting thread condition: */
  tme_cond_t tme_serial_ms_rate_cond;
};

/* prototypes: */
int _tme_serial_ms_mssystems5_init _TME_P((struct tme_serial_ms *));

#endif /* !_SERIAL_SERIAL_MS_H */
