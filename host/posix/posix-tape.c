/* $Id: posix-tape.c,v 1.7 2006/09/30 12:35:01 fredette Exp $ */

/* host/posix/posix-tape.c - implementation of tapes on a POSIX system: */

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
_TME_RCSID("$Id: posix-tape.c,v 1.7 2006/09/30 12:35:01 fredette Exp $");

/* includes: */
#include <tme/generic/tape.h>
#include <tme/threads.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/uio.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else  /* HAVE_STDARG_H */
#include <varargs.h>
#endif /* HAVE_STDARG_H */

/* macros: */

/* tape flags: */
#define TME_POSIX_TAPE_FLAG_RO		TME_BIT(0)
#define TME_POSIX_TAPE_FLAG_DIRTY	TME_BIT(1)

/* the size of the control callout ring: */
#define TME_POSIX_TAPE_CONTROL_RING_SIZE	(16)

/* the callout flags: */
#define TME_POSIX_TAPE_CALLOUT_CHECK		(0)
#define TME_POSIX_TAPE_CALLOUT_RUNNING		TME_BIT(0)
#define TME_POSIX_TAPE_CALLOUTS_MASK		(-2)
#define  TME_POSIX_TAPE_CALLOUT_CONTROL		TME_BIT(1)

/* types: */

/* a posix tape control: */
struct tme_posix_tape_control {

  /* the control: */
  unsigned int tme_posix_tape_control_which;
};

/* a posix tape segment: */
struct tme_posix_tape_segment {

  /* tape segments are kept on a doubly linked list: */
  struct tme_posix_tape_segment *tme_posix_tape_segment_next;
  struct tme_posix_tape_segment *tme_posix_tape_segment_prev;

  /* the filename of this tape segment: */
  char *tme_posix_tape_segment_filename;

  /* the file descriptor of this tape segment: */
  int tme_posix_tape_segment_fd;

  /* this is nonzero iff this tape segment is a real tape: */
  int tme_posix_tape_segment_real_tape;
};

/* a posix tape: */
struct tme_posix_tape {

  /* backpointer to our element: */
  struct tme_element *tme_posix_tape_element;

  /* our mutex: */
  tme_mutex_t tme_posix_tape_mutex;

  /* our flags: */
  int tme_posix_tape_flags;

  /* the tape segments: */
  struct tme_posix_tape_segment *tme_posix_tape_segments;

  /* the connection: */
  struct tme_tape_connection *tme_posix_tape_connection;

  /* the callout flags: */
  int tme_posix_tape_callout_flags;

  /* the control ring buffer: */
  struct tme_posix_tape_control tme_posix_tape_controls[TME_POSIX_TAPE_CONTROL_RING_SIZE];
  unsigned int tme_posix_tape_control_head;
  unsigned int tme_posix_tape_control_tail;

  /* the current tape segment: */
  struct tme_posix_tape_segment *tme_posix_tape_segment_current;

  /* the block sizes: */
  unsigned long tme_posix_tape_block_size_min;
  unsigned long tme_posix_tape_block_size_max;
  unsigned long tme_posix_tape_block_size_fixed;

  /* the tape buffer: */
  unsigned long tme_posix_tape_buffer_size;
  tme_uint8_t *tme_posix_tape_buffer_data;
  unsigned long tme_posix_tape_buffer_flags;
  unsigned long tme_posix_tape_buffer_count_xfer;
};

/* this allocates the next control in the ring buffer: */
struct tme_posix_tape_control *
_tme_posix_tape_control_new(struct tme_posix_tape *posix_tape)
{
  int old_head;
  struct tme_posix_tape_control *control;

  /* abort if the ring buffer overflows: */
  old_head = posix_tape->tme_posix_tape_control_head;
  posix_tape->tme_posix_tape_control_head
    = ((old_head
	+ 1)
       & (TME_POSIX_TAPE_CONTROL_RING_SIZE
	  - 1));
  if ((posix_tape->tme_posix_tape_control_head
       == posix_tape->tme_posix_tape_control_tail)
      && (posix_tape->tme_posix_tape_connection
	  != NULL)) {
    abort();
  }

  /* return the control: */
  control = &posix_tape->tme_posix_tape_controls[old_head];
  return (control);
}

/* the posix tape callout function.  it must be called with the mutex locked: */
static void
_tme_posix_tape_callout(struct tme_posix_tape *posix_tape,
			int new_callouts)
{
  struct tme_tape_connection *conn_tape;
  unsigned int old_tail;
  struct tme_posix_tape_control *control;
  int callouts, later_callouts;
  int rc;
  
  /* add in any new callouts: */
  posix_tape->tme_posix_tape_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (posix_tape->tme_posix_tape_callout_flags
      & TME_POSIX_TAPE_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  posix_tape->tme_posix_tape_callout_flags
    |= TME_POSIX_TAPE_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = posix_tape->tme_posix_tape_callout_flags)
	  & TME_POSIX_TAPE_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    posix_tape->tme_posix_tape_callout_flags
      = (callouts
	 & ~TME_POSIX_TAPE_CALLOUTS_MASK);
    callouts
      &= TME_POSIX_TAPE_CALLOUTS_MASK;

    /* get the tape connection: */
    conn_tape = posix_tape->tme_posix_tape_connection;

    /* if we need to call out a control: */
    if (callouts & TME_POSIX_TAPE_CALLOUT_CONTROL) {

      /* there must be a control to call out: */
      old_tail = posix_tape->tme_posix_tape_control_tail;
      assert (old_tail
	      != posix_tape->tme_posix_tape_control_head);
      control
	= &posix_tape->tme_posix_tape_controls[old_tail];

      /* unlock the mutex: */
      tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);
      
      /* do the callout: */
      rc = TME_OK;
      if (conn_tape != NULL) {
	switch (control->tme_posix_tape_control_which) {
	case TME_TAPE_CONTROL_LOAD:
	case TME_TAPE_CONTROL_UNLOAD:
	  rc = ((*conn_tape->tme_tape_connection_control)
		(conn_tape,
		 control->tme_posix_tape_control_which));
	  break;
	default:
	  rc = TME_OK;
	  assert(FALSE);
	}
      }
	
      /* lock the mutex: */
      tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_POSIX_TAPE_CALLOUT_CONTROL;
      }

      /* otherwise, this callout was successful: */
      else {

	/* advance the tail pointer: */
	posix_tape->tme_posix_tape_control_tail
	  = ((old_tail
	      + 1)
	     & (TME_POSIX_TAPE_CONTROL_RING_SIZE
		- 1));

	/* if there are more controls to callout, call them out now: */
	if (posix_tape->tme_posix_tape_control_tail
	    != posix_tape->tme_posix_tape_control_head) {
	  posix_tape->tme_posix_tape_callout_flags
	    |= TME_POSIX_TAPE_CALLOUT_CONTROL;
	}
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  posix_tape->tme_posix_tape_callout_flags = later_callouts;
}

/* this closes and frees all tape segments: */
static void
_tme_posix_tape_segments_close(struct tme_posix_tape *posix_tape)
{
  struct tme_posix_tape_segment *segment;

  /* while we have segments: */
  for (; posix_tape->tme_posix_tape_segments != NULL; ) {

    /* get the next segment and remove it from the list: */
    segment = posix_tape->tme_posix_tape_segments;
    posix_tape->tme_posix_tape_segments = segment->tme_posix_tape_segment_next;

    /* if the segment file descriptor is open, close it: */
    if (segment->tme_posix_tape_segment_fd >= 0) {
      close(segment->tme_posix_tape_segment_fd);
    }

    /* free the segment filename: */
    tme_free(segment->tme_posix_tape_segment_filename);

    /* free the segment itself: */
    tme_free(segment);
  }

  /* there is no current segment: */
  posix_tape->tme_posix_tape_segment_current = NULL;
}

/* this unloads a tape: */
static int
_tme_posix_tape_unload(struct tme_posix_tape *posix_tape)
{

  /* close and free all tape segments: */
  _tme_posix_tape_segments_close(posix_tape);

  return (TME_OK);
}

/* this opens a tape segment and makes it the current segment: */
static int
_tme_posix_tape_segment_open(struct tme_posix_tape *posix_tape,
			     struct tme_posix_tape_segment *segment)
{
  
  /* there is no current segment: */
  posix_tape->tme_posix_tape_segment_current = NULL;

  /* open the segment: */
  segment->tme_posix_tape_segment_fd
    = open(segment->tme_posix_tape_segment_filename,
	   ((posix_tape->tme_posix_tape_flags & TME_POSIX_TAPE_FLAG_RO)
	    ? O_RDONLY
	    : O_RDWR));

  /* if the open failed: */
  if (segment->tme_posix_tape_segment_fd < 0) {
    return (errno);
  }

  /* set the current segment: */
  posix_tape->tme_posix_tape_segment_current = segment;
  return (TME_OK);
}

/* this rewinds a tape: */
static int
_tme_posix_tape_rewind(struct tme_posix_tape *posix_tape)
{
  struct tme_posix_tape_segment *segment;
  int rc;

  /* assume this will succeed: */
  rc = TME_OK;

  /* if there is a current segment and it's not the first
     segment: */
  segment = posix_tape->tme_posix_tape_segment_current;
  if (segment != NULL
      && segment != posix_tape->tme_posix_tape_segments) {

    /* this must be a normal file: */
    assert (!segment->tme_posix_tape_segment_real_tape);

    /* if the segment is open, close it: */
    if (segment->tme_posix_tape_segment_fd >= 0) {
      close(segment->tme_posix_tape_segment_fd);
      segment->tme_posix_tape_segment_fd = -1;
    }
  }

  /* get the first segment: */
  segment = posix_tape->tme_posix_tape_segments;

  /* if this is a real tape: */
  if (segment->tme_posix_tape_segment_real_tape) {

    /* the tape must be open: */
    assert (segment->tme_posix_tape_segment_fd >= 0);

    /* XXX TBD */
    abort();
  }

  /* otherwise, this is a normal file: */
  else {

    /* if the file isn't open, open it, else
       seek it to the beginning: */
    if (segment->tme_posix_tape_segment_fd < 0) {
      rc = _tme_posix_tape_segment_open(posix_tape,
					segment);
    }
    else {
      if (lseek(segment->tme_posix_tape_segment_fd,
		0, SEEK_SET) != 0) {
	rc = errno;
      }
    }
  }

  return (rc);
}

/* this skips file marks: */
static int
_tme_posix_tape_mark_skip(struct tme_posix_tape *posix_tape,
			  unsigned int count,
			  int forward)
{
  struct tme_posix_tape_segment *segment;
  int rc;
  
  segment = posix_tape->tme_posix_tape_segment_current;

  /* if we are at end-of-media, or if we're skipping no marks, do
     nothing: */
  if (segment == NULL
      || count == 0) {
    return (TME_OK);
  }

  /* this segment must be a normal file: */
  assert (!segment->tme_posix_tape_segment_real_tape);

  /* if this segment is open, close it: */
  if (segment->tme_posix_tape_segment_fd >= 0) {
    close(segment->tme_posix_tape_segment_fd);
    segment->tme_posix_tape_segment_fd = -1;
  }

  /* skip the marks: */
  for (; segment != NULL && count-- > 0; ) {
    segment
      = (forward
	 ? segment->tme_posix_tape_segment_next
	 : segment->tme_posix_tape_segment_prev);
  }

  /* for now, if we are skipping in reverse we must have a segment -
     what do we do otherwise? */
  assert (forward || segment != NULL);

  /* clear the active segment pointer: */
  posix_tape->tme_posix_tape_segment_current = NULL;

  /* if we have a segment, open it: */
  if (segment != NULL) {
    rc = _tme_posix_tape_segment_open(posix_tape,
				      segment);
    assert (rc == TME_OK);
  }

  /* if we are skipping in reverse, we must leave the tape on the
     beginning of media side of the file mark: */
  if (!forward) {
    rc = (lseek(segment->tme_posix_tape_segment_fd,
		0,
		SEEK_END) < 0);
    assert (rc == 0);
  }

  return (TME_OK);
}

/* this finishes a transfer.  it must be called with the mutex locked: */
static int
_tme_posix_tape_xfer1(struct tme_posix_tape *posix_tape,
		      int *_flags,
		      unsigned long *_count_xfer,
		      unsigned long *_count_bytes,
		      int xfer_read)
{
  struct tme_posix_tape_segment *segment;
  unsigned long count_xfer, count_bytes_user, count_bytes_tape;
  unsigned long xfer_factor, block_size;
  long rc;
  
  /* assume that we will return no flags: */
  *_flags = 0;

  /* get the transfer count: */
  count_xfer = posix_tape->tme_posix_tape_buffer_count_xfer;

  /* get any forced block size: */
  block_size = posix_tape->tme_posix_tape_block_size_min;
  if (block_size != posix_tape->tme_posix_tape_block_size_min) {
    block_size = 0;
  }

  /* if the request is for one or more blocks at a fixed block size: */
  if (posix_tape->tme_posix_tape_buffer_flags
      & TME_TAPE_FLAG_FIXED) {
    
    /* the device must be in fixed block size mode: */
    xfer_factor = posix_tape->tme_posix_tape_block_size_fixed;
    if (xfer_factor == 0) {
      assert (block_size != 0);
      xfer_factor = block_size;
    }
    else {
      assert (block_size == 0
	      || block_size == xfer_factor);
      block_size = xfer_factor;
    }
  }

  /* otherwise, the request is for one block at a variable block size: */
  else {

    /* the device might be forcing a block size: */
    xfer_factor = 1;
    if (block_size == 0) {
      block_size = count_xfer;
    }
  }

  /* calculate the byte count for the user: */
  count_bytes_user = count_xfer * xfer_factor;

  /* calculate the byte count for the tape.  this is
     the byte count for the user rounded up to the
     block size: */
  count_bytes_tape = count_bytes_user % block_size;
  count_bytes_tape
    = (count_bytes_user
       + (count_bytes_tape
	  ? (block_size
	     - count_bytes_tape)
	  : 0));

  /* get the current segment: */
  segment = posix_tape->tme_posix_tape_segment_current;

  /* if this is a read: */
  if (xfer_read) {

    /* if we are out of segments: */
    if (segment == NULL) {

      /* act like we read zero bytes: */
      rc = 0;
    }

    /* otherwise, do the read: */
    else {

      /* do the read: */
      rc = read(segment->tme_posix_tape_segment_fd,
		posix_tape->tme_posix_tape_buffer_data,
		count_bytes_user);

      /* if this segment is not a real tape, and we're expected to
	 transfer more bytes than the user requested, seek over the
	 extra bytes, leaving the medium "positioned after the block": */
      if (!segment->tme_posix_tape_segment_real_tape
	  && count_bytes_tape > count_bytes_user) {
	lseek(segment->tme_posix_tape_segment_fd,
	      (count_bytes_tape - count_bytes_user),
	      SEEK_CUR);
      }
    }
  }

  /* otherwise, this is a write: */
  else {

    /* we must have a segment and it must be a real tape: */
    assert (segment != NULL
	    && segment->tme_posix_tape_segment_real_tape);

    /* do the write: */
    rc = write(segment->tme_posix_tape_segment_fd,
	       posix_tape->tme_posix_tape_buffer_data,
	       count_bytes_user);
  }

  /* if the transfer got an error: */
  if (rc < 0) {
    
    /* return the error: */
    /* XXX is this sufficient? */
    *_count_bytes = 0;
    *_count_xfer = 0;
    return (errno);
  }

  /* otherwise, we transferred some or none of the data: */
  else {
    
    /* return the final byte count: */
    *_count_bytes = rc;

    /* if the request was for one or more blocks at a fixed block size: */
    if (posix_tape->tme_posix_tape_buffer_flags
	& TME_TAPE_FLAG_FIXED) {

      /* return the number of blocks successfully transferred: */
      *_count_xfer = (rc / xfer_factor);
    }
    
    /* otherwise, the request is for one block at a variable block size: */
    else {
      
      /* return the actual size of the block transferred: */

      /* if the user asked for what we thought was a whole block, but
	 we transferred less, how much we did transfer is the actual
	 block size: */
      if (count_bytes_user == block_size
	  && (unsigned long) rc < block_size) {
	*_count_xfer = rc;
      }

      /* otherwise, if this segment is a real tape, we don't know how
	 long the block really was.  for now, we just return what we
	 are using as the block size: */
      /* XXX it may be possible to do an ioctl to get the true
	 residual: */
      else if (segment->tme_posix_tape_segment_real_tape) {
	*_count_xfer = block_size;
      }

      /* otherwise, this segment isn't a real tape, so we can return
	 what we are using as the block size: */
      else {
	*_count_xfer = block_size;
      }
    }
    
    /* if we didn't read as much from the tape as we were supposed to: */
    if ((unsigned long) rc < count_bytes_tape) {

      /* if we read an exact multiple of the block size (including
	 zero blocks), the read was short because of a file mark.
	 return the mark (EOF) condition: */
      if ((rc % block_size) == 0) {
	  
	/* return the mark condition: */
	*_flags |= TME_TAPE_FLAG_MARK;
	  
	/* if this segment was not a real tape device, skip the file
	   mark - i.e., move to the next segment: */
	if (segment != NULL
	    && !segment->tme_posix_tape_segment_real_tape) {
	  rc = _tme_posix_tape_mark_skip(posix_tape,
					 1, TRUE);
	  assert (rc == TME_OK);
	}
      }
      
      /* otherwise, the read was short because we read a partial
	 block.  return the incorrect length indication (ILI)
	 condition: */
      else {
	*_flags |= TME_TAPE_FLAG_ILI;
      }
    }
  }

  return (TME_OK);
}

/* this starts a transfer.  it must be called with the mutex locked: */
static int
_tme_posix_tape_xfer0(struct tme_posix_tape *posix_tape,
		      int flags,
		      unsigned long count_xfer,
		      unsigned long *_count_bytes)
{
  unsigned long xfer_factor, count_bytes;
  int old_flags;
  unsigned long old_count_xfer, old_count_bytes;
  int rc;

  /* if the buffer is dirty: */
  if (posix_tape->tme_posix_tape_flags
      & TME_POSIX_TAPE_FLAG_DIRTY) {
    
    /* write out the buffer: */
    rc = _tme_posix_tape_xfer1(posix_tape,
			       &old_flags,
			       &old_count_xfer,
			       &old_count_bytes,
			       FALSE);
    assert (rc == TME_OK);

    /* this buffer is no longer dirty: */
    posix_tape->tme_posix_tape_flags
      &= ~TME_POSIX_TAPE_FLAG_DIRTY;
  }

  /* set the parameters of this transfer: */
  posix_tape->tme_posix_tape_buffer_flags = flags;
  posix_tape->tme_posix_tape_buffer_count_xfer = count_xfer;
  
  /* if the request is for one or more blocks at a fixed block size: */
  if (posix_tape->tme_posix_tape_buffer_flags
      & TME_TAPE_FLAG_FIXED) {
    
    /* it is an error if the device isn't in fixed block size mode: */
    xfer_factor = posix_tape->tme_posix_tape_block_size_fixed;
    if (xfer_factor == 0) {
      xfer_factor = posix_tape->tme_posix_tape_block_size_min;
      if (xfer_factor != posix_tape->tme_posix_tape_block_size_max) {
	return (EINVAL);
      }
    }
  }

  /* otherwise, the request is for one block at a variable block size: */
  else {
    xfer_factor = 1;
  }

  /* calculate the byte count: */
  count_bytes = count_xfer * xfer_factor;

  /* if the buffer isn't big enough: */
  if (posix_tape->tme_posix_tape_buffer_size
      < count_bytes) {

    /* resize the buffer: */
    posix_tape->tme_posix_tape_buffer_size
      = count_bytes;
    posix_tape->tme_posix_tape_buffer_data
      = tme_renew(tme_uint8_t,
		  posix_tape->tme_posix_tape_buffer_data,
		  posix_tape->tme_posix_tape_buffer_size);
  }
  
  /* done: */
  *_count_bytes = count_bytes;
  return (TME_OK);
}

/* this releases the current buffer: */
static int
_tme_posix_tape_release(struct tme_tape_connection *conn_tape,
			int *_flags,
			unsigned long *_count_xfer)
{
  struct tme_posix_tape *posix_tape;
  unsigned long count_bytes;
  int rc;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) conn_tape->tme_tape_connection.tme_connection_element->tme_element_private;

  /* assume that this call will succeed: */
  rc = TME_OK;

  /* lock the mutex: */
  tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);

  /* if the buffer is dirty: */
  rc = TME_OK;
  if (posix_tape->tme_posix_tape_flags
      & TME_POSIX_TAPE_FLAG_DIRTY) {
    
    /* write out the buffer: */
    rc = _tme_posix_tape_xfer1(posix_tape,
			       _flags,
			       _count_xfer,
			       &count_bytes,
			       FALSE);

    /* this buffer is no longer dirty: */
    posix_tape->tme_posix_tape_flags
      &= ~TME_POSIX_TAPE_FLAG_DIRTY;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);

  return (rc);
}

/* this returns a read buffer: */
static int
_tme_posix_tape_read(struct tme_tape_connection *conn_tape,
		     int *_flags,
		     unsigned long *_count_xfer,
		     unsigned long *_count_bytes,
		     const tme_uint8_t **_buffer)
{
  struct tme_posix_tape *posix_tape;
  int rc;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) conn_tape->tme_tape_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);

  /* start the transfer: */
  rc = _tme_posix_tape_xfer0(posix_tape,
			     *_flags,
			     *_count_xfer,
			     _count_bytes);
  *_buffer = posix_tape->tme_posix_tape_buffer_data;

  /* if the start succeeded, finish the transfer: */
  if (rc == TME_OK) {
    rc = _tme_posix_tape_xfer1(posix_tape,
			       _flags,
			       _count_xfer,
			       _count_bytes,
			       TRUE);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);

  return (rc);
}

/* this returns a write buffer: */
static int
_tme_posix_tape_write(struct tme_tape_connection *conn_tape,
		      int flags,
		      unsigned long count_xfer,
		      unsigned long *_count_bytes,
		      tme_uint8_t **_buffer)
{
  struct tme_posix_tape *posix_tape;
  int rc;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) conn_tape->tme_tape_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);

  /* start the transfer: */
  rc = _tme_posix_tape_xfer0(posix_tape,
			     flags,
			     count_xfer,
			     _count_bytes);
  *_buffer = posix_tape->tme_posix_tape_buffer_data;

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);

  return (rc);
}

/* the tape control handler: */
#ifdef HAVE_STDARG_H
static int _tme_posix_tape_control(struct tme_tape_connection *conn_tape,
				   unsigned int control,
				   ...)
#else  /* HAVE_STDARG_H */
static int _tme_posix_tape_control(conn_tape, control, va_alist)
     struct tme_tape_connection *conn_tape;
     unsigned int control;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  struct tme_posix_tape *posix_tape;
  unsigned long *sizes;
  const unsigned long *csizes;
  unsigned int count;
  va_list control_args;
  int *_flags;
  int rc;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) conn_tape->tme_tape_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);

  /* start the variable arguments: */
#ifdef HAVE_STDARG_H
  va_start(control_args, control);
#else  /* HAVE_STDARG_H */
  va_start(control_args);
#endif /* HAVE_STDARG_H */

  /* dispatch on the sequence type: */
  switch (control) {

  case TME_TAPE_CONTROL_LOAD:
    _flags = va_arg(control_args, int *);
    *_flags = (posix_tape->tme_posix_tape_segments != NULL);
    rc = TME_OK;
    break;

  case TME_TAPE_CONTROL_UNLOAD:
    rc = _tme_posix_tape_unload(posix_tape);
    break;

  case TME_TAPE_CONTROL_MARK_WRITE:
    abort();

  case TME_TAPE_CONTROL_DENSITY_GET:
    abort();

  case TME_TAPE_CONTROL_DENSITY_SET:
    abort();

  case TME_TAPE_CONTROL_BLOCK_SIZE_GET:
    sizes = va_arg(control_args, unsigned long *);
    sizes[0] = posix_tape->tme_posix_tape_block_size_min;
    sizes[1] = posix_tape->tme_posix_tape_block_size_max;
    sizes[2] = posix_tape->tme_posix_tape_block_size_fixed;
    rc = TME_OK;
    break;

  case TME_TAPE_CONTROL_BLOCK_SIZE_SET:
    csizes = va_arg(control_args, const unsigned long *);

    /* the minimum block size must be less than or equal to
       the maximum block size: */
    if (csizes[0] > csizes[1]) {
      return (EINVAL);
    }

    /* if we aren't given a fixed block size: */
    if (csizes[2] == 0) {

      /* if the minimum and maximum block sizes are the same,
	 we are in fixed block size mode: */
      if (csizes[0] == csizes[1]) {
	posix_tape->tme_posix_tape_block_size_fixed = csizes[0];
      }

      /* otherwise, we are not in fixed block size mode: */
      else {
	posix_tape->tme_posix_tape_block_size_fixed = 0;
      }
    }

    /* otherwise, we are given a fixed block size.  it must
       be within the minimum and maximum block sizes: */
    else {
      if (csizes[2] < csizes[0]
	  || csizes[2] > csizes[1]) {
	return (EINVAL);
      }
      posix_tape->tme_posix_tape_block_size_fixed = csizes[2];
    }

    /* set the minimum and maximum block sizes: */
    posix_tape->tme_posix_tape_block_size_min = csizes[0];
    posix_tape->tme_posix_tape_block_size_max = csizes[1];
    rc = TME_OK;
    break;

  case TME_TAPE_CONTROL_MARK_SKIPF:
  case TME_TAPE_CONTROL_MARK_SKIPR:
    count = va_arg(control_args, unsigned int);
    rc = _tme_posix_tape_mark_skip(posix_tape,
				   count,
				   (control
				    == TME_TAPE_CONTROL_MARK_SKIPF));
    break;

  case TME_TAPE_CONTROL_REWIND:
    rc = _tme_posix_tape_rewind(posix_tape);
    break;

  default:
    abort();
  }

  /* end the variable arguments: */
  va_end(control_args);

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);

  return (rc);
}

/* our internal command function: */
static int
__tme_posix_tape_command(struct tme_posix_tape *posix_tape,
			 const char * const * args, 
			 char **_output)
{
  struct tme_posix_tape_segment *segment, **_prev;
  struct tme_posix_tape_control *control;
  struct stat statbuf;
  int flags;
  int arg_i;
  int usage;
  int rc;
  int new_callouts;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* check the command: */
  usage = FALSE;
  arg_i = 1;

  /* the "load" command: */
  if (TME_ARG_IS(args[arg_i], "load")) {
    arg_i++;

    /* if a tape is currently loaded, it must be unloaded first: */
    if (posix_tape->tme_posix_tape_segments != NULL) {
      tme_output_append_error(_output,
			      _("%s: tape already loaded; must unload first"),
			      args[0]);
      return (EBUSY);
    }

    /* check for flags, which must all come before the tape segment
       filenames: */
    flags = 0;
    for (;;) {
      
      /* the "read-only" flag: */
      if (TME_ARG_IS(args[arg_i], "read-only")) {
	flags |= TME_POSIX_TAPE_FLAG_RO;
	arg_i++;
      }

      else {
	break;
      }
    }
    
    /* all of the remaining arguments must be tape segment filenames.
       assume that all segments open successfully: */
    rc = TME_OK;

    /* open the tape segments: */
    segment = NULL;
    posix_tape->tme_posix_tape_segments = NULL;
    for (_prev = &posix_tape->tme_posix_tape_segments;
	 ;
	 _prev = &segment->tme_posix_tape_segment_next) {

      /* stop if we're out of filenames: */
      if (args[arg_i] == NULL) {
	break;
      }

      /* allocate a new tape segment and link it in: */
      segment = tme_new0(struct tme_posix_tape_segment, 1);
      segment->tme_posix_tape_segment_filename
	= tme_strdup(args[arg_i]);
      segment->tme_posix_tape_segment_next = NULL;
      segment->tme_posix_tape_segment_prev = *_prev;
      *_prev = segment;

      /* open the segment file: */
      segment->tme_posix_tape_segment_fd
	= open(segment->tme_posix_tape_segment_filename,
	       O_RDONLY);
      if (segment->tme_posix_tape_segment_fd < 0) {
	rc = errno;
	break;
      }

      /* stat the segment file: */
      if (fstat(segment->tme_posix_tape_segment_fd,
		&statbuf) < 0) {
	rc = errno;
	break;
      }

      /* if this is a block device: */
      if (S_ISBLK(statbuf.st_mode)) {
	  tme_output_append_error(_output,
				  _("cannot use a block device: %s"),
				  args[arg_i]);
	  rc = EINVAL;
	  break;
      }

      /* if this is a character device: */
      else if (S_ISCHR(statbuf.st_mode)) {

	/* if this is not the only segment: */
	if (posix_tape->tme_posix_tape_segments != segment
	    || args[arg_i + 1] != NULL) {
	  tme_output_append_error(_output,
				  _("a real device must be the only tape segment: "));
	  rc = EINVAL;
	  break;
	}

	/* this is a real tape: */
	segment->tme_posix_tape_segment_real_tape = TRUE;
      }

      /* otherwise, this is a regular file: */
      else {
	
	/* this tape must be read-only: */
	flags |= TME_POSIX_TAPE_FLAG_RO;
      }

      /* if this is not the first segment, close the file: */
      if (posix_tape->tme_posix_tape_segments != segment) {
	close(segment->tme_posix_tape_segment_fd);
	segment->tme_posix_tape_segment_fd = -1;
      }

      /* advance to the next segment filename: */
      arg_i++;
    }

    /* if we got an error: */
    if (rc != TME_OK) {

      /* append any segment filename to the error: */
      if (segment != NULL) {
	tme_output_append_error(_output,
				"%s",
				segment->tme_posix_tape_segment_filename);
      }

      /* close and free the segments: */
      _tme_posix_tape_segments_close(posix_tape);
    }

    /* otherwise, if we opened no segments: */
    else if (posix_tape->tme_posix_tape_segments == NULL) {
      tme_output_append_error(_output,
			      "%s %s load [read-only] { %s | %s [ .. %s ] }",
			      _("usage:"),
			      args[0],
			      _("DEVICE"),
			      _("FILENAME"),
			      _("FILENAME"));
      rc = EINVAL;
    }

    /* otherwise, a tape has now been loaded: */
    else {
      
      /* a tape has now been loaded: */
      posix_tape->tme_posix_tape_flags
	= flags;

      /* the first segment is still open.  make it the current segment: */
      posix_tape->tme_posix_tape_segment_current
	= posix_tape->tme_posix_tape_segments;
      
      /* call out a LOAD control: */
      control = _tme_posix_tape_control_new(posix_tape);
      control->tme_posix_tape_control_which = TME_TAPE_CONTROL_LOAD;
      new_callouts |= TME_POSIX_TAPE_CALLOUT_CONTROL;
    }
  }

  /* the "unload" command: */
  else if (TME_ARG_IS(args[arg_i], "unload")) {

    /* if no tape is currently loaded: */
    if (posix_tape->tme_posix_tape_segments == NULL) {
      tme_output_append_error(_output,
			      _("%s: no tape loaded"),
			      args[0]);
      return (ENXIO);
    }

    /* we must have no arguments: */
    if (args[arg_i + 1] != NULL) {
      tme_output_append_error(_output,
			      "%s %s unload",
			      _("usage:"),
			      args[0]);
      return (EINVAL);
    }

    /* unload the tape: */
    _tme_posix_tape_unload(posix_tape);

    /* call out an UNLOAD control: */
    control = _tme_posix_tape_control_new(posix_tape);
    control->tme_posix_tape_control_which = TME_TAPE_CONTROL_UNLOAD;
    new_callouts |= TME_POSIX_TAPE_CALLOUT_CONTROL;
    rc = TME_OK;
  }

  /* any other command: */
  else {
    if (args[arg_i] != NULL) {
      tme_output_append_error(_output,
			      "%s '%s', ",
			      _("unknown command"),
			      args[1]);
    }
    tme_output_append_error(_output,
			    _("available %s commands: %s"),
			    args[0],
			    "load unload");
    return (EINVAL);
  }

  /* make any new callouts: */
  _tme_posix_tape_callout(posix_tape, new_callouts);

  return (rc);
}

/* our command function: */
static int
_tme_posix_tape_command(struct tme_element *element,
			const char * const * args, 
			char **_output)
{
  struct tme_posix_tape *posix_tape;
  int rc;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);

  /* call the internal command function: */
  rc = __tme_posix_tape_command(posix_tape,
				args, 
				_output);

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);

  return (rc);
}

/* this breaks a posix tape connection: */
static int
_tme_posix_tape_connection_break(struct tme_connection *conn,
				 unsigned int state)
{
  abort();
}

/* this makes a posix tape connection: */
static int
_tme_posix_tape_connection_make(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_posix_tape *posix_tape;
  struct tme_tape_connection *conn_tape;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) conn->tme_connection_element->tme_element_private;

  /* both sides must be tape connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_TAPE);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_TAPE);

  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock the mutex: */
    tme_mutex_lock(&posix_tape->tme_posix_tape_mutex);

    /* save this connection to our list of connections: */
    conn_tape = (struct tme_tape_connection *) conn->tme_connection_other;
    posix_tape->tme_posix_tape_connection = conn_tape;

    /* unlock the mutex: */
    tme_mutex_unlock(&posix_tape->tme_posix_tape_mutex);
  }

  return (TME_OK);
}

/* this makes a new connection side for a posix tape: */
static int
_tme_posix_tape_connections_new(struct tme_element *element,
				const char * const *args,
				struct tme_connection **_conns,
				char **_output)
{
  struct tme_posix_tape *posix_tape;
  struct tme_tape_connection *conn_tape;
  struct tme_connection *conn;

  /* recover our data structure: */
  posix_tape = (struct tme_posix_tape *) element->tme_element_private;

  /* if we already have a connection, there's nothing to do: */
  if (posix_tape->tme_posix_tape_connection != NULL) {
    return (TME_OK);
  }

  /* create our side of a tape connection: */
  conn_tape = tme_new0(struct tme_tape_connection, 1);
  conn = &conn_tape->tme_tape_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_TAPE;
  conn->tme_connection_score = tme_tape_connection_score;
  conn->tme_connection_make = _tme_posix_tape_connection_make;
  conn->tme_connection_break = _tme_posix_tape_connection_break;

  /* fill in the tape connection: */
  conn_tape->tme_tape_connection_read
    = _tme_posix_tape_read;
  conn_tape->tme_tape_connection_write
    = _tme_posix_tape_write;
  conn_tape->tme_tape_connection_release
    = _tme_posix_tape_release;
  conn_tape->tme_tape_connection_control
    = _tme_posix_tape_control;
  
  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new posix tape function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_posix,tape) {
  struct tme_posix_tape *posix_tape;
  int flags;
  int arg_i;
  int usage;

  /* check our arguments: */
  flags = 0;
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

  /* start the tape structure: */
  posix_tape = tme_new0(struct tme_posix_tape, 1);
  posix_tape->tme_posix_tape_element = element;
  tme_mutex_init(&posix_tape->tme_posix_tape_mutex);
  posix_tape->tme_posix_tape_flags = flags;
  posix_tape->tme_posix_tape_segments = NULL;
  posix_tape->tme_posix_tape_segment_current = NULL;

  /* XXX these are fake values for now: */
  posix_tape->tme_posix_tape_block_size_min = 512;
  posix_tape->tme_posix_tape_block_size_max = 32768;
  posix_tape->tme_posix_tape_block_size_fixed = 0;

  /* start the tape buffer.  the 16384 isn't magic, it's just a
     starting point: */
  posix_tape->tme_posix_tape_buffer_size = 16384;
  posix_tape->tme_posix_tape_buffer_data
    = tme_new(tme_uint8_t, 
	      posix_tape->tme_posix_tape_buffer_size);

  /* fill the element: */
  element->tme_element_private = posix_tape;
  element->tme_element_connections_new = _tme_posix_tape_connections_new;
  element->tme_element_command = _tme_posix_tape_command;

  return (TME_OK);
}
