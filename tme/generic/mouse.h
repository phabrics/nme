/* $Id: mouse.h,v 1.1 2003/07/31 01:35:51 fredette Exp $ */

/* tme/generic/mouse.h - header file for generic mouse support: */

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

#ifndef _TME_GENERIC_MOUSE_H
#define _TME_GENERIC_MOUSE_H

#include <tme/common.h>
_TME_RCSID("$Id: mouse.h,v 1.1 2003/07/31 01:35:51 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* modifiers: */
#define TME_MOUSE_BUTTON_0		TME_BIT(0)
#define TME_MOUSE_BUTTON_1		TME_BIT(1)
#define TME_MOUSE_BUTTON_2		TME_BIT(2)
#define TME_MOUSE_BUTTON_3		TME_BIT(3)
#define TME_MOUSE_BUTTON_4		TME_BIT(4)

/* units: */
#define TME_MOUSE_UNITS_UNKNOWN		(0)

/* mouse controls: */
#define TME_MOUSE_CTRL_OK_READ		TME_BIT(0)

/* the undefined event time: */
#define TME_MOUSE_EVENT_TIME_UNDEF		(0)

/* this evaluates to nonzero iff a buffer is empty: */
#define tme_mouse_buffer_is_empty(b)		\
  ((b)->tme_mouse_buffer_head			\
   == ((b)->tme_mouse_buffer_tail))

/* this evaluates to nonzero iff a buffer is full: */
#define tme_mouse_buffer_is_full(b)		\
  ((((b)->tme_mouse_buffer_head + 1)		\
    & ((b)->tme_mouse_buffer_size - 1))		\
   == (b)->tme_mouse_buffer_tail)

/* types: */

/* a mouse event: */
struct tme_mouse_event {

  /* the button mask: */
  unsigned int tme_mouse_event_buttons;

  /* the X and Y deltas, and the units they are in: */
  int tme_mouse_event_delta_x;
  int tme_mouse_event_delta_y;
  unsigned int tme_mouse_event_delta_units;

  /* the time of the event: */
  tme_uint32_t tme_mouse_event_time;
};

/* a mouse buffer.  it is assumed that the user provides locking: */
struct tme_mouse_buffer {

  /* the buffer size.  this must always be a power of two: */
  unsigned int tme_mouse_buffer_size;

  /* the head and tail pointers: */
  unsigned int tme_mouse_buffer_head;
  unsigned int tme_mouse_buffer_tail;
  
  /* the buffered events: */
  struct tme_mouse_event *tme_mouse_buffer_events;
};

/* a mouse connection: */
struct tme_mouse_connection {

  /* the generic connection side: */
  struct tme_connection tme_mouse_connection;

  /* this is called when control lines change: */
  int (*tme_mouse_connection_ctrl) _TME_P((struct tme_mouse_connection *, 
					      unsigned int));

  /* this is called to read an event: */
  int (*tme_mouse_connection_read) _TME_P((struct tme_mouse_connection *, 
					   struct tme_mouse_event *,
					   unsigned int));
};

/* prototypes: */
struct tme_mouse_buffer *tme_mouse_buffer_new _TME_P((unsigned int));
void tme_mouse_buffer_destroy _TME_P((struct tme_mouse_buffer *));
int tme_mouse_buffer_copyin _TME_P((struct tme_mouse_buffer *,
				    _tme_const struct tme_mouse_event *));
int tme_mouse_buffer_copyout _TME_P((struct tme_mouse_buffer *,
				     struct tme_mouse_event *,
				     unsigned int));

#endif /* !_TME_GENERIC_MOUSE_H */
