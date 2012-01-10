/* $Id: serial.c,v 1.4 2003/05/05 23:14:34 fredette Exp $ */

/* generic/serial.c - generic serial implementation support: */

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
_TME_RCSID("$Id: serial.c,v 1.4 2003/05/05 23:14:34 fredette Exp $");

/* includes: */
#include <tme/generic/serial.h>

/* this initializes a serial buffer: */
int
tme_serial_buffer_init(struct tme_serial_buffer *buffer, unsigned int size)
{

  /* round the buffer size up to a power of two: */
  if (size & (size - 1)) {
    do {
      size &= (size - 1);
    } while (size & (size - 1));
    size <<= 1;
  }

  /* set the buffer size: */
  buffer->tme_serial_buffer_size = size;

  /* set the head and tail pointers: */
  buffer->tme_serial_buffer_head = 0;
  buffer->tme_serial_buffer_tail = 0;

  /* allocate the buffer data and flags: */
  buffer->tme_serial_buffer_data =
    tme_new(tme_uint8_t, size);
  buffer->tme_serial_buffer_data_flags =
    tme_new(tme_serial_data_flags_t, size);

  /* done: */
  return (TME_OK);
}

/* this copies data into a serial buffer: */
unsigned int
tme_serial_buffer_copyin(struct tme_serial_buffer *buffer,
			 const tme_uint8_t *data,
			 unsigned int count,
			 tme_serial_data_flags_t data_flags,
			 int copy_flags)
{
  unsigned int resid;
  unsigned int buffer_head, buffer_tail, buffer_size_mask;
  unsigned int buffer_size;

  /* get the head, tail, and size mask: */
  buffer_head = buffer->tme_serial_buffer_head;
  buffer_tail = buffer->tme_serial_buffer_tail;
  buffer_size_mask = buffer->tme_serial_buffer_size - 1;

  /* while we have more data to copy in: */
  for (resid = count; resid > 0; ) {
    
    /* if the buffer is full: */
    if (((buffer_head + 1) & buffer_size_mask)
	== buffer_tail) {
      
      /* if a full buffer means an overrun, mark it: */
      if (copy_flags & TME_SERIAL_COPY_FULL_IS_OVERRUN) {
	buffer->tme_serial_buffer_data_flags[buffer_head] |= TME_SERIAL_DATA_OVERRUN;
      }

      /* we're done copying in: */
      break;
    }

    /* otherwise, the buffer is not full, meaning there is always some
       space starting at the buffer head.  if the buffer head >= the
       buffer tail, there is space from the buffer head up to the end
       of the buffer, otherwise there is space from the buffer head up
       to one before the buffer tail: */
    buffer_size = ((buffer_head >= buffer_tail)
		   ? (buffer_size_mask - buffer_head) + 1
		   : (buffer_tail - buffer_head) - 1);

    /* don't copy in more data than is available: */
    buffer_size = TME_MIN(buffer_size, resid);
    assert(buffer_size > 0);

    /* copy in this data: */
    memcpy(buffer->tme_serial_buffer_data + buffer_head,
	   data,
	   buffer_size);
    memset(buffer->tme_serial_buffer_data_flags + buffer_head,
	   data_flags,
	   buffer_size);

    /* update and loop: */
    buffer_head = (buffer_head + buffer_size) & buffer_size_mask;
    data += buffer_size; 
    resid -= buffer_size;
  }

  /* store our new head pointer: */
  buffer->tme_serial_buffer_head = buffer_head;

  /* done: */
  return (count - resid);
}

/* this copies data out of a buffer: */
unsigned int
tme_serial_buffer_copyout(struct tme_serial_buffer *buffer,
			  tme_uint8_t *data,
			  unsigned int count,
			  tme_serial_data_flags_t *_data_flags,
			  int copy_flags)
{
  unsigned int resid;
  unsigned int buffer_head, buffer_tail, buffer_size_mask;
  unsigned int buffer_size;
  tme_serial_data_flags_t data_flags, *scan_flags;
  unsigned int scan_resid;

  /* get the head, tail, and size mask: */
  buffer_head = buffer->tme_serial_buffer_head;
  buffer_tail = buffer->tme_serial_buffer_tail;
  buffer_size_mask = buffer->tme_serial_buffer_size - 1;

  /* we can only return data with the same data flags: */
  data_flags = buffer->tme_serial_buffer_data_flags[buffer_tail];

  /* while we have more data to copy out: */
  for (resid = count; resid > 0; ) {
    
    /* if the buffer is empty: */
    if (buffer_tail == buffer_head) {

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

    /* only copy out data with the same buffer flags.  if we just
       wrapped in the buffer, we may find that no new data has the
       same buffer flags as data we already copied out: */
    scan_flags = buffer->tme_serial_buffer_data_flags + buffer_tail;
    scan_resid = buffer_size;
    for (; (*(scan_flags++) == data_flags
	    && --scan_resid > 0); );
    buffer_size -= scan_resid;
    if (buffer_size == 0) {
      break;
    }

    /* copy out this data: */
    if (data != NULL) {
      memcpy(data,
	     buffer->tme_serial_buffer_data + buffer_tail,
	     buffer_size);
      data += buffer_size; 
    }

    /* update and loop: */
    buffer_tail = (buffer_tail + buffer_size) & buffer_size_mask;
    resid -= buffer_size;
  }

  /* store our new tail pointer: */
  if (!(copy_flags & TME_SERIAL_COPY_PEEK)) {
    buffer->tme_serial_buffer_tail = buffer_tail;
  }

  /* done: */
  if (_data_flags != NULL) {
    *_data_flags = data_flags;
  }
  return (count - resid);
}

/* this returns the amount of busy space in the buffer: */
unsigned int
tme_serial_buffer_space_busy(const struct tme_serial_buffer *buffer)
{
  unsigned int buffer_head, buffer_tail, buffer_size;

  /* get the head, tail, and size: */
  buffer_head = buffer->tme_serial_buffer_head;
  buffer_tail = buffer->tme_serial_buffer_tail;
  buffer_size = buffer->tme_serial_buffer_size;

  if (buffer_head >= buffer_tail) {
    return (buffer_head - buffer_tail);
  }
  else {
    return (buffer_size - (buffer_tail - buffer_head));
  }
}

/* this returns the amount of free space in the buffer: */
unsigned int
tme_serial_buffer_space_free(const struct tme_serial_buffer *buffer)
{
  /* you can't completely fill the buffer: */
  return ((buffer->tme_serial_buffer_size - 1)
	  - tme_serial_buffer_space_busy(buffer));
}
