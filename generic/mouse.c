/* $Id: mouse.c,v 1.3 2010/02/07 14:29:53 fredette Exp $ */

/* generic/mouse.c - generic mouse implementation support: */

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
_TME_RCSID("$Id: mouse.c,v 1.3 2010/02/07 14:29:53 fredette Exp $");

/* includes: */
#include <tme/generic/mouse.h>
#include <errno.h>

/* macros: */

/* types: */

/* this creates a new mouse buffer: */
struct tme_mouse_buffer *
tme_mouse_buffer_new(unsigned int size)
{
  struct tme_mouse_buffer *buffer;

  /* round the buffer size up to a power of two: */
  if (size & (size - 1)) {
    do {
      size &= (size - 1);
    } while (size & (size - 1));
    size <<= 1;
  }

  /* allocate the buffer: */
  buffer = tme_new0(struct tme_mouse_buffer, 1);

  /* set the buffer size: */
  buffer->tme_mouse_buffer_size = size;

  /* set the head and tail pointers: */
  buffer->tme_mouse_buffer_head = 0;
  buffer->tme_mouse_buffer_tail = 0;

  /* allocate the buffer events: */
  buffer->tme_mouse_buffer_events
    = tme_new(struct tme_mouse_event, size);

  /* done: */
  return (buffer);
}

/* this destroys a mouse buffer: */
void
tme_mouse_buffer_destroy(struct tme_mouse_buffer *buffer)
{

  /* free the events: */
  tme_free(buffer->tme_mouse_buffer_events);

  /* destroy the buffer itself: */
  tme_free(buffer);
}

/* this returns nonzero if both deltas are nonzero and have
   different signs: */
static inline int
_tme_mouse_deltas_opposite(int delta, int delta_last)
{
  int opposite;

  opposite = (delta_last ^ delta) < 0;
  opposite -= (delta == 0);
  opposite -= (delta_last == 0);
  return (opposite > 0);
}

/* this copies an event into a mouse buffer: */
int
tme_mouse_buffer_copyin(struct tme_mouse_buffer *buffer,
			_tme_const struct tme_mouse_event *event)
{
  unsigned int buffer_head, buffer_size_mask;
  struct tme_mouse_event *event_last;
  unsigned int unmergeable_mash;

  buffer_head = buffer->tme_mouse_buffer_head;
  buffer_size_mask = buffer->tme_mouse_buffer_size - 1;

  /* if the buffer is not empty: */
  if (buffer_head != buffer->tme_mouse_buffer_tail) {

    /* get the last event in the buffer: */
    event_last = &buffer->tme_mouse_buffer_events[(buffer_head - 1) & buffer_size_mask];

    /* the new event can't be merged if both delta Xs are nonzero and
       have different signs: */
    unmergeable_mash
      = _tme_mouse_deltas_opposite(event->tme_mouse_event_delta_x,
				   event_last->tme_mouse_event_delta_x);

    /* the new event can't be merged if both delta Ys are nonzero and
       have different signs: */
    unmergeable_mash
      |= _tme_mouse_deltas_opposite(event->tme_mouse_event_delta_y,
				    event_last->tme_mouse_event_delta_y);

    /* the new event can't be merged if its button mask is
       different: */
    unmergeable_mash
      |= (event->tme_mouse_event_buttons
	 ^ event_last->tme_mouse_event_buttons);

    /* the new event can't be merged if its delta units are
       different: */
    unmergeable_mash
      |= (event->tme_mouse_event_delta_units
	  ^ event_last->tme_mouse_event_delta_units);

    /* if the new event can't be merged: */
    if (__tme_predict_false(unmergeable_mash)) {

      /* nothing to do */
    }

    /* otherwise, the new event can be merged: */
    else {

      /* merge the event: */
      /* XXX FIXME - we should check for integer overflow here: */
      event_last->tme_mouse_event_delta_x += event->tme_mouse_event_delta_x;
      event_last->tme_mouse_event_delta_y += event->tme_mouse_event_delta_y;
      event_last->tme_mouse_event_time = event->tme_mouse_event_time;
      return (TME_OK);
    }
  }

  /* if the buffer is full: */
  if (((buffer_head + 1) & buffer_size_mask)
      == buffer->tme_mouse_buffer_tail) {
    return (EAGAIN);
  }

  /* put this event into the buffer: */
  buffer->tme_mouse_buffer_events[buffer_head]
    = *event;

  /* advance the head: */
  buffer->tme_mouse_buffer_head
    = (buffer_head + 1) & buffer_size_mask;

  return (TME_OK);
}

/* this copies an event out of a mouse buffer: */
int
tme_mouse_buffer_copyout(struct tme_mouse_buffer *buffer,
			 struct tme_mouse_event *events,
			 unsigned int count)
{
  unsigned int buffer_head, buffer_tail;
  unsigned int buffer_size, buffer_size_mask;
  unsigned int resid;

  /* get the head, tail, and size mask: */
  buffer_head = buffer->tme_mouse_buffer_head;
  buffer_tail = buffer->tme_mouse_buffer_tail;
  buffer_size_mask = buffer->tme_mouse_buffer_size - 1;

  for (resid = count; resid > 0; ) {

    /* if the buffer is empty: */
    if (buffer_tail == buffer->tme_mouse_buffer_head) {

      /* we're done copying out: */
      break;
    }

    /* otherwise, the buffer is not empty, meaning there is always some
       data starting at the buffer tail.  if the buffer tail > the
       buffer head, there is space from the buffer tail up to the end
       of the buffer, otherwise there is space from the buffer tail up
       to the buffer head: */
    buffer_size = ((buffer_tail > buffer_head)
		   ? (buffer_size_mask - buffer_tail) + 1
		   : (buffer_head - buffer_tail));

    /* don't copy out more data than there is space available: */
    buffer_size = TME_MIN(buffer_size, resid);
    assert(buffer_size > 0);

    /* copy out the data: */
    memcpy(events,
	   buffer->tme_mouse_buffer_events + buffer_tail,
	   sizeof(struct tme_mouse_event) * buffer_size);
    events += buffer_size;

    /* update and loop: */
    buffer_tail = (buffer_tail + buffer_size) & buffer_size_mask;
    resid -= buffer_size;
  }

  buffer->tme_mouse_buffer_tail = buffer_tail;
  return (count - resid);
}
